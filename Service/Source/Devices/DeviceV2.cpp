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
#include "Tools.h"
//======================================================================================================================
bool DeviceV2::pClassRegistered = DeviceInterface::RegisterType("V2", DeviceV2::CreateInstance);
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
      if(This->BuffersAppend() == false) {
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
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
bool DeviceV2::BuffersAppend() {
  if(pUsed == pBuffLen) return false;
  float *VoltagePos = &pVoltageSamples[pVoltageChannelCount * pWritePos];
  float *CurrentPos = &pCurrentSamples[pCurrentChannelCount * pWritePos];
  float *ReadPos    = pReadSamples;
  if(pChannelMask & 0x40) *VoltagePos++ = *ReadPos++ * pVoltageGain[0];
  if(pChannelMask & 0x20) *CurrentPos++ = *ReadPos++ * pCurrentGain[0];
  if(pChannelMask & 0x10) *VoltagePos++ = *ReadPos++ * pVoltageGain[1];
  if(pChannelMask & 0x08) *CurrentPos++ = *ReadPos++ * pCurrentGain[1];
  if(pChannelMask & 0x04) *VoltagePos++ = *ReadPos++ * pVoltageGain[2];
  if(pChannelMask & 0x02) *CurrentPos++ = *ReadPos++ * pCurrentGain[2];
  if(pChannelMask & 0x01) *CurrentPos++ = *ReadPos++ * pCurrentGain[3];
  pWritePos = (pWritePos + 1) % pBuffLen;
  pUsed++;
  uint64_t Sampletime = Tools::GetTime();
  if(pLastUpdate < (Sampletime - 500)) {
    char ThreadName[16];
    sprintf(ThreadName, "DeviceV2_%02d", pUsed * 100 / pBuffLen);
    pthread_setname_np(pCaptureThread, ThreadName);
    pLastUpdate = Sampletime;
  }
  for(uint8_t i = 0; i < pAnalyzersCount; i++) {
    if(pAnalyzers[i].Analyzer->IsFinished() == false) return true;
  }
  bool Started = false;
  for(uint8_t Sample = 0; (Sample < 5) && (pUsed > 0) && (Started == false); Sample++) {
    for(uint8_t i = 0; i < pAnalyzersCount; i++) {
      
//      if(pAnalyzers[0].Analyzer->BufferAppend(&pVoltageSamples[pReadPos * pVoltageChannelCount + pAnalyzers[0].OffsetV], &pCurrentSamples[pReadPos * pCurrentChannelCount + pAnalyzers[0].OffsetC], Sampletime) == true) Started = true;
    }
    pReadPos = (pReadPos + 1) % pBuffLen;
    pUsed--;
  }
  return true;
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
  uint8_t BufferIN[DEVICEV2_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[DEVICEV2_TRANSFERS];
  for(int i = 0; i < DEVICEV2_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
  while(!This->pTerminate) {
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
      for(int i = 0; i < DEVICEV2_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(TransferIN[i], This->pDeviceHandle, DEVICEV2_ENDPOINT_IN, BufferIN[i], sizeof(BufferIN[i]), ReadCallback, This, -1);
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
      This->pUSBState = USB_RESET;
      int Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x01, 0x0000, 0x0000, NULL, 0, 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not stop device with serial %s (%d)\n", This->pSerial, Error);
      usleep(1000000);
      This->pUSBState = USB_WAIT;
      float Conf[7] = {This->pVoltageOffset[0], This->pCurrentOffset[0], This->pVoltageOffset[1], This->pCurrentOffset[1], This->pVoltageOffset[2], This->pCurrentOffset[2], This->pCurrentOffset[3]};
      Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x00, (This->pChannelMask << 8) | 0x0E, 0x0000, (uint8_t*)Conf, sizeof(Conf), 0);
      if(Error < 0) Tools::LogMsg("ERROR", "Can not setup device with serial %s (%d)\n", This->pSerial, Error);
    }
  }
  return NULL;
}
AnalyzerInterface *DeviceV2::FindAnalyzer(const char *serial) {
  for(uint8_t i = 0; i < pAnalyzersCount; i++) {
    if(strcmp(serial, pAnalyzers[i].Analyzer->GetID()) == 0) return pAnalyzers[i].Analyzer;
  }
  return NULL;
}
DeviceInterface *DeviceV2::CreateInstance(json_object* config) {
  DeviceV2 *This = new DeviceV2();
  Tools::LogMsg("INFO", "Starting DeviceV2 driver...\n");
  for(int i = 0; i < 3; i++) This->pVoltageGain[i]   = 1;
  for(int i = 0; i < 3; i++) This->pVoltageOffset[i] = 0;
  for(int i = 0; i < 4; i++) This->pCurrentGain[i]   = 1;
  for(int i = 0; i < 4; i++) This->pCurrentOffset[i] = 0;
  This->pUsed                  = 0;
  This->pVoltageChannelCount   = 0;
  This->pCurrentChannelCount   = 0;
  This->pChannelMask           = 0;
  This->pChannelCount          = 0;
  This->pReadPos               = 0;
  This->pWritePos              = 0;
  This->pLastUpdate            = 0;
  This->pDeviceHandle          = NULL;
  This->pNewDevice             = NULL;
  This->pOversample            = 0;
  This->pADCSamplerate         = 20000;
  This->pUSBState              = USB_WAIT;
  This->pSerial                = (char*)"000000000000000000000000";
  This->pAnalyzersCount        = 0;
  float NominalVoltage   = 230.0;
  float NominalFrequency = 50.0;
  struct json_object *Find, *Find2;
  Tools::JSONUInt32(config, "Samplerate", &This->pADCSamplerate);

  if((json_object_object_get_ex(config, "Oversample", &Find) == true) && (json_object_get_type(Find) == json_type_int)) This->pOversample = json_object_get_int(Find);
  Tools::JSONString(config, (char*)"Serial", &This->pSerial, false);
  if((json_object_object_get_ex(config, "Serial", &Find) == true) && (json_object_get_type(Find) == json_type_string)) This->pSerial = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalVoltage = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalFrequency = json_object_get_double(Find);
  if(Tools::JSONArrayFloat(config, (char*)"VoltageGain", This->pVoltageGain, 3) == false) {
    Tools::LogMsg("ERROR", "CaptureV2: Voltage gain must have 3 double values\n");
    return NULL;
  }
  if(Tools::JSONArrayFloat(config, "VoltageOffset", This->pVoltageOffset, 3) == false) {
    Tools::LogMsg("ERROR", "DeviceV2: Voltage offset must have 3 double values\n");
    return NULL;
  }
  if(Tools::JSONArrayFloat(config, "CurrentGain", This->pCurrentGain, 4) == false) {
    Tools::LogMsg("ERROR", "DeviceV2: current gain must have 4 double values\n");
    return NULL;
  }
  if(Tools::JSONArrayFloat(config, "CurrentOffset", This->pCurrentOffset, 4) == false) {
    Tools::LogMsg("ERROR", "DeviceV2: current offset must have 4 double values\n");
    return NULL;
  }
  This->pBuffLen = DEVICEV2_BUFFERSECONDS * This->pADCSamplerate / This->pOversample;
  This->pVoltageSamples = (float*)malloc(sizeof(float) * This->pBuffLen);
  This->pCurrentSamples = (float*)malloc(sizeof(float) * This->pBuffLen);
  if((This->pVoltageSamples == NULL) || (This->pCurrentSamples == NULL)) {
    Tools::LogMsg("ERROR", "CaptureV2: Can not allocate memory for capture buffers\n");
    return NULL;
  }  
  if((json_object_object_get_ex(config, "Outputs", &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    This->pAnalyzers = (Analyzer_t*)realloc(This->pAnalyzers, sizeof(Analyzer_t*) * json_object_array_length(Find));
    for(int i = 0; i < json_object_array_length(Find); i++) {
      json_object *Output = json_object_array_get_idx(Find, i);
      char *ReadMode = (char*)"3Phase";
      char *Name     = (char*)"Unnamed input";
      if((json_object_object_get_ex(Output, "DisplayName", &Find2) == true) && (json_object_get_type(Find2) == json_type_string)) Name = (char*)json_object_get_string(Find2);
      if((json_object_object_get_ex(Output, "Mode", &Find2) == true) && (json_object_get_type(Find2) == json_type_string)) ReadMode = (char*)json_object_get_string(Find2);
      if(strcmp("3Phase", ReadMode) == 0) {
        bool UseNeutral = true;
        if((json_object_object_get_ex(Output, "NeutralCurrent", &Find2) == true) && (json_object_get_type(Find2) == json_type_boolean)) UseNeutral = json_object_get_boolean(Find2);
        if(((UseNeutral == true) && (This->pChannelMask != 0)) || ((UseNeutral == false) && ((This->pChannelMask & 0x2A) != 0))) {
          Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
          return NULL;  
        }
        This->pChannelMask = This->pChannelMask | ((UseNeutral == true) ? 0x7F : 0x7E);
        This->pAnalyzers[This->pAnalyzersCount].Analyzer = new AnalyzerSoft(This->pADCSamplerate / This->pOversample, strdup(This->pSerial), NominalVoltage, NominalFrequency, Name, 50.0, 0.1, UseNeutral ? AnalyzerInterface::PHASE3N : AnalyzerInterface::PHASE3);
        This->pAnalyzers[This->pAnalyzersCount].OffsetV = 0;
        This->pAnalyzers[This->pAnalyzersCount].OffsetC = 0;
        This->pAnalyzersCount++;
      } else if(strcmp("1Phase", ReadMode) == 0) {
        char    *CurrentChannelText = (char*)"R";
        uint8_t  VoltageMask = 0;
        uint8_t  CurrentMask = 0;
        if((json_object_object_get_ex(Output, "VoltageChannel", &Find2) == true) && (json_object_get_type(Find2) == json_type_string)) {
          char *VoltageChannelText = (char*)json_object_get_string(Find2);
          if(strcmp("R", VoltageChannelText) == 0) {
            VoltageMask = 0x40;
            This->pAnalyzers[This->pAnalyzersCount].OffsetV = 0;
          } else if(strcmp("S", VoltageChannelText) == 0) {
            VoltageMask = 0x10;
            This->pAnalyzers[This->pAnalyzersCount].OffsetV = 1;
          } else if(strcmp("T", VoltageChannelText) == 0) {
            VoltageMask = 0x04;
            This->pAnalyzers[This->pAnalyzersCount].OffsetV = 2;
          } else {
            Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S' or 'T'\n");
            return NULL;
          }
        }
        if((json_object_object_get_ex(Output, "CurrentChannel", &Find2) == true) && (json_object_get_type(Find2) == json_type_string)) {
          CurrentChannelText = (char*)json_object_get_string(Find2);
          if(strcmp("R", CurrentChannelText) == 0) {
            CurrentMask = 0x20;
            This->pAnalyzers[This->pAnalyzersCount].OffsetC = 0;
          } else if(strcmp("S", CurrentChannelText) == 0) {
            CurrentMask = 0x08;
            This->pAnalyzers[This->pAnalyzersCount].OffsetC = 1;
          } else if(strcmp("T", CurrentChannelText) == 0) {
            CurrentMask = 0x02;
            This->pAnalyzers[This->pAnalyzersCount].OffsetC = 2;
          } else if(strcmp("N", CurrentChannelText) == 0) {
            CurrentMask = 0x01;
            This->pAnalyzers[This->pAnalyzersCount].OffsetC = 3;
          } else {
            Tools::LogMsg("ERROR", "CaptureV2: Invalid channel, try 'R', 'S', 'T' or 'N'\n");
            return NULL;
          }
        }
        if((This->pChannelMask & CurrentMask) != 0) {
          Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, same current channel can not be used twice\n");
          return NULL;
        }
        This->pChannelMask = This->pChannelMask | CurrentMask | VoltageMask;
        char *ID = (char*)malloc(strlen(This->pSerial) + 2);
        sprintf(ID, "%s%s", This->pSerial, CurrentChannelText);
        This->pAnalyzers[This->pAnalyzersCount].Analyzer = new AnalyzerSoft(This->pADCSamplerate / This->pOversample, ID, NominalVoltage, NominalFrequency, Name, 50.0, 0.1, AnalyzerInterface::PHASE1);
        This->pAnalyzersCount++;
      } else {
        Tools::LogMsg("ERROR", "CaptureV2: Invalid output config, Mode must be one '1Phase' or '3Phase'\n");
        return NULL;
      }
    }
  }
  //count voltage, current and totals channels
  if(This->pChannelMask & 0x40) This->pVoltageChannelCount++;
  if(This->pChannelMask & 0x20) This->pCurrentChannelCount++;
  if(This->pChannelMask & 0x10) This->pVoltageChannelCount++;
  if(This->pChannelMask & 0x08) This->pCurrentChannelCount++;
  if(This->pChannelMask & 0x04) This->pVoltageChannelCount++;
  if(This->pChannelMask & 0x02) This->pCurrentChannelCount++;
  if(This->pChannelMask & 0x01) This->pCurrentChannelCount++;
  This->pChannelCount = This->pVoltageChannelCount + This->pCurrentChannelCount;
  //Adjust sample offset on 1phase devices
  for(uint8_t i = 0; i < This->pAnalyzersCount; i++) {
    if((This->pAnalyzers[i].OffsetV > 1) && ((This->pChannelMask && 0x10) == 0)) This->pAnalyzers[i].OffsetV--;
    if((This->pAnalyzers[i].OffsetV > 0) && ((This->pChannelMask && 0x40) == 0)) This->pAnalyzers[i].OffsetV--;
    if((This->pAnalyzers[i].OffsetC > 2) && ((This->pChannelMask && 0x02) == 0)) This->pAnalyzers[i].OffsetC--;
    if((This->pAnalyzers[i].OffsetC > 1) && ((This->pChannelMask && 0x08) == 0)) This->pAnalyzers[i].OffsetC--;
    if((This->pAnalyzers[i].OffsetC > 0) && ((This->pChannelMask && 0x20) == 0)) This->pAnalyzers[i].OffsetC--;
  }
  int Error = libusb_init(NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Can not open libUSB...\n");
    return NULL;
  }
  if(pthread_create(&This->pCaptureThread, NULL, Capture_Thread, This) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(NULL);
    return NULL;
  }
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV2_VID, DEVICEV2_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, This, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Failed to register libUSB callback\n");
    This->pTerminate = true;
    pthread_join(This->pCaptureThread, NULL);
    return NULL;
  }
  return This;
}
DeviceV2::~DeviceV2() {
  pTerminate = true;
  Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
  pthread_join(pCaptureThread, NULL);
  if(pVoltageSamples) free(pVoltageSamples);
  if(pCurrentSamples) free(pCurrentSamples);
  for(int i = 0; i < pAnalyzersCount; i++) {
    free(pAnalyzers[i].Analyzer->GetID());
    delete pAnalyzers[i].Analyzer;
  }
  free(pAnalyzers);
}
//======================================================================================================================