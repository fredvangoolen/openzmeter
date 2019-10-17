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
    string Name = pSerial;
    if(AnalyzerSetup.Mode == Analyzer::PHASE1) {
      string Current = config["Outputs"][n]["Current"];
      if(config["Outputs"][n]["Voltage"] == "CH1") AnalyzerSetup.Voltage[0] = 0;
      if(config["Outputs"][n]["Voltage"] == "CH2") AnalyzerSetup.Voltage[0] = 1;
      if(config["Outputs"][n]["Voltage"] == "CH3") AnalyzerSetup.Voltage[0] = 2;
      if(Current.find("CH1") != string::npos) AnalyzerSetup.Current[0] = 0;
      if(Current.find("CH2") != string::npos) AnalyzerSetup.Current[0] = 1;
      if(Current.find("CH3") != string::npos) AnalyzerSetup.Current[0] = 2;
      AnalyzerSetup.CurrentInvert[0] = (Current.find("-") != string::npos);
      Name += "_1P_CH" + to_string(AnalyzerSetup.Current[0]);
    } else {
      for(uint i = 0; i < 3; i++) {
        if(config["Outputs"][n]["Voltage"][i] == "CH1") AnalyzerSetup.Voltage[i] = 0;
        if(config["Outputs"][n]["Voltage"][i] == "CH2") AnalyzerSetup.Voltage[i] = 1;
        if(config["Outputs"][n]["Voltage"][i] == "CH3") AnalyzerSetup.Voltage[i] = 2;
      }
      for(uint i = 0; i < ((AnalyzerSetup.Mode == Analyzer::PHASE3) ? 3 : 4); i++) {
        string Current = config["Outputs"][n]["Current"][i];
        if(Current.find("CH1") != string::npos) AnalyzerSetup.Current[i] = 0;
        if(Current.find("CH2") != string::npos) AnalyzerSetup.Current[i] = 1;
        if(Current.find("CH3") != string::npos) AnalyzerSetup.Current[i] = 2;
        if(Current.find("CH4") != string::npos) AnalyzerSetup.Current[i] = 3;
        AnalyzerSetup.CurrentInvert[i] = (Current.find("-") != string::npos);
      }
      Name += (AnalyzerSetup.Mode == Analyzer::PHASE3) ? "_3P" : "_3PN";
    }
    pAnalyzersSetup.push_back(AnalyzerSetup);
    pAnalyzers.push_back(new AnalyzerSoft(Name, AnalyzerSetup.Mode, "/Devices/" + pSerial + "/Outputs/" + to_string(n) + "/Analyzer", DEVICEV2_01_SAMPLEFREQ));
  }
  pAnalyzersLock.WriteUnlock();
  if(libusb_init(&pContext) < 0) {
    LOGFile::Error("Can not open libUSB...\n");
    return false;
  }
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    pCaptureThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    libusb_exit(pContext);
    return false;
  }
  pthread_setname_np(pCaptureThread, "DeviceV2_01");
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
  ConfigOUT["CurrentGain"] = vector<float>(4, 0.0);
  ConfigOUT["CurrentOffset"] = vector<float>(4, 0.0);
  ConfigOUT["CurrentLevel"] = vector<float>(4, 1000000.0);
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["Outputs"] = json::array();
  if(config.is_object()) {
    if(config.contains("VoltageGain") && config["VoltageGain"].is_array() && (config["VoltageGain"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(config["VoltageGain"][n].is_number() && (config["VoltageGain"][n] >= -10.0) && (config["VoltageGain"][n] <= 10.0)) ConfigOUT["VoltageGain"][n] = config["VoltageGain"][n];
      }
    }
    if(config.contains("VoltageOffset") && config["VoltageOffset"].is_array() && (config["VoltageOffset"].size() == 3)) {
      for(uint n = 0; n < 3; n++) {
        if(config["VoltageOffset"][n].is_number() && (config["VoltageOffset"][n] >= -20.0) && (config["VoltageOffset"][n] <= 20.0)) ConfigOUT["VoltageOffset"][n] = config["VoltageOffset"][n];
      }
    }
    if(config.contains("CurrentGain") && config["CurrentGain"].is_array() && (config["CurrentGain"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentGain"][n].is_number() && (config["CurrentGain"][n] >= -10.0) && (config["CurrentGain"][n] <= 10.0)) ConfigOUT["CurrentGain"][n] = config["CurrentGain"][n];
      }
    }
    if(config.contains("CurrentOffset") && config["CurrentOffset"].is_array() && (config["CurrentOffset"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentOffset"][n].is_number() && (config["CurrentOffset"][n] >= -1000.0) && (config["CurrentOffset"][n] <= 1000.0)) ConfigOUT["CurrentOffset"][n] = config["CurrentOffset"][n];
      }
    }
    if(config.contains("CurrentLevel") && config["CurrentLevel"].is_array() && (config["CurrentLevel"].size() == 4)) {
      for(uint n = 0; n < 4; n++) {
        if(config["CurrentLevel"][n].is_number() && (config["CurrentLevel"][n] > 0.001) && (config["CurrentLevel"][n] <= 1000000.0)) ConfigOUT["CurrentLevel"][n] = config["CurrentLevel"][n];
      }
    }
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Outputs") && config["Outputs"].is_array()) {
      uint8_t CurrentMask = 0x00;
      for(uint8_t n = 0; n < config["Outputs"].size(); n++) {
        if(!config["Outputs"][n].is_object()) continue;
        json Output     = json::object();
        bool Error      = false;
        Output["Mode"]  = "1PHASE";
        if(config["Outputs"][n].contains("Mode") && config["Outputs"][n]["Mode"].is_string() && ((config["Outputs"][n]["Mode"] == "1PHASE") || (config["Outputs"][n]["Mode"] == "3PHASE") || (config["Outputs"][n]["Mode"] == "3PHASE+N"))) Output["Mode"] = config["Outputs"][n]["Mode"];
        if(Output["Mode"] == "1PHASE") {
          Output["Voltage"] = "CH1";
          Output["Current"] = "+CH1";
          if(config["Outputs"][n].contains("Voltage") && config["Outputs"][n]["Voltage"].is_string() && ((config["Outputs"][n]["Voltage"] == "CH1") || (config["Outputs"][n]["Voltage"] == "CH2") || (config["Outputs"][n]["Voltage"] == "CH3"))) Output["Voltage"] = config["Outputs"][n]["Voltage"];
          if(config["Outputs"][n].contains("Current") && config["Outputs"][n]["Current"].is_string() && ((config["Outputs"][n]["Current"] == "+CH1") || (config["Outputs"][n]["Current"] == "-CH1") || (config["Outputs"][n]["Current"] == "+CH2") || (config["Outputs"][n]["Current"] == "-CH2") || (config["Outputs"][n]["Current"] == "+CH3") || (config["Outputs"][n]["Current"] == "-CH3") || (config["Outputs"][n]["Current"] == "+CH4") || (config["Outputs"][n]["Current"] == "-CH4"))) Output["Current"] = config["Outputs"][n]["Current"];
          string CurrentCH = Output["Current"];
          uint8_t Channel = CurrentCH[3] - '1';
          if((CurrentMask & (1 << Channel)) != 0) Error = true;
          CurrentMask |= (1 << Channel);
        } else {
          Output["Voltage"] = json::parse("[\"CH1\", \"CH2\", \"CH3\"]");
          Output["Current"] = ((Output["Mode"] == "3PHASE") ? json::parse("[\"+CH1\", \"+CH2\", \"+CH3\"]") : json::parse("[\"+CH1\", \"+CH2\", \"+CH3\", \"+CH4\"]"));
          if(config["Outputs"][n].contains("Voltage") && config["Outputs"][n]["Voltage"].is_array() && (config["Outputs"][n]["Voltage"].size() == 3) && config["Outputs"][n].contains("Current") && config["Outputs"][n]["Current"].is_array() && (config["Outputs"][n]["Current"].size() == ((Output["Mode"] == "3PHASE") ? 3 : 4))) {
            for(uint8_t i = 0; i < 3; i++) {
              if(config["Outputs"][n]["Voltage"][i].is_string() && ((config["Outputs"][n]["Voltage"][i] == "CH1") || (config["Outputs"][n]["Voltage"][i] == "CH2") || (config["Outputs"][n]["Voltage"][i] == "CH3"))) Output["Voltage"][i] = config["Outputs"][n]["Voltage"][i];
            }
            for(uint8_t i = 0; i < ((Output["Mode"] == "3PHASE") ? 3 : 4); i++) {
              if(config["Outputs"][n]["Current"][i].is_string() && ((config["Outputs"][n]["Current"][i] == "+CH1") || (config["Outputs"][n]["Current"][i] == "-CH1") || (config["Outputs"][n]["Current"][i] == "+CH2") || (config["Outputs"][n]["Current"][i] == "-CH2") || (config["Outputs"][n]["Current"][i] == "+CH3") || (config["Outputs"][n]["Current"][i] == "-CH3") || (config["Outputs"][n]["Current"][i] == "+CH4") || (config["Outputs"][n]["Current"][i] == "-CH4"))) Output["Current"][i] = config["Outputs"][n]["Current"][i];
            }
          }
        }
        for(uint8_t i = 0; i < ((Output["Mode"] == "3PHASE") ? 3 : 4); i++) {
          string CurrentCH = Output["Current"][i];
          uint8_t Channel = CurrentCH[3] - '1';
          if((CurrentMask & (1 << Channel)) != 0) Error = true;
          CurrentMask |= (1 << Channel);
        }
        Output["Analyzer"] = AnalyzerSoft::CheckConfig(config["Outputs"][n].contains("Analyzer") ? config["Outputs"][n]["Analyzer"] : json::object());
        if(Error == false) ConfigOUT["Outputs"].push_back(Output);
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
      This->pCapture.Voltage[0] = ((This->pCapture.Voltage[0] * 0.110266113281) * (1.0 + (This->pVoltageGain[0] / 100.0))) + This->pVoltageOffset[0];
      This->pCapture.Voltage[1] = ((This->pCapture.Voltage[1] * 0.110266113281) * (1.0 + (This->pVoltageGain[1] / 100.0))) + This->pVoltageOffset[1];
      This->pCapture.Voltage[2] = ((This->pCapture.Voltage[2] * 0.110266113281) * (1.0 + (This->pVoltageGain[2] / 100.0))) + This->pVoltageOffset[2];
      This->pCapture.Current[0] = ((This->pCapture.Current[0] * 201.416015625) * (1.0 + (This->pCurrentGain[0] / 100.0))) / This->pCurrentLevel[0] + This->pCurrentOffset[0];
      This->pCapture.Current[1] = ((This->pCapture.Current[1] * 201.416015625) * (1.0 + (This->pCurrentGain[1] / 100.0))) / This->pCurrentLevel[1] + This->pCurrentOffset[1];
      This->pCapture.Current[2] = ((This->pCapture.Current[2] * 201.416015625) * (1.0 + (This->pCurrentGain[2] / 100.0))) / This->pCurrentLevel[2] + This->pCurrentOffset[2];
      This->pCapture.Current[3] = ((This->pCapture.Current[3] * 201.416015625) * (1.0 + (This->pCurrentGain[3] / 100.0))) / This->pCurrentLevel[3] + This->pCurrentOffset[3];
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
  for(uint8_t n = 0; n < Config["Outputs"].size(); n++) Config["Outputs"][n].erase("Analyzer");
  Sample_t Avg;
  Avg.Voltage[0] = 0.0;
  Avg.Voltage[1] = 0.0;
  Avg.Voltage[2] = 0.0;
  Avg.Current[0] = 0.0;
  Avg.Current[1] = 0.0;
  Avg.Current[2] = 0.0;
  Avg.Current[3] = 0.0;
  pAnalyzersLock.ReadLock();
  for(deque<Sample_t>::iterator it = pAverageValues.begin(); it != pAverageValues.end(); it++) {
    Avg.Voltage[0] += it->Voltage[0];
    Avg.Voltage[1] += it->Voltage[1];
    Avg.Voltage[2] += it->Voltage[2];
    Avg.Current[0] += it->Current[0];
    Avg.Current[1] += it->Current[1];
    Avg.Current[2] += it->Current[2];
    Avg.Current[3] += it->Current[3];
  }
  pAnalyzersLock.ReadUnlock();
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
  req.FromJSON(Config);
}
void DeviceV2_01::SetDevice(HTTPRequest &req) {
  pthread_mutex_lock(&pMutex);
  DeviceState State = pState;
  pthread_mutex_unlock(&pMutex);
  StopDevice();
  json Params = req.GetParams();
  json Config = CheckConfig(Params);
  json Orig = ConfigManager::ReadConfig("/Devices/" + pSerial);
  for(uint32_t n = 0; n < Config["Outputs"].size(); n++) {
    if(n >= Orig["Outputs"].size()) break;
    if(Config["Outputs"][n]["Mode"] == Orig["Outputs"][n]["Mode"]) Config["Outputs"][n] == Orig["Outputs"][n];
  }
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
  pAverageValues.push_back(pCapture);
  if(pAverageValues.size() > DEVICEV2_01_SAMPLEFREQ) pAverageValues.pop_front();
  for(uint8_t i = 0; i < pAnalyzers.size(); i++) {
    if(((AnalyzerSoft*)pAnalyzers[i])->IsFinished() == false) {
      pAnalyzersLock.ReadUnlock();
      return true;
    }
  }
  pAnalyzersLock.ReadUnlock();
  bool Started = false;
  while((pBuffer.empty() == false) && (Started == false)) {
    Sample_t Sample = pBuffer.front();
    pBuffer.pop_front();
    pAnalyzersLock.ReadLock();
    for(uint32_t i = 0; i < pAnalyzers.size(); i++) {
      vector<float> Voltage, Current;
      for(uint8_t n = 0; n < ((pAnalyzersSetup[i].Mode == Analyzer::PHASE1) ? 1 : 3); n++) Voltage.push_back(Sample.Voltage[pAnalyzersSetup[i].Voltage[n]]);
      for(uint8_t n = 0; n < ((pAnalyzersSetup[i].Mode == Analyzer::PHASE1) ? 1 : (pAnalyzersSetup[i].Mode == Analyzer::PHASE3) ? 3 : 4); n++) Current.push_back(Sample.Current[pAnalyzersSetup[i].Current[n]] * (pAnalyzersSetup[i].CurrentInvert[n] ? -1.0 : 1.0));
      if(((AnalyzerSoft*)pAnalyzers[i])->BufferAppend(Voltage, Current, Sampletime) == true) Started = true;
    }
    pAnalyzersLock.ReadUnlock();
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
  libusb_release_interface(This->pDeviceHandle, 0);
  libusb_close(This->pDeviceHandle);
  libusb_exit(This->pContext);
  return NULL;
}
//======================================================================================================================