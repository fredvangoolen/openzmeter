// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include <pthread.h>
#include "LogFile.h"
#include "DeviceV2.h"
#include "DeviceManager.h"
#include "ConfigFile.h"
#include "AnalyzerManager.h"
#include "Tools.h"
#include "MessageLoop.h"
#include "BluetoothManager.h"
#include "WifiManager.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV2::pRegistered = DeviceV2::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV2::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceV2::GetInstances() {
  libusb_context *Context;
  int Error = libusb_init(&Context);
  if(Error < 0) return LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
  libusb_device **Devs;
  int Count = libusb_get_device_list(NULL, &Devs);
  if(Count < 0) return LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Count));
  for(int i = 0; i < Count; ++i) {
    libusb_device_descriptor Descriptor;
    Error = libusb_get_device_descriptor(Devs[i], &Descriptor);
    if(Error < 0) return LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    if(Descriptor.idVendor != DEVICEV2_VID) continue;
    if(Descriptor.idProduct != DEVICEV2_PID) continue;
    if(Descriptor.bcdDevice != 0x0201) continue;
    libusb_device_handle *Dev_Handle = NULL;
    Error = libusb_open(Devs[i], &Dev_Handle);
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      continue;
    }
    char String[256];
    Error = libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iManufacturer, (unsigned char*)String, sizeof(String));
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV2_MANSTRING) != 0) {
      libusb_close(Dev_Handle);
      continue;
    }
    Error = libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String));
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV2_PRODSTRING) != 0) {
      libusb_close(Dev_Handle);
      continue;
    }
    Error = libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iSerialNumber, (unsigned char*)String, sizeof(String));
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      libusb_close(Dev_Handle);
      continue;
    }
    libusb_close(Dev_Handle);
    DeviceManager::AddDevice(new DeviceV2(String));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
}
DeviceV2::DeviceV2(const string &serial) : Device(serial, "V2", "Capture device V2") {
  LogFile::Info("Starting DeviceV2 with serial '%s' ...", serial.c_str());
//  pBatteryVoltage     = 0;
//  pMainsPowered       = false;
//  pCharging           = false;
  pAverageLocks       = PTHREAD_RWLOCK_INITIALIZER;
  pConfigLock         = PTHREAD_MUTEX_INITIALIZER;
  pThread             = 0;
  pChannelsDelay      = NULL;
  pButtonAction       = false;
  pDeviceHandle       = NULL;
  pNewDevice          = NULL;
  pSubmittedTransfers = 0;
  pRunAverageCounter  = 0;
  pTerminate          = false;
  pCaptureSize        = 0xFF;
  for(uint8_t n = 0; n < 7; n++) pRunAverageValues[n] = 0;
  for(uint8_t n = 0; n < 7; n++) pAverageValues[n] = 0.0;
  for(uint8_t n = 0; n < 7; n++) pFilterZ1[n] = 0.0;
  for(uint8_t n = 0; n < 7; n++) pFilterZ2[n] = 0.0;
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  int Error = libusb_init(&pContext);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    return;
  }
  if(pthread_create(&pThread, NULL, [](void *args) -> void* { return ((DeviceV2*)args)->Thread(); }, this) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return;
  }
  if(pthread_setname_np(pThread, "DeviceV2") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  TaskPool::SetThreadProperties(pThread, TaskPool::HIGHEST);
  Error = libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE, DEVICEV2_VID, DEVICEV2_PID, LIBUSB_HOTPLUG_MATCH_ANY, [](struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) { return ((DeviceV2*)user_data)->PlugCallback(ctx, dev, event); }, this, NULL);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    libusb_exit(pContext);
  }
  LogFile::Info("DeviceV2 with serial '%s' started", serial.c_str());
}
uint32_t DeviceV2::AnalyzersCount() {
  pthread_mutex_lock(&pConfigLock);
  uint32_t Count = pAnalyzers.size();
  pthread_mutex_unlock(&pConfigLock);
  return Count;
}
void DeviceV2::ClearAnalyzers() {
  pthread_mutex_lock(&pConfigLock);
  while(pAnalyzers.size() > 0) {
    AnalyzerManager::DelAnalyzer(pAnalyzers.back()->Serial());
    delete pAnalyzers.back();
    pAnalyzers.pop_back();
  }
  pthread_mutex_unlock(&pConfigLock);
}
DeviceV2::~DeviceV2() {
  LogFile::Info("Stopping DeviceV2 with serial '%s'...", pSerial.c_str());
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  if(pChannelsDelay != NULL) free(pChannelsDelay);
  LogFile::Info("DeviceV2 with serial '%s' stopped", pSerial.c_str());
}
json DeviceV2::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = vector<float>(3, 1.0);
  ConfigOUT["VoltageOffset"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentGain"] = vector<float>(4, 1.0);
  ConfigOUT["CurrentOffset"] = vector<float>(4, 0.0);
  ConfigOUT["CurrentDelay"] = vector<uint8_t>(4, 0);
  ConfigOUT["Enabled"] = false;
  ConfigOUT["HighGain"] = false;
  ConfigOUT["BlockDC"] = true;
  ConfigOUT["Outputs"] = json::array();
  ConfigOUT["Calculated"] = json::object();
  ConfigOUT["Calculated"]["Enabled"]  = false;
  ConfigOUT["Calculated"]["Voltage"]  = "CH1";
  ConfigOUT["Calculated"]["Current"]  = vector<string>(4, "UNUSED");
  ConfigOUT["Calculated"]["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_array() && (config["VoltageGain"].size() == ConfigOUT["VoltageGain"].size())) {
      for(uint8_t n = 0; n < ConfigOUT["VoltageGain"].size(); n++) {
        if(config["VoltageGain"][n].is_number() && (config["VoltageGain"][n] > 0.0) && (config["VoltageGain"][n] <= 1000.0)) ConfigOUT["VoltageGain"][n] = config["VoltageGain"][n];
      }
    }
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_array() && (config["VoltageOffset"].size() == ConfigOUT["VoltageOffset"].size())) {
      for(uint8_t n = 0; n < ConfigOUT["VoltageOffset"].size(); n++) {
        if(config["VoltageOffset"][n].is_number() && (config["VoltageOffset"][n] >= -200.0) && (config["VoltageOffset"][n] <= 200.0)) ConfigOUT["VoltageOffset"][n] = config["VoltageOffset"][n];
      }
    }
    if(config.contains("CurrentGain") && config["CurrentGain"].is_array() && (config["CurrentGain"].size() == ConfigOUT["CurrentGain"].size())) {
      for(uint8_t n = 0; n < ConfigOUT["CurrentGain"].size(); n++) {
        if(config["CurrentGain"][n].is_number() && (config["CurrentGain"][n] > 0.0) && (config["CurrentGain"][n] <= 1000.0)) ConfigOUT["CurrentGain"][n] = config["CurrentGain"][n];
      }
    }
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_array() && (config["CurrentOffset"].size() == ConfigOUT["CurrentOffset"].size())) {
      for(uint8_t n = 0; n < ConfigOUT["CurrentOffset"].size(); n++) {
        if(config["CurrentOffset"][n].is_number() && (config["CurrentOffset"][n] >= -200.0) && (config["CurrentOffset"][n] <= 200.0)) ConfigOUT["CurrentOffset"][n] = config["CurrentOffset"][n];
      }
    }
    if(config.contains("CurrentDelay") && config["CurrentDelay"].is_array() && (config["CurrentDelay"].size() == ConfigOUT["CurrentDelay"].size())) {
      for(uint8_t n = 0; n < ConfigOUT["CurrentDelay"].size(); n++) {
        if(config["CurrentDelay"][n].is_number_unsigned() && (config["CurrentOffset"][n] <= 200.0)) ConfigOUT["CurrentDelay"][n] = config["CurrentDelay"][n];
      }
    }
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("HighGain") && config["HighGain"].is_boolean()) ConfigOUT["HighGain"] = config["HighGain"];
    if(config.contains("BlockDC") && config["BlockDC"].is_boolean()) ConfigOUT["BlockDC"] = config["BlockDC"];
    if(config.contains("Outputs") && config["Outputs"].is_array()) {
      uint8_t CurrentMask = 0x00;
      for(auto Tmp : config["Outputs"]) {
        if(!Tmp.is_object()) continue;
        json Output     = json::object();
        bool Error      = false;
        Output["Mode"]  = "1PHASE";
        Output["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
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
          Output["Voltage"] = {"CH1", "CH2", "CH3"};
          Output["Current"] = {"+CH1", "+CH2", "+CH3"};
          if(Output["Mode"] == "3PHASE+N") Output["Current"].push_back("+CH4");
          if(Tmp.contains("Voltage") && Tmp["Voltage"].is_array() && (Tmp["Voltage"].size() == Output["Voltage"].size()) && Tmp.contains("Current") && Tmp["Current"].is_array() && (Tmp["Current"].size() == Output["Current"].size())) {
            for(uint8_t i = 0; i < Output["Voltage"].size(); i++) {
              if(Tmp["Voltage"][i].is_string() && ((Tmp["Voltage"][i] == "CH1") || (Tmp["Voltage"][i] == "CH2") || (Tmp["Voltage"][i] == "CH3"))) Output["Voltage"][i] = Tmp["Voltage"][i];
            }
            for(uint8_t i = 0; i < Output["Current"].size(); i++) {
              if(Tmp["Current"][i].is_string() && ((Tmp["Current"][i] == "+CH1") || (Tmp["Current"][i] == "-CH1") || (Tmp["Current"][i] == "+CH2") || (Tmp["Current"][i] == "-CH2") || (Tmp["Current"][i] == "+CH3") || (Tmp["Current"][i] == "-CH3") || (Tmp["Current"][i] == "+CH4") || (Tmp["Current"][i] == "-CH4"))) Output["Current"][i] = Tmp["Current"][i];
            }
          }
          for(uint8_t i = 0; i < Output["Current"].size(); i++) {
            string CurrentCH = Output["Current"][i];
            uint8_t Channel = CurrentCH[3] - '1';
            if((CurrentMask & (1 << Channel)) != 0) Error = true;
            CurrentMask |= (1 << Channel);
          }
        }
        if(Tmp.contains("Analyzer") && Tmp["Analyzer"].is_object()) Output["Analyzer"] = AnalyzerSoft::CheckConfig(Tmp["Analyzer"], (Output["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (Output["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N, false);
        if(Error == false) ConfigOUT["Outputs"].push_back(Output);
      }
    }
    if(ConfigOUT["Outputs"].size() == 0) {
      json Output     = json::object();
      Output["Mode"]  = "1PHASE";
      Output["Voltage"] = "CH1";
      Output["Current"] = "+CH1";
      Output["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
      ConfigOUT["Outputs"].push_back(Output);
    }
    if(config.contains("Calculated") && config["Calculated"].is_object()) {
      if(config["Calculated"].contains("Analyzer") && config["Calculated"]["Analyzer"].is_object()) ConfigOUT["Calculated"]["Analyzer"] = AnalyzerSoft::CheckConfig(config["Calculated"]["Analyzer"], Analyzer::PHASE1, false);
      if(config["Calculated"].contains("Voltage") && config["Calculated"]["Voltage"].is_string() && ((config["Calculated"]["Voltage"] == "CH1") || (config["Calculated"]["Voltage"] == "CH2") || (config["Calculated"]["Voltage"] == "CH3"))) ConfigOUT["Calculated"]["Voltage"] = config["Calculated"]["Voltage"];
      vector<bool> AvaliableChannels = vector<bool>(4, false);
      for(auto Tmp : ConfigOUT["Outputs"]) {
        if((Tmp["Mode"] != "1PHASE") || (Tmp["Voltage"] != ConfigOUT["Calculated"]["Voltage"])) continue;
        string CurrentCH = Tmp["Current"];
        AvaliableChannels[CurrentCH[3] - '1'] = true;
      }
      uint8_t UsedCurrentChannels = 0;
      if(config["Calculated"].contains("Current") && config["Calculated"]["Current"].is_array() && (config["Calculated"]["Current"].size() == 4)) {
        for(uint8_t i = 0; i < 4; i++) {
          if(config["Calculated"]["Current"][i].is_string() && ((config["Calculated"]["Current"][i] == "ADD") || (config["Calculated"]["Current"][i] == "SUB") || (config["Calculated"]["Current"][i] == "UNUSED"))) {
            ConfigOUT["Calculated"]["Current"][i] = config["Calculated"]["Current"][i];
            if(AvaliableChannels[i] == false) ConfigOUT["Calculated"]["Current"][i] = "UNUSED";
            if(ConfigOUT["Calculated"]["Current"][i] != "UNUSED") UsedCurrentChannels++;
          }
        }
      }
      if((UsedCurrentChannels > 1) && config["Calculated"].contains("Enabled") && config["Calculated"]["Enabled"].is_boolean()) ConfigOUT["Calculated"]["Enabled"] = config["Calculated"]["Enabled"];
    }
  }
  return ConfigOUT;
}
void DeviceV2::Config(const json& config) {
  ClearAnalyzers();
  pthread_mutex_lock(&pConfigLock);
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  for(uint8_t Ch = 0; Ch < 3; Ch++) {
    pVoltageGain[Ch] = 0.110266113281 * config["VoltageGain"][Ch].get<float>();
    pVoltageOffset[Ch] = config["VoltageOffset"][Ch];
  }
  uint8_t  Delays[4];
  uint8_t  MaxDelay = 0;
  uint16_t DelayLen = 0;
  pHighGain = config["HighGain"];
  pBlockDC = config["BlockDC"];
  pEnabled = config["Enabled"];
  for(uint8_t Ch = 0; Ch < 4; Ch++) {
    pCurrentGain[Ch] = config["CurrentGain"][Ch].get<float>() * (pHighGain ? 0.08132749817 : 0.15165198897);
    pCurrentOffset[Ch] = config["CurrentOffset"][Ch];
    Delays[Ch] = config["CurrentDelay"][Ch];
    if(Delays[Ch] > MaxDelay) MaxDelay = Delays[Ch];
  }
  for(uint8_t Ch = 0; Ch < 7; Ch++) pChannelsDelayPos[Ch] = 0;
  for(uint8_t Ch = 0; Ch < 3; Ch++) pChannelsDelaySize[Ch] = MaxDelay + 1;
  for(uint8_t Ch = 0; Ch < 4; Ch++) pChannelsDelaySize[3 + Ch] = (MaxDelay + 1) - Delays[Ch];
  for(uint8_t Ch = 0; Ch < 7; Ch++) DelayLen = DelayLen + pChannelsDelaySize[Ch];
  pChannelsDelay = (int16_t*)realloc(pChannelsDelay, sizeof(int16_t) * DelayLen);
  memset(pChannelsDelay, 0, sizeof(int16_t) * DelayLen);
  for(auto &Tmp : config["Outputs"]) {
    Analyzer::AnalyzerMode  Mode = (Tmp["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (Tmp["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
    string                  Name = pSerial;
    uint8_t VoltageMap[3];
    uint8_t CurrentMap[4];
    bool    InvertMap[4];
    if(Mode == Analyzer::PHASE1) {
      string Voltage = Tmp["Voltage"];
      string Current = Tmp["Current"];
      VoltageMap[0] = (Voltage[2] - '1');
      CurrentMap[0] = (Current[3] - '1') + 3;
      InvertMap[0]  = (Current[0] == '+') ? false : true;
      Name          = Name + "_CH" + to_string(Current[3] - '0') + "_1P";
    } else {
      for(uint8_t Ch = 0; Ch < Tmp["Voltage"].size(); Ch++) {
        string Voltage = Tmp["Voltage"][Ch];
        VoltageMap[Ch] = (Voltage[2] - '1');
      }
      for(uint8_t Ch = 0; Ch < Tmp["Current"].size(); Ch++) {
        string Current =  Tmp["Current"][Ch];
        CurrentMap[Ch] = Current[3] - '1' + 3;
        InvertMap[Ch] = (Current[0] == '-');
      }
      Name = Name + ((Mode == Analyzer::PHASE3) ? "_3P" : "_3PN");
    }
    pAnalyzers.push_back(new DeviceV2Analyzer(Name, Mode, "/Devices/" + pSerial + "/Outputs/" + to_string(pAnalyzers.size()) + "/Analyzer", 24000.0, VoltageMap, CurrentMap, InvertMap));
    AnalyzerManager::AddAnalyzer(pAnalyzers.back());
  }
  if(config["Calculated"]["Enabled"]) {
    string Voltage = config["Calculated"]["Voltage"];
    float CurrentMap[4];
    for(uint8_t Ch = 0; Ch < 4; Ch++) CurrentMap[Ch] = (config["Calculated"]["Current"][Ch] == "UNUSED") ? 0.0 : (config["Calculated"]["Current"][Ch] == "ADD") ? 1.0 : -1.0;
    pAnalyzers.push_back(new DeviceV2Analyzer(pSerial + "_CAL_1P", "/Devices/" + pSerial + "/Calculated/Analyzer", 24000.0, Voltage[2] - '1', CurrentMap));
    AnalyzerManager::AddAnalyzer(pAnalyzers.back());
  }
  pthread_mutex_unlock(&pConfigLock);
}
void *DeviceV2::Thread() {
  uint64_t     LastTime       = 0;
  USBTransfer *SignalTransfer = new USBTransfer(this, 2);
  USBTransfer *Transfers[DEVICEV2_TRANSFERS];
  for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) Transfers[n] = new USBTransfer(this, 8192);
  while((pTerminate == false) || (pSubmittedTransfers > 0)) {
    struct timeval tv = { 0, 10000 };
    int Result = libusb_handle_events_timeout_completed(pContext, &tv, NULL);
    if(Result < 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
    if(pTerminate == true) continue;
    if(pDeviceHandle != NULL) {
      uint64_t Time = Tools::GetTime64();
      if((Time - 1000) > LastTime) {
        LastTime = Time;
        uint16_t Status = 0x0000;
        if(pEnabled == false) {
          Status |= 0x0001;
        } else {
          bool Online = true;
          for(auto &Tmp : pAnalyzers) {
            if(Tmp->State() != Analyzer::ONLINE) Online = false;
          }
          Status |= (Online ? 0x0002 : 0x0003);
        }
        WifiManager::WifiStat WifiStat = WifiManager::Enabled();
        if(WifiStat == WifiManager::CONFIGURED) {
          Status |= 0x0004;
        } else if(WifiStat == WifiManager::CONNECTED) {
          Status |= 0x000C;
        } else if(WifiStat == WifiManager::INTERNET) {
          Status |= 0x0008;
        }
        if(BluetoothManager::Enabled()) Status |= 0x0010;
        if((Time - pLastEvent) < 2000) Status |= 0x0020;
        if(pHighGain == true) Status |= 0x4000;
        SignalTransfer->Submit(&Status, sizeof(Status));
        pSubmittedTransfers++;
      }
    }
    if(pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(pNewDevice, &Descriptor);
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        pNewDevice = NULL;
        continue;
      }
      Result = libusb_open(pNewDevice, &pDeviceHandle);
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(pDeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      if(pSerial != Serial) {
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      Result = libusb_kernel_driver_active(pDeviceHandle, 0);
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      } else if(Result == 1) {
        Result = libusb_detach_kernel_driver(pDeviceHandle, 0);
        if(Result < 0) {
          LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
          libusb_close(pDeviceHandle);
          pDeviceHandle = NULL;
          pNewDevice = NULL;
          continue;
        }
      }
      Result = libusb_claim_interface(pDeviceHandle, 0);
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      pCaptureSize = 0xFF;
      SignalTransfer->Setup(pDeviceHandle, DEVICEV2_ENDPOINT_OUT, [](struct libusb_transfer *transfer) { ((DeviceV2*)transfer->user_data)->ReadCallback(transfer); }, -1);
      for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) {
        Transfers[n]->Submit(pDeviceHandle, DEVICEV2_ENDPOINT_IN, [](struct libusb_transfer *transfer) { ((DeviceV2*)transfer->user_data)->ReadCallback(transfer); }, -1);
        pSubmittedTransfers++;
      }
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      LogFile::Info("DeviceV2 with serial '%s' connected", pSerial.c_str());
      pNewDevice = NULL;
      LastTime = 0;
    }
  }
  if(pDeviceHandle != NULL) libusb_release_interface(pDeviceHandle, 0);
  if(pDeviceHandle != NULL) libusb_close(pDeviceHandle);
  delete SignalTransfer;
  for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) delete Transfers[n];
  libusb_exit(pContext);
  return NULL;
}
void DeviceV2::ReadCallback(struct libusb_transfer *transfer) {
  bool StatusRecv = false;
  if((transfer->endpoint == DEVICEV2_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    uint16_t *Samples = (uint16_t*)transfer->buffer;
    pthread_mutex_lock(&pConfigLock);
    for(int Pos = 0; Pos < (transfer->actual_length / 2); Pos++) {
      uint8_t  Seq    = Samples[Pos] >> 13;
      uint16_t Sample = (Samples[Pos] & 0x1FFF);
      if(Seq == 7) {
        if(StatusRecv == true) continue;
        uint8_t Action  = (Sample & 0x0300) >> 8;
        if((Action == CLICK) && (pButtonAction == false)) {
          MessageLoop::SendMessage("Bluetooth_Toogle");
        } else if((Action == LONG_CLICK) && (pButtonAction == false)) {
          MessageLoop::SendMessage("PowerOff");
        }
//        pBatteryVoltage = 0;
//        pCharging       = false;
//        pMainsPowered   = false;
        pButtonAction = (Action != NONE);
        StatusRecv = true;
      } else if(pCaptureSize == Seq) {
        pCapture[pCaptureSize] = Sample - 4096;
        if(pCaptureSize == 6) {
          for(uint8_t Ch = 0; Ch < 7; Ch++) pRunAverageValues[Ch] += pCapture[Ch];
          pRunAverageCounter++;
          if(pRunAverageCounter == 240000) {
            pthread_rwlock_wrlock(&pAverageLocks);
            for(uint8_t Ch = 0; Ch < 7; Ch++) {
              pAverageValues[Ch] = pRunAverageValues[Ch] / 240000.0;
              pRunAverageValues[Ch] = 0;
            }
            pthread_rwlock_unlock(&pAverageLocks);
            pRunAverageCounter = 0;
          }
          if(pEnabled == true) {
            float Values[7];
            int16_t *DelayPtr = pChannelsDelay;
            for(uint8_t Ch = 0; Ch < 7; Ch++) {
              Values[Ch] = DelayPtr[pChannelsDelayPos[Ch]];
              DelayPtr[pChannelsDelayPos[Ch]] = pCapture[Ch];
              pChannelsDelayPos[Ch] = (pChannelsDelayPos[Ch] + 1) % pChannelsDelaySize[Ch];
              DelayPtr = &DelayPtr[pChannelsDelaySize[Ch]];
            }
            if(pBlockDC) { //Si el filtro DC esta activado
              Values[0] = Values[0] * pVoltageGain[0];
              Values[1] = Values[1] * pVoltageGain[1];
              Values[2] = Values[2] * pVoltageGain[2];
              Values[3] = Values[3] * pCurrentGain[0];
              Values[4] = Values[4] * pCurrentGain[1];
              Values[5] = Values[5] * pCurrentGain[2];
              Values[6] = Values[6] * pCurrentGain[3];
              for(uint8_t Ch = 0; Ch < 7; Ch++) {
                float In = Values[Ch];
                Values[Ch] = In - pFilterZ1[Ch] + 0.99999 * pFilterZ2[Ch];
                pFilterZ1[Ch] = In;
                pFilterZ2[Ch] = Values[Ch];
              }
            } else {
              Values[0] = (Values[0] + pVoltageOffset[0]) * pVoltageGain[0];
              Values[1] = (Values[1] + pVoltageOffset[1]) * pVoltageGain[1];
              Values[2] = (Values[2] + pVoltageOffset[2]) * pVoltageGain[2];
              Values[3] = (Values[3] + pCurrentOffset[0]) * pCurrentGain[0];
              Values[4] = (Values[4] + pCurrentOffset[1]) * pCurrentGain[1];
              Values[5] = (Values[5] + pCurrentOffset[2]) * pCurrentGain[2];
              Values[6] = (Values[6] + pCurrentOffset[3]) * pCurrentGain[3];
            }
            for(auto &Tmp : pAnalyzers) Tmp->BufferAppend(Values);
          }
          pCaptureSize = 0;
        } else {
          pCaptureSize++;
        }
      } else {
        if((Seq == 0) && (pCaptureSize > 0x10)) {
          pCaptureSize--;
        } else if((Seq == 0) && (pCaptureSize == 0x10)) {
          pCapture[0] = Sample - 4096;
          pCaptureSize = 1;
        } else {
          if(pCaptureSize < 0x10) LogFile::Warning("Lost sample, restarting adquisition");
          pCaptureSize = 0;
          break;
        }
      }
    }
    pthread_mutex_unlock(&pConfigLock);
  }
  if((pTerminate == false) && (transfer->endpoint == DEVICEV2_ENDPOINT_IN)) {
    libusb_submit_transfer(transfer);
  } else {
    pSubmittedTransfers--;
  }
}
int DeviceV2::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event) {
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (pDeviceHandle == NULL)) {
    pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(pDeviceHandle);
      if(dev == CurrentDev) {
        LogFile::Info("DeviceV2 with serial '%s' disconnected", pSerial.c_str());
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void DeviceV2::GetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  for(uint8_t n = 0; n < Config["Outputs"].size(); n++) Config["Outputs"][n].erase("Analyzer");
  pthread_rwlock_rdlock(&pAverageLocks);
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = json::array();
  Config["Wave_Average"]["Voltage"].push_back(pAverageValues[0]);
  Config["Wave_Average"]["Voltage"].push_back(pAverageValues[1]);
  Config["Wave_Average"]["Voltage"].push_back(pAverageValues[2]);
  Config["Wave_Average"]["Current"] = json::array();
  Config["Wave_Average"]["Current"].push_back(pAverageValues[3]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[4]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[5]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[6]);
  pthread_rwlock_unlock(&pAverageLocks);
  pthread_mutex_lock(&pConfigLock);
//  Config["MainsPowered"] = pMainsPowered;
//  Config["Charging"] = pCharging;
//  Config["BatteryVoltage"] = pBatteryVoltage;
  pthread_mutex_unlock(&pConfigLock);
  req.Success(Config);
}
void  DeviceV2::SetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json ConfigIN = req.Params();
  json Orig = ConfigFile::ReadConfig("/Devices/" + pSerial);
  if(ConfigIN.contains("Outputs") && Orig.contains("Outputs") && ConfigIN["Outputs"].is_array() && Orig["Outputs"].is_array()) {
    for(uint8_t n = 0; n < ConfigIN["Outputs"].size(); n++) {
      if(n < Orig["Outputs"].size() && (ConfigIN["Outputs"][n].contains("Mode") && Orig["Outputs"][n].contains("Mode")) && Orig["Outputs"][n].contains("Analyzer") && (ConfigIN["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"])) ConfigIN["Outputs"][n]["Analyzer"] = Orig["Outputs"][n]["Analyzer"];
    }
  }
  Config(CheckConfig(ConfigIN));
  req.Success(json::object());
}
//----------------------------------------------------------------------------------------------------------------------