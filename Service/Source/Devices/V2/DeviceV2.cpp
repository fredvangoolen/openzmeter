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
#include "AnalyzerManager.h"
#include "DeviceManager.h"
#include "ConfigFile.h"
#include "LogFile.h"
#include "Tools.h"
#include "MessageLoop.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV2::pRegistered = DeviceV2::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV2::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceV2::GetInstances() {
  libusb_context *Context;
  if(libusb_init(&Context) < 0) {
    LogFile::Error("DeviceV2", "Can not open libUSB...");
    return;
  }
  libusb_device **Devs;
  int Count = libusb_get_device_list(NULL, &Devs);
  if(Count < 0) {
    LogFile::Error("DeviceV2", "Can not enumerate USB devices...");
    return;
  }
  for(int i = 0; i < Count; ++i) {
    libusb_device_descriptor Descriptor;
    if(libusb_get_device_descriptor(Devs[i], &Descriptor) < 0) {
      LogFile::Error("DeviceV2", "Can not load USB descriptor...");
      return;
    }
    if(Descriptor.idVendor != DEVICEV2_VID) continue;
    if(Descriptor.idProduct != DEVICEV2_PID) continue;
    if(Descriptor.bcdDevice != DEVICEV2_VERSION) continue;
    libusb_device_handle *Dev_Handle = NULL;
    if(libusb_open(Devs[i], &Dev_Handle) != LIBUSB_SUCCESS) {
      LogFile::Error("DeviceV2", "Can not open USB device...\n");
      continue;
    }
    char String[256];
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iManufacturer, (unsigned char*)String, sizeof(String)) < 0) {
      LogFile::Error("DeviceV2", "Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV2_MANSTRING) != 0) continue;
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String)) < 0) {
      LogFile::Error("DeviceV2", "Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV2_PRODSTRING) != 0) continue;
    if(libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iSerialNumber, (unsigned char*)String, sizeof(String)) < 0) {
      LogFile::Error("DeviceV2", "Can not load USB string...\n");
      libusb_close(Dev_Handle);
      continue;
    }
    libusb_close(Dev_Handle);
    LogFile::Info("DeviceV2", "Adding 'V2' device driver instance with serial '%s'", String);
    DeviceManager::AddDevice(new DeviceV2(String));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
}
DeviceV2::DeviceV2(const string &serial) : Device(serial, "V2", "Capture device V2") {
  pBuffer             = (int16_t*)malloc(DEVICEV2_BUFFERSIZE * sizeof(uint16_t) * 7);
  pAverageLocks       = PTHREAD_RWLOCK_INITIALIZER;
  pConfigLock         = PTHREAD_MUTEX_INITIALIZER;
  pThread             = 0;
  pBufferHead         = 0;
  pBufferTail         = 0;
  pChannelsDelay      = NULL;
  pButtonAction       = false;
  pDeviceHandle       = NULL;
  pNewDevice          = NULL;
  pSubmittedTransfers = 0;
  pRunAverageCounter  = 0;
  pTerminate          = false;
  pCaptureSize        = 0xFF;
  pAnalyzersCount     = 0;
  memset(pAnalyzers, 0, sizeof(pAnalyzers));
  memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
  memset(pAverageValues, 0 , sizeof(pAverageValues));
  Configure(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  if(libusb_init(&pContext) < 0) {
    LogFile::Error(pSerial.c_str(), "Can not open libUSB...\n");
    return;
  }
  if(pthread_create(&pThread, NULL, [](void *args) -> void* { return ((DeviceV2*)args)->Thread(); }, this) != 0) {
    LogFile::Error(pSerial.c_str(), "%s:%d -> %s\n", __FILE__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return;
  }
  pthread_setname_np(pThread, "DeviceV2");
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE, DEVICEV2_VID, DEVICEV2_PID, LIBUSB_HOTPLUG_MATCH_ANY, [](struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) { return ((DeviceV2*)user_data)->PlugCallback(ctx, dev, event); }, this, NULL) < 0) {
    LogFile::Error(pSerial.c_str(), "Failed to register libUSB calback...\n");
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    libusb_exit(pContext);
  }
}
uint32_t DeviceV2::AnalyzersCount() {
  pthread_mutex_lock(&pConfigLock);
  uint32_t Count = pAnalyzersCount;
  pthread_mutex_unlock(&pConfigLock);
  return Count;
}
void DeviceV2::ClearAnalyzers() {
  pthread_mutex_lock(&pConfigLock);
  pRunningAnalyzers.clear();
  for(uint8_t n = 0; n < pAnalyzersCount; n++) {
    AnalyzerManager::DelAnalyzer(pAnalyzers[n]->Serial());
    delete pAnalyzers[n];
    pAnalyzers[n] = NULL;
  }
  pAnalyzersCount = 0;
  pthread_mutex_unlock(&pConfigLock);
}
DeviceV2::~DeviceV2() {
  LogFile::Info(pSerial.c_str(), "Stopping device with serial '%s'", pSerial.c_str());
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  if(pBuffer != NULL) free(pBuffer);
  if(pChannelsDelay != NULL) free(pChannelsDelay);
  LogFile::Info(pSerial.c_str(), "Device with serial '%s' stopped", pSerial.c_str());
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
          Output["Voltage"] = json::parse("[\"CH1\", \"CH2\", \"CH3\"]");
          Output["Current"] = ((Output["Mode"] == "3PHASE") ? json::parse("[\"+CH1\", \"+CH2\", \"+CH3\"]") : json::parse("[\"+CH1\", \"+CH2\", \"+CH3\", \"+CH4\"]"));
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
void DeviceV2::Configure(const json& config) {
  ClearAnalyzers();
  pthread_mutex_lock(&pConfigLock);
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  for(uint8_t Ch = 0; Ch < 3; Ch++) {
    pVoltageGain[Ch] = config["VoltageGain"][Ch];
    pVoltageOffset[Ch] = config["VoltageOffset"][Ch];
  }
  uint8_t  MaxDelay = 0;
  uint16_t DelayLen = 0;
  uint8_t  Delays[4];
  for(uint8_t Ch = 0; Ch < 4; Ch++) {
    pCurrentGain[Ch] = config["CurrentGain"][Ch];
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
  pHighGain = config["HighGain"];
  pEnabled = config["Enabled"];
  for(auto Tmp : config["Outputs"]) {
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
      Name          = Name + "_1P_CH" + to_string(Current[3] - '0');
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
    pAnalyzers[pAnalyzersCount] = new DeviceV2Analyzer(Name, Mode, "/Devices/" + pSerial + "/Outputs/" + to_string(pAnalyzersCount) + "/Analyzer", DEVICEV2_SAMPLEFREQ, VoltageMap, CurrentMap, InvertMap);
    AnalyzerManager::AddAnalyzer(pAnalyzers[pAnalyzersCount]);
    pAnalyzersCount++;
  }
  if(config["Calculated"]["Enabled"]) {
    string Voltage = config["Calculated"]["Voltage"];
    float CurrentMap[4];
    for(uint8_t Ch = 0; Ch < 4; Ch++) CurrentMap[Ch] = (config["Calculated"]["Current"][Ch] == "UNUSED") ? 0.0 : (config["Calculated"]["Current"][Ch] == "ADD") ? 1.0 : -1.0;
    pAnalyzers[pAnalyzersCount] = new DeviceV2Analyzer(pSerial + "_1P_CAL", "/Devices/" + pSerial + "/Calculated/Analyzer", DEVICEV2_SAMPLEFREQ, Voltage[2] - '1', CurrentMap);
    AnalyzerManager::AddAnalyzer(pAnalyzers[pAnalyzersCount]);
    pAnalyzersCount++;
  }
  pthread_mutex_unlock(&pConfigLock);
}
void *DeviceV2::Thread() {
  nice(-20);
  uint8_t      TimeCounter    = 0;
  USBTransfer *SignalTransfer = new USBTransfer(this, 2);
  USBTransfer *Transfers[DEVICEV2_TRANSFERS];
  for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) Transfers[n] = new USBTransfer(this, 8192);
  while((pTerminate == false) || (pSubmittedTransfers > 0)) {
    struct timeval tv = { 0, 10000 };
    int Result = libusb_handle_events_timeout_completed(pContext, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) LogFile::Error(pSerial.c_str(), "libusb_handle_events_timeout_completed returns code %d", Result);
    if(pTerminate == true) continue;
    if((pDeviceHandle != NULL) && (TimeCounter++ > 50)) {
      TimeCounter = 0;
      uint16_t Status = 0x0006; //Stat=GREEN y Wifi=RED
//      SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
//      SysStatus->Lock.ReadLock();
//      if(SysStatus->RAM_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;              //Stat=YELLOW
//      if(SysStatus->CPU_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;              //Stat=YELLOW
//      if(SysStatus->Disk_Usage > 90.0) Status = (Status & 0xFFFC) | 0x0003;             //Stat=YELLOW
//      if(SysStatus->Time_IsSet == false) Status = (Status & 0xFFFC) | 0x0001;           //Stat=RED
//      if(SysStatus->Events == true) Status = (Status & 0xC01C) | 0x0021;                //Stat=RED and short BEEP
//      if(SysStatus->WIFI_Network.empty() == false) Status = (Status & 0xFFF3) | 0x000C; //Wifi=GREEN
//      if(SysStatus->BT_Network.empty() == false) Status = (Status & 0xFFEF) | 0x0010;   //Bt=BLUE
//      SysStatus->Lock.Unlock();
      Status |= (pHighGain == true) ? 0x4000 : 0x0000;
      SignalTransfer->Submit(&Status, sizeof(Status));
      pSubmittedTransfers++;
    }
    if(pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(pNewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        LogFile::Error(pSerial.c_str(), "libusb_get_device_descriptor return code %d", Result);
        pNewDevice = NULL;
        continue;
      }
      Result = libusb_open(pNewDevice, &pDeviceHandle);
      if(Result != LIBUSB_SUCCESS) {
        LogFile::Error(pSerial.c_str(), "libusb_open return %d", Result);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      char Serial[256];
      Result = libusb_get_string_descriptor_ascii(pDeviceHandle, Descriptor.iSerialNumber, (unsigned char*)Serial, sizeof(Serial));
      if(Result < 0) {
        LogFile::Error(pSerial.c_str(), "libusb_get_string_descriptor_ascii returns %d", Result);
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
      if(Result < LIBUSB_SUCCESS) {
        LogFile::Error(pSerial.c_str(), "libusb_kernel_driver_active returns %d", Result);
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      } else if(Result == 1) {
        Result = libusb_detach_kernel_driver(pDeviceHandle, 0);
        if(Result < LIBUSB_SUCCESS) {
          LogFile::Error(pSerial.c_str(), "libusb_detach_kernel_driver returns %d", Result);
          libusb_close(pDeviceHandle);
          pDeviceHandle = NULL;
          pNewDevice = NULL;
          continue;
        } else {
          LogFile::Info(pSerial.c_str(), "Kernel driver dettached for DeviceV2 with serial '%s'", pSerial.c_str());
        }
      }
      Result = libusb_claim_interface(pDeviceHandle, 0);
      if(Result != LIBUSB_SUCCESS) {
        LogFile::Error(pSerial.c_str(), "libusb_claim_interface returns %d", Result);
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      } else {
        LogFile::Info(pSerial.c_str(), "Claimed interface 0 for DeviceV2 with serial '%s'", pSerial.c_str());
      }
      pCaptureSize = 0xFF;
      SignalTransfer->Setup(pDeviceHandle, DEVICEV2_ENDPOINT_OUT, [](struct libusb_transfer *transfer) { ((DeviceV2*)transfer->user_data)->ReadCallback(transfer); }, 1000);
      for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) {
        Transfers[n]->Submit(pDeviceHandle, DEVICEV2_ENDPOINT_IN, [](struct libusb_transfer *transfer) { ((DeviceV2*)transfer->user_data)->ReadCallback(transfer); }, -1);
        pSubmittedTransfers++;
      }
      if(Result != LIBUSB_SUCCESS) {
        LogFile::Error(pSerial.c_str(), "libusb_submit_transfer returns %d", Result);
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      LogFile::Info(pSerial.c_str(), "Capture started on DeviceV2 with serial '%s'", pSerial.c_str());
      pNewDevice = NULL;
      TimeCounter = 0;
    }
  }
  if(pDeviceHandle != 0) libusb_release_interface(pDeviceHandle, 0);
  if(pDeviceHandle != 0) libusb_close(pDeviceHandle);
  free(SignalTransfer);
  for(uint8_t n = 0; n < DEVICEV2_TRANSFERS; n++) free(Transfers[n]);
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
        float Battery_Volt = 2.96 + ((Sample & 0x00FC) >> 2) * 0.02;
//        LogFile::SetValue("Devices", "/Battery_Voltage", Battery_Volt);
//        float Battery_Level  = (Battery_Volt - 2.96) * 80.65;
//        Status::SetValue("/Battery_Level", (Battery_Level > 100.0) ? 100.0 : Battery_Level);
//        Status::SetValue("/Charging", (Sample & 0x0002) ? true: false);
//        Status::SetValue("/Powered", (Sample & 0x0001) ? true: false);
        uint8_t Action = (Sample & 0x0300) >> 8;
        if((Action == CLICK) && (pButtonAction == false)) {
          MessageLoop::SendMessage("Bluetooth_Toogle");
        } else if((Action == LONG_CLICK) && (pButtonAction == false)) {
          MessageLoop::SendMessage("PowerOff");
        }
        pButtonAction = (Action == NONE);
        StatusRecv = true;
      } else if(pCaptureSize == Seq) {
        pCapture[pCaptureSize] = Sample - 4096;
        if(pCaptureSize == 6) {
          BufferAppend();
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
          if(pCaptureSize < 0x10) LogFile::Warning(pSerial.c_str(), "Lost sample, restarting adquisition");
          pCaptureSize = 0;
          break;
        }
      }
    }
    pthread_mutex_unlock(&pConfigLock);
  }
  if(pTerminate == false) {
    if(transfer->endpoint == DEVICEV2_ENDPOINT_IN) {
      libusb_submit_transfer(transfer);
    } else if(transfer->endpoint == DEVICEV2_ENDPOINT_OUT) {
      pSubmittedTransfers--;
    }
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
void DeviceV2::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
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
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void  DeviceV2::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
  json Orig = ConfigFile::ReadConfig("/Devices/" + pSerial);
  if(Config.contains("Outputs") && Orig.contains("Outputs") && Config["Outputs"].is_array() && Orig["Outputs"].is_array()) {
    for(uint8_t n = 0; n < Config["Outputs"].size(); n++) {
      if(n < Orig["Outputs"].size() && (Config["Outputs"][n].contains("Mode") && Orig["Outputs"][n].contains("Mode")) && Orig["Outputs"][n].contains("Analyzer") && (Config["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"])) Config["Outputs"][n]["Analyzer"] = Orig["Outputs"][n]["Analyzer"];
    }
  }
  Configure(CheckConfig(Config));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void DeviceV2::BufferAppend() {
  for(uint8_t Ch = 0; Ch < 7; Ch++) pRunAverageValues[Ch] += pCapture[Ch];
  pRunAverageCounter++;
  if(pRunAverageCounter == (DEVICEV2_SAMPLEFREQ * 10)) {
    pthread_rwlock_wrlock(&pAverageLocks);
    for(uint8_t Ch = 0; Ch < 7; Ch++) {
      pAverageValues[Ch] = pRunAverageValues[Ch] / (DEVICEV2_SAMPLEFREQ * 10.0);
      pRunAverageValues[Ch] = 0;
    }
    pthread_rwlock_unlock(&pAverageLocks);
    pRunAverageCounter = 0;
  }
  if(pEnabled == false) return;
  uint32_t Next = (pBufferHead + 1) % DEVICEV2_BUFFERSIZE;
  if(Next != pBufferTail) {
    memcpy(&pBuffer[pBufferHead * 7], pCapture, sizeof(pCapture));
    pBufferHead = Next;
  } else {
    pBufferHead = 0;
    pBufferTail = 0;
    LogFile::Error("Capture buffer overflow in device '%s'", pSerial.c_str());
  }
  while(pRunningAnalyzers.size() > 0) {
    DeviceV2Analyzer *Analyzer = pRunningAnalyzers.front();
    if(Analyzer->IsWorking() == true) return;
    uint64_t LastEvent = Analyzer->GetLastEvent();
    if(LastEvent > pLastEvent) pLastEvent = LastEvent;
    pRunningAnalyzers.pop_front();
  }
  for(uint8_t Max = 0; (Max < 2) && (pBufferTail != pBufferHead) && (pRunningAnalyzers.size() == 0); Max++) {
    float Values[7];
    int16_t *DelayPtr = pChannelsDelay;
    int16_t *Ptr      = &pBuffer[pBufferTail * 7];
    for(uint8_t Ch = 0; Ch < 7; Ch++) {
      Values[Ch] = DelayPtr[pChannelsDelayPos[Ch]];
      DelayPtr[pChannelsDelayPos[Ch]] = Ptr[Ch];
      pChannelsDelayPos[Ch] = (pChannelsDelayPos[Ch] + 1) % pChannelsDelaySize[Ch];
      DelayPtr = &DelayPtr[pChannelsDelaySize[Ch]];
    }
    uint32_t Used = (pBufferHead > pBufferTail) ? pBufferHead - pBufferTail : DEVICEV2_BUFFERSIZE - pBufferTail + pBufferHead;
    pBufferTail = (pBufferTail + 1) % DEVICEV2_BUFFERSIZE;
    Values[0] = (Values[0] + pVoltageOffset[0]) * 0.110266113281 * pVoltageGain[0];
    Values[1] = (Values[1] + pVoltageOffset[1]) * 0.110266113281 * pVoltageGain[1];
    Values[2] = (Values[2] + pVoltageOffset[2]) * 0.110266113281 * pVoltageGain[2];
    Values[3] = (Values[3] + pCurrentOffset[0]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[0];
    Values[4] = (Values[4] + pCurrentOffset[1]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[1];
    Values[5] = (Values[5] + pCurrentOffset[2]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[2];
    Values[6] = (Values[6] + pCurrentOffset[3]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[3];
    for(uint8_t n = 0; n < pAnalyzersCount; n++) {
      if(pAnalyzers[n]->BufferAppend(Values, Used) == true) pRunningAnalyzers.push_back(pAnalyzers[n]);
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
