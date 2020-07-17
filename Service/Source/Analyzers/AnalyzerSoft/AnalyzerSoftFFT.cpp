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
#include "AnalyzerSoftFFT.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t     AnalyzerSoftFFT::pFFTMutex   = PTHREAD_MUTEX_INITIALIZER;
uint32_t            AnalyzerSoftFFT::pFFTRunning = 0;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftFFT::AnalyzerSoftFFT(const string &name) {
  pWorker      = new Worker(name, [&](void *a) { this->FFT_Func(); }, -15);
  pDataPtr     = NULL;
  pInLen       = 0;
  pOutLen      = 0;
  pCycles      = 10;
  pReservedLen = 0;
}
AnalyzerSoftFFT::~AnalyzerSoftFFT() {
  pWorker->Wait();
  pthread_mutex_lock(&pFFTMutex);
  if(pFFTRunning == 0) {
    LogFile::Debug("Clearing FFT cached plans");
    fftwf_cleanup();
  }
  if(pDataPtr != NULL) free(pDataPtr);
  pthread_mutex_unlock(&pFFTMutex);
}
void AnalyzerSoftFFT::Config(const uint8_t inCycles) {
  pCycles = inCycles;
}
void AnalyzerSoftFFT::Run() {
  pWorker->Run(this);
}
void AnalyzerSoftFFT::Wait() {
  pWorker->Wait();
}
void AnalyzerSoftFFT::PushSample(const float val) {
  if(pInLen == pReservedLen) {
    pReservedLen = pInLen + 1;
    pDataPtr = (Complex_t*)realloc(pDataPtr, sizeof(Complex_t) * pReservedLen);
  }
  pDataPtr[pInLen].Im = 0.0;
  pDataPtr[pInLen].Re = val;
  pInLen++;
}
void AnalyzerSoftFFT::FFT_Func() {
  pOutLen = (pInLen / 2) + 1;
  pthread_mutex_lock(&pFFTMutex);
  fftwf_plan Plan = fftwf_plan_dft_1d(pInLen, (fftwf_complex*)pDataPtr, (fftwf_complex*)pDataPtr, FFTW_FORWARD, FFTW_ESTIMATE);
  pFFTRunning++;
  pthread_mutex_unlock(&pFFTMutex);
  fftwf_execute(Plan);
  pthread_mutex_lock(&pFFTMutex);
  fftwf_destroy_plan(Plan);
  pFFTRunning--;
  pthread_mutex_unlock(&pFFTMutex);
  for(uint16_t i = 0; i < pOutLen; i++) {
    pDataPtr[i].Re = pDataPtr[i].Re / pInLen;
    pDataPtr[i].Im = pDataPtr[i].Im / pInLen;
  }
  pInLen = 0;
}
uint16_t AnalyzerSoftFFT::OutLen() const {
  return pOutLen;
}
AnalyzerSoftFFT::Complex_t AnalyzerSoftFFT::Harmonic(const uint8_t index) const {
  return Bin((index + 1) * pCycles);
}
AnalyzerSoftFFT::Complex_t AnalyzerSoftFFT::Bin(const uint32_t index) const {
  Complex_t Complex = pDataPtr[index];
  Complex.Re *= M_SQRT2;
  Complex.Im *= M_SQRT2;
  return Complex;
}
float AnalyzerSoftFFT::HarmonicPower(const uint8_t index) const {
  return BinPower((index + 1) * pCycles);
}
float AnalyzerSoftFFT::BinPower(const uint32_t index) const {
  Complex_t Complex = pDataPtr[index];
  return sqrt(Complex.Re * Complex.Re + Complex.Im * Complex.Im) / M_SQRT2 * 2.0;
}
float AnalyzerSoftFFT::CalcAngle(const Complex_t &a, const Complex_t &b) {
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
// ---------------------------------------------------------------------------------------------------------------------