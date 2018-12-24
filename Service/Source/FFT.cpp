/* ================================== zMeter ===========================================================================
 * File:     FFT.cpp
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <errno.h>
#include <string.h>
#include <math.h>
#include <fftw3.h>
#include "FFT.h"
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
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name);
}
FFT::~FFT() {
  if(pThread == 0) return;
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  if(pthread_join(pThread, NULL) != 0) Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
}
bool FFT::Start(vector<float> *in) {
  pthread_mutex_lock(&pThreadMutex);
  if(pInput != NULL) {
    pthread_mutex_unlock(&pThreadMutex);
    return false;
  }
//  pLen = len;
  pInput = in;
  pthread_mutex_unlock(&pThreadMutex);
  pthread_cond_signal(&pThreadCond);
  return true;
}
void *FFT::FFT_Thread(void *args) {
  ((FFT*)args)->Process();
  return NULL;
}
void FFT::Process() {
  while(true) {
    pthread_mutex_lock(&pThreadMutex);
    while(pInput == NULL) {
      if(pTerminate == true) return;
      pthread_cond_wait(&pThreadCond, &pThreadMutex);
    }
    pthread_mutex_unlock(&pThreadMutex);
    pthread_mutex_lock(&pPlannerMutex);
    Output.reserve((pInput->size() / 2) + 1);
    fftwf_plan Plan = fftwf_plan_dft_r2c_1d(pInput->size(), &(*pInput)[0], (fftwf_complex*)&Output[0], FFTW_ESTIMATE);
    pthread_mutex_unlock(&pPlannerMutex);
    fftwf_execute_dft_r2c(Plan, &(*pInput)[0], (fftwf_complex*)&Output[0]);
    Output.resize((pInput->size() / 2) + 1);
    for(uint i = 0; i < Output.size(); i++) {
      Output[i].Real = Output[i].Real / pInput->size();
      Output[i].Imag = Output[i].Imag / pInput->size();
    }
    pthread_mutex_lock(&pPlannerMutex);
    fftwf_destroy_plan(Plan);
    pthread_mutex_unlock(&pPlannerMutex);
    pthread_mutex_lock(&pThreadMutex);
    pInput = NULL;
    pthread_mutex_unlock(&pThreadMutex);
    pthread_cond_signal(&pThreadCond);
  }
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