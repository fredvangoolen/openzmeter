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
#include "DeviceV1_01.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1_01::pRegistered = DeviceV1_01::GetInstances();
// Funciones ===========================================================================================================
bool DeviceV1_01::GetInstances() {
  libusb_context *Context;
  if(libusb_init(&Context) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return false;
  }
  libusb_device **Devs;
  int Count = libusb_get_device_list(NULL, &Devs);
  if(Count < 0) {
    LOGFile::Error("Can not enumerate USB devices...\n");
    return false;
  }
  for(int i = 0; i < Count; ++i) {
    libusb_device_descriptor Descriptor;
    if(libusb_get_device_descriptor(Devs[i], &Descriptor) < 0) {
      LOGFile::Error("Can not load USB descriptor...\n");
      return false;
    }
    if(Descriptor.idVendor != DEVICEV1_01_VID) continue;
    if(Descriptor.idProduct != DEVICEV1_01_PID) continue;
    if(Descriptor.bcdDevice != DEVICEV1_01_VERSION) continue;
    libusb_device_handle *Dev_Handle = NULL;
    if(libusb_open(Devs[i], &Dev_Handle) != LIBUSB_SUCCESS) {
      LOGFile::Error("Can not open USB device...\n");
    }
    char String[256];
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iManufacturer, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV1_01_MANSTRING) != 0) continue;
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV1_01_PRODSTRING) != 0) continue;    
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iSerialNumber, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    libusb_close(Dev_Handle);
    LOGFile::Info("Adding 'V1.01' device driver instance with serial '%s'\n", String);
    DeviceManager::AddDevice(new DeviceV1_01(String));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
  return true;
}
DeviceV1_01::DeviceV1_01(string serial) {
  pDeviceHandle  = NULL;
  pNewDevice     = NULL;
  pSerial        = serial;
  pDriver        = "V1.01";
  pName          = "Capture device V1.01";
  pVoltageGain   = 0.014704938926227089;
  pCurrentGain   = 0.0015625;
  pVoltageOffset = 0;
  pCurrentOffset = 0;
  pAnalyzer      = NULL;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerInterface *DeviceV1_01::FindAnalyzer(string serial) {
  if(pAnalyzer == NULL) return NULL;
  return (serial == pAnalyzer->Serial()) ? pAnalyzer : NULL;
}
uint32_t DeviceV1_01::AnalyzersCount() {
  return (pAnalyzer == NULL) ? 0 : 1;
}
bool DeviceV1_01::Start() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'V1.01' driver configuration for serial '%s'\n", pSerial.c_str());
  bool AutoStart = false;
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  if(Config.is_object()) {
    if(Config["VoltageGain"].is_number()) pVoltageGain = Config["VoltageGain"];
    if(Config["VoltageOffset"].is_number()) pVoltageOffset = Config["VoltageOffset"];
    if(Config["CurrentGain"].is_number()) pCurrentGain = Config["CurrentGain"];
    if(Config["CurrentOffset"].is_number()) pCurrentOffset = Config["CurrentOffset"];
    if(Config["AutoStart"].is_boolean()) AutoStart = Config["AutoStart"];
  }
  Config = json::object();
  Config["VoltageGain"] = pVoltageGain;
  Config["VoltageOffset"] = pVoltageOffset;
  Config["CurrentGain"] = pCurrentGain;
  Config["CurrentOffset"] = pCurrentOffset;
  Config["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      return true;
    }
  }
  LOGFile::Info("Starting 'V1.01' driver for serial '%s'\n", pSerial.c_str());
  pTerminate    = false;
  pDeviceHandle = NULL;
  pNewDevice    = NULL;
  pAnalyzer     = new AnalyzerSoft(pSerial, AnalyzerInterface::PHASE1, DEVICEV1_01_SAMPLEFREQ);
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV1_01_VID, DEVICEV1_01_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
    LOGFile::Error("Failed to register libUSB calback...\n");
    pTerminate = true;
    pthread_join(pCaptureThread, NULL);
    libusb_exit(pContext);
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pState = RUNNING;
  pthread_mutex_unlock(&pMutex);
  return true;
}
bool DeviceV1_01::Stop() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'V1.01' driver for serial '%s'\n", pSerial.c_str());
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  if(pAnalyzer != NULL) delete pAnalyzer;
  pAnalyzer = NULL;
  pState = STOPPED;
  pthread_mutex_unlock(&pMutex);
  return true;
}
DeviceV1_01::~DeviceV1_01() {
  Stop();
}
void DeviceV1_01::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV1_01 *This = (DeviceV1_01*)transfer->user_data;
  for(int Block = 0; Block < (transfer->actual_length / 64); Block++) {
    int16_t *Pkg   = (int16_t*)&transfer->buffer[64 * Block];
    for(uint8_t Count = 0; Count < 16; Count++) {
      if(Pkg[Count * 2 + 0] == -32768) continue;
      float Voltage = (Pkg[Count * 2 + 0] - This->pVoltageOffset) * This->pVoltageGain;
      float Current = (Pkg[Count * 2 + 1] - This->pCurrentOffset) * This->pCurrentGain;
      if(This->BuffersAppend(Voltage, Current) == false) {
        LOGFile::Error("Capture buffer overflow! Posible lost of samples\n");
        break;
      }
    }
  }
  if(This->pTerminate == false) 
    libusb_submit_transfer(transfer);
  else
    libusb_free_transfer(transfer);
}
int DeviceV1_01::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV1_01 *This = (DeviceV1_01*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);	
      if(dev == CurrentDev) {
        LOGFile::Info("DeviceV1.01 with serial '%s' disconnected\n", This->pSerial.c_str());
//        This->pTerminateSignal = true;
        pthread_join(This->pSignalThread, NULL);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
bool DeviceV1_01::BuffersAppend(float voltageSample, float currentSample) {
//  static int File = 0;
//  if(File == 0) File = open("Samples.raw", O_CREAT | O_WRONLY);
//  char Tmp[100];
//  sprintf(Tmp, "%f, %f\n", voltageSample, currentSample);
//  write(File, Tmp, strlen(Tmp));
  
//  if(pBuffer.full()) return false;
  Sample_t Sample = { voltageSample, currentSample };
  pBuffer.push_back(Sample);
  uint64_t Sampletime = Tools::GetTime();
//  if(pLastUpdate < (Sampletime - 500)) {
//    char ThreadName[16];
//    sprintf(ThreadName, "DeviceV1_00_%d", pBuffer.size());
//    pthread_setname_np(pCaptureThread, ThreadName);
//    pLastUpdate = Sampletime;
//  }
  if(pAnalyzer->IsFinished() == false) return true;
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample = pBuffer.front();
    pBuffer.pop_front();
    vector<float> Voltage = {Sample.Voltage};
    vector<float> Current = {Sample.Current};
    if(pAnalyzer->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
  }
  return true;
}
void *DeviceV1_01::Capture_Thread(void *args) {
  DeviceV1_01 *This = (DeviceV1_01*)args;
  for(int i = 0; i < DEVICEV1_01_TRANSFERS; i++) This->pTransfers[i].Transfer = libusb_alloc_transfer(0);
  while(!This->pTerminate) {
    struct timeval tv = { 1, 0 };
    int Result = libusb_handle_events_timeout_completed(This->pContext, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) LOGFile::Error("libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(This->pNewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_get_device_descriptor return code %d\n", Result);
        This->pNewDevice = NULL;
        continue;
      }
      Result = libusb_open(This->pNewDevice, &This->pDeviceHandle);
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_open return %d\n", Result);
        This->pDeviceHandle = NULL;
        This->pNewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(This->pDeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        LOGFile::Error("libusb_get_string_descriptor_ascii returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      if(This->pSerial != Serial) {
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }        
      Result = libusb_kernel_driver_active(This->pDeviceHandle, 0);
      if(Result < LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_kernel_driver_active returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else if(Result == 1) { 
        Result = libusb_detach_kernel_driver(This->pDeviceHandle, 0);
        if(Result < LIBUSB_SUCCESS) {
          LOGFile::Error("libusb_detach_kernel_driver returns %d\n", Result);
          libusb_close(This->pDeviceHandle);
          This->pDeviceHandle = NULL; 
          This->pNewDevice = NULL;
          continue;
        } else {
          LOGFile::Info("Kernel driver dettached for DeviceV1.01 with serial '%s'\n", This->pSerial.c_str());
        }
      }
      Result = libusb_claim_interface(This->pDeviceHandle, 0);
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_claim_interface returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      } else {
        LOGFile::Info("Claimed interface 0 for DeviceV1.01 with serial %s\n", This->pSerial.c_str());
      }
      for(int i = 0; i < DEVICEV1_01_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(This->pTransfers[i].Transfer, This->pDeviceHandle, DEVICEV1_01_ENDPOINT_IN, This->pTransfers[i].Buffer, sizeof(This->pTransfers[i].Buffer), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(This->pTransfers[i].Transfer);
      }
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      LOGFile::Info("Capture started on DeviceV1.01 with serial '%s'\n", This->pSerial.c_str());
//      This->pTerminateSignal = false;
//      if(pthread_create(&This->pSignalThread, NULL, Signal_Thread, This) != 0) {
//        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//      }
      This->pNewDevice = NULL;
    }
  }
  libusb_release_interface(This->pDeviceHandle, 0);
  libusb_close(This->pDeviceHandle);
  libusb_exit(This->pContext);
  return NULL;
}
void *DeviceV1_01::Sig_Thread(void* args) {
//  DeviceV1_01 *This = (DeviceV1_01*)args;
//  bool Toogle = false;
//  while(!This->pTerminateSignal) {
    AnalyzerInterface::Params_t Params;
/*    This->pAnalyzer->GetParams(&Params, NULL);
    uint16_t Current = 0;
    uint16_t Voltage = 1 << 6;
    uint8_t  Beep    = 0;
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
  //}
  return NULL;
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
