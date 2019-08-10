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
bool DeviceVirtual::GetInstances() {
  LOGFile::Info("Adding 'Virtual' device driver instance with serial 'VIRTUAL'\n");
  DeviceManager::AddDevice(new DeviceVirtual());
  return true;
}
DeviceVirtual::DeviceVirtual() {
  pSampleRate = 10000;
  pVoltage    = 230.0;
  pFrequency  = 50.0;
  pMode       = Analyzer::PHASE1;
  pThread     = 0;
  pMutex      = PTHREAD_MUTEX_INITIALIZER;          
  pSerial     = "VIRTUAL";
  pDriver     = "Virtual";
  pName       = "Emulate most of real capture device for testing";
}
DeviceVirtual::~DeviceVirtual() {
  StopDevice();
  WaitInstances();
}
bool DeviceVirtual::Start(const json &config) {
  pVoltage = config["Voltage"];
  pFrequency = config["Frequency"];
  pSampleRate = config["Samplerate"];
  pCurrent = config["Current"];
  pCurrentPhase = config["CurrentPhase"];    
  pMode = (config["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (config["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
  pTerminate = false;
  string ModeString = config["Mode"];
  pAnalyzersLock.WriteLock();
  pAnalyzers.push_back(new AnalyzerSoft(pSerial + "_" + ModeString, pMode, "/Devices/" + pSerial + "/Analyzer", pSampleRate));
  pAnalyzersLock.WriteUnlock();
  if(pthread_create(&pThread, NULL, Capture_Thread, this) != 0) {
    pThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return false;
  }
  pthread_setname_np(pThread, "DeviceVirtual");
  pState = RUNNING;
  return true;
}
json DeviceVirtual::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Voltage"] = 230.0;
  ConfigOUT["Frequency"] = 50.0;
  ConfigOUT["Samplerate"] = 10000.0;
  ConfigOUT["Current"] = 5.0;
  ConfigOUT["CurrentPhase"] = 0.0;
  ConfigOUT["Mode"] = "1PHASE";
  ConfigOUT["AutoStart"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object());
  if(config.is_object()) {
    if(config.contains("Voltage") && config["Voltage"].is_number() && (config["Voltage"] >= 10) && (config["Voltage"] <= 1000)) ConfigOUT["Voltage"] = config["Voltage"];
    if(config.contains("Frequency") && config["Frequency"].is_number() && (config["Frequency"] >= 0) && (config["Frequency"] <= 100)) ConfigOUT["Frequency"] = config["Frequency"];
    if(config.contains("Samplerate") && config["Samplerate"].is_number() && (config["Samplerate"] >= 10000) && (config["Samplerate"] <= 100000)) ConfigOUT["Samplerate"] = config["Samplerate"];
    if(config.contains("Current") && config["Current"].is_number() && (config["Current"] >= 0.1) && (config["Current"] <= 1000)) ConfigOUT["Current"] = config["Current"];
    if(config.contains("CurrentPhase") && config["CurrentPhase"].is_number() && (config["CurrentPhase"] >= -180.0) && (config["CurrentPhase"] <= 180.0)) ConfigOUT["CurrentPhase"] = config["CurrentPhase"];    
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "1PHASE") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"]);
  }
  return ConfigOUT;
}
bool DeviceVirtual::StartDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'Virtual' driver configuration\n");
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
  LOGFile::Info("Starting 'Virtual' driver\n");
  bool Result = Start(Config);  
  pthread_mutex_unlock(&pMutex);
  return Result;
}
bool DeviceVirtual::StopDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'Virtual' driver...\n");
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pThread = 0;
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pAnalyzersLock.WriteUnlock();
  pState = STOPPED;
  pthread_mutex_unlock(&pMutex);
  return true;
}
void DeviceVirtual::GetDevice(HTTPRequest &req) {
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  req.FromJSON(Config);
}
void DeviceVirtual::SetDevice(HTTPRequest &req) {
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
void *DeviceVirtual::Capture_Thread(void *args) {
  DeviceVirtual *This = (DeviceVirtual*)args;
  uint64_t StartTime        = Tools::GetTime();
  uint64_t GeneratedSamples = 0;
  AnalyzerSoft *Analyzer    = (AnalyzerSoft*)This->pAnalyzers[0];
  int32_t Phase = (This->pSampleRate / This->pFrequency) * (This->pCurrentPhase / 360.0);
  while(!This->pTerminate) {
    vector<float> Voltage;
    vector<float> Current;
    uint64_t Sampletime = Tools::GetTime();
    if((Analyzer->IsFinished() == true) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency));
      Current.push_back(This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples - Phase) / This->pSampleRate * This->pFrequency));
      if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
        Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0));
        Current.push_back(This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples - Phase) / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0));
        Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0));
        Current.push_back(This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples - Phase) / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0));
      }
      if(This->pMode == Analyzer::PHASE3) Current.push_back(Current[0] + Current[1] + Current[2]);
      GeneratedSamples++;
      Analyzer->BufferAppend(Voltage, Current, Sampletime);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
//======================================================================================================================
