// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include <fftw3.h>
#include <math.h>
#include "AnalyzerSoftFFTs.h"
#include "AnalyzerSoftBlock.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t     AnalyzerSoftFFTs::pFFTMutex   = PTHREAD_MUTEX_INITIALIZER;
uint32_t            AnalyzerSoftFFTs::pFFTRunning = 0;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftFFTs::AnalyzerSoftFFTs(const Analyzer::AnalyzerMode mode) : pVoltageChannels(Analyzer::VoltageChannels(mode)), pCurrentChannels(Analyzer::CurrentChannels(mode)) {
  for(uint8_t i = 0; i < pVoltageChannels; i++) {
    pFFTVoltageData[i].DataPtr     = NULL;
    pFFTVoltageData[i].ReservedLen = 0;
    pFFTVoltageData[i].Task        = NULL;
  }
  for(uint8_t i = 0; i < pCurrentChannels; i++) {
    pFFTCurrentData[i].DataPtr     = NULL;
    pFFTCurrentData[i].ReservedLen = 0;
    pFFTCurrentData[i].Task        = NULL;
  }
  pInLen  = 0;
  pOutLen = 0;
  pCycles = 0;
}
AnalyzerSoftFFTs::~AnalyzerSoftFFTs() {
  for(uint8_t i = 0; i < pVoltageChannels; i++) {
    if(pFFTVoltageData[i].Task != NULL) TaskPool::Wait(pFFTVoltageData[i].Task);
    if(pFFTVoltageData[i].DataPtr != NULL) free(pFFTVoltageData[i].DataPtr);
  }
  for(uint8_t i = 0; i < pCurrentChannels; i++) {
    if(pFFTCurrentData[i].Task != NULL) TaskPool::Wait(pFFTCurrentData[i].Task);
    if(pFFTCurrentData[i].DataPtr != NULL) free(pFFTCurrentData[i].DataPtr);
  }
  pthread_mutex_lock(&pFFTMutex);
  if(pFFTRunning == 0) {
    LogFile::Debug("Clearing FFT cached plans");
    fftwf_cleanup();
  }
  pthread_mutex_unlock(&pFFTMutex);
}
void AnalyzerSoftFFTs::Run(const AnalyzerSoftBlock *block, const uint8_t inCycles) {
  pCycles = inCycles;
  pInLen  = block->Samples();
  pOutLen = (pInLen / 2) + 1;
  for(uint8_t i = 0; i < pVoltageChannels; i++) {
    if(pInLen > pFFTVoltageData[i].ReservedLen) {
      pFFTVoltageData[i].ReservedLen = pInLen;
      pFFTVoltageData[i].DataPtr = (Complex_t*)realloc(pFFTVoltageData[i].DataPtr, sizeof(Complex_t) * (pFFTVoltageData[i].ReservedLen + 1));
    }
    for(uint16_t n = 0; n < pInLen; n++) {
      pFFTVoltageData[i].DataPtr[n].Im = 0.0;
      pFFTVoltageData[i].DataPtr[n].Re = block->VoltageSample(i, n);
    }
    pFFTVoltageData[i].Task = TaskPool::Run([&, i]() { this->FFT_Func(&pFFTVoltageData[i]); }, TaskPool::ABOUTNORMAL);
  }
  for(uint8_t i = 0; i < pCurrentChannels; i++) {
    if(pInLen > pFFTCurrentData[i].ReservedLen) {
      pFFTCurrentData[i].ReservedLen = pInLen;
      pFFTCurrentData[i].DataPtr = (Complex_t*)realloc(pFFTCurrentData[i].DataPtr, sizeof(Complex_t) * (pFFTCurrentData[i].ReservedLen + 1));
    }
    for(uint16_t n = 0; n < pInLen; n++) {
      pFFTCurrentData[i].DataPtr[n].Im = 0.0;
      pFFTCurrentData[i].DataPtr[n].Re = block->CurrentSample(i, n);
    }
    pFFTCurrentData[i].Task = TaskPool::Run([&, i]() { this->FFT_Func(&pFFTCurrentData[i]); }, TaskPool::ABOUTNORMAL);
  }
}
void AnalyzerSoftFFTs::Wait() {
  for(uint8_t i = 0; i < pVoltageChannels; i++) {
    if(pFFTVoltageData[i].Task != NULL) TaskPool::Wait(pFFTVoltageData[i].Task);
  }
  for(uint8_t i = 0; i < pCurrentChannels; i++) {
    if(pFFTCurrentData[i].Task != NULL) TaskPool::Wait(pFFTCurrentData[i].Task);
  }
}
void AnalyzerSoftFFTs::FFT_Func(const FFT_t *data) {
  pthread_mutex_lock(&pFFTMutex);
  fftwf_plan Plan = fftwf_plan_dft_1d(pInLen, (fftwf_complex*)data->DataPtr, (fftwf_complex*)data->DataPtr, FFTW_FORWARD, FFTW_ESTIMATE);
  pFFTRunning++;
  pthread_mutex_unlock(&pFFTMutex);
  fftwf_execute(Plan);
  pthread_mutex_lock(&pFFTMutex);
  fftwf_destroy_plan(Plan);
  pFFTRunning--;
  pthread_mutex_unlock(&pFFTMutex);
  for(uint16_t i = 0; i < pOutLen; i++) {
    data->DataPtr[i].Re = data->DataPtr[i].Re / pInLen;
    data->DataPtr[i].Im = data->DataPtr[i].Im / pInLen;
  }
}
uint16_t AnalyzerSoftFFTs::OutLen() const {
  return pOutLen;
}
AnalyzerSoftFFTs::Complex_t AnalyzerSoftFFTs::VoltageHarmonic(const uint8_t channel, const uint8_t index) const {
  Complex_t Complex = pFFTVoltageData[channel].DataPtr[(index + 1) * pCycles];
  Complex.Re *= M_SQRT2;
  Complex.Im *= M_SQRT2;
  return Complex;
}
AnalyzerSoftFFTs::Complex_t AnalyzerSoftFFTs::CurrentHarmonic(const uint8_t channel, const uint8_t index) const {
  Complex_t Complex = pFFTCurrentData[channel].DataPtr[(index + 1) * pCycles];
  Complex.Re *= M_SQRT2;
  Complex.Im *= M_SQRT2;
  return Complex;
}
float AnalyzerSoftFFTs::VoltageHarmonicPower(const uint8_t channel, const uint8_t index) const {
  return VoltageBinPower(channel, (index + 1) * pCycles);
}
float AnalyzerSoftFFTs::CurrentHarmonicPower(const uint8_t channel, const uint8_t index) const {
  return CurrentBinPower(channel, (index + 1) * pCycles);
}
float AnalyzerSoftFFTs::CalcAngle(const Complex_t &a, const Complex_t &b) {
  float Tmp;
  if((a.Re == 0.0) && (a.Im == 0.0))
    Tmp = 0.0;
  else if(a.Re < 0.0)
    Tmp = atan(a.Im / a.Re);
  else
    Tmp = atan(a.Im / a.Re) + M_PI;
  if((b.Re == 0.0) && (b.Im == 0.0))
    Tmp = Tmp;
  else if(b.Re < 0.0)
    Tmp = Tmp - atan(b.Im / b.Re);
  else
    Tmp = Tmp - atan(b.Im / b.Re) + M_PI;
  Tmp = fmod(Tmp, 2.0 * M_PI);
  if(Tmp > M_PI) Tmp = Tmp -  2.0 * M_PI;
  return Tmp;
}
float AnalyzerSoftFFTs::VoltageBinPower(const uint8_t channel, const uint16_t index) const {
  Complex_t Complex = pFFTVoltageData[channel].DataPtr[index];
  return sqrt(Complex.Re * Complex.Re + Complex.Im * Complex.Im) / M_SQRT2 * 2.0;
}
float AnalyzerSoftFFTs::CurrentBinPower(const uint8_t channel, const uint16_t index) const {
  Complex_t Complex = pFFTCurrentData[channel].DataPtr[index];
  return sqrt(Complex.Re * Complex.Re + Complex.Im * Complex.Im) / M_SQRT2 * 2.0;
}
bool AnalyzerSoftFFTs::GetSeriesJSON(const string &serie, json &result) {
  if(serie == "Voltage_FFT") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      json Tmp2 = json::array();
      for(uint16_t i = 0; i < pOutLen; i++) Tmp2.push_back(VoltageBinPower(n, i));
      Tmp1.push_back(Tmp2);
    }
    result["Voltage_FFT"] = Tmp1;
  } else if(serie == "Current_FFT") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) {
      json Tmp2 = json::array();
      for(uint16_t i = 0; i < pOutLen; i++) Tmp2.push_back(CurrentBinPower(n, i));
      Tmp1.push_back(Tmp2);
    }
    result["Current_FFT"] = Tmp1;
  } else {
    return false;
  }
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------