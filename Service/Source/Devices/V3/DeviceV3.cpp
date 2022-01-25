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
#include "DeviceV3.h"
#include "DeviceManager.h"
#include "ConfigFile.h"
#include "AnalyzerManager.h"
#include "Tools.h"
#include "MessageLoop.h"
#include "BluetoothManager.h"
#include "WifiManager.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV3::pRegistered = DeviceV3::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV3::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceV3::GetInstances() {
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
    if(Descriptor.idVendor != DEVICEV3_VID) continue;
    if(Descriptor.idProduct != DEVICEV3_PID) continue;
    if(Descriptor.bcdDevice != DEVICEV3_VERSION) continue;
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
    if(strcmp(String, DEVICEV3_MANSTRING) != 0) {
      libusb_close(Dev_Handle);
      continue;
    }
    Error = libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String));
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV3_PRODSTRING) != 0) {
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
    DeviceManager::AddDevice(new DeviceV3(String));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
}
DeviceV3::DeviceV3(const string &serial) : Device(serial, "V3", "Capture device V3 (Solar)") {
  LogFile::Info("Starting DeviceV3 with serial '%s' ...", serial.c_str());
  pAverageLocks       = PTHREAD_RWLOCK_INITIALIZER;
  pConfigLock         = PTHREAD_MUTEX_INITIALIZER;
  pThread             = 0;
  pChannelsDelay      = NULL;
  pDeviceHandle       = NULL;
  pNewDevice          = NULL;
  pSubmittedTransfers = 0;
  pRunAverageCounter  = 0;
  pAnalyzersCount     = 0;
  pTerminate          = false;
  pCaptureSize        = 0xFF;
  memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
  memset(pAverageValues, 0 , sizeof(pAverageValues));
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  int Error = libusb_init(&pContext);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    return;
  }
  if(pthread_create(&pThread, NULL, [](void *args) -> void* { return ((DeviceV3*)args)->Thread(); }, this) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return;
  }
  if(pthread_setname_np(pThread, "DeviceV3") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  TaskPool::SetThreadProperties(pThread, TaskPool::HIGHEST);
  Error = libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE, DEVICEV3_VID, DEVICEV3_PID, LIBUSB_HOTPLUG_MATCH_ANY, [](struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) { return ((DeviceV3*)user_data)->PlugCallback(ctx, dev, event); }, this, NULL);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    libusb_exit(pContext);
  }
  LogFile::Info("DeviceV3 with serial '%s' started", serial.c_str());
}
uint32_t DeviceV3::AnalyzersCount() {
  pthread_mutex_lock(&pConfigLock);
  uint32_t Count = pAnalyzersCount;
  pthread_mutex_unlock(&pConfigLock);
  return Count;
}
void DeviceV3::ClearAnalyzers() {
  pthread_mutex_lock(&pConfigLock);
  for(uint8_t n = 0; n < pAnalyzersCount; n++) {
    AnalyzerManager::DelAnalyzer(pAnalyzers[n]->Serial());
    delete pAnalyzers[n];
  }
  pAnalyzersCount = 0;
  pthread_mutex_unlock(&pConfigLock);
}
DeviceV3::~DeviceV3() {
  LogFile::Info("Stopping DeviceV3 with serial '%s'...", pSerial.c_str());
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  if(pChannelsDelay != NULL) free(pChannelsDelay);
  LogFile::Info("DeviceV3 with serial '%s' stopped", pSerial.c_str());
}
json DeviceV3::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = 100.0;
  ConfigOUT["VoltageOffset"] = 0.0;
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Outputs"] = json::array();
  for(uint8_t n = 0; n < 8; n++) {
    json Output = json::object();
    Output["CurrentGain"] = 100.0;
    Output["CurrentOffset"] = 0.0;
    Output["Enabled"] = false;
    Output["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
    ConfigOUT["Outputs"].push_back(Output);
  }
  ConfigOUT["Calculated"] = json::object();
  ConfigOUT["Calculated"]["Enabled"]  = false;
  ConfigOUT["Calculated"]["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_number() && (config["VoltageGain"] > 90.0) && (config["VoltageGain"] <= 110.0)) ConfigOUT["VoltageGain"] = config["VoltageGain"];
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_number() && (config["VoltageOffset"] >= -200.0) && (config["VoltageOffset"] <= 200.0)) ConfigOUT["VoltageOffset"] = config["VoltageOffset"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Outputs") && config["Outputs"].is_array() && (config["Outputs"].size() == 8)) {
      for(uint8_t n = 0; n < 8; n++) {
        if(!config["Outputs"][n].is_object()) continue;
        if(config["Outputs"][n].contains("CurrentGain") && config["Outputs"][n]["CurrentGain"].is_number() && (config["Outputs"][n]["CurrentGain"] > 90.0) && (config["Outputs"][n]["CurrentGain"] <= 110.0)) ConfigOUT["Outputs"][n]["CurrentGain"] = config["Outputs"][n]["CurrentGain"];
        if(config["Outputs"][n].contains("CurrentOffset") && config["Outputs"][n]["CurrentOffset"].is_number() && (config["Outputs"][n]["CurrentOffset"] >= -200.0) && (config["Outputs"][n]["CurrentOffset"] <= 200.0)) ConfigOUT["Outputs"][n]["CurrentOffset"] = config["Outputs"][n]["CurrentOffset"];
        if(config["Outputs"][n].contains("Enabled") && config["Outputs"][n]["Enabled"].is_boolean()) ConfigOUT["Outputs"][n]["Enabled"] = config["Outputs"][n]["Enabled"];
        if(config["Outputs"][n].contains("Analyzer") && config["Outputs"][n]["Analyzer"].is_object()) ConfigOUT["Outputs"][n]["Analyzer"] = AnalyzerSoft::CheckConfig(config["Outputs"][n]["Analyzer"], Analyzer::PHASE1, false);
      }
    }
    if(config.contains("Calculated") && config["Calculated"].is_object()) {
      if(config["Calculated"].contains("Analyzer") && config["Calculated"]["Analyzer"].is_object()) ConfigOUT["Calculated"]["Analyzer"] = AnalyzerSoft::CheckConfig(config["Calculated"]["Analyzer"], Analyzer::PHASE1, false);
      if(config["Calculated"].contains("Enabled") && config["Calculated"]["Enabled"].is_boolean()) ConfigOUT["Calculated"]["Enabled"]= config["Calculated"]["Enabled"];
    }
  }
  return ConfigOUT;
}
void DeviceV3::Config(const json& config) {
  ClearAnalyzers();
  pthread_mutex_lock(&pConfigLock);
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  pVoltageGain = config["VoltageGain"];
  pVoltageOffset = config["VoltageOffset"];
  pEnabled = config["Enabled"];
  for(uint8_t n = 0; n < 8; n++) {
    if(config["Outputs"][n]["Enabled"] == false) {
      pCurrentGain[n] = 0.0;
      pCurrentOffset[n] = 0.0;
    } else {
      pCurrentGain[n]   = config["Outputs"][n]["CurrentGain"];
      pCurrentOffset[n] = config["Outputs"][n]["CurrentOffset"];
      pAnalyzers[pAnalyzersCount] = new DeviceV3Analyzer(pSerial + "_CH" + to_string(n + 1) + "_1P", "/Devices/" + pSerial + "/Outputs/" + to_string(n) + "/Analyzer", n);
      AnalyzerManager::AddAnalyzer(pAnalyzers[pAnalyzersCount]);
      pAnalyzersCount++;
    }
  }
  if(config["Calculated"]["Enabled"]) {
    pAnalyzers[pAnalyzersCount] = new DeviceV3Analyzer(pSerial + "_CAL_1P", "/Devices/" + pSerial + "/Calculated/Analyzer");
    AnalyzerManager::AddAnalyzer(pAnalyzers[pAnalyzersCount]);
    pAnalyzersCount++;
  }
  pthread_mutex_unlock(&pConfigLock);
}
void *DeviceV3::Thread() {
  USBTransfer *Transfers[DEVICEV3_TRANSFERS];
  for(uint8_t n = 0; n < DEVICEV3_TRANSFERS; n++) Transfers[n] = new USBTransfer(this, 4096);
  while((pTerminate == false) || (pSubmittedTransfers > 0)) {
    struct timeval tv = { 0, 10000 };
    int Result = libusb_handle_events_timeout_completed(pContext, &tv, NULL);
    if(Result < 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
    if(pTerminate == true) continue;
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
      for(uint8_t n = 0; n < DEVICEV3_TRANSFERS; n++) {
        Transfers[n]->Submit(pDeviceHandle, DEVICEV3_ENDPOINT_IN, [](struct libusb_transfer *transfer) { ((DeviceV3*)transfer->user_data)->ReadCallback(transfer); }, -1);
        pSubmittedTransfers++;
      }
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      LogFile::Info("DeviceV3 with serial '%s' connected", pSerial.c_str());
      pNewDevice = NULL;
    }
  }
  if(pDeviceHandle != NULL) libusb_release_interface(pDeviceHandle, 0);
  if(pDeviceHandle != NULL) libusb_close(pDeviceHandle);
  for(uint8_t n = 0; n < DEVICEV3_TRANSFERS; n++) free(Transfers[n]);
  libusb_exit(pContext);
  return NULL;
}
void DeviceV3::ReadCallback(struct libusb_transfer *transfer) {
  if((transfer->endpoint == DEVICEV3_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    uint16_t *Samples = (uint16_t*)transfer->buffer;
    pthread_mutex_lock(&pConfigLock);
    /* 00xxxxxx xxxxxxxx -> Unused word
       01xxxxxx xxxxxxxx -> Status word (Unused)
       10xxxxxx xxxxxxxx -> First data channel
       11xxxxxx xxxxxxxx -> Other data channels
    */
    for(int Pos = 0; Pos < (transfer->actual_length / 2); Pos++) {
      uint16_t Type   = (Samples[Pos] & 0xC000);
      uint16_t Sample = (Samples[Pos] & 0x3FFF);
      if((Type == 0x0000) || (Type == 0x4000)) {
        continue;
      } else if((Type == 0x8000) && (pCaptureSize == 0)) {
        pCapture[0] = Sample;
        pCaptureSize = 1;
      } else if((Type == 0xC000) && (pCaptureSize < 8)) {
        pCapture[pCaptureSize] = Sample;
        pCaptureSize++;
      } else if((Type == 0xC000) && (pCaptureSize == 8)) {
        pCapture[8] = Sample;
        pCaptureSize = 0;
        for(uint8_t n = 0; n < 9; n++) pRunAverageValues[n] += pCapture[n];
        pRunAverageCounter++;
        if(pRunAverageCounter == (DEVICEV3_SAMPLEFREQ * 10)) {
          pthread_rwlock_wrlock(&pAverageLocks);
          for(uint8_t n = 0; n < 9; n++) {
            pAverageValues[n] = pRunAverageValues[n] / (DEVICEV3_SAMPLEFREQ * 10.0);
            pRunAverageValues[n] = 0;
          }
          pthread_rwlock_unlock(&pAverageLocks);
          pRunAverageCounter = 0;
        }
        if(pEnabled == true) {
          float Values[9];
          Values[0] = (pCapture[4] + pVoltageOffset) * pVoltageGain;
          Values[1] = (pCapture[3] + pCurrentOffset[0]) * pCurrentGain[0];
          Values[2] = (pCapture[3] + pCurrentOffset[1]) * pCurrentGain[1];
          Values[3] = (pCapture[3] + pCurrentOffset[2]) * pCurrentGain[2];
          Values[4] = (pCapture[4] + pCurrentOffset[3]) * pCurrentGain[3];
          Values[5] = (pCapture[5] + pCurrentOffset[4]) * pCurrentGain[4];
          Values[6] = (pCapture[6] + pCurrentOffset[5]) * pCurrentGain[5];
          Values[7] = (pCapture[6] + pCurrentOffset[6]) * pCurrentGain[6];
          Values[8] = (pCapture[6] + pCurrentOffset[7]) * pCurrentGain[7];
          for(uint8_t n = 0; n < pAnalyzersCount; n++) pAnalyzers[n]->BufferAppend(Values);
        }
      } else if((Type == 0x8000) && (pCaptureSize > 0x10)) {
        pCaptureSize--;
      } else if((Type == 0x8000) && (pCaptureSize == 0x10)) {
        pCapture[0] = Sample;
        pCaptureSize = 1;
      } else if(pCaptureSize < 0x10) {
        LogFile::Warning("Lost sample, restarting adquisition");
        pCaptureSize = 0xFF;
        break;
      }
    }
    pthread_mutex_unlock(&pConfigLock);
  }
  if((pTerminate == false) && (transfer->endpoint == DEVICEV3_ENDPOINT_IN)) {
    libusb_submit_transfer(transfer);
  } else {
    pSubmittedTransfers--;
  }
}
int DeviceV3::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event) {
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (pDeviceHandle == NULL)) {
    pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(pDeviceHandle);
      if(dev == CurrentDev) {
        LogFile::Info("DeviceV3 with serial '%s' disconnected", pSerial.c_str());
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void DeviceV3::GetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  for(uint8_t n = 0; n < Config["Outputs"].size(); n++) Config["Outputs"][n].erase("Analyzer");
  pthread_rwlock_rdlock(&pAverageLocks);
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = pAverageValues[4];
  Config["Wave_Average"]["Current"] = json::array();
  Config["Wave_Average"]["Current"].push_back(pAverageValues[3]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[2]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[1]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[0]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[8]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[7]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[6]);
  Config["Wave_Average"]["Current"].push_back(pAverageValues[5]);
  pthread_rwlock_unlock(&pAverageLocks);
  req.Success(Config);
}
void DeviceV3::SetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json ConfigIN = req.Params();
  json Orig = ConfigFile::ReadConfig("/Devices/" + pSerial);
  if(ConfigIN.contains("Outputs") && Orig.contains("Outputs") && ConfigIN["Outputs"].is_array() && Orig["Outputs"].is_array()) {
    for(uint8_t n = 0; n < ConfigIN["Outputs"].size(); n++) {
      if(n < Orig["Outputs"].size() && (ConfigIN["Outputs"][n].contains("Mode") && Orig["Outputs"][n].contains("Mode")) && Orig["Outputs"][n].contains("Analyzer") && (ConfigIN["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"])) ConfigIN["Outputs"][n]["Analyzer"] = Orig["Outputs"][n]["Analyzer"];
    }
  }
  if(ConfigIN.contains("Calculated") && ConfigIN["Calculated"].is_object() && Orig.contains("Calculated") && Orig["Calculated"].contains("Analyzer")) ConfigIN["Calculated"]["Analyzer"] = Orig["Calculated"]["Analyzer"];
  Config(CheckConfig(ConfigIN));
  req.Success(json::object());
}
//----------------------------------------------------------------------------------------------------------------------