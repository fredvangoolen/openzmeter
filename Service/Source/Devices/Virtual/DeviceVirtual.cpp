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
  pSerial     = "VIRTUAL";
  pDriver     = "Virtual";
  pName       = "Emulate most of real capture device for testing";
}
DeviceVirtual::~DeviceVirtual() {
  StopDevice();
}
bool DeviceVirtual::StartDevice() {
  pAnalyzersLock.WriteLock();
  if(pState == RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Loading 'Virtual' driver configuration\n");
  bool AutoStart = false;
  json ConfigIN = ConfigManager::ReadConfig("/Devices/" + pSerial);
  if(ConfigIN.is_object()) {
    if(ConfigIN["Voltage"].is_number()) pVoltage = ConfigIN["Voltage"];
    if(ConfigIN["Frequency"].is_number()) pFrequency = ConfigIN["Frequency"];
    if(ConfigIN["Samplerate"].is_number()) pSampleRate = ConfigIN["Samplerate"];
    if(ConfigIN["AutoStart"].is_boolean()) AutoStart = ConfigIN["AutoStart"];
    if(ConfigIN["Current"].is_number()) pCurrent = ConfigIN["Current"];
    if(ConfigIN["CurrentPhase"].is_number()) pCurrentPhase = ConfigIN["CurrentPhase"];    
    if(ConfigIN["Mode"].is_string() && ((ConfigIN["Mode"] == "1PHASE") || (ConfigIN["Mode"] == "3PHASE") || (ConfigIN["Mode"] == "3PHASE+N"))) pMode = (ConfigIN["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (ConfigIN["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
  }
  if((pVoltage < 10) || (pVoltage > 350)) pVoltage = 230.0;
  if((pFrequency < 10) || (pFrequency > 100)) pFrequency = 50.0;
  if((pCurrent < 0.1) || (pCurrent > 1000)) pCurrent = 5.0;
  if((pCurrentPhase < -180.0) || (pCurrentPhase > 180.0)) pCurrentPhase = 0;
  if((pSampleRate < (pFrequency * 100))) pSampleRate = (pFrequency * 100);
  json ConfigOUT = json::object();
  ConfigOUT["Voltage"] = pVoltage;
  ConfigOUT["Frequency"] = pFrequency;
  ConfigOUT["Samplerate"] = pSampleRate;
  ConfigOUT["Current"] = pCurrent;
  ConfigOUT["CurrentPhase"] = pCurrentPhase;
  ConfigOUT["Mode"] = (pMode == Analyzer::PHASE1) ? "1PHASE" : (pMode == Analyzer::PHASE3) ? "3PHASE" : "3PHASE+N";
  ConfigOUT["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, ConfigOUT);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pAnalyzersLock.WriteUnlock();
      return true;
    }
  }
  string ModeString = ConfigOUT["Mode"];
  LOGFile::Info("Starting 'Virtual' driver\n");
  pTerminate = false;
  pAnalyzers.push_back(new AnalyzerSoft(pSerial + "_" + ModeString, pMode, ConfigIN["Analyzer"], pSampleRate));
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pAnalyzersLock.WriteUnlock();
    StopDevice();
    return false;
  }
  pState = RUNNING;
  pAnalyzersLock.WriteUnlock();
  return true;
}
bool DeviceVirtual::StopDevice() {
  pAnalyzersLock.WriteLock();
  if(pState != RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Stopping 'Virtual' driver...\n");
  pTerminate = true;
  pthread_join(pCaptureThread, NULL);
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pState = STOPPED;
  pAnalyzersLock.WriteUnlock();
  return true;
}
void DeviceVirtual::GetDevice(HTTPRequest &req) {
//  ReadLock();
  req.FromJSON(ConfigManager::ReadConfig("/Devices/" + pSerial));
//  ReadUnlock();
}
void DeviceVirtual::SetDevice(HTTPRequest &req) {
//  json Params = req.GetParams();
//  if(!Params["Settings"].is_object()) return req.FromString("'Settings' param is required", MHD_HTTP_BAD_REQUEST);
//  WriteLock();
//  DeviceState State = pState;
//  ConfigManager::WriteConfig("/Devices/" + pSerial, Params["Settings"]);
//  WriteUnlock();
//  json Response;
//  if(State == Device::RUNNING) {
//    StopDevice();
//    Response["Result"] = StartDevice();
//  } else {
//    Response["Result"] = true;
//  }
//  req.FromJSON(Response);
}
void *DeviceVirtual::Capture_Thread(void *args) {
  int Prio = sched_get_priority_max(SCHED_RR);
  if(Prio == -1) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio - 20;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise thread priority -> %s\n", __func__, __LINE__, strerror(errno));
  }
  DeviceVirtual *This = (DeviceVirtual*)args;
  pthread_setname_np(This->pCaptureThread, "DeviceVirtual");
  uint64_t StartTime        = Tools::GetTime();
  uint64_t GeneratedSamples = 0;
  AnalyzerSoft *Analyzer    = (AnalyzerSoft*)This->pAnalyzers[0];
  while(!This->pTerminate) {
    vector<float> Voltage;
    vector<float> Current;
    uint64_t Sampletime = Tools::GetTime();
    if((Analyzer->IsFinished() == true) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency));
      Current.push_back(                                 (((Sampletime % 20000) < 10000) ? 0.5 : 0.02) * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency));
//      Voltage[0] = Voltage[0] + Voltage[0] * 0.05 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 300.0));
//      Current[0] = Current[0] + Current[0] * 0.1 * sin((2.0 * M_PI * GeneratedSamples) / (This->pSampleRate * 360.0));

//      if(((Sampletime - StartTime) > 20000) && ((Sampletime - StartTime) < 30000)) {
//        Voltage[0] *= 0.94;
//      } else if(((Sampletime - StartTime) >= 30000) && ((Sampletime - StartTime) < 40000)) {
//        Voltage[0] *= 0.8;
//      } else if(((Sampletime - StartTime) >= 40000) && ((Sampletime - StartTime) < 50000)) {
//        Voltage[0] *= 0.01;
//      }
      if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
        Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0));
        Voltage[1] = Voltage[1] + Voltage[1] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
        Current.push_back(-5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0));
        Current[1] = Current[1] + Current[1] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 120.0)) / (This->pSampleRate * 100.0));
        Voltage.push_back(This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0));
        Voltage[2] = Voltage[2] + Voltage[2] * 0.05 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 100.0)) / (This->pSampleRate * 100.0));
        Current.push_back(-5.0 * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0));
        Current[2] = Current[2] + Current[2] * 0.1 * sin((2.0 * M_PI * (GeneratedSamples - This->pSampleRate * 240.0)) / (This->pSampleRate * 100.0));
      }
      if(This->pMode == Analyzer::PHASE3) {
        Current.push_back(Current[0] + Current[1] + Current[2]);
      }
      GeneratedSamples++;
      Analyzer->BufferAppend(Voltage, Current, Sampletime);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
//======================================================================================================================
