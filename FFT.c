/* ================================== zMeter ===========================================================================
 * File:     FFT.c
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <fftw3.h>
#include "Main.h"
/* FFT de muestras de entrada ==========================================================================================
 * Realiza FFT en paralelo usando FFTW o la funcion interna (actualmente FFTW, aunque la interna funciona) */

// Tipos privados ======================================================================================================
void *FFT_Thread(void *args) {
  FFT_t *FFT = (FFT_t *)args;
  fftwf_execute_dft_r2c(FFT->Plan, FFT->Input, (fftwf_complex*)FFT->Result);
  for(int i = 0; i < ceil(FFT->Len / 2 + 1); i++) {
    FFT->Result[i].Real = FFT->Result[i].Real / FFT->Len;
    FFT->Result[i].Imag = FFT->Result[i].Imag / FFT->Len;
  }
  FFT->Len = ceil(FFT->Len / 2 + 1);
  return NULL;  
}
FFT_t *FFT_Start(float *data, uint32_t len) {
  FFT_t *FFT = (FFT_t *)malloc(sizeof(FFT_t) + sizeof(Complex_t) * ceil(len / 2));
  FFT->Plan = fftwf_plan_dft_r2c_1d(len, data, (fftwf_complex*)FFT->Result, FFTW_ESTIMATE);
  FFT->Len = len;
  FFT->Input = data;
  if(pthread_create(&FFT->Thread, NULL, FFT_Thread, (void*)FFT) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(FFT->Plan);
    free(FFT);
    return NULL;
  }
  return FFT;
}
void FFT_Wait(FFT_t *fft) {
  if(pthread_join(fft->Thread, NULL) != 0) {  
    LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
    fftwf_destroy_plan(fft->Plan);
    return;
  }
  fftwf_destroy_plan(fft->Plan);
  return;
}
// =====================================================================================================================