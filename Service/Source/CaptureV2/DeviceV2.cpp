/* ================================== zMeter ===========================================================================
 * File:     DeviceV2.cpp
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
#include "DeviceV2.h"
#include "../Tools.h"
// Funciones ===========================================================================================================
void DeviceV2::ReadCallback(struct libusb_transfer *transfer) {
  uint16_t *Buff16 = (uint16_t*)transfer->buffer;
  DeviceV2 *This = (DeviceV2*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    uint16_t *BlockPtr = &Buff16[32 * Block];
    uint8_t   Pos      = 0;
    pthread_mutex_lock(&This->pCaptureMutex);
    if(This->pSyncState == WAIT) continue;
    if(BlockPtr[0] == 0xFFFF) {
      if(This->pSyncState == SYNC) Tools::LogMsg("WARN", "DeviceV2 lost sync. Try reduce capture speed\n");
      This->pSyncState = SYNC;
      This->pUsed = 0;
      Pos++;
    }
    while(This->pUsed > (This->pBuffLen - 32)) {
      if(This->pTerminate) return;
      Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
      pthread_cond_wait(&This->pWriteWait, &This->pCaptureMutex);
    }
    for(uint8_t ReadPos = Pos; ReadPos < 32; ReadPos++) {
      if(BlockPtr[ReadPos] == 0xFFFF) break;
      This->pBuffer[This->pWritePos] = BlockPtr[ReadPos];
      This->pWritePos = (This->pWritePos + 1) % This->pBuffLen;
      This->pUsed++;
    }
    pthread_cond_signal(&This->pReadWait);
    pthread_mutex_unlock(&This->pCaptureMutex); 
  }
  if(This->pTerminate == false) libusb_submit_transfer(transfer);
}
int DeviceV2::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV2 *This = (DeviceV2*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);	
      if(dev == CurrentDev) {
        Tools::LogMsg("INFO", "Device DeviceV2 with serial '%s' disconnected\n", This->pSerial);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void *DeviceV2::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 5;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  }
  DeviceV2 *This = (DeviceV2*)args;
  pthread_setname_np(This->CaptureThread, "DeviceV2_Init");
  uint8_t BufferIN[CAPTUREV2_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[CAPTUREV2_TRANSFERS];
  for(int i = 0; i < CAPTUREV2_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
  uint32_t Update = 0;
  while(!This->pTerminate) {
    if((Update % 10) == 0) {
      char ThreadName[16] = "DeviceV2_Wait";
      if(This->pDeviceHandle != NULL) sprintf(ThreadName, "DeviceV2_%03d", This->pUsed * 100 / This->pBuffLen); 
      pthread_setname_np(This->CaptureThread, ThreadName);
    }
    Update++;
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
          Tools::LogMsg("INFO", "Kernel driver dettached for DeviceV2 with serial %s\n", This->pSerial);
        }
      }
      Result = libusb_claim_interface(This->pDeviceHandle, 0);
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_claim_interface returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else {
        Tools::LogMsg("INFO", "Claimed interface 1 on DeviceV2 with serial %s\n", This->pSerial);
      }
      This->pSyncState = WAIT;
      for(int i = 0; i < CAPTUREV2_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(TransferIN[i], This->pDeviceHandle, CAPTUREV2_ENDPOINT_IN, BufferIN[i], sizeof(BufferIN[i]), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(TransferIN[i]);
      }
      if(Result != LIBUSB_SUCCESS) {
        Tools::LogMsg("ERROR", "libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      Tools::LogMsg("INFO", "Capture started on DeviceV2 with serial %s\n", This->pSerial);
      This->pNewDevice = NULL;
      int Error = libusb_control_transfer(This->pDeviceHandle, 0x21, 0x00, 0x0000, 0, NULL, 0, 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not stop device with serial %s (%d)\n", This->pSerial, Error);
      usleep(1000000);
      Error = libusb_control_transfer(This->pDeviceHandle, 0x21, 0x00, (This->pChannelMask << 8) | 0x21, 0, NULL, 0, 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not setup device with serial %s (%d)\n", This->pSerial, Error);
      This->pSyncState = NOSYNC;
    }
  }
  return NULL;
}
void *DeviceV2::Analize_Thread(void* args) {
  DeviceV2 *This = (DeviceV2*)args;
  if(pthread_setschedprio(0, -10) == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  pthread_setname_np(This->AnalyzeThread, "AnalyzeV2");
  while(!This->pTerminate) {
    pthread_mutex_lock(&This->pAnalyzeMutex);
    while(This->pUsed < (This->GetMaxSamples() * This->pChannelCount)) {
      if(This->pTerminate) return NULL;
      pthread_cond_wait(&This->pReadWait, &This->pAnalyzeMutex);
    }
    uint64_t Start = Tools::GetTime();
//    This->pWork.Params.Timestamp = Start - ((This->pUsed * 1000) / (This->pSamplerate * This->pChannelCount));
    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
//  for(uint i = 0; i < Len; i++) {
//    int16_t Voltage = Buff16[i * 2 + 0];
//    uint16_t Current = Buff16[i * 2 + 1];
//    This->Buffer[This->WritePos].Voltage = (Voltage - This->VoltageOffset) * This->VoltageGain;
//    This->Buffer[This->WritePos].Current = (Current - This->CurrentOffset) * This->CurrentGain;
//    This->WritePos = (This->WritePos + 1) % CAPTUREMONO_BUFFERSIZE;
//  }
    uint32_t Samples = 0;
    while(Samples < This->GetMaxSamples()) {
      for(uint8_t Channel = 0; Channel < This->pChannelCount; Channel++) {
        //for(uint8_t Analyzer = 0; Analyzer < This->pAnalyzersCount; Analyzer++) {
//          if(Analyzer != 0) continue;
          This->pAnalyzers[0]->pWork.CurrentSamples[3][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;
          This->pAnalyzers[0]->pWork.CurrentSamples[2][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;          
          This->pAnalyzers[0]->pWork.VoltageSamples[2][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;        
          This->pAnalyzers[0]->pWork.CurrentSamples[1][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;
          This->pAnalyzers[0]->pWork.VoltageSamples[1][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;        
          This->pAnalyzers[0]->pWork.CurrentSamples[0][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;        
          This->pAnalyzers[0]->pWork.VoltageSamples[0][Samples] = This->pBuffer[This->pReadPos]; 
          This->pReadPos = (This->pReadPos + 1) % This->pBuffLen;        

          
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
        //}
      
      }
      Samples++;
    }
    printf("%f\n", This->pAnalyzers[0]->pWork.CurrentSamples[3][0]);
      fflush(stdout);
    
    
    This->pUsed -= Samples * This->pChannelCount;
    This->pAnalyzers[0]->pWork.Samples = Samples;
    pthread_cond_signal(&This->pWriteWait);
    pthread_mutex_unlock(&This->pAnalyzeMutex);
//    if(Discard > 0) {
//      Discard--;
//      This->ResetWork();
//      This->ResetParams(&This->pWork.Params);
//    } else {
//      This->Analyze(0.1);
    This->pAnalyzers[0]->Analyze(0.1);
      uint64_t AnalyzeEnd = Tools::GetTime();
      This->pAnalyzers[0]->SaveWorkParams();
      uint64_t SaveEnd = Tools::GetTime();
      Tools::LogMsg("DEBUG", "Process of block %" PRIu64 " takes %umS (%" PRIu64 "mS mS with store)\n", This->pSequenceID, AnalyzeEnd - Start, SaveEnd - Start);
//    }
  }
  return NULL;
}
DeviceV2 *DeviceV2::Init(json_object* config) {
  DeviceV2 *Device = new DeviceV2();
  bool     ValidDriver      = false;
  bool     ValidParams      = true;
  uint8_t  ChannelMask      = 0;

  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Driver") == 0)) {
      if(strcmp("CaptureV2", (char*)json_object_get_string(JSON_Val)) == 0) ValidDriver = true;
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "ID") == 0)) {
      Device->pID = json_object_get_int(JSON_Val);
      if((Device->pID % 4) != 0) {
        ValidParams = false;
        Tools::LogMsg("ERROR", "Device ID must be multiple of 4\n");
      }
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Oversample") == 0)) {
      Device->pOversample = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Samplerate") == 0)) {
      Device->pSamplerate = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Serial") == 0)) {
      Device->pSerial = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "VoltageGain") == 0)) {
      if(json_object_array_length(JSON_Val) != 3) {
        ValidParams = false;
      } else {
        for(int i = 0; i < 3; i++) {
          if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: Voltage gain must have 3 double values\n");
          } else {
            Device->pVoltageGain[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
          }
        }
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "VoltageOffset") == 0)) {
      if(json_object_array_length(JSON_Val) != 3) {
        ValidParams = false;
      } else {
        for(int i = 0; i < 3; i++) {
          if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: Voltage offset must have 3 double values\n");
          } else {
            Device->pVoltageOffset[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
          }
        }
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "CurrentGain") == 0)) {
      if(json_object_array_length(JSON_Val) != 4) {
        ValidParams = false;
      } else {
        for(int i = 0; i < 4; i++) {
          if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: current gain must have 4 double values\n");
          } else {
            Device->pCurrentGain[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
          }
        }
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "CurrentOffset") == 0)) {
      if(json_object_array_length(JSON_Val) != 4) {
        ValidParams = false;
      } else {
        for(int i = 0; i < 4; i++) {
          if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: current offset must have 4 double values\n");
          } else {
            Device->pCurrentOffset[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
          }
        }
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "Outputs") == 0)) {
      int Count = json_object_array_length(JSON_Val);
      Device->pAnalyzers = (Analyzer**)malloc(sizeof(Analyzer*) * Count);
      for(int i = 0; i < Count; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_object) continue;
        char *DisplayName = (char*)"Default Capture";
        Analyzer::RouteType Type = Analyzer::FASE_3N;
        uint32_t ID = 0;
        uint8_t VoltageChannel = 6;
        uint8_t CurrentChannel = 5;
        json_object_object_foreach(json_object_array_get_idx(JSON_Val, i), JSON_KeySub, JSON_ValSub) {
          enum json_type JSON_TypeSub = json_object_get_type(JSON_ValSub);
          if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "DisplayName") == 0)) {
            DisplayName = (char*)json_object_get_string(JSON_ValSub);
          } else if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "Mode") == 0)) {
            char *ModeText = (char*)json_object_get_string(JSON_ValSub);
            if(strcmp("3Phase+N", ModeText) == 0) {
              Type = Analyzer::FASE_3N;
            } else if(strcmp("3Phase", ModeText) == 0) {
              Type = Analyzer::FASE_3;
            } else if(strcmp("1Phase", ModeText) == 0) {
              Type = Analyzer::FASE_1;
            } else {
              ValidParams = false;
              Tools::LogMsg("ERROR", "CaptureV2: invalid output mode, try '3Phase+N'. '3Phase' or '1Phase'\n");
            }
          } else if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "VoltageChannel") == 0)) {
            char *ChannelText = (char*)json_object_get_string(JSON_ValSub);
            if(strcmp("R", ChannelText) == 0) {
              VoltageChannel = 6;
            } else if(strcmp("S", ChannelText) == 0) {
              VoltageChannel = 4;
            } else if(strcmp("T", ChannelText) == 0) {
              VoltageChannel = 2;
            } else {
              ValidParams = false;
              Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S' or 'T'\n");
            }
          } else if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "CurrentChannel") == 0)) {
            char *ChannelText = (char*)json_object_get_string(JSON_ValSub);
            if(strcmp("R", ChannelText) == 0) {
              CurrentChannel = 5;
            } else if(strcmp("S", ChannelText) == 0) {
              CurrentChannel = 3;
            } else if(strcmp("T", ChannelText) == 0) {
              CurrentChannel = 1;
            } else if(strcmp("N", ChannelText) == 0) {
              CurrentChannel = 0;
            } else {
              ValidParams = false;
              Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S', 'T' or 'N'\n");
            }
          }
        }
        if(Type == Analyzer::FASE_3N) {
          if(ChannelMask != 0) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
          } else {
            ChannelMask |= 0x7F;
          }
        } else if(Type == Analyzer::FASE_3) {
          if((ChannelMask & 0x2A) != 0) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
          } else {
            ChannelMask |= 0x7E; 
          }
        } else if(Type == Analyzer::FASE_1) {
          if((ChannelMask & (1 << CurrentChannel)) != 0) {
            ValidParams = false;
            Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
          } else {
            ChannelMask |= (1 << CurrentChannel) | (1 << VoltageChannel);
          }
        }
        if(ValidParams == true) {
          Device->pAnalyzers[Device->pAnalyzersCount] = new Analyzer(Device, true, Device->pID + Device->pAnalyzersCount);
          Device->pAnalyzersCount++;
        }
      }
    }
  }
  Device->pChannelMask = ChannelMask;
  if(ValidDriver & ValidParams) return Device;
  delete Device;
  return NULL;
}
DeviceV2::DeviceV2() {
  for(int i = 0; i < 3; i++) pVoltageGain[i]   = 1;
  for(int i = 0; i < 3; i++) pVoltageOffset[i] = 0;
  for(int i = 0; i < 4; i++) pCurrentGain[i]   = 1;
  for(int i = 0; i < 4; i++) pCurrentOffset[i] = 0;
  pSerial           = NULL;
  pBuffer           = NULL;
  pChannelMask      = 0;
  pChannelCount     = 0;
  pReadPos          = 0;
  pWritePos         = 0;
  pUsed             = 0;
  pDeviceHandle     = NULL;
  pNewDevice        = NULL;
  pOversample       = 0;
  pBuffLen          = 0;
  pSyncState        = WAIT;
  pthread_cond_init(&pWriteWait, NULL);
  pthread_cond_init(&pReadWait, NULL);
  pthread_mutex_init(&pAnalyzeMutex, NULL);
  pthread_mutex_init(&pCaptureMutex, NULL);
}
bool DeviceV2::Start() {
  Tools::LogMsg("INFO", "Starting CaptureV2 adquisition thread...\n");
  pChannelCount = 0;
  for(uint8_t Mask = 0x40; Mask != 0; Mask >>= 1) {
    if(pChannelMask & Mask) pChannelCount++;
  }
  pBuffLen = CAPTUREV2_BUFFERSIZE * pSamplerate * pChannelCount;
  pBuffer = (uint16_t*)malloc(pBuffLen * sizeof(uint16_t));
  if(pBuffer == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return false;
  }

  //  for(int i = 0; i < 5; i++) {
//    float Q = 1.0;
//    float K = tan(M_PI * ((float)Tools::Electrical.NominalFrequency / CAPTURE_V2_SAMPLEFREQ));
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
    return false;
  }
  if(pthread_create(&CaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    free(pBuffer);
    return false;
  }
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , CAPTUREV2_VID, CAPTUREV2_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
    pthread_join(CaptureThread, NULL);
    free(pBuffer);
    return false;
  }
  if(pthread_create(&AnalyzeThread, NULL, Analize_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_join(CaptureThread, NULL);
    free(pBuffer);
    return false;
  }
}
DeviceV2::~DeviceV2() {
  if(pBuffer) {
    pTerminate = true;
    pthread_cond_signal(&pWriteWait);
    pthread_cond_signal(&pReadWait);
    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
    pthread_join(CaptureThread, NULL);
    pthread_join(AnalyzeThread, NULL);
    free(pBuffer);
  }
}
uint32_t DeviceV2::SamplesCopy(float **dest, uint32_t offset, uint32_t len) {
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
