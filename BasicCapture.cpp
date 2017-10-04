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
#include "Tools.h"
// Funciones ===========================================================================================================
void BasicCapture::ReadCallback(struct libusb_transfer *transfer) {
  int16_t *Buff16 = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  BasicCapture *This = (BasicCapture*)transfer->user_data;
  pthread_mutex_lock(&This->Mutex);
  while(This->Used > (This->Size - Len)) {
    if(This->Terminate) return;
    Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
    pthread_cond_wait(&This->WriteWait, &This->Mutex);
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
  pthread_mutex_unlock(&This->Mutex);
  if(This->Terminate == false) libusb_submit_transfer(transfer);
}
void *BasicCapture::CaptureThread(void *args) {
  struct sched_param Prio;
  Prio.sched_priority = sched_get_priority_max(SCHED_OTHER);
  if(pthread_setschedparam(0, SCHED_OTHER, &Prio) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  BasicCapture *This = (BasicCapture*)args;  
  pthread_setname_np(This->Thread, "Adquisition");
  uint8_t Buffer_A[8192], Buffer_B[8192];
  struct libusb_transfer *Transfer_A = libusb_alloc_transfer(0);
  struct libusb_transfer *Transfer_B = libusb_alloc_transfer(0);
  libusb_fill_bulk_transfer(Transfer_A, This->DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_A,  sizeof(Buffer_A), ReadCallback, This, 500);
  libusb_fill_bulk_transfer(Transfer_B, This->DeviceHandle, ADQUISITION_ENDPOINT_IN, Buffer_B,  sizeof(Buffer_B), ReadCallback, This, 500);
  libusb_submit_transfer(Transfer_A);
  libusb_submit_transfer(Transfer_B);
  while(!This->Terminate) libusb_handle_events_completed(NULL, NULL);
  return NULL;
}
void *BasicCapture::AnalizerThread(void* args) {
  uint64_t PrevTimestamp, Agreg3S_Start;
  pthread_setname_np(Thread, "Analysys");
  while(!Terminate) {
    Work = Adquisition_GetWork();
    if((Work == NULL) || (Work->SequenceID == 0)) {
      PrevTimestamp = Work->Timestamp;
      Agreg3S_Start = Work->Timestamp;
      continue;
    }
    Params = calloc(sizeof(Params_t), 1);
    FFT_t *FFT_Voltage = FFT_Start(Work->VoltageSamples, Work->Samples);         // STEP 1: Start calculating FFT (in parallel)
    FFT_t *FFT_Current = FFT_Start(Work->CurrentSamples, Work->Samples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    float V = 0.0;
    float I = 0.0;
    float P = 0.0;
    for(int i = 0; i < Work->Samples; i++) {
      V += pow(Work->VoltageSamples[i], 2.0);
      I += pow(Work->CurrentSamples[i], 2.0);
      P += Work->VoltageSamples[i] * Work->CurrentSamples[i];
    }
    Params->Active_Power = P / Work->Samples;
    Params->RMS_V = sqrt(V / Work->Samples);
    Params->RMS_I = sqrt(I / Work->Samples);
    Params->Apparent_Power = Params->RMS_V * Params->RMS_I;
    Params->Flag = Events_Detect(Work);
    Params->Frequency = Analyzer_Frequency(Work);
    FFT_Wait(FFT_Voltage);
//    printf("Offset %f\n", FFT_Voltage->Result[0].Real);
    for(int i = 0; i < 50; i++) {
      Params->VoltageHarmonics[i] = sqrt(pow(FFT_Voltage->Result[10 + i * 10].Real, 2.0) + pow(FFT_Voltage->Result[10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(i > 0) Params->THD_V += pow(Params->VoltageHarmonics[i], 2.0);
    }
    Params->THD_V = sqrt(Params->THD_V) / Params->VoltageHarmonics[0];
    FFT_Wait(FFT_Current);
    if(Params->RMS_I > PROCESSING_CURRENTOFFSET) {
//      float Active_Power = 0.0;
  //  float Reactive_Power = 0.0;
//      float Apparent_Power = 0.0;
      for(int i = 10; i <= 500 ; i = i + 10) {
        float CurrentHarmonic = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
        float VoltageHarmonic = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;  
        float PHI = CalcPhi(FFT_Voltage->Result[i], FFT_Current->Result[i]);
//        Active_Power   += VoltageHarmonic * CurrentHarmonic * cos(PHI);
//        Apparent_Power += VoltageHarmonic * CurrentHarmonic;
//        Reactive_Power += VoltageHarmonic * CurrentHarmonic * sin(PHI);        
//        if(((i % 10) != 0) || (i > 500) || (i < 10)) continue;
        int Harmonic = (i / 10) - 1;
        Params->CurrentHarmonics[Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
          Params->Fundamental_Active_Power   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          Params->Fundamental_Reactive_Power = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          Params->Fundamental_Apparent_Power = VoltageHarmonic * CurrentHarmonic;
          Params->Fundamental_Power_Factor   = Params->Fundamental_Active_Power / Params->Fundamental_Apparent_Power;
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          Params->Phi = PHI * 180.0 / M_PI;
//          printf("PHI: %f (%fº\n", PHI, PHI * 180.0 / M_PI);

        }
        if(Harmonic > 0) Params->THD_I += pow(CurrentHarmonic, 2.0);
      }
//      printf("FFT  -> Active: %f  Reactive: %f  Apparent: %f\n", Active_Power, Reactive_Power, Apparent_Power);
//      printf("Other-> Active: %f  Reactive: %f  Apparent: %f\n", Params->Active_Power, Params->Fundamental_Reactive_Power, Params->Apparent_Power);
      Params->THD_I = sqrt(Params->THD_I) / Params->CurrentHarmonics[0];
      Params->Active_Energy = (Params->Active_Power * Work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
      Params->Power_Factor = Params->Active_Power / Params->Apparent_Power;
      Params->Fundamental_Reactive_Energy = (Params->Fundamental_Reactive_Power * Work->Samples) / (ADQUISITION_SAMPLEFREQ * 3600.0);
    } else {
      Params->RMS_I = 0.0;
      memset(Work->CurrentSamples, 0, Work->Samples * sizeof(float));
      memset(FFT_Current->Result, 0, FFT_Current->Len * sizeof(Complex_t));
    }
    if(((Work->SequenceID) % 15 == 0) && (Work->Cycles_Len == 20)) {
      float FFT_V[600];
      float FFT_I[600];
      for(int i = 0; i < 600; i++) {
        FFT_V[i] = sqrt(pow(FFT_Voltage->Result[i].Real, 2.0) + pow(FFT_Voltage->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
        FFT_I[i] = sqrt(pow(FFT_Current->Result[i].Real, 2.0) + pow(FFT_Current->Result[i].Imag, 2.0)) / M_SQRT2 * 2.0;
      }
      RequestServer_SetValues(Work->Timestamp, Params, Work->VoltageSamples, Work->CurrentSamples, FFT_V, FFT_I, Work->Samples, 600);
    }
    free(FFT_Voltage);
    free(FFT_Current);
    DB_Save(PARAMS_10C, 0, Work->Timestamp, Params, true);
    if((Work->SequenceID % 15) == 0) DB_Save(PARAMS_3S, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 60000) * 60000) != ((PrevTimestamp / 60000) * 60000)) DB_Save(PARAMS_1M, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 600000) * 600000) != ((PrevTimestamp / 600000) * 600000)) DB_Save(PARAMS_10M, 0, PrevTimestamp, NULL, true);
    if(((Work->Timestamp / 3600000) * 3600000) != ((PrevTimestamp / 3600000) * 3600000)) DB_Save(PARAMS_1H, 0, PrevTimestamp, NULL, true);
    PrevTimestamp = Work->Timestamp;
    free(Work);
  }
}
BasicCapture::BasicCapture(char* device, uint32_t bufferLen, float baseFreq) {
  Tools::LogMsg("INFO", "Starting adquisition thread...\n");
  Buffer = NULL;
  ReadPos = 0;
  WritePos = 0;
  Size = bufferLen;
  Used = 0;
  SequenceID = 0;
  DeviceHandle = NULL;
  Terminate = false;
  Thread = 0;
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
  if(pthread_mutex_init(&Mutex, NULL) != 0) {                                  
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
  if(pthread_create(&Thread, NULL, CaptureThread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    free(Buffer);
    return;
  }
}
BasicCapture::~BasicCapture() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    Terminate = true;
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(Thread, NULL);
    free(Buffer);
  }
}
float BasicCapture::ApplyFilter(float in, AdquisitionFilter_t* filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
/* Devuelve una estructura de trabajo con 10 ciclos completos o si no se detectan ciclos con 236mS */
Work_t *BasicCapture::GetWork() {
  Work_t *Work = (Work_t*)calloc(sizeof(Work_t), 1);
  if(Work == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  Work->Cycles_Len = 0;
  Work->Cycles[Work->Cycles_Len].Start = 0;
  struct timeval Time;
  pthread_mutex_lock(&Mutex);
  
  while(Used < (MAX_FRAME + ADQUISITION_SAFESAMPLES)) {
    if(Terminate) return NULL;
    pthread_cond_wait(&ReadWait, &Mutex);
  }
  if(gettimeofday(&Time, NULL) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    free(Work);
    return NULL;
  }
  Work->Timestamp = ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000) - ((Used * 1000) / ADQUISITION_SAMPLEFREQ);
  float LastSample = +100; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
  for(Work->Samples = 0; Work->Samples < MAX_FRAME; Work->Samples++) {
    Work->VoltageSamples[Work->Samples] = Buffer[ReadPos].Voltage;
    Work->CurrentSamples[Work->Samples] = Buffer[ReadPos].Current;
    float Out = Work->VoltageSamples[Work->Samples];
    for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &Filters[i]);
    if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
      Work->Cycles[Work->Cycles_Len].End = Work->Samples;
      Work->Cycles_Len++;
      if(Work->Cycles_Len == 20) break;
      Work->Cycles[Work->Cycles_Len].Start = Work->Samples;
    }
    LastSample = Out;
    ReadPos = (ReadPos + 1) % Size;
  }
  Work->SequenceID = SequenceID++;
  Used -= Work->Samples;
  pthread_cond_signal(&WriteWait);
  pthread_mutex_unlock(&Mutex);
  if(Work->Cycles_Len == 0) {
    Work->Cycles[Work->Cycles_Len].End = Work->Samples;
    Work->Cycles_Len++;
  }
  return Work;
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