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
#include "DeviceV1.h"
#include "Tools.h"
#include "Analyzers/AnalyzerSoft.h"
// Funciones ===========================================================================================================
void DeviceV1::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV1 *This = (DeviceV1*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    int16_t *Pkg   = (int16_t*)&transfer->buffer[64 * Block];
    for(uint8_t Count = 0; Count < 16; Count++) {
      float Voltage = (Pkg[Count * 2 + 0] - This->pVoltageOffset) * This->pVoltageGain;
      float Current = (Pkg[Count * 2 + 1] - This->pCurrentOffset) * This->pCurrentGain;
//      if(This->BuffersAppend(&Voltage, &Current) == false) {
//        //NO HAY SITIO!!! que hago?
//        Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
//      }
    }
  }
//  int16_t *Buff16 = (int16_t*)transfer->buffer;
//  uint     Len = transfer->actual_length / 4;
//  pthread_mutex_lock(&This->CaptureMutex);
//  while(This->Used > (CAPTUREMONO_BUFFERSIZE - Len - This->Samplerate * EVENT_ENVELOPETIME)) {
//    if(This->pTerminate) return;
//    Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
//    pthread_cond_wait(&This->WriteWait, &This->CaptureMutex);
//  }
//  for(uint i = 0; i < Len; i++) {
//    int16_t Voltage = Buff16[i * 2 + 0];
//    int16_t Current = Buff16[i * 2 + 1];
//    This->Buffer[This->WritePos].Voltage = (Voltage - This->pVoltageOffset) * This->pVoltageGain;
//    This->Buffer[This->WritePos].Current = (Current - This->pCurrentOffset) * This->pCurrentGain;
//    This->WritePos = (This->WritePos + 1) % CAPTUREMONO_BUFFERSIZE;
//  }
//  This->Used += Len;
//  pthread_cond_signal(&This->ReadWait);
//  pthread_mutex_unlock(&This->CaptureMutex);
  if(This->pTerminate == false) libusb_submit_transfer(transfer);
}
int DeviceV1::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV1 *This = (DeviceV1*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);	
      if(dev == CurrentDev) {
        Tools::LogMsg("INFO", "Device CaptureMono with serial '%s' disconnected\n", This->pSerial);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void  *DeviceV1::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 5;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  }
  DeviceV1 *This = (DeviceV1*)args;
//  pthread_setname_np(This->CaptureThread, "CapMono_Init");
  uint8_t BufferIN[CAPTUREMONO_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[CAPTUREMONO_TRANSFERS];
  for(int i = 0; i < CAPTUREMONO_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
//  uint32_t Update = 0;
  while(!This->pTerminate) {
//    if((Update % 10) == 0) {
//      char ThreadName[16];
//      sprintf(ThreadName, "CapMono_%03d", This->Used * 100 / CAPTUREMONO_BUFFERSIZE);
//      pthread_setname_np(This->CaptureThread, ThreadName);
//    }
//    Update++;
    struct timeval tv = { 1, 0 };
    int Result = libusb_handle_events_timeout_completed(NULL, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) Tools::LogMsg("ERROR", "libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(This->pNewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_get_device_descriptor return code %d\n", Result);
        This->pNewDevice = NULL;
        continue;
      }
      Result = libusb_open(This->pNewDevice, &This->pDeviceHandle);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_open return %d\n", Result);
        This->pDeviceHandle = NULL;
        This->pNewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(This->pDeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        Tools::LogMsg("ERROR", "libusb_get_string_descriptor_ascii returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      if(strcmp(This->pSerial, Serial) != 0) {
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }        
      Result = libusb_kernel_driver_active(This->pDeviceHandle, 0);
      if(Result < LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_kernel_driver_active returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else if(Result == 1) { 
        Result = libusb_detach_kernel_driver(This->pDeviceHandle, 0);
        if(Result < LIBUSB_SUCCESS) {
          Tools::LogMsg("ERROR", "libusb_detach_kernel_driver returns %d\n", Result);
          libusb_close(This->pDeviceHandle);
          This->pDeviceHandle = NULL; 
          This->pNewDevice = NULL;
          continue;
        } else {
          Tools::LogMsg("INFO", "Kernel driver dettached for CaptureMono device with serial %s\n", This->pSerial);
        }
      }
      Result = libusb_claim_interface(This->pDeviceHandle, 1);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_claim_interface returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else {
        Tools::LogMsg("INFO", "Claimed interface 1 on CaptureMono device with serial %s\n", This->pSerial);
      }
      for(int i = 0; i < CAPTUREMONO_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(TransferIN[i], This->pDeviceHandle, CAPTUREMONO_ENDPOINT_IN, BufferIN[i], sizeof(BufferIN[i]), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(TransferIN[i]);
      }
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      Tools::LogMsg("INFO", "Capture started on CaptureMono device with serial %s\n", This->pSerial);
      This->pNewDevice = NULL;
    }
  }
  return NULL;
}
//void  *DeviceV1::Analize_Thread(void* args) {
//  DeviceV1 *This = (DeviceV1*)args;
//  if(pthread_setschedprio(0, -10) == -1) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return NULL;
//  }
//  pthread_setname_np(This->AnalyzeThread, "AnalysisMono");
//  int Discard = 100;
//  while(!This->pTerminate) {
//    pthread_mutex_lock(&This->AnalyzeMutex);
//    while(This->Used < (This->GetMaxSamples() + This->SampleRate() * EVENT_ENVELOPETIME)) {
//      if(This->pTerminate) return NULL;
//      pthread_cond_wait(&This->ReadWait, &This->AnalyzeMutex);
//    }
//    uint64_t Start = Tools::GetTime();
//    This->pWork.Params.Timestamp = Start - ((This->Used * 1000) / CAPTUREMONO_SAMPLEFREQ);
//    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
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
//    pthread_cond_signal(&This->WriteWait);
//    pthread_mutex_unlock(&This->AnalyzeMutex);
//    if(Discard > 0) {
//      Discard--;
//      This->ResetWork();
//      This->ResetParams(&This->pWork.Params);
//    } else {
//      This->Analyze(0.1);
//      uint64_t AnalyzeEnd = Tools::GetTime();
//      This->SaveWorkParams();
//      uint64_t SaveEnd = Tools::GetTime();
//      Tools::LogMsg("DEBUG", "Process of block %" PRIu64 " takes %umS (%" PRIu64 "mS mS with store)\n", This->pWork.SequenceID, AnalyzeEnd - Start, SaveEnd - Start);
//    }
//  }
//  return NULL;
//}
DeviceV1::DeviceV1(json_object* config) : DeviceInterface() {
  Tools::LogMsg("INFO", "Starting BasicCapture adquisition thread...\n");
//  Buffer           = NULL;
//  ReadPos          = 0;
//  WritePos         = 0;
//  Used             = 0;
  pDeviceHandle          = NULL;
  pNewDevice             = NULL;
  pSerial                = (char*)"000000000000000000000000";
  pVoltageGain           = 1;
  pCurrentGain           = 1;
  pVoltageOffset         = 0;
  pCurrentOffset         = 0;
  float NominalVoltage   = 230.0;
  float NominalFrequency = 50.0;
  char *Name             = (char*)"Unnamed input";
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Serial") == 0)) {
      pSerial = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "VoltageGain") == 0)) {
      pVoltageGain = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "VoltageOffset") == 0)) {
      pVoltageOffset = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "CurrentGain") == 0)) {
      pCurrentGain = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "CurrentOffset") == 0)) {
      pCurrentOffset = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalVoltage") == 0)) {
      NominalVoltage = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalFrequency") == 0)) {
      NominalFrequency = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DisplayName") == 0)) {
      Name = (char*)json_object_get_string(JSON_Val);
    }
  }
  AnalyzerSoft *Analyzer = new AnalyzerSoft(15625.0, pSerial, NominalVoltage, NominalFrequency, Name, 50.0, 0.1, AnalyzerInterface::PHASE1);
  AddAnalyzer(Analyzer);
  //  InitAnalyzerDB();
