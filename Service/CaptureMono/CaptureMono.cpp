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
#include "CaptureMono.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
void   CaptureMono::ReadCallback(struct libusb_transfer *transfer) {
  int16_t *Buff16 = (int16_t*)transfer->buffer;
  int      Len = transfer->actual_length / 4;
  CaptureMono *This = (CaptureMono*)transfer->user_data;
  pthread_mutex_lock(&This->CaptureMutex);
  while(This->Used > (This->Size - Len)) {
    if(This->pTerminate) return;
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
  if(This->pTerminate == false) libusb_submit_transfer(transfer);
}
int CaptureMono::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  CaptureMono *This = (CaptureMono*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->DeviceHandle == NULL)) {
    This->NewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->DeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->DeviceHandle);	
      if(dev == CurrentDev) {
        Tools::LogMsg("INFO", "Device CaptureMono with serial '%s' disconnected\n", This->Serial);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void  *CaptureMono::Capture_Thread(void *args) {
  if(pthread_setschedprio(0, -20) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  CaptureMono *This = (CaptureMono*)args;
  pthread_setname_np(This->CaptureThread, "CapMono_Init");
  uint8_t BufferIN[3][32768];
  struct libusb_transfer *TransferIN[3];
  TransferIN[0] = libusb_alloc_transfer(0);
  TransferIN[1] = libusb_alloc_transfer(0);
  TransferIN[2] = libusb_alloc_transfer(0);
  uint32_t Update = 0;
  while(!This->pTerminate) {
    if((Update % 10) == 0) {
      char ThreadName[16];
      sprintf(ThreadName, "CapMono_%03d", This->Used * 100 / This->Size);
      pthread_setname_np(This->CaptureThread, ThreadName);
    }
    Update++;
    struct timeval tv = { 1, 0 };
    int Result = libusb_handle_events_timeout_completed(NULL, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) Tools::LogMsg("ERROR", "libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->NewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(This->NewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_get_device_descriptor return code %d\n", Result);
        This->NewDevice = NULL;
        continue;
      }
      Result = libusb_open(This->NewDevice, &This->DeviceHandle);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_open return %d\n", Result);
        This->DeviceHandle = NULL;
        This->NewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(This->DeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        Tools::LogMsg("ERROR", "libusb_get_string_descriptor_ascii returns %d\n", Result);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      }
      if(strcmp(This->Serial, Serial) != 0) {
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      }        
      Result = libusb_kernel_driver_active(This->DeviceHandle, 0);
      if(Result < LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_kernel_driver_active returns %d\n", Result);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      } else if(Result == 1) { 
        Result = libusb_detach_kernel_driver(This->DeviceHandle, 0);
        if(Result < LIBUSB_SUCCESS) {
          Tools::LogMsg("ERROR", "libusb_detach_kernel_driver returns %d\n", Result);
          libusb_close(This->DeviceHandle);
          This->DeviceHandle = NULL; 
          This->NewDevice = NULL;
          continue;
        } else {
          Tools::LogMsg("INFO", "Kernel driver dettached for CaptureMono device with serial %s\n", This->Serial);
        }
      }
      Result = libusb_claim_interface(This->DeviceHandle, 0);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_claim_interface returns %d\n", Result);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      } else {
        Tools::LogMsg("INFO", "Claimed interface 0 on CaptureMono device with serial %s\n", This->Serial);
      }
      libusb_fill_bulk_transfer(TransferIN[0], This->DeviceHandle, ADQUISITION_ENDPOINT_IN, BufferIN[0], sizeof(BufferIN[0]), ReadCallback, This, 500);
      libusb_fill_bulk_transfer(TransferIN[1], This->DeviceHandle, ADQUISITION_ENDPOINT_IN, BufferIN[1], sizeof(BufferIN[1]), ReadCallback, This, 500);
      libusb_fill_bulk_transfer(TransferIN[2], This->DeviceHandle, ADQUISITION_ENDPOINT_IN, BufferIN[2], sizeof(BufferIN[2]), ReadCallback, This, 500);
      Result = libusb_submit_transfer(TransferIN[0]) | libusb_submit_transfer(TransferIN[1]) | libusb_submit_transfer(TransferIN[2]);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      }
      Tools::LogMsg("INFO", "Capture started on CaptureMono device with serial %s\n", This->Serial);
      This->NewDevice = NULL;
    }
  }
  return NULL;
}
void  *CaptureMono::Analize_Thread(void* args) {
  CaptureMono *This = (CaptureMono*)args;
  if(pthread_setschedprio(0, -10) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(This->AnalyzeThread, "AnalysisMono");
  while(!This->pTerminate) {
    pthread_mutex_lock(&This->AnalyzeMutex);
    while(This->Used < (This->GetMaxSamples())) {
      if(This->pTerminate) return NULL;
      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
    }
    uint64_t Start = Tools::GetTime();
    This->pWork.Params.Timestamp = Start - ((This->Used * 1000) / ADQUISITION_SAMPLEFREQ);
    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
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
    This->Used -= This->pWork.Samples;
    pthread_cond_signal(&This->WriteWait);
    pthread_mutex_unlock(&This->AnalyzeMutex);
    This->Analyze(0.1);
    uint64_t AnalyzeEnd = Tools::GetTime();
    This->SaveWorkParams();
    uint64_t SaveEnd = Tools::GetTime();
    Tools::LogMsg("DEBUG", "Process of block %llu takes %umS (%llumS mS with store)\n", This->pWork.SequenceID, AnalyzeEnd - Start, SaveEnd - Start);
  }
}
CaptureMono::CaptureMono(const char *serial, float voltageGain, int voltageOffset, float currentGain, int currentOffset, uint32_t bufferLen, enum Freq_t freq, uint16_t id) : CaptureInterface(15625, false, freq, id) {
  Tools::LogMsg("INFO", "Starting BasicCapture adquisition thread...\n");
  Buffer           = NULL;
  ReadPos          = 0;
  WritePos         = 0;
  Size             = bufferLen;
  Used             = 0;
  DeviceHandle     = NULL;
  NewDevice        = NULL;
  Serial           = serial;
  VoltageGain      = voltageGain;
  CurrentGain      = currentGain;
  VoltageOffset    = voltageOffset;
  CurrentOffset    = currentOffset;
  pthread_cond_init(&WriteWait, NULL);
  pthread_cond_init(&ReadWait, NULL);
  pthread_mutex_init(&AnalyzeMutex, NULL);
  pthread_mutex_init(&CaptureMutex, NULL);
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * ((float)NominalFrequency() / ADQUISITION_SAMPLEFREQ));
    float Norm = 1 / (1 + K / Q + K * K);
    Filters[i].A0 = K / Q * Norm;
    Filters[i].A1 = 0;
    Filters[i].A2 = -Filters[i].A0;
    Filters[i].B1 = 2 * (K * K - 1) * Norm;
    Filters[i].B2 = (1 - K / Q + K * K) * Norm;
    Filters[i].Z1 = 0.0;
    Filters[i].Z2 = 0.0;            
  }
  int Error = libusb_init(NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo la apertura de libUSB...\n");
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
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , CAPTUREMONO_VID, CAPTUREMONO_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
    pTerminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pTerminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
}
CaptureMono::~CaptureMono() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    pTerminate = true;
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(CaptureThread, NULL);
    pthread_join(AnalyzeThread, NULL);
    free(Buffer);
  }
}
//void CaptureMono::SamplesCopy(float *dest, int32_t offset, uint32_t len) {
//  for(int i = 0; i < len; i++) 
//    dest[i] = Buffer[(ReadPos - offset + i) % Size].Voltage;
//}
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