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
    This->Buffer[This->WritePos].Voltage = Buff16[i * 2 + 0];
    This->Buffer[This->WritePos].Current = Buff16[i * 2 + 1];
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
    Work->VoltageSamples[Work->Samples] = (Buffer[ReadPos].Voltage - Tools::Electrical.VoltageOffset) * Tools::Electrical.VoltageGain;
    Work->CurrentSamples[Work->Samples] = (Buffer[ReadPos].Current - Tools::Electrical.CurrentOffset) * Tools::Electrical.CurrentGain;;
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
    dest[i] = (Buffer[(ReadPos - offset + i) % Size].Voltage - Tools::Electrical.VoltageOffset) * Tools::Electrical.VoltageGain;
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