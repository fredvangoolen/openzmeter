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
  DeviceV2 *This = (DeviceV2*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    int16_t *Pkg   = (int16_t*)&transfer->buffer[64 * Block];
    uint8_t  Count = 0;
    //Si no estoy esperando muestras omito bloque
    if(This->pUSBState == USB_RESET) continue;
    //Si es un bloque de inicio de secuencia reinicio los contadores
    if(Pkg[0] == -32768) {
      if(This->pUSBState == USB_SYNC) Tools::LogMsg("WARN", "DeviceV2 lost sync. Try reduce capture speed\n");
      This->pUSBState = USB_SYNC;
      This->BuffersReset();
      This->pChannel = 0;
      Count++;
    }
    //Si estoy esperando sincronizacion y no la tengo continuo
    if(This->pUSBState == USB_WAIT) continue;
    //Añado muestras del bloque al buffer
    for(; Count < 32; Count++) {
      if(Pkg[Count] == -32768) continue;
      This->pReadSamples[This->pChannel++] = Pkg[Count];
      if(This->pChannel != This->pChannelCount) continue;
      float VoltageChannels[3];
      float CurrentChannels[4];
      float *VoltagePos = VoltageChannels;
      float *CurrentPos = CurrentChannels;
      float *ReadPos    = This->pReadSamples;
      if(This->pChannelMask & 0x40) *VoltagePos++ = *ReadPos++ * This->pVoltageGain[0];
      if(This->pChannelMask & 0x20) *CurrentPos++ = *ReadPos++ * This->pCurrentGain[0];
      if(This->pChannelMask & 0x10) *VoltagePos++ = *ReadPos++ * This->pVoltageGain[1];
      if(This->pChannelMask & 0x08) *CurrentPos++ = *ReadPos++ * This->pCurrentGain[1];
      if(This->pChannelMask & 0x04) *VoltagePos++ = *ReadPos++ * This->pVoltageGain[2];
      if(This->pChannelMask & 0x02) *CurrentPos++ = *ReadPos++ * This->pCurrentGain[2];
      if(This->pChannelMask & 0x01) *CurrentPos++ = *ReadPos++ * This->pCurrentGain[3];
      if(This->BuffersAppend(VoltageChannels, CurrentChannels) == false) {
        //NO HAY SITIO!!! que hago?
        Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
      }
      This->pChannel = 0;
    }
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
        This->pActive = false;
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
  uint8_t BufferIN[CAPTUREV2_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[CAPTUREV2_TRANSFERS];
  for(int i = 0; i < CAPTUREV2_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
//  uint32_t Update = 0;
  while(!This->pTerminate) {
//    if((Update % 10) == 0) {
//      char ThreadName[16] = "DeviceV2C_Wait";
//      if(This->pDeviceHandle != NULL) sprintf(ThreadName, "DeviceV2C_%03d", This->pUsed * 100 / This->pBuffLen); 
//      pthread_setname_np(This->pCaptureThread, ThreadName);
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
      This->pUSBState = USB_WAIT;
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
      This->pActive = true;
      This->pNewDevice = NULL;
      int Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x01, 0x0000, 0x0000, NULL, 0, 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not stop device with serial %s (%d)\n", This->pSerial, Error);
      This->pUSBState = USB_RESET;
      usleep(1000000);
      This->pUSBState = USB_WAIT;
      float Conf[7] = {This->pVoltageOffset[0], This->pCurrentOffset[0], This->pVoltageOffset[1], This->pCurrentOffset[1], This->pVoltageOffset[2], This->pCurrentOffset[2], This->pCurrentOffset[3]};
      Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x00, (This->pChannelMask << 8) | 0x0E, 0x0000, (uint8_t*)Conf, sizeof(Conf), 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not setup device with serial %s (%d)\n", This->pSerial, Error);
    }
  }
  return NULL;
}
//inline int16_t DeviceV2::GetSample() {
//  int16_t Result = pBuffer[pReadPos];
//  pReadPos = (pReadPos + 1) % pBuffLen;
//  pUsed    = (pUsed - 1);
//  return Result;
//}
//void DeviceV2::AppendSamples() {
//  DeviceV2 *This = (DeviceV2*)args;
//  if(pthread_setschedprio(0, -10) == -1) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return NULL;
//  }
//  pthread_setname_np(This->AnalyzeThread, "DeviceV2A");
//  float VoltageChannels[3];
//  float CurrentChannels[4];
//  while(!This->pTerminate) {
//    This->BuffersClear();
    //Espero a tener muestras disponibles (ATOMIC)
//    pthread_mutex_lock(&This->pBufferMutex);
//    while(This->pUsed < (This->pMaxSamples * This->pChannelCount)) {
//      if(This->pTerminate) return NULL;
//      pthread_cond_wait(&This->pReadWait, &This->pBufferMutex);
//    }

      
      
      
      
      ////      for(uint8_t Channel = 0; Channel < This->pChannelCount; Channel++) {
//        //for(uint8_t Analyzer = 0; Analyzer < This->pAnalyzersCount; Analyzer++) {
////          if(Analyzer != 0) continue;
//        
//          
//          //      This->pWork.VoltageSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Voltage;
////      This->pWork.CurrentSamples[0][This->pWork.Samples] = This->Buffer[This->ReadPos].Current;
////      float Out = This->pWork.VoltageSamples[0][This->pWork.Samples];
////      for(int i = 0; i < 5; i++) Out = ApplyFilter(Out, &This->Filters[i]);
////      if(((LastSample >= 0.0) && (Out < 0.0)) || ((LastSample <= 0.0) && (Out > 0.0))) {
////        This->pWork.Cycles[This->pWork.Cycles_Len].End = This->pWork.Samples;
////        This->pWork.Cycles_Len++;
////        if(This->pWork.Cycles_Len == 20) break;
////        This->pWork.Cycles[This->pWork.Cycles_Len].Start = This->pWork.Samples;
////      }
////      LastSample = Out;
//        //}
//      
      
      
    

//    float LastSample = +20; //Siempre comienzo con cierto offset para que el primer flanco sea el positivo
////  for(uint i = 0; i < Len; i++) {
////    int16_t Voltage = Buff16[i * 2 + 0];
////    uint16_t Current = Buff16[i * 2 + 1];
////    This->Buffer[This->WritePos].Voltage = (Voltage - This->VoltageOffset) * This->VoltageGain;
////    This->Buffer[This->WritePos].Current = (Current - This->CurrentOffset) * This->CurrentGain;
////    This->WritePos = (This->WritePos + 1) % CAPTUREMONO_BUFFERSIZE;
////  }
      
////      }
//      Samples++;
//    }
//    float AVG[7] = {0,0,0,0,0,0,0};
//    for(int i = 0; i < Samples; i++) {
//        AVG[0] += This->pAnalyzers[0]->pWork.VoltageSamples[0][0];
//        AVG[1] += This->pAnalyzers[0]->pWork.CurrentSamples[0][0];
//        AVG[2] += This->pAnalyzers[0]->pWork.VoltageSamples[1][0];
//        AVG[3] += This->pAnalyzers[0]->pWork.CurrentSamples[1][0];
//        AVG[4] += This->pAnalyzers[0]->pWork.VoltageSamples[2][0];
//        AVG[5] += This->pAnalyzers[0]->pWork.CurrentSamples[2][0];
//        AVG[6] += This->pAnalyzers[0]->pWork.CurrentSamples[3][0];
//    }
//    printf("(V_R: %.5f) (I_R: %.5f) (V_S: %.5f) (I_S: %.5f) (V_T: %.5f) (I_T: %.5f) (I_N: %.5f)\n", 
//            AVG[0] / Samples, AVG[1] / Samples,
//            AVG[2] / Samples, AVG[3] / Samples,
//            AVG[4] / Samples, AVG[5] / Samples,
//            AVG[6] / Samples);
//      fflush(stdout);
//    
//    
//    
//    This->pAnalyzers[0]->pWork.Samples = Samples;
//    pthread_cond_signal(&This->pWriteWait);

//    This->CalcFFTs();
//    for(uint8_t i = 0; i < This->pAnalyzersCount; i++) {
//    
//      This->pAnalyzers[i]->Analyze(This->pVoltageSamples, This->pVoltageFFT, This->pCurrentSamples, This->pCurrentFFT, &This->pSamples, 1, Timestamp);
//    }
    ////    if(Discard > 0) {
////      Discard--;
////      This->ResetWork();
////      This->ResetParams(&This->pWork.Params);
////    } else {
////      This->Analyze(0.1);
//    This->pAnalyzers[0]->Analyze(0.1);
    //  uint64_t AnalyzeEnd = Tools::GetTime();
//      This->pAnalyzers[0]->SaveWorkParams();
  //    uint64_t SaveEnd = Tools::GetTime();
//      Tools::LogMsg("DEBUG", "DeviceV2 takes %umS to analyze block\n", AnalyzeEnd - Start, SaveEnd - Start);
////    }
//  }
//  return NULL;
//}
DeviceV2::DeviceV2(json_object* config) : DeviceInterface((char*)"DeviceV2") {
  pSerial           = NULL;
  for(int i = 0; i < 3; i++) pVoltageGain[i]   = 1;
  for(int i = 0; i < 3; i++) pVoltageOffset[i] = 0;
  for(int i = 0; i < 4; i++) pCurrentGain[i]   = 1;
  for(int i = 0; i < 4; i++) pCurrentOffset[i] = 0;
  pChannelMask      = 0;
  pChannelCount     = 0;
  pDeviceHandle     = NULL;
  pNewDevice        = NULL;
  pOversample       = 0;
  pADCSamplerate    = 20000;
//  pMappings         = NULL;
  pUSBState         = USB_WAIT;
  struct json_object *Find;
  if((json_object_object_get_ex(config, "Samplerate", &Find) == true) && (json_object_get_type(Find) == json_type_int)) {
    pADCSamplerate = json_object_get_int(Find);
  }
  if((json_object_object_get_ex(config, "Oversample", &Find) == true) && (json_object_get_type(Find) == json_type_int)) {
    pOversample = json_object_get_int(Find);
  }
  float Samplerate = pADCSamplerate / pOversample;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Serial") == 0)) {
      pSerial = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "VoltageGain") == 0)) {
      if(json_object_array_length(JSON_Val) != 3) {
        Tools::LogMsg("ERROR", "CaptureV2: Voltage gain must have 3 double values\n");
        return;
      }
      for(int i = 0; i < 3; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
            Tools::LogMsg("ERROR", "CaptureV2: Voltage gain must have 3 double values\n");
            return;
        }
        pVoltageGain[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "VoltageOffset") == 0)) {
      if(json_object_array_length(JSON_Val) != 3) {
        Tools::LogMsg("ERROR", "CaptureV2: Voltage offset must have 3 double values\n");
        return;
      }
      for(int i = 0; i < 3; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
          Tools::LogMsg("ERROR", "CaptureV2: Voltage offset must have 3 double values\n");
          return;
        }
        pVoltageOffset[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "CurrentGain") == 0)) {
      if(json_object_array_length(JSON_Val) != 4) {
        Tools::LogMsg("ERROR", "CaptureV2: current gain must have 4 double values\n");
        return;
      }
      for(int i = 0; i < 4; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
          Tools::LogMsg("ERROR", "CaptureV2: current gain must have 4 double values\n");
          return;
        }
        pCurrentGain[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
      }
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "CurrentOffset") == 0)) {
      if(json_object_array_length(JSON_Val) != 4) {
        Tools::LogMsg("ERROR", "CaptureV2: current offset must have 4 double values\n");
        return;
      }
      for(int i = 0; i < 4; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_double) {
          Tools::LogMsg("ERROR", "CaptureV2: current offset must have 4 double values\n");
          return;
        }
        pCurrentOffset[i] = json_object_get_double(json_object_array_get_idx(JSON_Val, i));
      }
    } else if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "3Phase+N") == 0)) {
      uint32_t  ID = 0;
      char     *Name= (char*)"3Phase+N";
      json_object_object_foreach(JSON_Val, JSON_KeySub, JSON_ValSub) {
        enum json_type JSON_TypeSub = json_object_get_type(JSON_ValSub);
        if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "DisplayName") == 0)) {
          Name = (char*)json_object_get_string(JSON_ValSub);
        } else if((JSON_TypeSub == json_type_int) && (strcmp(JSON_KeySub, "ID") == 0)) {
          ID = json_object_get_int(JSON_ValSub);
        }
      }
      if(pChannelMask != 0) {
        Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
        return;
      }
      pChannelMask |= 0x7F;
