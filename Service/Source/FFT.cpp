/* ================================== zMeter ===========================================================================
 * File:     FFT.cpp
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <fftw3.h>
#include "FFT.h"
#include <errno.h>
#include <string.h>
#include <math.h>

#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t FFT::pPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
FFT::FFT(char *name) {
  pTerminate = false;
  pInput = NULL;
  pthread_cond_init(&pThreadCond, NULL);
  pthread_mutex_init(&pThreadMutex, NULL);
  pThread = 0;
  if(pthread_create(&pThread, NULL, FFT_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name);
}
FFT::~FFT() {
  if(pThread == 0) return;
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  if(pthread_join(pThread, NULL) != 0) LOGFile::Error("%s:%d -> (Unknown)\n", __func__, __LINE__);
}
void FFT::Start(vector<float> *in) {
  pthread_mutex_lock(&pThreadMutex);
  if(pInput != NULL) {
    pthread_mutex_unlock(&pThreadMutex);
    return;
  }
  pInput = in;
  pthread_mutex_unlock(&pThreadMutex);
  pthread_cond_signal(&pThreadCond);
}
void *FFT::FFT_Thread(void *args) {
  FFT *This = (FFT*)args;
  while(true) {
    pthread_mutex_lock(&This->pThreadMutex);
    while(This->pInput == NULL) {
      if(This->pTerminate == true) {
        pthread_mutex_unlock(&This->pThreadMutex);
        return NULL;
      }
      pthread_cond_wait(&This->pThreadCond, &This->pThreadMutex);
    }
    pthread_mutex_unlock(&This->pThreadMutex);
    This->Output.reserve((This->pInput->size() / 2) + 1);
    pthread_mutex_lock(&This->pPlannerMutex);
    fftwf_plan Plan = fftwf_plan_dft_r2c_1d(This->pInput->size(), &(*This->pInput)[0], (fftwf_complex*)&This->Output[0], FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
    pthread_mutex_unlock(&pPlannerMutex);
    fftwf_execute_dft_r2c(Plan, &(*This->pInput)[0], (fftwf_complex*)&This->Output[0]);
    This->Output.resize((This->pInput->size() / 2) + 1);
    for(uint i = 0; i < This->Output.size(); i++) {
      This->Output[i].Real = This->Output[i].Real / This->pInput->size();
      This->Output[i].Imag = This->Output[i].Imag / This->pInput->size();
    }
    pthread_mutex_lock(&This->pPlannerMutex);
    fftwf_destroy_plan(Plan);
    pthread_mutex_unlock(&This->pPlannerMutex);
    pthread_mutex_lock(&This->pThreadMutex);
    This->pInput = NULL;
    pthread_mutex_unlock(&This->pThreadMutex);
    pthread_cond_signal(&This->pThreadCond);
  }
  return NULL;
}
void FFT::Wait() {
  pthread_mutex_lock(&pThreadMutex);
  while(pInput != NULL) {
    if(pTerminate == true) return;
    pthread_cond_wait(&pThreadCond, &pThreadMutex);
  } 
  pthread_mutex_unlock(&pThreadMutex);
}
float FFT::CalcAngle(Complex_t a, Complex_t b) {
  float Tmp;
  if((a.Real == 0) && (a.Imag == 0)) 
    Tmp = 0;
  else if(a.Real < 0)
    Tmp = atan(a.Imag / a.Real);
  else
    Tmp = atan(a.Imag / a.Real) + M_PI;
  if((b.Real == 0) && (b.Imag == 0)) 
    return Tmp;
  else if(b.Real < 0)
    return Tmp - atan(b.Imag / b.Real);
  else
    return Tmp - atan(b.Imag / b.Real) + M_PI;
}
float FFT::BinPower(uint32_t bin) {
  return sqrt(pow(Output[bin].Real, 2.0) + pow(Output[bin].Imag, 2.0)) / M_SQRT2 * 2.0;
}