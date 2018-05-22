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
#include "CaptureV2.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
void   CaptureV2::ReadCallback(struct libusb_transfer *transfer) {
  uint16_t *Buff16 = (uint16_t*)transfer->buffer;
  uint     Len = transfer->actual_length / 4;
  CaptureV2 *This = (CaptureV2*)transfer->user_data;
  pthread_mutex_lock(&This->CaptureMutex);
//  while(This->Used > (CAPTURE_V2_BUFFERSIZE - Len - This->SampleRate() * EVENT_ENVELOPETIME)) {
//    if(This->pTerminate) return;
//    Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
//    pthread_cond_wait(&This->WriteWait, &This->CaptureMutex);
//  }
  for(uint i = 0; i < Len; i++) {
    int16_t Voltage = Buff16[i * 2 + 0];
    uint16_t Current = Buff16[i * 2 + 1];
//    This->Buffer[This->WritePos].Voltage = (Voltage - This->VoltageOffset) * This->VoltageGain;
//    This->Buffer[This->WritePos].Current = (Current - This->CurrentOffset) * This->CurrentGain;
//    This->WritePos = (This->WritePos + 1) % CAPTUREMONO_BUFFERSIZE;
  }
  This->Used += Len;
  pthread_cond_signal(&This->ReadWait);
  pthread_mutex_unlock(&This->CaptureMutex);
  if(Tools::Terminate == false) libusb_submit_transfer(transfer);
}
int CaptureV2::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  CaptureV2 *This = (CaptureV2*)user_data;
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
void  *CaptureV2::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 5;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  }
  CaptureV2 *This = (CaptureV2*)args;
  pthread_setname_np(This->CaptureThread, "CapMono_Init");
  uint8_t BufferIN[CAPTURE_V2_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[CAPTURE_V2_TRANSFERS];
  for(int i = 0; i < CAPTURE_V2_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
  uint32_t Update = 0;
  while(!Tools::Terminate) {
    if((Update % 10) == 0) {
      char ThreadName[16];
      sprintf(ThreadName, "CapV2_%03d", This->Used * 100 / CAPTURE_V2_BUFFERSIZE);
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
      Result = libusb_claim_interface(This->DeviceHandle, 1);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_claim_interface returns %d\n", Result);
        libusb_close(This->DeviceHandle);
        This->DeviceHandle = NULL; 
        This->NewDevice = NULL;
        continue;
      } else {
        Tools::LogMsg("INFO", "Claimed interface 1 on CaptureMono device with serial %s\n", This->Serial);
      }
      for(int i = 0; i < CAPTURE_V2_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(TransferIN[i], This->DeviceHandle, CAPTURE_V2_ENDPOINT_IN, BufferIN[i], sizeof(BufferIN[i]), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(TransferIN[i]);
      }
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
void  *CaptureV2::Analize_Thread(void* args) {
  CaptureV2 *This = (CaptureV2*)args;
  if(pthread_setschedprio(0, -10) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(This->AnalyzeThread, "AnalysisMono");
  int Discard = 100;
  while(!Tools::Terminate) {
    pthread_mutex_lock(&This->AnalyzeMutex);
//    while(This->Used < (This->GetMaxSamples() + This->SampleRate() * EVENT_ENVELOPETIME)) {
//      if(This->pTerminate) return NULL;
//      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
//    }
    uint64_t Start = Tools::GetTime();
//    This->pWork.Params.Timestamp = Start - ((This->Used * 1000) / CAPTUREMONO_SAMPLEFREQ);
    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
//    for(This->pWork.Samples = 0; This->pWork.Samples < This->GetMaxSamples(); This->pWork.Samples++) {
//      This->pWork.VoltageSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Voltage;
//      This->pWork.CurrentSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Current;
//      float Out = This->pWork.VoltageSamples[0][This->pWork.Samples];
//      for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &This->Filters[i]);
//      if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
//        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
//        This->pWork.Cycles_Len++;
//        if(This->pWork.Cycles_Len == 20) break;
//        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
//      }
//      LastSample = Out;
//      This->ReadPos = (This->ReadPos + 1) % CAPTUREMONO_BUFFERSIZE;
//    }
//    This->Used -= This->pWork.Samples;
    pthread_cond_signal(&This->WriteWait);
    pthread_mutex_unlock(&This->AnalyzeMutex);
    if(Discard > 0) {
      Discard--;
//      This->ResetWork();
//      This->ResetParams(&This->pWork.Params);
    } else {
//      This->Analyze(0.1);
      uint64_t AnalyzeEnd = Tools::GetTime();
//      This->SaveWorkParams();
      uint64_t SaveEnd = Tools::GetTime();
//      Tools::LogMsg("DEBUG", "Process of block %" PRIu64 " takes %umS (%" PRIu64 "mS mS with store)\n", This->pWork.SequenceID, AnalyzeEnd - Start, SaveEnd - Start);
    }
  }
  return NULL;
}
bool CaptureV2::CreateInstance(json_object* config) {
  //Leo configuracion, por si es del tipo que quiero
  //Si lo es inicio creo una instancia y añado los dispositivos necesarios
//    json_object_object_foreach(config, JSON_Key, JSON_Val) {
//    enum json_type JSON_Type = json_object_get_type(JSON_Val);
//    if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastLogin") == 0)) {
//      LastLogin = json_object_get_int64(JSON_Val);
//    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) {
//      Port = json_object_get_int(JSON_Val);
//    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "FilePath") == 0)) {
//      FilePath = (char*)json_object_get_string(JSON_Val);
//    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) {
//      Password = (char*)json_object_get_string(JSON_Val);
//    }
//  }
  CaptureV2 *Device = new CaptureV2();
  Device->Analyzers[0] = new Analyzer(30000, true, 4);
  return false;
}
CaptureV2::CaptureV2(/*const char *serial, float *voltageGain, int *voltageOffset, float *currentGain, int *currentOffset, uint16_t id*/) {
  Tools::LogMsg("INFO", "Starting CaptureV2 adquisition thread...\n");
  Buffer           = NULL;
  ReadPos          = 0;
  WritePos         = 0;
  Used             = 0;
  DeviceHandle     = NULL;
  NewDevice        = NULL;
//  Serial           = serial;
//  VoltageGain      = voltageGain;
//  CurrentGain      = currentGain;
//  VoltageOffset    = voltageOffset;
//  CurrentOffset    = currentOffset;
  pthread_cond_init(&WriteWait, NULL);
  pthread_cond_init(&ReadWait, NULL);
  pthread_mutex_init(&AnalyzeMutex, NULL);
  pthread_mutex_init(&CaptureMutex, NULL);
  for(int i = 0; i < 5; i++) {
    float Q = 1.0;
    float K = tan(M_PI * ((float)Tools::Electrical.NominalFrequency / CAPTURE_V2_SAMPLEFREQ));
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
  Buffer = (float*)malloc(CAPTURE_V2_BUFFERSIZE * sizeof(float));
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
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , CAPTURE_V2_VID, CAPTURE_V2_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
//    pTerminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pTerminate = true;
    pthread_join(CaptureThread, NULL);
    free(Buffer);
    return;
  }
}
CaptureV2::~CaptureV2() {
  if(Buffer) {
    pthread_cond_signal(&WriteWait);
    pthread_cond_signal(&ReadWait);
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(CaptureThread, NULL);
    pthread_join(AnalyzeThread, NULL);
    free(Buffer);
  }
}
uint32_t CaptureV2::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
  for(uint i = 0; i < len; i++) {
//    uint32_t Pos = ((ReadPos + CAPTUREMONO_BUFFERSIZE) - offset + i) % CAPTUREMONO_BUFFERSIZE;
//    dest[i][0] = Buffer[Pos].Voltage;
  }
  return len;
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
