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
#include "SysSupport.h"
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
      continue;
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
json DeviceV2_01::CheckConfig(const json& config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = vector<float>(3, 0.0);
  ConfigOUT["VoltageOffset"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentGain"] = vector<float>(4, 0.0);
  ConfigOUT["CurrentOffset"] = vector<float>(4, 0.0);
  ConfigOUT["CurrentLevel"] = vector<float>(4, 1000000.0);
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["HighGain"] = false;
  ConfigOUT["Outputs"] = json::array();
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_array() && (config["VoltageGain"].size() == ConfigOUT["VoltageGain"].size())) {
      for(unsigned n = 0; n < ConfigOUT["VoltageGain"].size(); n++) {
        if(config["VoltageGain"][n].is_number() && (config["VoltageGain"][n] >= -10.0) && (config["VoltageGain"][n] <= 10.0)) ConfigOUT["VoltageGain"][n] = config["VoltageGain"][n];
      }
    }
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_array() && (config["VoltageOffset"].size() == ConfigOUT["VoltageOffset"].size())) {
      for(unsigned n = 0; n < ConfigOUT["VoltageOffset"].size(); n++) {
        if(config["VoltageOffset"][n].is_number() && (config["VoltageOffset"][n] >= -20.0) && (config["VoltageOffset"][n] <= 20.0)) ConfigOUT["VoltageOffset"][n] = config["VoltageOffset"][n];
      }
    }
    if(config.contains("CurrentGain") && config["CurrentGain"].is_array() && (config["CurrentGain"].size() == ConfigOUT["CurrentGain"].size())) {
      for(unsigned n = 0; n < ConfigOUT["CurrentGain"].size(); n++) {
        if(config["CurrentGain"][n].is_number() && (config["CurrentGain"][n] >= -10.0) && (config["CurrentGain"][n] <= 10.0)) ConfigOUT["CurrentGain"][n] = config["CurrentGain"][n];
      }
    }
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_array() && (config["CurrentOffset"].size() == ConfigOUT["CurrentOffset"].size())) {
      for(unsigned n = 0; n < ConfigOUT["CurrentOffset"].size(); n++) {
        if(config["CurrentOffset"][n].is_number() && (config["CurrentOffset"][n] >= -1000.0) && (config["CurrentOffset"][n] <= 1000.0)) ConfigOUT["CurrentOffset"][n] = config["CurrentOffset"][n];
      }
    }
    if(config.contains("CurrentLevel") && config["CurrentLevel"].is_array() && (config["CurrentLevel"].size() == ConfigOUT["CurrentLevel"].size())) {
      for(unsigned n = 0; n < ConfigOUT["CurrentLevel"].size(); n++) {
        if(config["CurrentLevel"][n].is_number() && (config["CurrentLevel"][n] > 0.001) && (config["CurrentLevel"][n] <= 1000000.0)) ConfigOUT["CurrentLevel"][n] = config["CurrentLevel"][n];
      }
    }
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("HighGain") && config["HighGain"].is_boolean()) ConfigOUT["HighGain"] = config["HighGain"];
    if(config.contains("Outputs") && config["Outputs"].is_array()) {
      uint8_t CurrentMask = 0x00;
      for(auto Tmp : config["Outputs"]) {
        if(!Tmp.is_object()) continue;
        json Output     = json::object();
        bool Error      = false;
        Output["Mode"]  = "1PHASE";
        if(Tmp.contains("Mode") && Tmp["Mode"].is_string() && ((Tmp["Mode"] == "1PHASE") || (Tmp["Mode"] == "3PHASE") || (Tmp["Mode"] == "3PHASE+N"))) Output["Mode"] = Tmp["Mode"];
        if(Output["Mode"] == "1PHASE") {
          Output["Voltage"] = "CH1";
          Output["Current"] = "+CH1";
          if(Tmp.contains("Voltage") && Tmp["Voltage"].is_string() && ((Tmp["Voltage"] == "CH1") || (Tmp["Voltage"] == "CH2") || (Tmp["Voltage"] == "CH3"))) Output["Voltage"] = Tmp["Voltage"];
          if(Tmp.contains("Current") && Tmp["Current"].is_string() && ((Tmp["Current"] == "+CH1") || (Tmp["Current"] == "-CH1") || (Tmp["Current"] == "+CH2") || (Tmp["Current"] == "-CH2") || (Tmp["Current"] == "+CH3") || (Tmp["Current"] == "-CH3") || (Tmp["Current"] == "+CH4") || (Tmp["Current"] == "-CH4"))) Output["Current"] = Tmp["Current"];
          string CurrentCH = Output["Current"];
          uint8_t Channel = CurrentCH[3] - '1';
          if((CurrentMask & (1 << Channel)) != 0) Error = true;
          CurrentMask |= (1 << Channel);
        } else {
          Output["Voltage"] = json::parse("[\"CH1\", \"CH2\", \"CH3\"]");
          Output["Current"] = ((Output["Mode"] == "3PHASE") ? json::parse("[\"+CH1\", \"+CH2\", \"+CH3\"]") : json::parse("[\"+CH1\", \"+CH2\", \"+CH3\", \"+CH4\"]"));
          if(Tmp.contains("Voltage") && Tmp["Voltage"].is_array() && (Tmp["Voltage"].size() == Output["Voltage"].size()) && Tmp.contains("Current") && Tmp["Current"].is_array() && (Tmp["Current"].size() == Output["Current"].size())) {
            for(unsigned i = 0; i < Output["Voltage"].size(); i++) {
              if(Tmp["Voltage"][i].is_string() && ((Tmp["Voltage"][i] == "CH1") || (Tmp["Voltage"][i] == "CH2") || (Tmp["Voltage"][i] == "CH3"))) Output["Voltage"][i] = Tmp["Voltage"][i];
            }
            for(unsigned i = 0; i < Output["Current"].size(); i++) {
              if(Tmp["Current"][i].is_string() && ((Tmp["Current"][i] == "+CH1") || (Tmp["Current"][i] == "-CH1") || (Tmp["Current"][i] == "+CH2") || (Tmp["Current"][i] == "-CH2") || (Tmp["Current"][i] == "+CH3") || (Tmp["Current"][i] == "-CH3") || (Tmp["Current"][i] == "+CH4") || (Tmp["Current"][i] == "-CH4"))) Output["Current"][i] = Tmp["Current"][i];
            }
          }
          for(unsigned i = 0; i < Output["Current"].size(); i++) {
            string CurrentCH = Output["Current"][i];
            uint8_t Channel = CurrentCH[3] - '1';
            if((CurrentMask & (1 << Channel)) != 0) Error = true;
            CurrentMask |= (1 << Channel);
          }
        }
        Output["Analyzer"] = AnalyzerSoft::CheckConfig(Tmp.contains("Analyzer") ? Tmp["Analyzer"] : json::object());
        if(Error == false) ConfigOUT["Outputs"].push_back(Output);
      }      
    }
  }
  return ConfigOUT;
}
DeviceV2_01::DeviceV2_01(const string &serial) : Device(serial, "V2.01", "Capture device V2.01"), pSignalTransfer(this, 2) {
  pVoltageGain = vector<float>(3, 0.0);
  pVoltageOffset = vector<float>(3, 0.0);
  pCurrentGain = vector<float>(4, 0.0);
  pCurrentOffset = vector<float>(4, 0.0);
  pCurrentLevel = vector<float>(4, 1000000.0);
}
DeviceV2_01::~DeviceV2_01() {
  StopDevice();
}
DeviceV2_01::Transfer::Transfer(DeviceV2_01 *parent, uint32_t size) {
  pBufferSize = size;
  pParent = parent;
  pBuffer = (uint8_t*)malloc(size);
  pTransfer = libusb_alloc_transfer(0);
}
DeviceV2_01::Transfer::~Transfer() {
  if(pBuffer != NULL) free(pBuffer);
  if(pTransfer != NULL) libusb_free_transfer(pTransfer);
}
int DeviceV2_01::Transfer::Submit() {
  return libusb_submit_transfer(pTransfer);
}
int DeviceV2_01::Transfer::Submit(libusb_device_handle* deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
  Setup(deviceHandle, endpoint, callback, timeout);
  return Submit();
}
void DeviceV2_01::Transfer::Setup(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
  libusb_fill_bulk_transfer(pTransfer, deviceHandle, endpoint, pBuffer, pBufferSize, callback, pParent, timeout);
}
int DeviceV2_01::Transfer::Submit(void *data, uint32_t size) {
  memcpy(pBuffer, data, size);
  libusb_submit_transfer(pTransfer);
  return Submit();
}
bool  DeviceV2_01::StartDevice() {
  pMembersLock.WriteLock();
  if(pState == RUNNING) {
    pMembersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Loading 'V2.01' driver configuration for serial '%s'\n", pSerial.c_str());
  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + pSerial));
  bool AutoStart = Config["AutoStart"];
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pMembersLock.WriteUnlock();
      return true;
    }
  }
  LOGFile::Info("Starting 'V2.01' driver for serial '%s'\n", pSerial.c_str());
  for(uint8_t n = 0; n < pVoltageGain.size(); n++) pVoltageGain[n] = Config["VoltageGain"][n];
  for(uint8_t n = 0; n < pVoltageOffset.size(); n++) pVoltageOffset[n] = Config["VoltageOffset"][n];
  for(uint8_t n = 0; n < pCurrentGain.size(); n++) pCurrentGain[n] = Config["CurrentGain"][n];
  for(uint8_t n = 0; n < pCurrentOffset.size(); n++) pCurrentOffset[n] = Config["CurrentOffset"][n];
  for(uint8_t n = 0; n < pCurrentLevel.size(); n++) pCurrentLevel[n] = Config["CurrentLevel"][n];
  pHighGain = Config["HighGain"];
  pTerminate = false;
  for(auto Tmp : Config["Outputs"]) {
    Analyzer_t AnalyzerSetup;
    AnalyzerSetup.Mode = (Tmp["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (Tmp["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
    string Name = pSerial;
    if(AnalyzerSetup.Mode == Analyzer::PHASE1) {
      string Voltage = Tmp["Voltage"];
      AnalyzerSetup.Voltage.push_back(Voltage[2] - '1');
      string Current = Tmp["Current"];
      CurrentCH_t CurrentCH;
      CurrentCH.Index = Current[3] - '1';
      CurrentCH.Invert = (Current[0] == '+') ? 1.0 : -1.0;
      AnalyzerSetup.Current.push_back(CurrentCH);
      Name += "_1P_CH" + to_string(CurrentCH.Index + 1);
    } else {
      for(auto Voltage : Tmp["Voltage"]) {
        string VoltCH = Voltage;
        AnalyzerSetup.Voltage.push_back(VoltCH[2] - '1');
      }
      for(auto Current : Tmp["Current"]) {
        string CurrCH = Current;
        CurrentCH_t CurrentCH;
        CurrentCH.Index = CurrCH[3] - '1';
        CurrentCH.Invert = (CurrCH[0] == '+') ? 1.0 : -1.0;
        AnalyzerSetup.Current.push_back(CurrentCH);
      }
      Name += (AnalyzerSetup.Mode == Analyzer::PHASE3) ? "_3P" : "_3PN";
    }
    AnalyzerSetup.Parent = new AnalyzerSoft(Name, AnalyzerSetup.Mode, "/Devices/" + pSerial + "/Outputs/" + to_string(pAnalyzersSetup.size()) + "/Analyzer", DEVICEV2_01_SAMPLEFREQ);
    pAnalyzersSetup.push_back(AnalyzerSetup);
    pAnalyzers.push_back(AnalyzerSetup.Parent);
  }
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
  } else {
    if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      libusb_exit(pContext);
    } else {
      pButtonAction = false;
      pDeviceHandle = NULL;
      pNewDevice    = NULL;
      pSubmittedTransfers = 0;
      pCapture.clear();
      pthread_setname_np(pCaptureThread, "DeviceV2_01");
      if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE, DEVICEV2_01_VID, DEVICEV2_01_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
        LOGFile::Error("Failed to register libUSB calback...\n");
        pTerminate = true;
        pthread_join(pCaptureThread, NULL);
        libusb_exit(pContext);
      } else {
        pthread_setname_np(pCaptureThread, "DeviceV2_01");
        pState = RUNNING;
      }
    }
  }
  pMembersLock.WriteUnlock();
  return (pState == RUNNING);
}
bool  DeviceV2_01::StopDevice() {
  pMembersLock.WriteLock();
  if(pState != RUNNING) {
    pMembersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Stopping 'V2.01' driver for serial '%s'\n", pSerial.c_str());
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    pAnalyzersSetup.pop_back();
    delete Analyzer;    
  }
  pState = STOPPED;
  pMembersLock.WriteUnlock();
  return true;
}
void *DeviceV2_01::Capture_Thread(void *args) {
  DeviceV2_01 *This   = (DeviceV2_01*)args;
  uint64_t   LastSignalSend = 0;
  This->pTransfers.reserve(DEVICEV2_01_TRANSFERS);
  for(auto n = 0; n < DEVICEV2_01_TRANSFERS; n++) This->pTransfers.emplace_back(This, 8192);
  while((This->pTerminate == false) || (This->pSubmittedTransfers > 0)) {
    struct timeval tv = { 1, 0 };
    int Result = libusb_handle_events_timeout_completed(This->pContext, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) LOGFile::Error("libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->pTerminate == true) continue;
    if((This->pDeviceHandle != NULL) && (LastSignalSend < (Tools::GetTime() - 500))) {
      uint16_t Status = 0x0006;
      json SysStatus = SysSupport::GetSystemDetails();
      if(SysStatus.contains("RAM_Usage") && SysStatus["RAM_Usage"].is_number()) {
        float RAM_Usage = SysStatus["RAM_Usage"];
        if(RAM_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;
      }
      if(SysStatus.contains("CPU_Usage") && SysStatus["CPU_Usage"].is_number()) {
        float CPU_Usage = SysStatus["CPU_Usage"];
        if(CPU_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;
      } 
      if(SysStatus.contains("Disk_Usage") && SysStatus["Disk_Usage"].is_number()) {
        float Disk_Usage = SysStatus["Disk_Usage"];
        if(Disk_Usage > 90.0) Status = (Status & 0xFFFC) | 0x0003;
      }
      if(SysStatus.contains("Charging") && SysStatus["Charging"].is_boolean()) {
        bool Charging = SysStatus["Charging"];
        if(Charging) Status = (Status & 0xFFFC) | 0x0003;
      }
      if(SysStatus.contains("Updates") && SysStatus["Updates"].is_boolean()) {
        bool Updates = SysStatus["Updates"];
        if(Updates) Status = (Status & 0xFFFC) | 0x0003;
      }
      if(SysStatus.contains("Event") && SysStatus["Event"].is_boolean()) {
        bool Event = SysStatus["Event"];
        if(Event == true) Status = (Status & 0xC01C) | 0x0021;
      }
      if(SysStatus.contains("WIFI_Network") && SysStatus["WIFI_Network"].is_string()) {
        Status = (Status & 0xFFF3) | 0x000C;
        if(SysStatus.contains("NTP_Enabled") && SysStatus["NTP_Enabled"].is_boolean()) {
          bool Enabled = SysStatus["NTP_Enabled"];
          if(Enabled) Status = (Status & 0xFFF3) | 0x0008;
        }
      }
      if(SysStatus.contains("Bluetooth_Network") && SysStatus["Bluetooth_Network"].is_string()) {
        Status = (Status & 0xFFEF) | 0x0010;
      }
      Status |= (This->pHighGain == true) ? 0x4000 : 0x0000;
      This->pSignalTransfer.Submit(&Status, sizeof(Status));
      This->pSubmittedTransfers++;
      LastSignalSend = Tools::GetTime();
    }
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
      This->pSignalTransfer.Setup(This->pDeviceHandle, DEVICEV2_01_ENDPOINT_OUT, ReadCallback, -1);
      for(Transfer &Tmp : This->pTransfers) {
        Tmp.Submit(This->pDeviceHandle, DEVICEV2_01_ENDPOINT_IN, ReadCallback, -1);
        This->pSubmittedTransfers++;
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
      LastSignalSend = 0;
      This->pLastSignalRecv = 0;
    }
  }
  if(This->pDeviceHandle != 0) libusb_release_interface(This->pDeviceHandle, 0);
  if(This->pDeviceHandle != 0) libusb_close(This->pDeviceHandle);
  libusb_exit(This->pContext);
  This->pTransfers.clear();
  return NULL;
}
void  DeviceV2_01::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV2_01 *This = (DeviceV2_01*)transfer->user_data;
  if((transfer->endpoint == DEVICEV2_01_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    uint16_t *Samples = (uint16_t*)transfer->buffer;
    for(int Pos = 0; Pos < (transfer->actual_length / 2); Pos++) {
      uint8_t  Seq    = Samples[Pos] >> 13;
      uint16_t Sample = (Samples[Pos] & 0x1FFF);
      if(Seq == 7) {
        if(This->pLastSignalRecv < (Tools::GetTime() - 1000)) {
          json Status = json::object();
          float BattVoltage = 2.96 + ((Sample & 0x00FC) >> 2) * 0.02;
          float BattLevel  = (BattVoltage - 2.96) * 80.65;
          if(BattLevel > 100.0) BattLevel = 100.0;
          Status["Battery_Voltage"] = BattVoltage;
          Status["Battery_Level"] = BattLevel;
          Status["Charging"] = (Sample & 0x0002) ? true: false;
          Status["Powered"] = (Sample & 0x0001) ? true: false;
          SysSupport::SetSystemDetails(Status);
          uint8_t Action = (Sample & 0x0300) >> 8;
          if((Action == CLICK) && (This->pButtonAction == false)) {
            SysSupport::SetSystemActions("ToogleBluetooth");
          } else if((Action == LONG_CLICK) && (This->pButtonAction == false)) {
            SysSupport::SetSystemActions("PowerOff");
          }
          This->pButtonAction = (Action == NONE);
          This->pLastSignalRecv = Tools::GetTime();
        }    
        continue;
      }
      if(This->pCapture.size() != Seq) {
        LOGFile::Debug("Lost sample, restarting adquisition\n");
        This->pCapture.clear();
        break;
      }
      This->pCapture.push_back(Sample - 4096);
      if(This->pCapture.size() == 7) {
        if(This->BuffersAppend() == false) {
          LOGFile::Error("Capture buffer overflow! Posible lost of samples\n");
          break;
        }
        This->pCapture.clear();
      }
    }
  }
  if(This->pTerminate == false) {
    if(transfer->endpoint == DEVICEV2_01_ENDPOINT_IN) {
      libusb_submit_transfer(transfer);
    } else if(transfer->endpoint == DEVICEV2_01_ENDPOINT_OUT) {
      This->pSubmittedTransfers--;
    }
  } else {
    This->pSubmittedTransfers--;
  }
}
int   DeviceV2_01::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
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
void  DeviceV2_01::GetDevice(HTTPRequest &req) {
  pMembersLock.ReadLock();
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  for(uint8_t n = 0; n < Config["Outputs"].size(); n++) Config["Outputs"][n].erase("Analyzer");
  Sample_t Avg;
  Avg.Voltage[0] = 0.0;
  Avg.Voltage[1] = 0.0;
  Avg.Voltage[2] = 0.0;
  Avg.Current[0] = 0.0;
  Avg.Current[1] = 0.0;
  Avg.Current[2] = 0.0;
  Avg.Current[3] = 0.0;
  pAverageLocks.ReadLock();
  for(deque<Sample_t>::iterator it = pAverageValues.begin(); it != pAverageValues.end(); it++) {
    Avg.Voltage[0] += it->Voltage[0];
    Avg.Voltage[1] += it->Voltage[1];
    Avg.Voltage[2] += it->Voltage[2];
    Avg.Current[0] += it->Current[0];
    Avg.Current[1] += it->Current[1];
    Avg.Current[2] += it->Current[2];
    Avg.Current[3] += it->Current[3];
  }
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = json::array();
  Config["Wave_Average"]["Voltage"].push_back(Avg.Voltage[0] / pAverageValues.size());
  Config["Wave_Average"]["Voltage"].push_back(Avg.Voltage[1] / pAverageValues.size());
  Config["Wave_Average"]["Voltage"].push_back(Avg.Voltage[2] / pAverageValues.size());
  Config["Wave_Average"]["Current"] = json::array();
  Config["Wave_Average"]["Current"].push_back(Avg.Current[0] / pAverageValues.size());
  Config["Wave_Average"]["Current"].push_back(Avg.Current[1] / pAverageValues.size());
  Config["Wave_Average"]["Current"].push_back(Avg.Current[2] / pAverageValues.size());
  Config["Wave_Average"]["Current"].push_back(Avg.Current[3] / pAverageValues.size());
  pAverageLocks.ReadUnlock();
  pMembersLock.ReadUnlock();
  req.FromJSON(Config);
}
void  DeviceV2_01::SetDevice(HTTPRequest &req) {
  pMembersLock.ReadLock();
  DeviceState State = pState;
  pMembersLock.ReadUnlock();
  StopDevice();
  json Params = req.GetParams();
  json Config = CheckConfig(Params);
  json Orig = ConfigManager::ReadConfig("/Devices/" + pSerial);
  for(unsigned n = 0; n < Config["Outputs"].size(); n++) {
    if(n >= Orig["Outputs"].size()) break;
    if(Config["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"]) Config["Outputs"][n] == Orig["Outputs"][n];
  }
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  json Response = json::object();
  Response["Result"] = (State == RUNNING) ? StartDevice() : true;
  req.FromJSON(Response);
}
bool  DeviceV2_01::BuffersAppend() {
  Sample_t Sample;
  Sample.Voltage[0] = (((float)pCapture[0] * 0.110266113281) * (1.0 + (pVoltageGain[0] / 100.0))) + pVoltageOffset[0];
  Sample.Voltage[1] = (((float)pCapture[1] * 0.110266113281) * (1.0 + (pVoltageGain[1] / 100.0))) + pVoltageOffset[1];
  Sample.Voltage[2] = (((float)pCapture[2] * 0.110266113281) * (1.0 + (pVoltageGain[2] / 100.0))) + pVoltageOffset[2];
  Sample.Current[0] = (((float)pCapture[3] * 201.416015625) * (1.0 + (pCurrentGain[0] / 100.0))) / pCurrentLevel[0] + pCurrentOffset[0];
  Sample.Current[1] = (((float)pCapture[4] * 201.416015625) * (1.0 + (pCurrentGain[1] / 100.0))) / pCurrentLevel[1] + pCurrentOffset[1];
  Sample.Current[2] = (((float)pCapture[5] * 201.416015625) * (1.0 + (pCurrentGain[2] / 100.0))) / pCurrentLevel[2] + pCurrentOffset[2];
  Sample.Current[3] = (((float)pCapture[6] * 201.416015625) * (1.0 + (pCurrentGain[3] / 100.0))) / pCurrentLevel[3] + pCurrentOffset[3];
  pBuffer.push_back(Sample);
  pAverageLocks.WriteLock();
  pAverageValues.push_back(Sample);
  if(pAverageValues.size() > DEVICEV2_01_SAMPLEFREQ) pAverageValues.pop_front();  
  pAverageLocks.WriteUnlock();
  for(auto Tmp : pAnalyzersSetup) {
    if(Tmp.Parent->IsFinished() == false) return true;
  }
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample_t Sample = pBuffer.front();
    pBuffer.pop_front();
    for(auto Tmp : pAnalyzersSetup) {
      vector<float> Voltage, Current;
      for(auto VoltCH : Tmp.Voltage) Voltage.push_back(Sample.Voltage[VoltCH]);
      for(auto CurrCH : Tmp.Current) Current.push_back(Sample.Current[CurrCH.Index] * CurrCH.Invert);
      if(Tmp.Parent->BufferAppend(Voltage, Current) == true) Started = true;
    }
  }
  return true;
}
//======================================================================================================================