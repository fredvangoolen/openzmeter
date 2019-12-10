/* ================================== zMeter ===========================================================================
 * File:     CaptureVirtual.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "DeviceVirtual.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceVirtual::pRegistered = DeviceVirtual::GetInstances();
// Funciones ===========================================================================================================
bool DeviceVirtual::GetInstances(){
  LOGFile::Info("Adding 'Virtual' device driver instance with serial 'VIRTUAL'\n");
  DeviceManager::AddDevice(new DeviceVirtual());
  return true;
}
json DeviceVirtual::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Voltage"] = 230.0;
  ConfigOUT["Frequency"] = 50.0;
  ConfigOUT["SampleRate"] = 10000.0;
  ConfigOUT["Current"] = 5.0;
  ConfigOUT["CurrentPhase"] = 0.0;
  ConfigOUT["Mode"] = "1PHASE";
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object());
  if(config.is_object()) {
    if(config.contains("Voltage") && config["Voltage"].is_number() && (config["Voltage"] >= 10) && (config["Voltage"] <= 1000)) ConfigOUT["Voltage"] = config["Voltage"];
    if(config.contains("Frequency") && config["Frequency"].is_number() && (((config["Frequency"] >= 10) && (config["Frequency"] <= 100)) || (config["Frequency"] == 0.0))) ConfigOUT["Frequency"] = config["Frequency"];
    if(config.contains("SampleRate") && config["SampleRate"].is_number() && (config["SampleRate"] >= 10000) && (config["SampleRate"] <= 100000)) ConfigOUT["SampleRate"] = config["SampleRate"];
    if(config.contains("Current") && config["Current"].is_number() && (config["Current"] >= 0.1) && (config["Current"] <= 1000)) ConfigOUT["Current"] = config["Current"];
    if(config.contains("CurrentPhase") && config["CurrentPhase"].is_number() && (config["CurrentPhase"] >= -180.0) && (config["CurrentPhase"] <= 180.0)) ConfigOUT["CurrentPhase"] = config["CurrentPhase"];    
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "1PHASE") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"]);
  }
  return ConfigOUT;
}
DeviceVirtual::DeviceVirtual() : Device("VIRTUAL", "Virtual", "Emulate most of real capture device for testing") {
  pMembersLock.WriteLock();
  pSampleRate = 10000;
  pVoltage    = 230.0;
  pFrequency  = 50.0;
  pMode       = Analyzer::PHASE1;
  pMembersLock.WriteUnlock();
}
DeviceVirtual::~DeviceVirtual() {
  StopDevice();
}
bool DeviceVirtual::StartDevice() {
  pMembersLock.WriteLock();
  if(pState == RUNNING) {
    pMembersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Loading 'Virtual' driver configuration\n");
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
  LOGFile::Info("Starting 'Virtual' driver\n");
  pVoltage      = Config["Voltage"];
  pFrequency    = Config["Frequency"];
  pSampleRate   = Config["SampleRate"];
  pCurrent      = Config["Current"];
  pCurrentPhase = Config["CurrentPhase"];    
  pMode         = (Config["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (Config["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
  pTerminate    = false;
  pState        = STOPPED;
  string ModeString = Config["Mode"];
  pAnalyzers.push_back(new AnalyzerSoft(pSerial + "_" + ModeString, pMode, "/Devices/" + pSerial + "/Analyzer", pSampleRate));
  if(pthread_create(&pThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pThread, "DeviceVirtual");
    pState = RUNNING;
  }
  pMembersLock.WriteUnlock();
  return (pState == RUNNING);
}
bool DeviceVirtual::StopDevice() {
  pMembersLock.WriteLock();
  if(pState != RUNNING) {
    pMembersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Stopping 'Virtual' driver...\n");
  pTerminate = true;
  pthread_join(pThread, NULL);
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pState = STOPPED;
  pMembersLock.WriteUnlock();
  return true;
}
void DeviceVirtual::GetDevice(HTTPRequest &req) {
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  req.FromJSON(Config);
}
void DeviceVirtual::SetDevice(HTTPRequest &req) {
  pMembersLock.ReadLock();
  DeviceState State = pState;
  pMembersLock.ReadUnlock();
  StopDevice();
  json Params = req.GetParams();
  json Config = CheckConfig(Params);
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  json Response = json::object();
  Response["Result"] = (State == RUNNING) ? StartDevice() : true;
  req.FromJSON(Response);
}
void *DeviceVirtual::Capture_Thread(void *args) {
  DeviceVirtual *This = (DeviceVirtual*)args;
  uint64_t       StartTime        = Tools::GetTime();
  uint64_t       GeneratedSamples = 0;
  AnalyzerSoft  *Analyzer         = (AnalyzerSoft*)This->pAnalyzers[0];
  int32_t        Phase            = (This->pSampleRate / This->pFrequency) * (This->pCurrentPhase / 360.0);
  while(!This->pTerminate) {
    float Voltage[3], Current[4];
    uint64_t Sampletime = Tools::GetTime();
    if((Analyzer->IsFinished() == true) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      if(This->pFrequency > 0.0) {
        Voltage[0] = This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency);
        Current[0] = This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency);
        if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
          Voltage[1] = This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0);
          Current[1] = This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0);
          Voltage[2] = This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0);
          Current[2] = This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0);
        }
        if(This->pMode == Analyzer::PHASE3) Current[3] = Current[0] + Current[1] + Current[2];
      } else {
        Voltage[0] = This->pVoltage;
        Current[0] = This->pCurrent;
        if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
          Voltage[1] = This->pVoltage;
          Current[1] = This->pCurrent;
          Voltage[2] = This->pVoltage;
          Current[2] = This->pCurrent;
        }
        if(This->pMode == Analyzer::PHASE3) Current[3] = 0.0;
      }
      GeneratedSamples++;
      Analyzer->BufferAppend(Voltage, Current);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
//======================================================================================================================
