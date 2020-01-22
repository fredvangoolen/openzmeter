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
#include "FFT.h"
#include <fftw3.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t AnalyzerSoft::FFT::pPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::FFT::FFT(const string &name) {
  pTerminate = false;
  if(pthread_create(&pThread, NULL, FFT_Thread, (void*)this) != 0) {
    pThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name.c_str());
}
AnalyzerSoft::FFT::~FFT() {
  pTerminate = true;
  pWorkerLocks.Abort(&pThread);
}
void AnalyzerSoft::FFT::Start(vector<float> &in) {
  pInput = &in;
  pWorkerLocks.WakeWorker();
}
float AnalyzerSoft::FFT::BinPower(const uint idx) {
  return sqrt(norm(Output[idx])) / M_SQRT2 * 2.0;
}
void *AnalyzerSoft::FFT::FFT_Thread(void *args) {
  FFT *This = (FFT*)args;
  while(This->pTerminate == false) {
    if(This->pWorkerLocks.WorkerWaitTask() == false) return NULL;
    This->Output.resize((This->pInput->size() / 2) + 1, complex<float>(0.0, 0.0));
    pthread_mutex_lock(&This->pPlannerMutex);
    fftwf_plan Plan = fftwf_plan_dft_r2c_1d(This->pInput->size(), &((*This->pInput)[0]), (fftwf_complex*)&This->Output[0], FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    pthread_mutex_unlock(&pPlannerMutex);
    fftwf_execute_dft_r2c(Plan, &((*This->pInput)[0]), (fftwf_complex*)&This->Output[0]);
    pthread_mutex_lock(&This->pPlannerMutex);
    fftwf_destroy_plan(Plan);
    pthread_mutex_unlock(&pPlannerMutex);
    for(uint i = 0; i < This->Output.size(); i++) This->Output[i] = This->Output[i] / (float)(This->pInput->size());
    This->pWorkerLocks.WorkerEndsTask();
  }
  return NULL;
}
void AnalyzerSoft::FFT::Wait() {
  pWorkerLocks.WaitWorker();
}
float AnalyzerSoft::FFT::CalcAngle(const complex<float> &a, const complex<float> &b) {
  float Tmp;
  if((a.real() == 0) && (a.imag() == 0))
    Tmp = 0;
  else if(a.real() < 0)
    Tmp = atan(a.imag() / a.real());
  else
    Tmp = atan(a.imag() / a.real()) + M_PI;
  if((b.real() == 0) && (b.imag() == 0))
    Tmp = Tmp;
  else if(b.real() < 0)
    Tmp = Tmp - atan(b.imag() / b.real());
  else
    Tmp = Tmp - atan(b.imag() / b.real()) + M_PI;
  Tmp = fmod(Tmp, 2.0 * M_PI);
  if(Tmp > M_PI) Tmp = Tmp -  2.0 * M_PI;
  return Tmp;
}