//      pMappings = (Mapping_t*)realloc(pMappings, sizeof(Mapping_t) * (pAnalyzersCount + 1));
//      pMappings[pAnalyzersCount].VoltageChannel = 0;
//      pMappings[pAnalyzersCount].CurrentChannel = 0;
      AddAnalyzer(new Analyzer(Name, true, ID, Samplerate, 0.1));
    } else if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "3Phase") == 0)) {
      uint32_t  ID = 0;
      char     *Name= (char*)"3Phase";
      json_object_object_foreach(JSON_Val, JSON_KeySub, JSON_ValSub) {
        enum json_type JSON_TypeSub = json_object_get_type(JSON_ValSub);
        if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "DisplayName") == 0)) {
          Name = (char*)json_object_get_string(JSON_ValSub);
        } else if((JSON_TypeSub == json_type_int) && (strcmp(JSON_KeySub, "ID") == 0)) {
          ID = json_object_get_int(JSON_ValSub);   
        }
      }
      if((pChannelMask & 0x2A) != 0) {
        Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
        return;
      }
      pChannelMask |= 0x7E; 
//      pMappings = (Mapping_t*)realloc(pMappings, sizeof(Mapping_t) * (pAnalyzersCount + 1));
//      pMappings[pAnalyzersCount].VoltageChannel = 0;
//      pMappings[pAnalyzersCount].CurrentChannel = 0;
      AddAnalyzer(new Analyzer(Name, false, ID, Samplerate, 0.1));
    } else if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "1Phase") == 0)) {
      uint32_t  ID = 0;
      char     *Name= (char*)"1Phase";
      uint8_t   VoltageChannel = 6;
      uint8_t   CurrentChannel = 5;
      json_object_object_foreach(JSON_Val, JSON_KeySub, JSON_ValSub) {
        enum json_type JSON_TypeSub = json_object_get_type(JSON_ValSub);
        if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "DisplayName") == 0)) {
          Name = (char*)json_object_get_string(JSON_ValSub);
        } else if((JSON_TypeSub == json_type_int) && (strcmp(JSON_KeySub, "ID") == 0)) {
          ID = json_object_get_int(JSON_ValSub);   
        } else if((JSON_TypeSub == json_type_string) && (strcmp(JSON_KeySub, "VoltageChannel") == 0)) {
          char *ChannelText = (char*)json_object_get_string(JSON_ValSub);
          if(strcmp("R", ChannelText) == 0) {
            VoltageChannel = 6;
          } else if(strcmp("S", ChannelText) == 0) {
            VoltageChannel = 4;
          } else if(strcmp("T", ChannelText) == 0) {
            VoltageChannel = 2;
          } else {
            Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S' or 'T'\n");
            return;
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
            Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S', 'T' or 'N'\n");
            return;
          }
          if((pChannelMask & (1 << CurrentChannel)) != 0) {
            Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
            return;
          }
        }
      }
      pChannelMask |= (1 << CurrentChannel) | (1 << VoltageChannel);
