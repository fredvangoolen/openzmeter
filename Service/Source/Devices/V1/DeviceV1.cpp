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
#include "DeviceV1.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1::pRegistered = DeviceV1::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceV1::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
vector<Device*>  DeviceV1::GetInstances() {
  vector<Device*> Result;
  libusb_context *Context;
  if(libusb_init(&Context) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return Result;
  }
  libusb_device **Devs;
  int Count = libusb_get_device_list(NULL, &Devs);
  if(Count < 0) {
    LOGFile::Error("Can not enumerate USB devices...\n");
    return Result;
  }
  for(int i = 0; i < Count; ++i) {
    libusb_device_descriptor Descriptor;
    if(libusb_get_device_descriptor(Devs[i], &Descriptor) < 0) {
      LOGFile::Error("Can not load USB descriptor...\n");
      return Result;
    }
    if(Descriptor.idVendor != DEVICEV1_VID) continue;
    if(Descriptor.idProduct != DEVICEV1_PID) continue;
    if((Descriptor.bcdDevice != 0x0100) && (Descriptor.bcdDevice != 0x0101)) continue;
    uint16_t HWVersion = Descriptor.bcdDevice;
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
    if(strcmp(String, DEVICEV1_MANSTRING) != 0) continue;
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
    LOGFile::Info("Adding 'V1' device driver instance with serial '%s'\n", String);
    Result.push_back(new DeviceV1(String, HWVersion));
  }
  libusb_free_device_list(Devs, Count);
  libusb_exit(Context);
  return Result;
}
DeviceV1::DeviceV1(const string &serial, const uint16_t version) : Device(serial, "V1", "Capture device V1") {
  pBuffer             = (int16_t*)malloc(DEVICEV1_BUFFERSIZE * sizeof(uint16_t) * 2);
  pConfigLock         = PTHREAD_MUTEX_INITIALIZER;
  pThread             = 0;
  pBufferHead         = 0;
  pBufferTail         = 0;
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
  Configure(CheckConfig(ConfigManager::ReadConfig("/Devices/" + Serial)));
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return;
  }
  if(pthread_create(&pThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return;
  }
  pthread_setname_np(pThread, "DeviceV1");
  if(libusb_hotplug_register_callback(pContext, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED | LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), LIBUSB_HOTPLUG_ENUMERATE , DEVICEV1_VID, DEVICEV1_PID, LIBUSB_HOTPLUG_MATCH_ANY, PlugCallback, this, NULL) < 0) {
    LOGFile::Error("Failed to register libUSB calback...\n");
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    libusb_exit(pContext);
  }
}
DeviceV1::~DeviceV1() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  if(pBuffer != NULL) free(pBuffer);
}
json DeviceV1::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["VoltageGain"] = 0.0;
  ConfigOUT["VoltageOffset"] = 0.0;
  ConfigOUT["CurrentGain"] = 0.0;
  ConfigOUT["CurrentOffset"] = 0.0;
  ConfigOUT["CurrentInvert"] = false;
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1);
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_number() && (config["VoltageGain"] >= -10.0) && (config["VoltageGain"] <= 10.0)) ConfigOUT["VoltageGain"] = config["VoltageGain"];
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_number() && (config["VoltageOffset"] >= -20.0) && (config["VoltageOffset"] <= 20.0)) ConfigOUT["VoltageOffset"] = config["VoltageOffset"];
    if(config.contains("CurrentGain") && config["CurrentGain"].is_number() && (config["CurrentGain"] >= -10.0) && (config["CurrentGain"] <= 10.0)) ConfigOUT["CurrentGain"] = config["CurrentGain"];
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_number() && (config["CurrentOffset"] >= -1.0) && (config["CurrentOffset"] <= 1.0)) ConfigOUT["CurrentOffset"] = config["CurrentOffset"];
    if(config.contains("CurrentInvert") && config["CurrentInvert"].is_boolean()) ConfigOUT["CurrentInvert"] = config["CurrentInvert"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"], Analyzer::PHASE1);
  }
  return ConfigOUT;
}
void DeviceV1::Configure(const json& config) {
  pthread_mutex_lock(&pConfigLock);
  ConfigManager::WriteConfig("/Devices/" + Serial, config);
  ClearAnalyzers();
  pAnalyzer = NULL;
  pVoltageGain = config["VoltageGain"];
  pVoltageOffset = config["VoltageOffset"];
  pCurrentGain = config["CurrentGain"];
  pCurrentOffset = config["CurrentOffset"];
  pCurrentInvert = config["CurrentInvert"];
  pEnabled = config["Enabled"];
  pVoltageDelayPos = 0;
  memset(pVoltageDelay, 0, sizeof(pVoltageDelay));
  pAnalyzer = new AnalyzerSoft(Serial, Analyzer::PHASE1, "/Devices/" + Serial + "/Analyzer", DEVICEV1_SAMPLEFREQ);
  AddAnalyzer(pAnalyzer);
  pthread_mutex_unlock(&pConfigLock);
}
void *DeviceV1::Capture_Thread(void *args) {
  DeviceV1 *This = (DeviceV1*)args;
  nice(-20);
  for(uint8_t n = 0; n < DEVICEV1_TRANSFERS; n++) This->pTransfers[n].Init(This, 32768);
  while((This->pTerminate == false) || (This->pSubmittedTransfers > 0)) {
    struct timeval tv = { 0, 100000 };
    int Result = libusb_handle_events_timeout_completed(This->pContext, &tv, NULL);
    if(Result < LIBUSB_SUCCESS) LOGFile::Error("libusb_handle_events_timeout_completed returns code %d\n", Result);
    if(This->pTerminate == true) continue;
    if(This->pNewDevice != NULL) {
      struct libusb_device_descriptor Descriptor;
      Result = libusb_get_device_descriptor(This->pNewDevice, &Descriptor);
      if(Result < LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_get_device_descriptor return code %d\n", Result);
        This->pDeviceHandle = NULL;
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
      if(This->Serial != Serial) {
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
          LOGFile::Info("Kernel driver dettached for DeviceV1 with serial '%s'\n", This->Serial.c_str());
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
        LOGFile::Info("Claimed interface 0 for DeviceV1 with serial %s\n", This->Serial.c_str());
      }
      for(Transfer &Tmp : This->pTransfers) {
        Result |= Tmp.Submit(This->pDeviceHandle, DEVICEV1_ENDPOINT_IN, ReadCallback, 1000);
        This->pSubmittedTransfers++;
      }
      if(Result != LIBUSB_SUCCESS) {
        LOGFile::Error("libusb_submit_transfer returns %d\n", Result);
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
        This->pNewDevice = NULL;
        continue;
      }
      LOGFile::Info("Capture started on DeviceV1 with serial '%s'\n", This->Serial.c_str());
      This->pNewDevice = NULL;
    }
  }
  if(This->pDeviceHandle != NULL) libusb_release_interface(This->pDeviceHandle, 0);
  if(This->pDeviceHandle != NULL) libusb_close(This->pDeviceHandle);
  if(This->pContext != NULL) libusb_exit(This->pContext);
  return NULL;
}
void DeviceV1::ReadCallback(struct libusb_transfer *transfer) {
  DeviceV1 *This = (DeviceV1*)transfer->user_data;
  if((transfer->endpoint == DEVICEV1_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    int16_t *Samples = (int16_t*)transfer->buffer;
    pthread_mutex_lock(&This->pConfigLock);
    for(uint16_t Pos = 0; Pos < (transfer->actual_length / 2); Pos = Pos +2) {
      if(Samples[Pos] != -32768) This->BufferAppend(&Samples[Pos]);
    }
    pthread_mutex_unlock(&This->pConfigLock);
  }
  if(This->pTerminate == false) {
    libusb_submit_transfer(transfer);
  } else {
    LOGFile::Debug("DeviceV1 with serial '%s' flushing buffers (%u pending)\n", This->Serial.c_str(), This->pSubmittedTransfers);
    This->pSubmittedTransfers--;
  }
}
int DeviceV1::PlugCallback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
  DeviceV1 *This = (DeviceV1*)user_data;
  if((event == LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED) && (This->pDeviceHandle == NULL)) {
    This->pNewDevice = dev;
  } else if(event == LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT) {
    if(This->pDeviceHandle != NULL) {
      libusb_device *CurrentDev = libusb_get_device(This->pDeviceHandle);
      if(dev == CurrentDev) {
        LOGFile::Info("DeviceV1 with serial '%s' disconnected\n", This->Serial.c_str());
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void DeviceV1::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/Devices/" + Serial);
  Config.erase("Analyzer");
  pAverageLocks.ReadLock();
  Config["Wave_Average"] = json::object();
  Config["Wave_Average"]["Voltage"] = pAverageValues[0];
  Config["Wave_Average"]["Current"] = pAverageValues[1];
  pAverageLocks.Unlock();
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceV1::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
  json Orig = ConfigManager::ReadConfig("/Devices/" + Serial);
  Config["Analyzer"] = Orig["Analyzer"];
  Configure(CheckConfig(Config));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void DeviceV1::BufferAppend(int16_t *sample) {
  for(uint8_t Ch = 0; Ch < 2; Ch++) pRunAverageValues[Ch] = pRunAverageValues[Ch] + sample[Ch];
  pRunAverageCounter++;
  if(pRunAverageCounter == DEVICEV1_SAMPLEFREQ * 10) {
    pAverageLocks.WriteLock();
    for(uint8_t Ch = 0; Ch < 2; Ch++) pAverageValues[Ch] = pRunAverageValues[Ch] / (DEVICEV1_SAMPLEFREQ * 10.0);
    pAverageLocks.Unlock();
    memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
    pRunAverageCounter = 0;
  }
  if((pAnalyzer == NULL) || (pEnabled == false)) return;
  uint32_t Next = (pBufferHead + 1) % DEVICEV1_BUFFERSIZE;
  if(Next != pBufferTail) {
    memcpy(&pBuffer[pBufferHead * 2], sample, sizeof(uint16_t) * 2);
    pBufferHead = Next;
  } else {
    pBufferHead = 0;
    pBufferTail = 0;
    LOGFile::Error("Capture buffer overflow in device '%s'\n", Serial.c_str());
  }
  if(pAnalyzer->IsWorking() == true) return;
  bool AnalyzerRun  = false;
  float VoltageMult = (pHWVersion == 0x0100) ? 0.0106834716796 : 0.0142446289062;
  while((pBufferTail != pBufferHead) && (AnalyzerRun == false)) {
    int16_t *Ptr = &pBuffer[pBufferTail * 2];
    pBufferTail = (pBufferTail + 1) % DEVICEV1_BUFFERSIZE;
    float Voltage = ((pVoltageDelay[pVoltageDelayPos] * VoltageMult) * (1.0 + (pVoltageGain / 100.0))) + pVoltageOffset;
    float Current = ((Ptr[1] * 0.0015625000000) * (1.0 + (pCurrentGain / 100.0)) * (pCurrentInvert ? -1.0 : 1.0)) + pCurrentOffset;
    pVoltageDelay[pVoltageDelayPos] = Ptr[0];
    pVoltageDelayPos = (pVoltageDelayPos + 1) % DEVICEV1_VOLTAGEDELAY;
    uint32_t Used = (pBufferHead > pBufferTail) ? pBufferHead - pBufferTail : DEVICEV1_BUFFERSIZE - pBufferTail + pBufferHead;
    if(pAnalyzer->BufferAppend(&Voltage, &Current, Used) == true) AnalyzerRun = true;
  }
//  static int File = 0;
//  if(File == 0) File = open("Samples.raw", O_CREAT | O_WRONLY);
//  char Tmp[100];
//  sprintf(Tmp, "%f, %f\n", voltageSample, currentSample);
//  write(File, Tmp, strlen(Tmp));

//  if(pBuffer.full()) return false;
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
DeviceV1::Transfer::Transfer() {
  pBuffer = NULL;
  pTransfer = libusb_alloc_transfer(0);
}
void DeviceV1::Transfer::Init(DeviceV1 *parent, uint32_t size) {
  pBufferSize = size;
  pParent = parent;
  pBuffer = (uint8_t*)realloc(pBuffer, size);
}
DeviceV1::Transfer::~Transfer() {
  if(pBuffer != NULL) free(pBuffer);
  if(pTransfer != NULL) libusb_free_transfer(pTransfer);
}
int DeviceV1::Transfer::Submit() {
  return libusb_submit_transfer(pTransfer);
}
int DeviceV1::Transfer::Submit(libusb_device_handle* deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
  libusb_fill_bulk_transfer(pTransfer, deviceHandle, endpoint, pBuffer, pBufferSize, callback, pParent, timeout);
  return Submit();
}

//======================================================================================================================
