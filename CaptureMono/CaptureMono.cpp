/* ================================== zMeter ===========================================================================
 * File:     CaptureMono.cpp
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
#include "CaptureMono.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
void   CaptureMono::ReadCallback(struct libusb_transfer *transfer) {
  int16_t *Buff16 = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  CaptureMono *This = (CaptureMono*)transfer->user_data;
  pthread_mutex_lock(&This->CaptureMutex);
  while(This->Used > (This->Size - Len)) {
    if(This->Terminate) return;
    Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
    pthread_cond_wait(&This->WriteWait, &This->CaptureMutex);
  }
  for(int i = 0; i < Len; i++) {
    int16_t Voltage = Buff16[i * 2 + 0];
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
void  *CaptureMono::Capture_Thread(void *args) {
  struct sched_param Prio;
  Prio.sched_priority = sched_get_priority_max(SCHED_OTHER);
  if(pthread_setschedparam(0, SCHED_OTHER, &Prio) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  CaptureMono *This = (CaptureMono*)args;  
  pthread_setname_np(This->CaptureThread, "CapMono_Init");
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
      sprintf(ThreadName, "CapMono_%03d", This->Used * 100 / This->Size);
      pthread_setname_np(This->CaptureThread, ThreadName);
    }
    Update++;
    libusb_handle_events_completed(NULL, NULL);
  }
  return NULL;
}
void  *CaptureMono::Analize_Thread(void* args) {
  CaptureMono *This = (CaptureMono*)args;
  pthread_setname_np(This->AnalyzeThread, "AnalysisMono");
  while(!This->Terminate) {
    pthread_mutex_lock(&This->AnalyzeMutex);
    while(This->Used < (This->GetMaxSamples() * 2)) {
      if(This->Terminate) return NULL;
      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
    }
    uint64_t Start = Tools::GetTime();
    This->pWork.Params.Timestamp = Start - ((This->Used * 1000) / ADQUISITION_SAMPLEFREQ);
    float LastSample = +100; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
    for(This->pWork.Samples = 0; This->pWork.Samples < This->GetMaxSamples(); This->pWork.Samples++) {
      
      This->pWork.VoltageSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Voltage;
      This->pWork.CurrentSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Current;
      float Out = This->pWork.VoltageSamples[0][This->pWork.Samples];
      for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &This->Filters[i]);
//printf("%f\n", Out);
      if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
        This->pWork.Cycles_Len++;
        if(This->pWork.Cycles_Len == 20) break;
        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
      }
      LastSample = Out;
      This->ReadPos = (This->ReadPos + 1) % This->Size;
    }
    This->Used -= This->pWork.Samples;
    pthread_cond_signal(&This->WriteWait);
    pthread_mutex_unlock(&This->AnalyzeMutex);
    This->Analyze(0.1);
    uint64_t AnalyzeEnd = Tools::GetTime();
    This->SaveWorkParams();
    uint64_t SaveEnd = Tools::GetTime();
    Tools::LogMsg("DEBUG", "Process of block %llu takes %.1fmS (%.1fmS mS with store)\n", This->pWork.SequenceID, (AnalyzeEnd - Start) / 1000.0, (SaveEnd - Start) / 1000.0);
  }
}
//float CaptureMono::CalculateFrequency(Work_t* work) {
//  static uint64_t NumSamples = 0;    //Numero de muestras leidas desde el ultimo calculo
//  static uint32_t NumCycles  = 0;    //Numero de ciclos desde ultimo calculo
//  float  Frequency = NAN;
//  NumSamples += work->Samples;
//  NumCycles += work->Cycles_Len;
//  if((((work->Timestamp / 1000000) % 10) == 0) && (NumSamples > (ADQUISITION_SAMPLEFREQ * 9))) {
//    Frequency = NumCycles / (NumSamples / ADQUISITION_SAMPLEFREQ) / 2.0;
//    NumSamples = 0;
//    NumCycles = 0;  
//  }
//  return Frequency;
//}
CaptureMono::CaptureMono(char* device, uint32_t bufferLen, float baseFreq) : CaptureInterface(15625, false, CaptureInterface::FREQ_50) {
  Tools::LogMsg("INFO", "Starting BasicCapture adquisition thread...\n");
  Buffer           = NULL;
  ReadPos          = 0;
  WritePos         = 0;
  Size             = bufferLen;
  Used             = 0;
  DeviceHandle     = NULL;
  Terminate        = false;
  
  VoltageGain = 0.0116585365854;             //Ganancia de tension
  CurrentGain = -0.00078125;             //Ganancia de corriente
  VoltageOffset = 292.0;           //Offset de tension
  CurrentOffset = 0.0;           //Offset de corriente
  
  
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
    Terminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
}
CaptureMono::~CaptureMono() {
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
void CaptureMono::SamplesCopy(float *dest, int32_t offset, uint32_t len) {
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