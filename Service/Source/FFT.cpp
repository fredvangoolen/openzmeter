/* ================================== zMeter ===========================================================================
 * File:     FFT.cpp
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */
pthread_mutex_t FFT::pPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
FFT::FFT(uint32_t maxLen) {
  
}
FFT::~FFT() {}


void *FFT::FFT_Thread(void *args) {
  FFT_t *FFT = (FFT_t *)args;
  fftwf_execute_dft_r2c(FFT->Plan, FFT->Input, (fftwf_complex*)FFT->Output);
  for(int i = 0; i < ceil(FFT->Len / 2 + 1); i++) {
    FFT->Output[i].Real = FFT->Output[i].Real / FFT->Len;
    FFT->Output[i].Imag = FFT->Output[i].Imag / FFT->Len;
  }
  return NULL;  
}
void FFT::FFT_Wait() {
  if(pthread_join(fft->Thread, NULL) != 0) Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
  pthread_mutex_lock(&pFFTPlannerMutex);
  fftwf_destroy_plan(fft->Plan);
  pthread_mutex_unlock(&pFFTPlannerMutex);  
  return;
}
bool FFT::Start(float *in, uint32_t len) {
  pthread_mutex_lock(&pFFTPlannerMutex);
  fft->Plan = fftwf_plan_dft_r2c_1d(pWork.Samples, in, (fftwf_complex*)out, FFTW_ESTIMATE);
  pthread_mutex_unlock(&pFFTPlannerMutex);  
  fft->Len = pWork.Samples;
  fft->Input = in;
  fft->Output = out;
  if(pthread_create(&fft->Thread, NULL, FFT_Thread, (void*)fft) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(fft->Plan);
    return false;
  }
  return true;
}