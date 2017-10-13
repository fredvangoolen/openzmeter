/* ================================== zMeter ===========================================================================
 * File:     BasicCapture.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <termios.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <poll.h>
#include <fcntl.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <libusb-1.0/libusb.h>
#include "BasicCapture.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
void   BasicCapture::ReadCallback(struct libusb_transfer *transfer) {
  int16_t *Buff16 = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  BasicCapture *This = (BasicCapture*)transfer->user_data;
  pthread_mutex_lock(&This->CaptureMutex);
  while(This->Used > (This->Size - Len)) {
    if(This->Terminate) return;
    Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
    pthread_cond_wait(&This->WriteWait, &This->CaptureMutex);
  }
  for(int i = 0; i < Len; i++) {
    uint16_t Voltage = Buff16[i * 2 + 0];
    uint16_t Current = Buff16[i * 2 + 1];
    This->Buffer[This->WritePos].Voltage = (Voltage - This->VoltageOffset) * This->VoltageGain;
    This->Buffer[This->WritePos].Current = (Current - This->CurrentOffset) * This->CurrentGain;
    This->WritePos = (This->WritePos + 1) % This->Size;
  }
  This->Used += Len;
  pthread_cond_signal(&This->ReadWait);
  pthread_mutex_unlock(&This->CaptureMutex);
  if(This->Terminate == false) libusb_submit_transfer(transfer);
}
void  *BasicCapture::Capture_Thread(void *args) {
  struct sched_param Prio;
  Prio.sched_priority = sched_get_priority_max(SCHED_OTHER);
  if(pthread_setschedparam(0, SCHED_OTHER, &Prio) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  BasicCapture *This = (BasicCapture*)args;  
  pthread_setname_np(This->CaptureThread, "BasicCap_Init");
  uint8_t Buffer_A[8192], Buffer_B[8192];
  struct libusb_transfer *Transfer_A = libusb_alloc_transfer(0);
  struct libusb_transfer *Transfer_B = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(Transfer_A, This->DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_A,  sizeof(Buffer_A), ReadCallback, This, 500);
  libusb_fill_bulk_transfer(Transfer_B, This->DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_B,  sizeof(Buffer_B), ReadCallback, This, 500);
  libusb_submit_transfer(Transfer_A);
  libusb_submit_transfer(Transfer_B);
  uint32_t Update = 0;
  while(!This->Terminate) {
    if((Update % 10) == 0) {
      char ThreadName[16];
      sprintf(ThreadName, "BasicCap_%03d", This->Used * 100 / This->Size);
      pthread_setname_np(This->CaptureThread, ThreadName);
    }
    Update++;
    libusb_handle_events_completed(NULL, NULL);
  }
  return NULL;
}
void  *BasicCapture::Analize_Thread(void* args) {
  BasicCapture *This = (BasicCapture*)args;
  pthread_setname_np(This->AnalyzeThread, "BasicAnalysys");
  while(!This->Terminate) {
    This->ResetWork();
    struct timeval Time;
    pthread_mutex_lock(&This->AnalyzeMutex);
    while(This->Used < (This->GetMaxSamples() * 2)) {
      if(This->Terminate) return NULL;
      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
    }
    if(gettimeofday(&Time, NULL) != 0) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      continue;
    }
    This->pWork.Timestamp = ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000) - ((This->Used * 1000) / ADQUISITION_SAMPLEFREQ);
    float LastSample = +100; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
    for(This->pWork.Samples = 0; This->pWork.Samples < This->GetMaxSamples(); This->pWork.Samples++) {
      This->pWork.VoltageSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Voltage;
      This->pWork.CurrentSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Current;
      float Out = This->pWork.VoltageSamples[0][This->pWork.Samples];
      for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &This->Filters[i]);
      if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
        This->pWork.Cycles_Len++;
        if(This->pWork.Cycles_Len == 20) break;
        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
      }
      LastSample = Out;
      This->ReadPos = (This->ReadPos + 1) % This->Size;
    }
    This->pWork.SequenceID = This->SequenceID++;
    This->Used -= This->pWork.Samples;
    pthread_cond_signal(&This->WriteWait);
    pthread_mutex_unlock(&This->AnalyzeMutex);
    if(This->pWork.Cycles_Len != 20) {
      memset(This->pWork.Cycles, 0, sizeof(Cycle_t) * 20);
      This->pWork.Cycles_Len = 0;
    }
    
    //  if((work == NULL) || (work->SequenceID == 0)) {
//    PrevTimestamp = work->Timestamp;
//    Agreg3S_Start = work->Timestamp;
//    return NULL;
//  }
//  Params_t *Params = (Params_t*)calloc(sizeof(Params_t), 1);
    FFT_t *FFT_Voltage = FFT_Start(This->pWork.VoltageSamples[0], This->pWork.Samples);         // STEP 1: Start calculating FFT (in parallel)
    FFT_t *FFT_Current = FFT_Start(This->pWork.CurrentSamples[0], This->pWork.Samples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    float V = 0.0;
    float I = 0.0;
    float P = 0.0;
    for(int i = 0; i < This->pWork.Samples; i++) {
      V += pow(This->pWork.VoltageSamples[0][i], 2.0);
      I += pow(This->pWork.CurrentSamples[0][i], 2.0);
      P += This->pWork.VoltageSamples[0][i] * This->pWork.CurrentSamples[0][i];
    }
    This->pWork.Active_Power[0] = P / This->pWork.Samples;
    This->pWork.RMS_V[0] = sqrt(V / This->pWork.Samples);
    This->pWork.RMS_I[0] = sqrt(I / This->pWork.Samples);
    This->pWork.Apparent_Power[0] = This->pWork.RMS_V[0] * This->pWork.RMS_I[0];
//  Params->Flag = false; //Events_Detect(This->Work);
////  This->Params->Frequency = Analyzer_Frequency(Work);
    FFT_Wait(FFT_Voltage);
    for(int i = 0; i < 50; i++) {
//    Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
//    if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
    }
//  Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
//  FFT_Wait(FFT_Current);
//  if(Params->RMS_I > PROCESSING_CURRENTOFFSET) {
//    for(int i = 10; i <= 500 ; i = i + 10) {
//      float CurrentHarmonic = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      float VoltageHarmonic = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;  
//      float PHI = CalcPhi(FFT_Voltage->Result[i], FFT_Current->Result[i]);
//      int Harmonic = (i / 10) - 1;
//      Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
//      if(Harmonic == 0) {
//        Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
//        Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
//        Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
//        Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
//        PHI = fmod(PHI, 2.0 * M_PI);
//        if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
//        Params->Phi = PHI * 180.0 / M_PI;
//      }
//      if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
//    }
//    Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];
//    Params->Active_Energy = (Params->Active_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//    Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
//    Params->Fundamental_Reactive_Energy = (Params->Fundamental_Reactive_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//  } else {
//    Params->RMS_I = 0.0;
//    memset(work->CurrentSamples, 0, work->Samples * sizeof(float));
//    memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
//  }
//  if(((work->SequenceID) % 15 == 0) && (work->Cycles_Len == 20)) {
//    float FFT_V[600];
//    float FFT_I[600];
//    for(int i = 0; i < 600; i++) {
//      FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//    }
////    RequestServer_SetValues(work->Timestamp, Params, work->VoltageSamples, work->CurrentSamples, FFT_V, FFT_I, work->Samples, 600);
//  }
  free(FFT_Voltage);
  free(FFT_Current);
////  DB_Save(PARAMS_10C, 0, work->Timestamp, Params, true);
////  if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
//  PrevTimestamp = work->Timestamp;
//  free(work);
//  return Params;

    
    
    
    
    This->SaveWorkParams();
    struct timeval Time2;
    if(gettimeofday(&Time2, NULL) != 0) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      continue;
    }
    uint64_t Begin = Time.tv_sec * 1000000  + Time.tv_usec;
    uint64_t End   = Time2.tv_sec * 1000000 + Time2.tv_usec;
    Tools::LogMsg("DEBUG", "Process of block %llu takes %.1fmS ( mS with store)\n", This->pWork.SequenceID, (End - Begin) / 1000.0);
  }
}
void  *BasicCapture::FFT_Thread(void *args) {
  FFT_t *FFT = (FFT_t *)args;
  fftwf_execute_dft_r2c(FFT->Plan, FFT->Input, (fftwf_complex*)FFT->Result);
  for(int i = 0; i < ceil(FFT->Len / 2 + 1); i++) {
    FFT->Result[i].Real = FFT->Result[i].Real / FFT->Len;
    FFT->Result[i].Imag = FFT->Result[i].Imag / FFT->Len;
  }
  FFT->Len = ceil(FFT->Len / 2 + 1);
  return NULL;  
}
float  BasicCapture::CalcPhi(Complex_t voltage, Complex_t current) {
  float Tmp;
  if((voltage.Real == 0) && (voltage.Imag == 0)) 
    Tmp = 0;
  else if(voltage.Real < 0)
    Tmp = atan(voltage.Imag / voltage.Real);
  else
    Tmp = atan(voltage.Imag / voltage.Real) + M_PI;
  
  if((current.Real == 0) && (current.Imag == 0)) 
    return Tmp;
  else if(current.Real < 0)
    return Tmp - atan(current.Imag / current.Real);
  else
    return Tmp - atan(current.Imag / current.Real) + M_PI;
}
void   BasicCapture::FFT_Wait(FFT_t *fft) {
  if(pthread_join(fft->Thread, NULL) != 0) {  
    Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
    fftwf_destroy_plan(fft->Plan);
    return;
  }
  fftwf_destroy_plan(fft->Plan);
  return;
}
float BasicCapture::CalculateFrequency(Work_t* work) {
  static uint64_t NumSamples = 0;    //Numero de muestras leidas desde el ultimo calculo
  static uint32_t NumCycles  = 0;    //Numero de ciclos desde ultimo calculo
  float  Frequency = NAN;
  NumSamples += work->Samples;
  NumCycles += work->Cycles_Len;
  if((((work->Timestamp / 1000000) % 10) == 0) && (NumSamples > (ADQUISITION_SAMPLEFREQ * 9))) {
    Frequency = NumCycles / (NumSamples / ADQUISITION_SAMPLEFREQ) / 2.0;
    NumSamples = 0;
    NumCycles = 0;  
  }
  return Frequency;
}
BasicCapture::FFT_t *BasicCapture::FFT_Start(float *data, uint32_t len) {
  FFT_t *FFT = (FFT_t *)malloc(sizeof(FFT_t) + sizeof(Complex_t) * ceil(len / 2));
  FFT->Plan = fftwf_plan_dft_r2c_1d(len, data, (fftwf_complex*)FFT->Result, FFTW_ESTIMATE);
  FFT->Len = len;
  FFT->Input = data;
  if(pthread_create(&FFT->Thread, NULL, FFT_Thread, (void*)FFT) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(FFT->Plan);
    free(FFT);
    return NULL;
  }
  return FFT;
}


//CaptureInterface::Params_t *BasicCapture::Analyze(Work_t *work) {
//  if((work == NULL) || (work->SequenceID == 0)) {
//    PrevTimestamp = work->Timestamp;
//    Agreg3S_Start = work->Timestamp;
//    return NULL;
//  }
//  Params_t *Params = (Params_t*)calloc(sizeof(Params_t), 1);
//  FFT_t *FFT_Voltage = FFT_Start(work->VoltageSamples, work->Samples);         // STEP 1: Start calculating FFT (in parallel)
//  FFT_t *FFT_Current = FFT_Start(work->CurrentSamples, work->Samples);
//  //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
//  float V = 0.0;
//  float I = 0.0;
//  float P = 0.0;
//  for(int i = 0; i < work->Samples; i++) {
//    V += pow(work->VoltageSamples[i], 2.0);
//    I += pow(work->CurrentSamples[i], 2.0);
//    P += work->VoltageSamples[i] * work->CurrentSamples[i];
//  }
//  Params->Active_Power = P / work->Samples;
//  Params->RMS_V = sqrt(V / work->Samples);
//  Params->RMS_I = sqrt(I / work->Samples);
//  Params->Apparent_Power = Params->RMS_V * Params->RMS_I;
//  Params->Flag = false; //Events_Detect(This->Work);
////  This->Params->Frequency = Analyzer_Frequency(Work);
//  FFT_Wait(FFT_Voltage);
//  for(int i = 0; i < 50; i++) {
//    Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
//    if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
//  }
//  Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
//  FFT_Wait(FFT_Current);
//  if(Params->RMS_I > PROCESSING_CURRENTOFFSET) {
//    for(int i = 10; i <= 500 ; i = i + 10) {
//      float CurrentHarmonic = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      float VoltageHarmonic = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;  
//      float PHI = CalcPhi(FFT_Voltage->Result[i], FFT_Current->Result[i]);
//      int Harmonic = (i / 10) - 1;
//      Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
//      if(Harmonic == 0) {
//        Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
//        Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
//        Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
//        Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
//        PHI = fmod(PHI, 2.0 * M_PI);
//        if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
//        Params->Phi = PHI * 180.0 / M_PI;
//      }
//      if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
//    }
//    Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];
//    Params->Active_Energy = (Params->Active_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//    Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
//    Params->Fundamental_Reactive_Energy = (Params->Fundamental_Reactive_Power * work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
//  } else {
//    Params->RMS_I = 0.0;
//    memset(work->CurrentSamples, 0, work->Samples * sizeof(float));
//    memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
//  }
//  if(((work->SequenceID) % 15 == 0) && (work->Cycles_Len == 20)) {
//    float FFT_V[600];
//    float FFT_I[600];
//    for(int i = 0; i < 600; i++) {
//      FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//      FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
//    }
////    RequestServer_SetValues(work->Timestamp, Params, work->VoltageSamples, work->CurrentSamples, FFT_V, FFT_I, work->Samples, 600);
//  }
//  free(FFT_Voltage);
//  free(FFT_Current);
////  DB_Save(PARAMS_10C, 0, work->Timestamp, Params, true);
////  if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
////  if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
//  PrevTimestamp = work->Timestamp;
//  free(work);
//  return Params;
//}
BasicCapture::BasicCapture(char* device, uint32_t bufferLen, float baseFreq) : CaptureInterface(15625, false, CaptureInterface::FREQ_50) {
  Tools::LogMsg("INFO", "Starting BasicCapture adquisition thread...\n");
  Buffer           = NULL;
  ReadPos          = 0;
  WritePos         = 0;
  Size             = bufferLen;
  Used             = 0;
  SequenceID       = 0;
  DeviceHandle     = NULL;
  Terminate        = false;
  LastCyclesCount  = 0;
  LastCyclesPos    = 0;
  LastCyclesLenght = 0;
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * (baseFreq / ADQUISITION_SAMPLEFREQ));
    float Norm = 1 / (1 + K / Q + K * K);
    Filters[i].A0 = K / Q * Norm;
    Filters[i].A1 = 0;
    Filters[i].A2 = -Filters[i].A0;
    Filters[i].B1 = 2 * (K * K - 1) * Norm;
    Filters[i].B2 = (1 - K / Q + K * K) * Norm;
    Filters[i].Z1 = 0.0;
    Filters[i].Z2 = 0.0;            
  }
  if(pthread_cond_init(&WriteWait, NULL) != 0) {                                  
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  } 
  if(pthread_cond_init(&ReadWait, NULL) != 0) {                                  
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;                                                                  
  } 
  if(pthread_mutex_init(&CaptureMutex, NULL) != 0) {                                  
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;                                                                  
  } 
  if(pthread_mutex_init(&AnalyzeMutex, NULL) != 0) {                                  
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;                                                                  
  } 
  int Error = libusb_init(NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo la apertura de libUSB...\n");
    return;
  }
  DeviceHandle = libusb_open_device_with_vid_pid(NULL, ADQUISITION_USB_VID, ADQUISITION_PID);
  if(!DeviceHandle) {
    Tools::LogMsg("ERROR", "Dispositivo de captura no conectado...\n");
    libusb_exit(NULL);
    return;
  }
  libusb_detach_kernel_driver(DeviceHandle, 0);
  Error = libusb_claim_interface(DeviceHandle, 0);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Dispositivo de captura ocupado...\n");
    libusb_close(DeviceHandle);
    libusb_exit(NULL);
    return;
  }
  Buffer = (Sample_t*)malloc(bufferLen * sizeof(Sample_t));
  if(Buffer == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return; 
  }
  if(pthread_create(&CaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    free(Buffer);
    return;
  }
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    Adquisition_Stop();
//    return false;
  }

}
BasicCapture::~BasicCapture() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    Terminate = true;
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(CaptureThread, NULL);
    pthread_join(AnalyzeThread, NULL);
    free(Buffer);
  }
}
float BasicCapture::ApplyFilter(float in, AdquisitionFilter_t* filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
void BasicCapture::SamplesCopy(float *dest, int32_t offset, uint32_t len) {
  for(int i = 0; i < len; i++) 
    dest[i] = Buffer[(ReadPos - offset + i) % Size].Voltage;
}
//void Adquisition_SetLEDs(float voltage, float current) {
//  uint8_t Val = 0;
//  int Voltage = round(((voltage / EVENT_VOLTAGE) - 1.0) * 50.0 + 4.0);
//  if(Voltage < 0) Voltage = 0;
//  if(Voltage > 8) Voltage = 8;
//  int Current = round((current / MAX_CURRENT) * 100.0 / 9.0);
//  if(Current < 0) Current = 0;
//  if(Current > 8) Current = 8;
//  Val = (Current << 4) + Voltage;
//}
//======================================================================================================================