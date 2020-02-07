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
DeviceRemote::DeviceRemote() : Device("REMOTE", "Remote", "Allow repplication of remote openZmeter devices in local machine") {
}
DeviceRemote::~DeviceRemote() {
  StopDevice();
}
bool DeviceRemote::StartDevice() {
  pMembersLock.WriteLock();
  if(pState == RUNNING) {
    pMembersLock.Unlock();
    return false;
  }
  LOGFile::Info("Loading 'Remote' driver configuration\n");
  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + pSerial));
  bool AutoStart = Config["AutoStart"];
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pMembersLock.Unlock();
      return true;
    }
  }
  LOGFile::Info("Starting 'Remote' driver\n");
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  for(uint n = 0; n < Config["Analyzers"].size(); n++) pAnalyzers.push_back(new AnalyzerRemote(Config["Analyzers"][n]));
  pState = RUNNING;
  HTTPServer::RegisterCallback("setRemote",  SetRemoteCallback);
  HTTPServer::RegisterCallback("getRemote",  GetRemoteCallback);
  pMembersLock.Unlock();
  return (pState == RUNNING);
}
bool DeviceRemote::StopDevice() {
  pMembersLock.WriteLock();
  if(pState != RUNNING) {
    pMembersLock.Unlock();
    return false;
  }
  LOGFile::Info("Stopping 'Remote' driver...\n");
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterCallback("getRemote");
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pState = STOPPED;
  pMembersLock.Unlock();
  return true;
}
void DeviceRemote::GetDevice(HTTPRequest &req) {
  pMembersLock.ReadLock();
  req.FromJSON(ConfigManager::ReadConfig("/Devices/" + pSerial));
  pMembersLock.ReadUnlock();
}
void DeviceRemote::SetDevice(HTTPRequest &req) {
}
void DeviceRemote::GetRemoteCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["Name"].is_string()) return req.FromString("'Analyzer' and 'Name' params are requiered", MHD_HTTP_BAD_REQUEST);
  pInstance->GetRemote(req);
}
void DeviceRemote::SetRemoteCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pInstance->SetRemote(req);
}
void DeviceRemote::GetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->GetRemote(req);
    pMembersLock.ReadUnlock();
    return;
  }
  pMembersLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceRemote::SetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->SetRemote(req);
    pMembersLock.ReadUnlock();
    return;
  }
  pMembersLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}