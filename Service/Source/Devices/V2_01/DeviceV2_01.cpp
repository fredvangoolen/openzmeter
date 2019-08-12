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
#include "DeviceManager.h"
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
  pCaptureThread = 0;
//  pSignalThread  = 0;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
  pVoltageGain.resize(3, 0.0);
  pCurrentGain.resize(4, 0.00);
  pVoltageOffset.resize(3, 0.0);
  pCurrentOffset.resize(4, 0.0);
  pCurrentLevel.resize(4, 1.0);
}
DeviceV2_01::~DeviceV2_01() {
  StopDevice();
  WaitInstances();
}
bool DeviceV2_01::Start(const json &config) {
  for(uint8_t n = 0; n < 3; n++) pVoltageGain[n] = config["VoltageGain"][n];
  for(uint8_t n = 0; n < 3; n++) pVoltageOffset[n] = config["VoltageOffset"][n];
  for(uint8_t n = 0; n < 4; n++) pCurrentGain[n] = config["CurrentGain"][n];
  for(uint8_t n = 0; n < 4; n++) pCurrentOffset[n] = config["CurrentOffset"][n];
  for(uint8_t n = 0; n < 4; n++) pCurrentLevel[n] = config["CurrentLevel"][n];
  pTerminate = false;
  pDeviceHandle = NULL;
  pNewDevice    = NULL;
  pAnalyzersLock.WriteLock();
  for(uint8_t n = 0; n < config["Outputs"].size(); n++) {
    Analyzer_t AnalyzerSetup;
    AnalyzerSetup.Mode = (config["Outputs"][n]["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (config["Outputs"][n]["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
    if(AnalyzerSetup.Mode == Analyzer::PHASE1) {
      AnalyzerSetup.OffsetC = config["Outputs"][n]["CurrentChannel"];
      AnalyzerSetup.OffsetV = config["Outputs"][n]["VoltageChannel"];
    }
    string Name = (AnalyzerSetup.Mode == Analyzer::PHASE1) ? pSerial + "_1P_" + to_string(AnalyzerSetup.OffsetC) : (AnalyzerSetup.Mode == Analyzer::PHASE3) ? pSerial + "_3P" : pSerial + "_3PN";
    pAnalyzersSetup.push_back(AnalyzerSetup);
    pAnalyzers.push_back(new AnalyzerSoft(Name, AnalyzerSetup.Mode, "/Devices/" + pSerial + "/Outputs/" + to_string(n) + "/Analyzer", DEVICEV2_01_SAMPLEFREQ));
  }
  pAnalyzersLock.WriteUnlock();
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    pCaptureThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return false;
  }
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV2_01_VID, DEVICEV2_01_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
    LOGFile::Error("Failed to register libUSB calback...\n");
    pTerminate = true;
    pthread_join(pCaptureThread, NULL);
    pCaptureThread = 0;
    libusb_exit(pContext);
    return false;
  }
  pState = RUNNING;
  return true;
}
json DeviceV2_01::CheckConfig(const json& config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = vector<float>(3, 0.0);
  ConfigOUT["VoltageOffset"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentGain"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentOffset"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentLevel"] = vector<float>(3, 0.0);
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["Outputs"] = json::array();
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_array() && (config["VoltageGain"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(config["VoltageGain"][n].is_number()) ConfigOUT["VoltageGain"][n] = config["VoltageGain"][n];
      }
    }
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_array() && (config["VoltageOffset"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(config["VoltageOffset"][n].is_number()) ConfigOUT["VoltageOffset"][n] = config["VoltageOffset"][n];
      }
    }
    if(config.contains("CurrentGain") && config["CurrentGain"].is_array() && (config["CurrentGain"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentGain"][n].is_number()) ConfigOUT["CurrentGain"][n] = config["CurrentGain"][n];
      }
    }
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_array() && (config["CurrentOffset"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentOffset"][n].is_number()) ConfigOUT["CurrentOffset"][n] = config["CurrentOffset"][n];
      }
    }
    if(config.contains("CurrentLevel") && config["CurrentLevel"].is_array() && (config["CurrentLevel"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentLevel"][n].is_number()) ConfigOUT["CurrentLevel"][n] = config["CurrentLevel"][n];
      }
    }
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Outputs") && config["Outputs"].is_array()) {
      uint8_t CurrentMask = 0x00;
      for(uint8_t n = 0; n < config["Outputs"].size(); n++) {
        if(!config["Outputs"][n].is_object()) continue;
        json Output     = json::object();
        Output["Mode"]  = "1PHASE";
        if(config["Outputs"][n].contains("Mode") && config["Outputs"][n]["Mode"].is_string() && ((config["Outputs"][n]["Mode"] == "1PHASE") || (config["Outputs"][n]["Mode"] == "3PHASE") || (config["Outputs"][n]["Mode"] == "3PHASE+N"))) Output["Mode"] = config["Outputs"][n]["Mode"];
        if(Output["Mode"] == "1PHASE") {
          Output["CurrentChannel"] = 0;
          Output["VoltageChannel"] = 0;
          if(config["Outputs"][n].contains("CurrentChannel") && config["Outputs"][n]["CurrentChannel"].is_number() && (config["Outputs"][n]["CurrentChannel"] >= 0) && (config["Outputs"][n]["CurrentChannel"] <= 3)) Output["CurrentChannel"] = config["Outputs"][n]["CurrentChannel"];
          if(config["Outputs"][n].contains("VoltageChannel") && config["Outputs"][n]["VoltageChannel"].is_number() && (config["Outputs"][n]["VoltageChannel"] >= 0) && (config["Outputs"][n]["VoltageChannel"] <= 2)) Output["VoltageChannel"] = config["Outputs"][n]["VoltageChannel"];
          uint8_t Channel = Output["CurrentChannel"];
          if((CurrentMask & (1 << Channel)) != 0) continue;
          CurrentMask |= (1 << Channel);
        } else if(Output["Mode"] == "3PHASE") {
          if((CurrentMask & 0x0F) != 0) continue;
          CurrentMask |= 0x0F;
        } else if(Output["Mode"] == "3PHASE+N") {
          if((CurrentMask & 0x0E) != 0) continue;
          CurrentMask |= 0x0E;
        }
        Output["Analyzer"] = AnalyzerSoft::CheckConfig(config[n]["Analyzer"]);
        ConfigOUT["Outputs"].push_back(Output);
      }      
    }
  }
  return ConfigOUT;
}
bool DeviceV2_01::StartDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'V2.01' driver configuration for serial '%s'\n", pSerial.c_str());
  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + pSerial));
  bool AutoStart = Config["AutoStart"];
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      return true;
    }
  }
  LOGFile::Info("Starting 'V2.01' driver for serial '%s'\n", pSerial.c_str());
  bool Result = Start(Config);  
  pthread_mutex_unlock(&pMutex);
  return Result;
}
bool DeviceV2_01::StopDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'V2.01' driver for serial '%s'\n", pSerial.c_str());
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    pAnalyzersSetup.pop_back();
    delete Analyzer;    
  }
  pAnalyzersLock.WriteUnlock();
  pState = STOPPED;
  pthread_mutex_unlock(&pMutex);
  return true;
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
  if(This->pTerminate == false) 
    libusb_submit_transfer(transfer);
  else
    libusb_free_transfer(transfer);
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
void DeviceV2_01::GetDevice(HTTPRequest &req) {
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  for(uint8_t n = 0; n < Config["Analyzers"].size(); n++) Config["Analyzers"][n].erase("Settings");
  req.FromJSON(Config);
}
void DeviceV2_01::SetDevice(HTTPRequest &req) {
  pthread_mutex_lock(&pMutex);
  DeviceState State = pState;
  pthread_mutex_unlock(&pMutex);
  StopDevice();
  json Params = req.GetParams();
  json Config = CheckConfig(Params);
  bool Result = true;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  pthread_mutex_lock(&pMutex);
  if((pState != RUNNING) && (State == RUNNING)) Result = Start(Config);
  pthread_mutex_unlock(&pMutex);
  json Response;
  Response["Result"] = Result;
  req.FromJSON(Response);
}
bool DeviceV2_01::BuffersAppend() {
  pBuffer.push_back(pCapture);
  uint64_t Sampletime = Tools::GetTime();
  pAnalyzersLock.ReadLock();
  for(uint8_t i = 0; i < pAnalyzers.size(); i++) {
    if(((AnalyzerSoft*)pAnalyzers[i])->IsFinished() == false) {
      pAnalyzersLock.ReadUnlock();
      return true;
    }
  }  
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample_t Sample = pBuffer.front();
    pBuffer.pop_front();
    for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
      vector<float> Voltage, Current;
      for(uint8_t n = 0; n < ((pAnalyzersSetup[i].Mode == Analyzer::PHASE1) ? 1 : 3); n++) Voltage.push_back(Sample.Voltage[pAnalyzersSetup[i].OffsetV + n]);
      for(uint8_t n = 0; n < ((pAnalyzersSetup[i].Mode == Analyzer::PHASE1) ? 1 : (pAnalyzersSetup[i].Mode == Analyzer::PHASE3) ? 3 : 4); n++) Current.push_back(Sample.Current[pAnalyzersSetup[i].OffsetC + n]);
      if(((AnalyzerSoft*)pAnalyzers[i])->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
    }
  }
  pAnalyzersLock.ReadUnlock();
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
  libusb_release_interface(This->pDeviceHandle, 0);
  libusb_close(This->pDeviceHandle);
  libusb_exit(This->pContext);
  return NULL;
}
//======================================================================================================================