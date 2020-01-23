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
  ConfigOUT["VoltageGain"] = vector<float>(3, 1.0);
  ConfigOUT["VoltageOffset"] = vector<float>(3, 0.0);
  ConfigOUT["CurrentGain"] = vector<float>(4, 1.0);
  ConfigOUT["CurrentOffset"] = vector<float>(4, 0.0);
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["HighGain"] = false;
  ConfigOUT["Outputs"] = json::array();
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_array() && (config["VoltageGain"].size() == ConfigOUT["VoltageGain"].size())) {
      for(unsigned n = 0; n < ConfigOUT["VoltageGain"].size(); n++) {
        if(config["VoltageGain"][n].is_number() && (config["VoltageGain"][n] > 0.0) && (config["VoltageGain"][n] <= 1000.0)) ConfigOUT["VoltageGain"][n] = config["VoltageGain"][n];
      }
    }
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_array() && (config["VoltageOffset"].size() == ConfigOUT["VoltageOffset"].size())) {
      for(unsigned n = 0; n < ConfigOUT["VoltageOffset"].size(); n++) {
        if(config["VoltageOffset"][n].is_number() && (config["VoltageOffset"][n] >= -200.0) && (config["VoltageOffset"][n] <= 200.0)) ConfigOUT["VoltageOffset"][n] = config["VoltageOffset"][n];
      }
    }
    if(config.contains("CurrentGain") && config["CurrentGain"].is_array() && (config["CurrentGain"].size() == ConfigOUT["CurrentGain"].size())) {
      for(unsigned n = 0; n < ConfigOUT["CurrentGain"].size(); n++) {
        if(config["CurrentGain"][n].is_number() && (config["CurrentGain"][n] > 0.0) && (config["CurrentGain"][n] <= 1000.0)) ConfigOUT["CurrentGain"][n] = config["CurrentGain"][n];
      }
    }
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_array() && (config["CurrentOffset"].size() == ConfigOUT["CurrentOffset"].size())) {
      for(unsigned n = 0; n < ConfigOUT["CurrentOffset"].size(); n++) {
        if(config["CurrentOffset"][n].is_number() && (config["CurrentOffset"][n] >= -200.0) && (config["CurrentOffset"][n] <= 200.0)) ConfigOUT["CurrentOffset"][n] = config["CurrentOffset"][n];
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
    if(ConfigOUT["Outputs"].size() == 0) {
      json Output     = json::object();
      Output["Mode"]  = "1PHASE";
      Output["Voltage"] = "CH1";
      Output["Current"] = "+CH1";
      Output["Analyzer"] = AnalyzerSoft::CheckConfig(json::object());
      ConfigOUT["Outputs"].push_back(Output);
    }
  }
  return ConfigOUT;
}
DeviceV2_01::DeviceV2_01(const string &serial) : Device(serial, "V2.01", "Capture device V2.01"), pSignalTransfer(this, 2) {
  pBuffer     = (int16_t*)malloc(DEVICEV2_01_BUFFERSIZE * sizeof(uint16_t) * 7);
  pBufferHead = 0;
  pBufferTail = 0;
}
DeviceV2_01::~DeviceV2_01() {
  StopDevice();
  free(pBuffer);
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
int DeviceV2_01::Transfer::Submit(libusb_device_handle *deviceHandle, uint8_t endpoint, libusb_transfer_cb_fn callback, int32_t timeout) {
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
  LOGFile::Info("Loading 'V2.01' driver configuration for serial '%s'\n", Serial.c_str());
  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + Serial));
  bool AutoStart = Config["AutoStart"];
  ConfigManager::WriteConfig("/Devices/" + Serial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pMembersLock.WriteUnlock();
      return true;
    }
  }
  LOGFile::Info("Starting 'V2.01' driver for serial '%s'\n", Serial.c_str());
  for(unsigned Ch = 0; Ch < 3; Ch++) {
    pVoltageGain[Ch] = Config["VoltageGain"][Ch];
    pVoltageOffset[Ch] = Config["VoltageOffset"][Ch];
  }
  for(unsigned Ch = 0; Ch < 4; Ch++) {
    pCurrentGain[Ch] = Config["CurrentGain"][Ch];
    pCurrentOffset[Ch] = Config["CurrentOffset"][Ch];
  }
  pHighGain = Config["HighGain"];
  pTerminate = false;
  for(auto Tmp : Config["Outputs"]) {
    Analyzer_t AnalyzerSetup;
    AnalyzerSetup.Mode = (Tmp["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (Tmp["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
    string Name = Serial;
    if(AnalyzerSetup.Mode == Analyzer::PHASE1) {
      string Voltage = Tmp["Voltage"];
      AnalyzerSetup.Voltage[0] = (Voltage[2] - '1');
      string Current = Tmp["Current"];
      AnalyzerSetup.Current[0] = Current[3] - '1' + 3;
      AnalyzerSetup.CurrentInvert[0] = (Current[0] == '+') ? 1.0 : -1.0;
      AnalyzerSetup.CurrentChannels = 1;
      AnalyzerSetup.VoltageChannels = 1;
      Name += "_1P_CH" + to_string(Current[3]);
    } else {
      for(unsigned Ch = 0; Ch < Tmp["Voltage"].size(); Ch++) {
        string Voltage = Tmp["Voltage"][Ch];
        AnalyzerSetup.Voltage[Ch] = (Voltage[2] - '1');
      }
      for(unsigned Ch = 0; Ch < Tmp["Current"].size(); Ch++) {
        string Current =  Tmp["Current"][Ch];
        AnalyzerSetup.Current[Ch] = Current[3] - '1' + 3;
        AnalyzerSetup.CurrentInvert[Ch] = (Current[0] == '+') ? 1.0 : -1.0;
      }
      AnalyzerSetup.CurrentChannels = Tmp["Current"].size();
      AnalyzerSetup.VoltageChannels = Tmp["Voltage"].size();
      Name += (AnalyzerSetup.Mode == Analyzer::PHASE3) ? "_3P" : "_3PN";
    }
    AnalyzerSetup.Parent = new AnalyzerSoft(Name, AnalyzerSetup.Mode, "/Devices/" + Serial + "/Outputs/" + to_string(pAnalyzersSetup.size()) + "/Analyzer", DEVICEV2_01_SAMPLEFREQ);
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
      pRunAverageCounter = 0;
      memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
      memset(pAverageValues, 0 , sizeof(pAverageValues));
      pCaptureSize = 0;
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
  LOGFile::Info("Stopping 'V2.01' driver for serial '%s'\n", Serial.c_str());
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
      uint16_t Status = 0x0006; //Stat=GREEN y Wifi=RED
      SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
      SysStatus->Lock.ReadLock();
      if(SysStatus->RAM_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;              //Stat=YELLOW
      if(SysStatus->CPU_Usage > 70.0) Status = (Status & 0xFFFC) | 0x0003;              //Stat=YELLOW
      if(SysStatus->Disk_Usage > 90.0) Status = (Status & 0xFFFC) | 0x0003;             //Stat=YELLOW
      if(SysStatus->Time_IsSet == false) Status = (Status & 0xFFFC) | 0x0001;           //Stat=RED
      if(SysStatus->Events == true) Status = (Status & 0xC01C) | 0x0021;                //Stat=RED and short BEEP
      if(SysStatus->WIFI_Network.empty() == false) Status = (Status & 0xFFF3) | 0x000C; //Wifi=GREEN
      if(SysStatus->BT_Network.empty() == false) Status = (Status & 0xFFEF) | 0x0010;   //Bt=BLUE
      SysStatus->Lock.ReadUnlock();
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
          LOGFile::Info("Kernel driver dettached for DeviceV2.01 with serial '%s'\n", This->Serial.c_str());
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
        LOGFile::Info("Claimed interface 0 for DeviceV2 with serial '%s'\n", This->Serial.c_str());
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
      LOGFile::Info("Capture started on DeviceV2.01 with serial '%s'\n", This->Serial.c_str());
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
  uint64_t Time = Tools::GetTime();
  if((transfer->endpoint == DEVICEV2_01_ENDPOINT_IN) && (transfer->status == LIBUSB_TRANSFER_COMPLETED)) {
    uint16_t *Samples = (uint16_t*)transfer->buffer;
    for(int Pos = 0; Pos < (transfer->actual_length / 2); Pos++) {
      uint8_t  Seq    = Samples[Pos] >> 13;
      uint16_t Sample = (Samples[Pos] & 0x1FFF);
      if(Seq == 7) {
        if(This->pLastSignalRecv < (Time - 500)) {
          SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
          SysStatus->Lock.WriteLock();
          SysStatus->Battery_Volt = 2.96 + ((Sample & 0x00FC) >> 2) * 0.02;
          SysStatus->Battery_Level  = (SysStatus->Battery_Volt - 2.96) * 80.65;
          if(SysStatus->Battery_Level > 100.0) SysStatus->Battery_Level = 100.0;
//          Status["Charging"] = (Sample & 0x0002) ? true: false;
//          Status["Powered"] = (Sample & 0x0001) ? true: false;
          SysStatus->Lock.WriteUnlock();
          uint8_t Action = (Sample & 0x0300) >> 8;
          if((Action == CLICK) && (This->pButtonAction == false)) {
            SysSupport::SetSystemActions("ToogleBluetooth");
          } else if((Action == LONG_CLICK) && (This->pButtonAction == false)) {
            SysSupport::SetSystemActions("PowerOff");
          }
          This->pButtonAction = (Action == NONE);
          This->pLastSignalRecv = Time;
        }
        continue;
      }
      if(This->pCaptureSize != Seq) {
        LOGFile::Debug("Lost sample, restarting adquisition\n");
        This->pCaptureSize = 0;
        break;
      }
      This->pCapture[This->pCaptureSize] = Sample - 4096;
      if(This->pCaptureSize == 6) {
        This->BuffersAppend();
        This->pCaptureSize = 0;
      } else {
        This->pCaptureSize++;
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
        LOGFile::Info("DeviceV2.01 with serial '%s' disconnected\n", This->Serial.c_str());
        libusb_close(This->pDeviceHandle);
        This->pDeviceHandle = NULL;
      }
    }
  }
  return 0;
}
void  DeviceV2_01::GetDevice(const json &params, Response &response) {
  pMembersLock.ReadLock();
  json Config = ConfigManager::ReadConfig("/Devices/" + Serial);
  for(uint8_t n = 0; n < Config["Outputs"].size(); n++) Config["Outputs"][n].erase("Analyzer");
  pAverageLocks.ReadLock();
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
  pAverageLocks.ReadUnlock();
  pMembersLock.ReadUnlock();
  response.set_content(Config.dump(), "application/json");
  response.status = HTTP_OK;
}
void  DeviceV2_01::SetDevice(const json &params, Response &response) {
  pMembersLock.ReadLock();
  DeviceState State = pState;
  pMembersLock.ReadUnlock();
  StopDevice();
  json Config = CheckConfig(params);
  json Orig = ConfigManager::ReadConfig("/Devices/" + Serial);
  for(unsigned n = 0; n < Config["Outputs"].size(); n++) {
    if(n >= Orig["Outputs"].size()) break;
    if(Config["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"]) Config["Outputs"][n] == Orig["Outputs"][n];
  }
  ConfigManager::WriteConfig("/Devices/" + Serial, Config);
  json Response = json::object();
  response.set_content("", "text/plain");
  response.status = ((State == RUNNING) ? StartDevice() : true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void  DeviceV2_01::BuffersAppend() {
  for(unsigned Ch = 0; Ch < 7; Ch++) pRunAverageValues[Ch] = pRunAverageValues[Ch] + pCapture[Ch];
  pRunAverageCounter++;
  if(pRunAverageCounter == DEVICEV2_01_SAMPLEFREQ * 10) {
    pAverageLocks.WriteLock();
    for(unsigned Ch = 0; Ch < 7; Ch++) pAverageValues[Ch] = pRunAverageValues[Ch] / (DEVICEV2_01_SAMPLEFREQ * 10.0);
    pAverageLocks.WriteUnlock();
    memset(pRunAverageValues, 0, sizeof(pRunAverageValues));
    pRunAverageCounter = 0;
  }
  uint32_t Next = (pBufferHead + 1) % DEVICEV2_01_BUFFERSIZE;
  if(Next != pBufferTail) {
    memcpy(&pBuffer[pBufferHead * 7], pCapture, sizeof(pCapture));
    pBufferHead = Next;
  } else {
    pBufferHead = 0;
    pBufferTail = 0;
    LOGFile::Error("Capture buffer overflow in device '%s'\n", Serial.c_str());
  }
  for(auto &Tmp : pAnalyzersSetup) {
    if(Tmp.Parent->IsFinished() == false) return;
  }
  bool AnalyzerRun = false;
  while((pBufferTail != pBufferHead) && (AnalyzerRun == false)) {
    int16_t *Ptr = &pBuffer[pBufferTail * 7];
    pBufferTail = (pBufferTail + 1) % DEVICEV2_01_BUFFERSIZE;
    float Values[7];
    Values[0] = ((float)Ptr[0] + pVoltageOffset[0]) * 0.110266113281 * pVoltageGain[0];
    Values[1] = ((float)Ptr[1] + pVoltageOffset[1]) * 0.110266113281 * pVoltageGain[1];
    Values[2] = ((float)Ptr[2] + pVoltageOffset[2]) * 0.110266113281 * pVoltageGain[2];
    Values[3] = ((float)Ptr[3] + pCurrentOffset[0]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[0];
    Values[4] = ((float)Ptr[4] + pCurrentOffset[1]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[1];
    Values[5] = ((float)Ptr[5] + pCurrentOffset[2]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[2];
    Values[6] = ((float)Ptr[6] + pCurrentOffset[3]) * (pHighGain ? 0.08132749817 : 0.15165198897) *  pCurrentGain[3];
    for(auto &Tmp : pAnalyzersSetup) {
      float Voltage[3], Current[4];
      //vector<float> Voltage(3), Current(4);
      for(unsigned Ch = 0; Ch < Tmp.VoltageChannels; Ch++) Voltage[Ch] = Values[Tmp.Voltage[Ch]];
      for(unsigned Ch = 0; Ch < Tmp.CurrentChannels; Ch++) Current[Ch] = Values[Tmp.Current[Ch]] * Tmp.CurrentInvert[Ch];
      if(Tmp.Parent->BufferAppend(Voltage, Current) == true) AnalyzerRun = true;
    }
  }
}
//======================================================================================================================
