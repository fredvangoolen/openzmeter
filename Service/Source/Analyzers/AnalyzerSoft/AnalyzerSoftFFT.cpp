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
  pOutLen      = 0;
  pReservedLen = 0;
  pInLen       = 0;
  pOutPtr      = NULL;
  pInPtr       = NULL;
  pWorker      = new Worker(name, [&](void *a) { this->FFT_Func(); }, -15);
}
AnalyzerSoftFFT::~AnalyzerSoftFFT() {
  pWorker->Wait();
  pthread_mutex_lock(&pFFTMutex);
  if(pFFTRunning == 0) {
    LogFile::Debug("Clearing FFT cached plans");
    fftwf_cleanup();
  }
  if(pOutPtr != NULL) free(pOutPtr);
  pthread_mutex_unlock(&pFFTMutex);
}
void AnalyzerSoftFFT::Run(const float *src, const uint32_t samples) {
  pInLen   = samples;
  pInPtr   = (float*)src;
  pOutLen  = (samples / 2) + 1;
  if(pOutLen > pReservedLen) pOutPtr = (Complex_t*)realloc(pOutPtr, sizeof(Complex_t) * pOutLen);
  pWorker->Run(this);
}
void AnalyzerSoftFFT::Wait() {
  pWorker->Wait();
}
void AnalyzerSoftFFT::FFT_Func() {
  pthread_mutex_lock(&pFFTMutex);
  fftwf_plan Plan = fftwf_plan_dft_r2c_1d(pInLen, pInPtr, (fftwf_complex*)pOutPtr, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  pFFTRunning++;
  pthread_mutex_unlock(&pFFTMutex);
  fftwf_execute_dft_r2c(Plan, pInPtr, (fftwf_complex*)pOutPtr);
  pthread_mutex_lock(&pFFTMutex);
  fftwf_destroy_plan(Plan);
  pFFTRunning--;
  pthread_mutex_unlock(&pFFTMutex);
  for(uint16_t i = 0; i < pOutLen; i++) {
    pOutPtr[i].Re = pOutPtr[i].Re / pInLen;
    pOutPtr[i].Im = pOutPtr[i].Im / pInLen;
  }
}
uint32_t AnalyzerSoftFFT::OutLen() const {
  return pOutLen;
}
AnalyzerSoftFFT::Complex_t AnalyzerSoftFFT::Bin(const uint32_t index) const {
  Complex_t Complex = pOutPtr[index];
  Complex.Re *= M_SQRT2;
  Complex.Im *= M_SQRT2;
  return Complex;
}
float AnalyzerSoftFFT::BinPower(const uint32_t index) const {
  Complex_t Complex = pOutPtr[index];
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