//      pMappings = (Mapping_t*)realloc(pMappings, sizeof(Mapping_t) * (pAnalyzersCount + 1));
//      pMappings[pAnalyzersCount].VoltageChannel = VoltageChannel;
//      pMappings[pAnalyzersCount].CurrentChannel = CurrentChannel;
      AddAnalyzer(new Analyzer(Name, VoltageChannel, CurrentChannel, ID, Samplerate, 0.1));
    }
  }
  uint8_t VoltageChannels = 0;
  uint8_t CurrentChannels = 0;
  if(pChannelMask & 0x40) VoltageChannels++;
  if(pChannelMask & 0x20) CurrentChannels++;
  if(pChannelMask & 0x10) VoltageChannels++;
  if(pChannelMask & 0x08) CurrentChannels++;
  if(pChannelMask & 0x04) VoltageChannels++;
  if(pChannelMask & 0x02) CurrentChannels++;
  if(pChannelMask & 0x01) CurrentChannels++;
  if(BuffersAlloc(VoltageChannels, CurrentChannels, Samplerate) == false) {
    Tools::LogMsg("ERROR", "CaptureV2: Can not allocate memory for capture buffers\n");
    return;
  }
  Tools::LogMsg("INFO", "Starting CaptureV2 adquisition thread...\n");
  pChannelCount = 0;
  for(uint8_t Mask = 0x40; Mask != 0; Mask >>= 1) {
    if(pChannelMask & Mask) pChannelCount++;
  }
  int Error = libusb_init(NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Can not open libUSB...\n");
    return;
  }
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return;
  }
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , CAPTUREV2_VID, CAPTUREV2_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
    pthread_join(pCaptureThread, NULL);
    return;
  }
}
DeviceV2::~DeviceV2() {
//  if(pBuffer) {
//    pTerminate = true;
//    pthread_cond_signal(&pWriteWait);
//    pthread_cond_signal(&pReadWait);
//    Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
//    pthread_join(pCaptureThread, NULL);
//    pthread_join(AnalyzeThread, NULL);
//    free(pBuffer);
//  }
}
//======================================================================================================================