//  pthread_cond_init(&WriteWait, NULL);
//  pthread_cond_init(&ReadWait, NULL);
//  pthread_mutex_init(&AnalyzeMutex, NULL);
//  pthread_mutex_init(&CaptureMutex, NULL);
//  for(int i = 0; i < 5; i++) {
 //   float Q = 1.0;
//    float K = tan(M_PI * ((float)Tools::Electrical.NominalFrequency / CAPTUREMONO_SAMPLEFREQ));
//    float Norm = 1 / (1 + K / Q + K * K);
//    Filters[i].A0 = K / Q * Norm;
//    Filters[i].A1 = 0;
//    Filters[i].A2 = -Filters[i].A0;
//    Filters[i].B1 = 2 * (K * K - 1) * Norm;
//    Filters[i].B2 = (1 - K / Q + K * K) * Norm;
//    Filters[i].Z1 = 0.0;
//    Filters[i].Z2 = 0.0;            
//  }
  int Error = libusb_init(NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo la apertura de libUSB...\n");
    return;
  }
//  Buffer = (Sample_t*)malloc(CAPTUREMONO_BUFFERSIZE * sizeof(Sample_t));
//  if(Buffer == NULL) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    libusb_exit(NULL);
//    return; 
//  }
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
//    free(Buffer);
    return;
  }
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , CAPTUREMONO_VID, CAPTUREMONO_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
    pTerminate = true;
    pthread_join(pCaptureThread, NULL);
//    free(Buffer);
    return;
  }
//  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pTerminate = true;
//    pthread_join(CaptureThread, NULL);
//    free(Buffer);
//    return;
//  }
}
DeviceV1::~DeviceV1() {
//  if(Buffer) {
//    pthread_cond_signal(&WriteWait);
//    pthread_cond_signal(&ReadWait);
//    pTerminate = true;
//    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
//    pthread_join(CaptureThread, NULL);
//    pthread_join(AnalyzeThread, NULL);
//    free(Buffer);
//  }
}
//uint32_t CaptureMono::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
//  for(uint i = 0; i < len; i++) {
//    uint32_t Pos = ((ReadPos + CAPTUREMONO_BUFFERSIZE) - offset + i) % CAPTUREMONO_BUFFERSIZE;
//    dest[i][0] = Buffer[Pos].Voltage;
//  }
//  return len;
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