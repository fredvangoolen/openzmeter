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
FFT::FFT(uint32_t maxLen) {
  Output = (Complex_t*)malloc(sizeof(Complex_t) * ((maxLen / 2) + 1));
}
FFT::~FFT() {
  Wait();
  free(Output);
}
bool FFT::Start(float *in, uint32_t len) {
  pthread_mutex_lock(&pPlannerMutex);
  pPlan = fftwf_plan_dft_r2c_1d(len, in, (fftwf_complex*)Output, FFTW_ESTIMATE);
  pthread_mutex_unlock(&pPlannerMutex);
  pLen = len;
  pInput = in;
  if(pthread_create(&pThread, NULL, FFT_Thread, (void*)this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(pPlan);
    return false;
  }
  return true;
}
void *FFT::FFT_Thread(void *args) {
  FFT *This = (FFT*)args;
  fftwf_execute_dft_r2c(This->pPlan, This->pInput, (fftwf_complex*)This->Output);
  This->OutputLen = (This->pLen / 2) + 1;
  for(int i = 0; i < This->OutputLen; i++) {
    This->Output[i].Real = This->Output[i].Real / This->pLen;
    This->Output[i].Imag = This->Output[i].Imag / This->pLen;
  }
  return NULL;  
}
void FFT::Wait() {
  if(pthread_join(pThread, NULL) != 0) Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
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