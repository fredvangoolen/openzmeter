/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "DeviceRemote.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::pRegistered = DeviceRemote::GetInstances();
DeviceRemote *DeviceRemote::pInstance = NULL;
// Funciones ===========================================================================================================
bool DeviceRemote::GetInstances() {
  LOGFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'\n");
  pInstance = new DeviceRemote();
  DeviceManager::AddDevice(pInstance);
  return true;
}
DeviceRemote::DeviceRemote() {
  pSerial = "REMOTE";
  pDriver = "Remote";
  pName   = "Allow repplication of remote openZmeter devices in local machine";
  pMutex  = PTHREAD_MUTEX_INITIALIZER;
}
DeviceRemote::~DeviceRemote() {
  StopDevice();
  WaitInstances();
  pInstance = NULL;
}
bool DeviceRemote::Start(const json &config) {
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }  
  for(uint n = 0; n < config["Analyzers"].size(); n++) pAnalyzers.push_back(new AnalyzerRemote(config["Analyzers"][n]));
  pAnalyzersLock.WriteUnlock();
  pState = RUNNING;
  return true;
}
json DeviceRemote::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Analyzers"] = json::array();
  ConfigOUT["AutoStart"] = false;
  if(config.is_object()) {
    if(config.contains("AutoStart") && config["AutoStart"].is_boolean()) ConfigOUT["AutoStart"] = config["AutoStart"];
    if(config.contains("Analyzers") && config["Analyzers"].is_array()) {
      for(uint n = 0; n < config["Analyzers"].size(); n++) {
        if(config["Analyzers"][n].is_string()) ConfigOUT["Analyzers"].push_back(config["Analyzers"][n]);
      }
    }
  }
  return ConfigOUT;
}
bool DeviceRemote::StartDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'Remote' driver configuration\n");
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
  LOGFile::Info("Starting 'Remote' driver\n");
  bool Result = Start(Config);
  HTTPServer::RegisterCallback("setRemote",  SetRemoteCallback);
  HTTPServer::RegisterCallback("getRemote",  GetRemoteCallback);
  pthread_mutex_unlock(&pMutex);
  return Result;
}
bool DeviceRemote::StopDevice() {
  pthread_mutex_lock(&pMutex);
  if(pState != RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Stopping 'Remote' driver...\n");
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterCallback("getRemote");
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
void DeviceRemote::GetDevice(HTTPRequest &req) {
  req.FromJSON(ConfigManager::ReadConfig("/Devices/" + pSerial));
}
void DeviceRemote::SetDevice(HTTPRequest &req) {
}
void DeviceRemote::GetRemoteCallback(HTTPRequest &req) {
  pInstance->Reference();
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pInstance->GetRemote(req);
  pInstance->Dereference();
}
void DeviceRemote::SetRemoteCallback(HTTPRequest &req) {
  pInstance->Reference();
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pInstance->SetRemote(req);
  pInstance->Dereference();
}
void DeviceRemote::GetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->GetRemote(req);
    pAnalyzersLock.ReadUnlock();
    return;
  }
  pAnalyzersLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceRemote::SetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->SetRemote(req);
    pAnalyzersLock.ReadUnlock();
    return;
  }
  pAnalyzersLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}