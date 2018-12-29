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
//======================================================================================================================
bool DeviceV1::pClassRegistered = DeviceInterface::RegisterType("V1", DeviceV1::CreateInstance);
// Funciones ===========================================================================================================
void DeviceV1::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV1 *This = (DeviceV1*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    int16_t *Pkg   = (int16_t*)&transfer->buffer[64 * Block];
    for(uint8_t Count = 0; Count < 16; Count++) {
      if(Pkg[Count * 2 + 0] == -32768) continue;
      float Voltage = (Pkg[Count * 2 + 0] - This->pVoltageOffset) * This->pVoltageGain;
      float Current = (Pkg[Count * 2 + 1] - This->pCurrentOffset) * This->pCurrentGain;
      if(This->BuffersAppend(Voltage, Current) == false) {
        //NO HAY SITIO!!! que hago?
        Tools::LogMsg("ERROR", "Capture buffer overflow! Posible lost of samples\n");
      }
    }
  }
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
        Tools::LogMsg("INFO", "DeviceV1 with serial '%s' disconnected\n", This->pSerial);
        This->pTerminateSignal = true;
        pthread_join(This->pSignalThread, NULL);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
bool DeviceV1::BuffersAppend(float voltageSample, float currentSample) {
  if(pBuffer.Full()) return false;
  Sample_t Sample = { voltageSample, currentSample };
  pBuffer.Push(Sample);
  uint64_t Sampletime = Tools::GetTime();
  if(pLastUpdate < (Sampletime - 500)) {
    char ThreadName[16];
    sprintf(ThreadName, "DeviceV1_%02d", pBuffer.Size() * 100 / pBuffer.Capacity());
    pthread_setname_np(pCaptureThread, ThreadName);
    pLastUpdate = Sampletime;
  }
  if(pAnalyzer->IsFinished() == false) return true;
  bool Started = false;
  while((pBuffer.Empty() == false) && (Started == false)) {
    vector<float> Voltage, Current;
    Sample = pBuffer.Pop();
    Voltage.push_back(Sample.Voltage);
    Current.push_back(Sample.Current);
    if(pAnalyzer->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
  }
  return true;
}
void *DeviceV1::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 5;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("WARN", "Cant raise thread priority -> %s\n", __func__, __LINE__, strerror(errno));
  }
  DeviceV1 *This = (DeviceV1*)args;
  uint8_t BufferIN[DEVICEV1_TRANSFERS][8192];
  struct libusb_transfer *TransferIN[DEVICEV1_TRANSFERS];
  for(int i = 0; i < DEVICEV1_TRANSFERS; i++) TransferIN[i] = libusb_alloc_transfer(0);
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
          Tools::LogMsg("INFO", "Kernel driver dettached for DeviceV1 device with serial %s\n", This->pSerial);
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
        Tools::LogMsg("INFO", "Claimed interface 1 on CaptureMono device with serial %s\n", This->pSerial);
      }
      for(int i = 0; i < DEVICEV1_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(TransferIN[i], This->pDeviceHandle, DEVICEV1_ENDPOINT_IN, BufferIN[i], sizeof(BufferIN[i]), ReadCallback, This, -1);
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
      This->pTerminateSignal = false;
      if(pthread_create(&This->pSignalThread, NULL, Signal_Thread, This) != 0) {
        Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      }
      This->pNewDevice = NULL;
    }
  }
  return NULL;
}
void *DeviceV1::Signal_Thread(void* args) {
  DeviceV1 *This = (DeviceV1*)args;
  bool Toogle = false;
  while(!This->pTerminateSignal) {
/*    AnalyzerInterface::Params_t Params;
    This->pAnalyzer->GetParams(&Params, NULL);
    uint16_t Current = 0;
    uint16_t Voltage = 1 << 6;
    uint8_t  Beep    = 0;
    return NULL;
    if(Params.RMS_I[0] > (( 1.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 0;
    if(Params.RMS_I[0] > (( 2.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 1;
    if(Params.RMS_I[0] > (( 3.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 2;
    if(Params.RMS_I[0] > (( 4.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 3;
    if(Params.RMS_I[0] > (( 5.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 4;
    if(Params.RMS_I[0] > (( 6.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 5;
    if(Params.RMS_I[0] > (( 7.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 6;
    if(Params.RMS_I[0] > (( 8.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 7;
    if(Params.RMS_I[0] > (( 9.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 8;
    if(Params.RMS_I[0] > ((10.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 9;
    if(Params.RMS_I[0] > ((11.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 10;
    if(Params.RMS_I[0] > ((12.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 11;
    if(Params.RMS_I[0] > ((13.0 / 13.0) * sqrt(This->pAnalyzer->GetMaxCurrent()))) Current |= 1 << 12;
    if(Params.RMS_I[0] > (0.85 * sqrt(This->pAnalyzer->GetMaxCurrent()))) Beep = 10;
    if(Params.RMS_V[0] < ((1.0 - (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 0;
    if(Params.RMS_V[0] < ((1.0 - (5.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 1;
    if(Params.RMS_V[0] < ((1.0 - (4.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 2;
    if(Params.RMS_V[0] < ((1.0 - (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 3;
    if(Params.RMS_V[0] < ((1.0 - (2.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 4;
    if(Params.RMS_V[0] < ((1.0 - (1.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 5;
    if(Params.RMS_V[0] > ((1.0 + (1.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 7;
    if(Params.RMS_V[0] > ((1.0 + (2.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 8;
    if(Params.RMS_V[0] > ((1.0 + (3.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 9;
    if(Params.RMS_V[0] > ((1.0 + (4.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 10;
    if(Params.RMS_V[0] > ((1.0 + (5.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 11;
    if(Params.RMS_V[0] > ((1.0 + (6.0 / 60.0)) * This->pAnalyzer->GetNominalVoltage())) Voltage |= 1 << 12;
    if(Params.Flag == true) {
      Beep = 50;
      if(Toogle == true) Voltage = 0x0000;
      Toogle = !Toogle;  
    }
    uint8_t Write[5] = {(uint8_t)(Current & 0xFF), (uint8_t)(Current >> 8), (uint8_t)(Voltage & 0xFF), (uint8_t)(Voltage >> 8), Beep};
    libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0xFF, 0x0000, 0x0000, Write, sizeof(Write), 0);
    uint8_t Read[2];
    libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0xFE, 0x0000, 0x0000, Read, sizeof(Read), 0);
    Tools::pBatteryLevel = Read[1];
    Tools::pBatteryCharging = (Read[0] & 0x02);
    Tools::pExternalPower = (Read[0] & 0x01);
    fflush(stdout);*/
    usleep(100000);
    //Envio transferencia de control
    //Leo datos, si hay algun boton pulsado llamo a los scripts!
  }
  return NULL;
}
AnalyzerInterface *DeviceV1::FindAnalyzer(const char *serial) {
  return (strcmp(serial, pAnalyzer->GetID()) == 0) ? pAnalyzer : NULL;
}
DeviceInterface *DeviceV1::CreateInstance(json_object *config) {
  DeviceV1 *This = new DeviceV1();
  Tools::LogMsg("INFO", "Starting DeviceV1 driver...\n");
  This->pBuffer.Reset(DEVICEV1_BUFFERSECONDS * DEVICEV1_SAMPLEFREQ);
//  This->pUsed                  = 0;
//  This->pBuffLen               = DEVICEV1_BUFFERSECONDS * DEVICEV1_SAMPLEFREQ;
//  This->pVoltageSamples        = (float*)malloc(sizeof(float) * This->pBuffLen);
//  This->pCurrentSamples        = (float*)malloc(sizeof(float) * This->pBuffLen);
//  This->pReadPos               = 0;
//  This->pWritePos              = 0;
  This->pLastUpdate            = 0;
  This->pDeviceHandle          = NULL;
  This->pNewDevice             = NULL;
  This->pSerial                = (char*)"000000000000000000000000";
  This->pVoltageGain           = 1;
  This->pCurrentGain           = 1;
  This->pVoltageOffset         = 0;
  This->pCurrentOffset         = 0;
  This->pAnalyzer              = NULL;
  float NominalVoltage         = 230.0;
  float NominalFrequency       = 50.0;
  char *Name                   = (char*)"Unnamed input";
//  if((This->pVoltageSamples == NULL) || (This->pCurrentSamples == NULL)) {
//    Tools::LogMsg("ERROR", "CaptureV2: Can not allocate memory for capture buffers\n");
//    return NULL;
//  }
  json_object *Find;
  if((json_object_object_get_ex(config, "Serial", &Find) == true) && (json_object_get_type(Find) == json_type_string)) This->pSerial = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "VoltageGain", &Find) == true) && (json_object_get_type(Find) == json_type_double)) This->pVoltageGain = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "VoltageOffset", &Find) == true) && (json_object_get_type(Find) == json_type_double)) This->pVoltageOffset = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "CurrentGain", &Find) == true) && (json_object_get_type(Find) == json_type_double)) This->pCurrentGain = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "CurrentOffset", &Find) == true) && (json_object_get_type(Find) == json_type_double)) This->pCurrentOffset = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalVoltage = json_object_get_double(Find);
  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalFrequency = json_object_get_double(Find);  
  if((json_object_object_get_ex(config, "DisplayName", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Name = (char*)json_object_get_string(Find);
  This->pAnalyzer = new AnalyzerSoft(DEVICEV1_SAMPLEFREQ, This->pSerial, NominalVoltage, NominalFrequency, Name, 50.0, 0.1, AnalyzerInterface::PHASE1);
  if((json_object_object_get_ex(config, "Remote", &Find) == true) && (json_object_get_type(Find) == json_type_object)) This->pAnalyzer->ConfigRemote(Find);
  if((json_object_object_get_ex(config, "Prices", &Find) == true) && (json_object_get_type(Find) == json_type_object)) This->pAnalyzer->ConfigPrices(Find);
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
  Error = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV1_VID, DEVICEV1_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, This, NULL);
  if(Error < 0) {
    Tools::LogMsg("ERROR", "Fallo al registrar callback de escucha libUSB...\n");
    This->pTerminate = true;
    pthread_join(This->pCaptureThread, NULL);
    return NULL;
  }
  return This;
}
DeviceV1::~DeviceV1() {
//  if(pVoltageSamples) free(pVoltageSamples);
//  if(pCurrentSamples) free(pCurrentSamples);
  if(pAnalyzer) delete pAnalyzer;
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
//int Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x01, 0x0000, 0x0000, NULL, 0, 0);
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