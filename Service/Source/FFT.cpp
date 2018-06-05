/* ================================== zMeter ===========================================================================
 * File:     FFT.cpp
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <errno.h>
#include <string.h>
#include <math.h>
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t FFT::pPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
FFT::FFT(uint32_t maxLen, char *name) {
  pTerminate = false;
  Output = (Complex_t*)malloc(sizeof(Complex_t) * ((maxLen / 2) + 1));
  pInput = NULL;
  pthread_cond_init(&pThreadCond, NULL);
  pthread_mutex_init(&pThreadMutex, NULL);
  if(pthread_create(&pThread, NULL, FFT_Thread, (void*)this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name);
}
//bool FFT::IsFinished() {
//  bool Result;
//  pthread_mutex_lock(&pThreadMutex);
//  Result = (pInput == NULL);
//  pthread_mutex_unlock(&pThreadMutex);
//  return Result;
//}
FFT::~FFT() {
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  if(pthread_join(pThread, NULL) != 0) Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
  free(Output);
}
bool FFT::Start(float *in, uint32_t len) {
  pthread_mutex_lock(&pThreadMutex);
  if(pInput != NULL) {
    pthread_mutex_unlock(&pThreadMutex);
    return false;
  }
  pLen = len;
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
    fftwf_plan Plan = fftwf_plan_dft_r2c_1d(pLen, pInput, (fftwf_complex*)Output, FFTW_ESTIMATE);
    pthread_mutex_unlock(&pPlannerMutex);
    fftwf_execute_dft_r2c(Plan, pInput, (fftwf_complex*)Output);
    OutputLen = (pLen / 2) + 1;
    for(int i = 0; i < OutputLen; i++) {
      Output[i].Real = Output[i].Real / pLen;
      Output[i].Imag = Output[i].Imag / pLen;
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