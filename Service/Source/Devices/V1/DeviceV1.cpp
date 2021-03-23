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
#include "DeviceV1.h"
#include "DeviceManager.h"
#include "ConfigFile.h"
#include "AnalyzerManager.h"
#include "Tools.h"
#include "USBTransfer.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1::pRegistered = DeviceV1::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceV1::GetInstances() {
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
    if(Descriptor.idVendor != DEVICEV1_VID) continue;
    if(Descriptor.idProduct != DEVICEV1_PID) continue;
    if((Descriptor.bcdDevice != 0x0100) && (Descriptor.bcdDevice != 0x0101)) continue;
    uint16_t HWVersion = Descriptor.bcdDevice;
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
    if(strcmp(String, DEVICEV1_MANSTRING) != 0) {
      libusb_close(Dev_Handle);
      continue;
    }
    Error = libusb_get_string_descriptor_ascii(Dev_Handle, Descriptor.iProduct, (unsigned char*)String, sizeof(String));
    if(Error < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
      libusb_close(Dev_Handle);
      continue;
    }
    if(strcmp(String, DEVICEV1_01_PRODSTRING) != 0) {
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
    DeviceManager::AddDevice(new DeviceV1(String, HWVersion));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
}
DeviceV1::DeviceV1(const string &serial, const uint16_t version) : Device(serial, "V1", "Capture device V1") {
  LogFile::Info("Starting DeviceV1 with serial '%s'...", serial.c_str());
  pAverageLocks       = PTHREAD_RWLOCK_INITIALIZER;
  pConfigLock         = PTHREAD_MUTEX_INITIALIZER;
  pThread             = 0;
  pHWVersion          = version;
  pVoltageGain        = 0.0;
  pCurrentGain        = 0.0;
  pVoltageOffset      = 0.0;
  pCurrentOffset      = 0.0;
  pCurrentInvert      = false;
  pTerminate          = false;
  pAnalyzer           = NULL;
  pDeviceHandle       = NULL;
  pNewDevice          = NULL;
  pSubmittedTransfers = 0;
  pRunAverageCounter  = 0;
  memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
  memset(pAverageValues, 0 , sizeof(pAverageValues));
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  int Error = libusb_init(&pContext);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    return;
  }
  if(pthread_create(&pThread, NULL, [](void *args) -> void* { return ((DeviceV1*)args)->Thread(); }, this) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return;
  }
  Worker::SetThreadProperties(pThread, "DeviceV1", Worker::HIGHEST);
  Error = libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV1_VID, DEVICEV1_PID, LIBUSB_HOTPLUG_MATCH_ANY, [](struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) { return ((DeviceV1*)user_data)->PlugCallback(ctx, dev, event); }, this, NULL);
  if(Error < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Error));
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    libusb_exit(pContext);
  }
  LogFile::Info("DeviceV1 with serial '%s' started", serial.c_str());
}
DeviceV1::~DeviceV1() {
  LogFile::Info("Stopping DeviceV1 with serial '%s'...", pSerial.c_str());
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  LogFile::Info("DeviceV1 with serial '%s' stopped", pSerial.c_str());
}
void DeviceV1::ClearAnalyzers() {
  pthread_mutex_lock(&pConfigLock);
  if(pAnalyzer != NULL) {
    AnalyzerManager::DelAnalyzer(pAnalyzer->Serial());
    delete pAnalyzer;
    pAnalyzer = NULL;
  }
  pthread_mutex_unlock(&pConfigLock);
}
json DeviceV1::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = 0.0;
  ConfigOUT["VoltageOffset"] = 0.0;
  ConfigOUT["CurrentGain"] = 0.0;
  ConfigOUT["CurrentOffset"] = 0.0;
  ConfigOUT["CurrentInvert"] = false;
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_number() && (config["VoltageGain"] >= -10.0) && (config["VoltageGain"] <= 10.0)) ConfigOUT["VoltageGain"] = config["VoltageGain"];
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_number() && (config["VoltageOffset"] >= -100.0) && (config["VoltageOffset"] <= 100.0)) ConfigOUT["VoltageOffset"] = config["VoltageOffset"];
    if(config.contains("CurrentGain") && config["CurrentGain"].is_number() && (config["CurrentGain"] >= -10.0) && (config["CurrentGain"] <= 10.0)) ConfigOUT["CurrentGain"] = config["CurrentGain"];
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_number() && (config["CurrentOffset"] >= -100.0) && (config["CurrentOffset"] <= 100.0)) ConfigOUT["CurrentOffset"] = config["CurrentOffset"];
    if(config.contains("CurrentInvert") && config["CurrentInvert"].is_boolean()) ConfigOUT["CurrentInvert"] = config["CurrentInvert"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"], Analyzer::PHASE1, false);
  }
  return ConfigOUT;
}
void DeviceV1::Config(const json& config) {
  ClearAnalyzers();
  pthread_mutex_lock(&pConfigLock);
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  pVoltageOffset   = config["VoltageOffset"];
  pVoltageGain     = config["VoltageGain"];
  pVoltageGain     = ((pHWVersion == 0x0100) ? 0.010683471 : 0.014244628) * (1.0 + (pVoltageGain / 100.0));
  pCurrentInvert   = config["CurrentInvert"];
  pCurrentOffset   = config["CurrentOffset"];
  pCurrentGain     = config["CurrentGain"];
  pCurrentGain     = 0.0015625 * (1.0 + (pCurrentGain / 100.0)) * (pCurrentInvert ? -1.0 : 1.0);
  pEnabled         = config["Enabled"];
  pVoltageDelayPos = 0;
  memset(pVoltageDelay, 0, sizeof(pVoltageDelay));
  pAnalyzer = new AnalyzerSoft(pSerial + "_1P", Analyzer::PHASE1, "/Devices/" + pSerial + "/Analyzer", DEVICEV1_SAMPLEFREQ);
  AnalyzerManager::AddAnalyzer(pAnalyzer);
  pthread_mutex_unlock(&pConfigLock);
}
uint32_t DeviceV1::AnalyzersCount() {
  pthread_mutex_lock(&pConfigLock);
  uint32_t Count = (pAnalyzer != NULL) ? 1 : 0;
  pthread_mutex_unlock(&pConfigLock);
  return Count;
}
void *DeviceV1::Thread() {
  USBTransfer *Transfers[DEVICEV1_TRANSFERS];
  for(uint8_t n = 0; n < DEVICEV1_TRANSFERS; n++) Transfers[n] = new USBTransfer(this, 8192);
  while((pTerminate == false) || (pSubmittedTransfers > 0)) {
    struct timeval tv = { 1, 0 };
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
      for(uint8_t n = 0; n < DEVICEV1_TRANSFERS; n++) {
        Result |= Transfers[n]->Submit(pDeviceHandle, DEVICEV1_ENDPOINT_IN, [](struct libusb_transfer *transfer) { ((DeviceV1*)transfer->user_data)->ReadCallback(transfer); }, -1);
        pSubmittedTransfers++;
      }
      if(Result < 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, libusb_error_name(Result));
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
        pNewDevice = NULL;
        continue;
      }
      LogFile::Info("DeviceV1 with serial '%s' connected", pSerial.c_str());
      pNewDevice = NULL;
    }
  }
  if(pDeviceHandle != NULL) libusb_release_interface(pDeviceHandle, 0);
  if(pDeviceHandle != NULL) libusb_close(pDeviceHandle);
  for(uint8_t n = 0; n < DEVICEV1_TRANSFERS; n++) delete Transfers[n];
  if(pContext != NULL) libusb_exit(pContext);
  return NULL;
}
void DeviceV1::ReadCallback(struct libusb_transfer *transfer) {
  if((transfer->endpoint == DEVICEV1_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    int16_t *Samples = (int16_t*)transfer->buffer;
    pthread_mutex_lock(&pConfigLock);
    for(uint16_t Pos = 0; Pos < (transfer->actual_length / 2); Pos = Pos + 2) {
      if(Samples[Pos] != -32768) {
        for(uint8_t Ch = 0; Ch < 2; Ch++) pRunAverageValues[Ch] += Samples[Pos + Ch];
        pRunAverageCounter++;
        if(pRunAverageCounter == (DEVICEV1_SAMPLEFREQ * 10)) {
          pthread_rwlock_wrlock(&pAverageLocks);
          for(uint8_t Ch = 0; Ch < 2; Ch++) pAverageValues[Ch] = pRunAverageValues[Ch] / (DEVICEV1_SAMPLEFREQ * 10.0);
          pthread_rwlock_unlock(&pAverageLocks);
          memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
          pRunAverageCounter = 0;
        }
        if((pAnalyzer != NULL) && (pEnabled == true)) {
          float Voltage = (pVoltageDelay[pVoltageDelayPos] + pVoltageOffset) * pVoltageGain;
          float Current = (Samples[Pos + 1] + pCurrentOffset) * pCurrentGain;
          pVoltageDelay[pVoltageDelayPos] = Samples[Pos];
          pVoltageDelayPos = (pVoltageDelayPos + 1) % DEVICEV1_VOLTAGEDELAY;
          pAnalyzer->BufferAppend(&Voltage, &Current);
        }
      }
    }
    pthread_mutex_unlock(&pConfigLock);
  }
  if(pTerminate == false) {
    libusb_submit_transfer(transfer);
  } else {
    pSubmittedTransfers--;
  }
}
int DeviceV1::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event) {
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (pDeviceHandle == NULL)) {
    pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(pDeviceHandle);
      if(dev == CurrentDev) {
        LogFile::Info("DeviceV1 with serial '%s' disconnected", pSerial.c_str());
        libusb_close(pDeviceHandle);
        pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void DeviceV1::GetDevice(HTTPRequest &req) {/*
  string UserName = req.UserName();
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  pthread_rwlock_rdlock(&pAverageLocks);
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = pAverageValues[0];
  Config["Wave_Average"]["Current"] = pAverageValues[1];
  pthread_rwlock_unlock(&pAverageLocks);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);*/
}
void DeviceV1::SetDevice(HTTPRequest &req) {/*
  string UserName = req.UserName();
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json ConfigIN = params;
  json Orig = ConfigFile::ReadConfig("/Devices/" + pSerial);
  ConfigIN["Analyzer"] = Orig["Analyzer"];
  Config(CheckConfig(ConfigIN));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);*/
}


//void *DeviceV1_01::Sig_Thread(void* args) {
//  DeviceV1_01 *This = (DeviceV1_01*)args;
//  bool Toogle = false;
//  while(!This->pTerminateSignal) {
//    Analyzer::Params_t Params;
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
//    usleep(100000);
    //Envio transferencia de control
    //Leo datos, si hay algun boton pulsado llamo a los scripts!
  //}
//  return NULL;
//}
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
//----------------------------------------------------------------------------------------------------------------------