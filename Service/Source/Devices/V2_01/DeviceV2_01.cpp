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
#include "DeviceV2_01.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV2_01::pRegistered = DeviceV2_01::GetInstances();
// Funciones ===========================================================================================================
bool DeviceV2_01::GetInstances() {
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
    if(Descriptor.idVendor != DEVICEV2_01_VID) continue;
    if(Descriptor.idProduct != DEVICEV2_01_PID) continue;
    if(Descriptor.bcdDevice != DEVICEV2_01_VERSION) continue;
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
    if(strcmp(String, DEVICEV2_01_MANSTRING) != 0) continue;
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV2_01_PRODSTRING) != 0) continue;    
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iSerialNumber, (unsigned char*)String, sizeof(String)) < 0) {
      LOGFile::Error("Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    libusb_close(Dev_Handle);
    LOGFile::Info("Adding 'V2.01' device driver instance with serial '%s'\n", String);
    DeviceManager::AddDevice(new DeviceV2_01(String));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
  return true;
}
DeviceV2_01::DeviceV2_01(string serial) {
  pDeviceHandle  = NULL;
  pNewDevice     = NULL;
  pSerial        = serial;
  pDriver        = "V2.01";
  pName          = "Capture device V2.01";
  pVoltageGain.resize(3, 0.110266113281);
  pCurrentGain.resize(4, 0.00604248046876);
  pVoltageOffset.resize(3, 0.0);
  pCurrentOffset.resize(4, 0.0);
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerInterface *DeviceV2_01::FindAnalyzer(string serial) {
  for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
    if(pAnalyzers[i].Analyzer == NULL) continue;
    if(serial == pAnalyzers[i].Analyzer->Serial()) return pAnalyzers[i].Analyzer;
  }
  return NULL;
}
uint32_t DeviceV2_01::AnalyzersCount() {
  uint32_t Count = 0;
  for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
    if(pAnalyzers[i].Analyzer == NULL) continue;
    Count++;
  }
  return Count++;
}
bool DeviceV2_01::Start() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'V2.01' driver configuration for serial '%s'\n", pSerial.c_str());
  bool AutoStart = false;
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  if(Config.is_object()) {
    if(Config["VoltageGain"].is_array() && (Config["VoltageGain"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(Config["VoltageGain"][n].is_number()) pVoltageGain[n] = Config["VoltageGain"][n];
      }
    }
    if(Config["VoltageOffset"].is_array() && (Config["VoltageOffset"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(Config["VoltageOffset"][n].is_number()) pVoltageOffset[n] = Config["VoltageOffset"][n];
      }
    }
    if(Config["CurrentGain"].is_array() && (Config["CurrentGain"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(Config["CurrentGain"][n].is_number()) pCurrentGain[n] = Config["CurrentGain"][n];
      }
    }
    if(Config["CurrentOffset"].is_array() && (Config["CurrentOffset"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(Config["CurrentOffset"][n].is_number()) pCurrentOffset[n] = Config["CurrentOffset"][n];
      }
    }
    if(Config["AutoStart"].is_boolean()) AutoStart = Config["AutoStart"];
    uint8_t CurrentMask = 0x00;
    if(Config["Outputs"].is_array()) {
      for(uint n = 0; n < Config["Outputs"].size(); n++) {
        Analyzer_t Analyzer;
        Analyzer.Analyzer = NULL;
        Analyzer.Mode = AnalyzerInterface::PHASE3;
        Analyzer.OffsetC = 0;
        Analyzer.OffsetV = 0;
        if(!Config["Outputs"][n].is_object()) continue;
        if(Config["Outputs"][n]["Mode"].is_string() && ((Config["Outputs"][n]["Mode"] == "1PHASE") || (Config["Outputs"][n]["Mode"] == "3PHASE") || (Config["Outputs"][n]["Mode"] == "3PHASE+N"))) Analyzer.Mode = (Config["Outputs"][n]["Mode"] == "1PHASE") ? AnalyzerInterface::PHASE1 : (Config["Outputs"][n]["Mode"] == "3PHASE") ? AnalyzerInterface::PHASE3 : AnalyzerInterface::PHASE3N;
        if(Analyzer.Mode == AnalyzerInterface::PHASE1) {
          if(Config["Outputs"][n]["CurrentChannel"].is_number() && (Config["Outputs"][n]["CurrentChannel"] >= 0) && (Config["Outputs"][n]["CurrentChannel"] <= 4)) Analyzer.OffsetC = Config["Outputs"][n]["CurrentChannel"];
          if(Config["Outputs"][n]["VoltageChannel"].is_number() && (Config["Outputs"][n]["VoltageChannel"] >= 0) && (Config["Outputs"][n]["VoltageChannel"] <= 3)) Analyzer.OffsetC = Config["Outputs"][n]["VoltageChannel"];
          if((CurrentMask & (1 << Analyzer.OffsetC)) != 0) continue;
          CurrentMask |= (1 << Analyzer.OffsetC);
        } else if(Analyzer.Mode == AnalyzerInterface::PHASE3) {
          if((CurrentMask & 0x0F) != 0) continue;
          CurrentMask |= 0x0F;
        } else if(Analyzer.Mode == AnalyzerInterface::PHASE3N) {
          if((CurrentMask & 0x0E) != 0) continue;
          CurrentMask |= 0x0E;
        }
        pAnalyzers.push_back(Analyzer);
      }
    }
  }
  Config = json::object();
  Config["VoltageGain"] = pVoltageGain;
  Config["VoltageOffset"] = pVoltageOffset;
  Config["CurrentGain"] = pCurrentGain;
  Config["CurrentOffset"] = pCurrentOffset;
  Config["AutoStart"] = AutoStart;
  Config["Outputs"] = json::array();
  for(uint n = 0; n < pAnalyzers.size(); n++) {
    json Analyzer;
    Analyzer["Mode"] = (pAnalyzers[n].Mode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pAnalyzers[n].Mode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N";
    if(pAnalyzers[n].Mode == AnalyzerInterface::PHASE1) {
      Analyzer["CurrentChannel"] = pAnalyzers[n].OffsetC;
      Analyzer["VoltageChannel"] = pAnalyzers[n].OffsetV;
    }
    Config["Outputs"].push_back(Analyzer);
  }
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      pAnalyzers.clear();
      return true;
    }
  }
  LOGFile::Info("Starting 'V2.01' driver for serial '%s'\n", pSerial.c_str());
  pTerminate    = false;
  pDeviceHandle = NULL;
  pNewDevice    = NULL;
  for(uint n = 0; n < pAnalyzers.size(); n++) {
    string Name = (pAnalyzers[n].Mode == AnalyzerInterface::PHASE1) ? pSerial + "_1P_" + to_string(pAnalyzers[n].OffsetC) : (pAnalyzers[n].Mode == AnalyzerInterface::PHASE3) ? pSerial + "_3P" : pSerial + "_3PN";
    pAnalyzers[n].Analyzer = new AnalyzerSoft(Name, pAnalyzers[n].Mode, DEVICEV2_01_SAMPLEFREQ);
  }
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pCaptureSync = -1;
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV2_01_VID, DEVICEV2_01_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
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
bool DeviceV2_01::Stop() {
  //Paro captura y espero a que se detenga
  //leo y elimino analizadores
  return true;
}
DeviceV2_01::~DeviceV2_01() {
  Stop();
//  if(pAnalyzer) delete pAnalyzer;
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
void DeviceV2_01::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV2_01 *This = (DeviceV2_01*)transfer->user_data;
  uint16_t *Samples = (uint16_t*)transfer->buffer;
  for(int Pos = 0; Pos < (transfer->actual_length / 2); Pos++) {
    uint8_t Seq = Samples[Pos] >> 13;
    if(This->pCaptureSync == -1) {
      if(Seq == 0) {
        This->pCaptureSync = 0;
      } else {
        continue;
      }
    }
    if(Seq == 7) {
      //Estado
      continue;
    }
    if(This->pCaptureSync != Seq) {
      LOGFile::Debug("Lost sample, restarting adquisition\n");
      This->pCaptureSync = -1;
      break;
    }
    ((float*)&This->pCapture)[This->pCaptureSync++] = ((float)(Samples[Pos] & 0x1FFF)) - 4096.0;
    if(This->pCaptureSync == 7) {
      This->pCapture.Voltage[0] = (This->pCapture.Voltage[0] - This->pVoltageOffset[0]) * This->pVoltageGain[0];
      This->pCapture.Voltage[1] = (This->pCapture.Voltage[1] - This->pVoltageOffset[1]) * This->pVoltageGain[1];
      This->pCapture.Voltage[2] = (This->pCapture.Voltage[2] - This->pVoltageOffset[2]) * This->pVoltageGain[2];
      This->pCapture.Current[0] = (This->pCapture.Current[0] - This->pCurrentOffset[0]) * This->pCurrentGain[0];
      This->pCapture.Current[1] = (This->pCapture.Current[1] - This->pCurrentOffset[1]) * This->pCurrentGain[1];
      This->pCapture.Current[2] = (This->pCapture.Current[2] - This->pCurrentOffset[2]) * This->pCurrentGain[2];
      This->pCapture.Current[3] = (This->pCapture.Current[3] - This->pCurrentOffset[3]) * This->pCurrentGain[3];
      if(This->BuffersAppend() == false) {
        LOGFile::Error("Capture buffer overflow! Posible lost of samples\n");
        break;
      }
      This->pCaptureSync = 0;
    }
  }
  if(This->pTerminate == false) libusb_submit_transfer(transfer);
}
int DeviceV2_01::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV2_01 *This = (DeviceV2_01*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);	
      if(dev == CurrentDev) {
        LOGFile::Info("DeviceV2.01 with serial '%s' disconnected\n", This->pSerial.c_str());
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
bool DeviceV2_01::BuffersAppend() {
  pBuffer.push_back(pCapture);
  uint64_t Sampletime = Tools::GetTime();
  for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
    if(pAnalyzers[i].Analyzer == NULL) continue;
    if(pAnalyzers[i].Analyzer->IsFinished() == false) return true;
  }
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample_t Sample = pBuffer.front();
    pBuffer.pop_front();
    for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
      if(pAnalyzers[i].Analyzer == NULL) continue;
      vector<float> Voltage, Current;
      for(uint8_t n = 0; n < ((pAnalyzers[i].Mode == AnalyzerInterface::PHASE1) ? 1 : 3); n++) Voltage.push_back(Sample.Voltage[pAnalyzers[i].OffsetV + n]);
      for(uint8_t n = 0; n < ((pAnalyzers[i].Mode == AnalyzerInterface::PHASE1) ? 1 : (pAnalyzers[i].Mode == AnalyzerInterface::PHASE3) ? 3 : 4); n++) Current.push_back(Sample.Current[pAnalyzers[i].OffsetC + n]);
      if(pAnalyzers[i].Analyzer->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
    }
  }
  return true;
}
void *DeviceV2_01::Capture_Thread(void *args) {
  DeviceV2_01 *This = (DeviceV2_01*)args;
  for(int i = 0; i < DEVICEV2_01_TRANSFERS; i++) This->pTransfers[i].Transfer = libusb_alloc_transfer(0);
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
          LOGFile::Info("Kernel driver dettached for DeviceV2.01 with serial '%s'\n", This->pSerial.c_str());
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
        LOGFile::Info("Claimed interface 0 for DeviceV2 with serial '%s'\n", This->pSerial.c_str());
      }
      for(int i = 0; i < DEVICEV2_01_TRANSFERS; i++) {
        libusb_fill_bulk_transfer(This->pTransfers[i].Transfer, This->pDeviceHandle, DEVICEV2_01_ENDPOINT_IN, This->pTransfers[i].Buffer, sizeof(This->pTransfers[i].Buffer), ReadCallback, This, -1);
        Result |= libusb_submit_transfer(This->pTransfers[i].Transfer);
      }
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL; 
        This->pNewDevice = NULL;
        continue;
      }
      LOGFile::Info("Capture started on DeviceV2.01 with serial '%s'\n", This->pSerial.c_str());
      This->pNewDevice = NULL;
//      This->pUSBState = USB_RESET;
//      int Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x01, 0x0000, 0x0000, NULL, 0, 0);
//      if(Error < 0) LOGFile::Error("Can not stop device with serial %s (%d)\n", This->pSerial, Error);
//      usleep(1000000);
//      This->pUSBState = USB_WAIT;
//      float Conf[7] = {This->pVoltageOffset[0], This->pCurrentOffset[0], This->pVoltageOffset[1], This->pCurrentOffset[1], This->pVoltageOffset[2], This->pCurrentOffset[2], This->pCurrentOffset[3]};
//      Error = libusb_control_transfer(This->pDeviceHandle, LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE, 0x00, (This->pChannelMask << 8) | 0x0E, 0x0000, (uint8_t*)Conf, sizeof(Conf), 0);
//      if(Error < 0) LOGFile::Error("Can not setup device with serial %s (%d)\n", This->pSerial, Error);
    }
  }
  for(int i = 0; i < DEVICEV2_01_TRANSFERS; i++) libusb_free_transfer(This->pTransfers[i].Transfer);
  return NULL;
}

//DeviceV2::~DeviceV2() {
//  pTerminate = true;
//  Tools::LogMsg("INFO", "Waiting for adquisition thread ends...\n");
//  pthread_join(pCaptureThread, NULL);
//  if(pVoltageSamples) free(pVoltageSamples);
//  if(pCurrentSamples) free(pCurrentSamples);
//  for(uint i = 0; i < pAnalyzers.size(); i++) {
//    delete pAnalyzers[i].Analyzer;
//    free(pAnalyzers[i].ID);
//  }
//  free(pAnalyzers);
//}
//======================================================================================================================