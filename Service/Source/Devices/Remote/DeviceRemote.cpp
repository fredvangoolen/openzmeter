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
}
DeviceRemote::~DeviceRemote() {
  StopDevice();
  WaitInstances();
  pInstance = NULL;
}
bool DeviceRemote::StartDevice() {
  pAnalyzersLock.WriteLock();
  if(pState == RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Loading 'Remote' driver configuration\n");
  bool AutoStart = false;
  json ConfigIN  = ConfigManager::ReadConfig("/Devices/" + pSerial);
  json ConfigOUT = json::object();
  ConfigOUT["Analyzers"] = json::array();
  if(ConfigIN["AutoStart"].is_boolean()) AutoStart = ConfigIN["AutoStart"];
  if(ConfigIN.is_object() && ConfigIN["Analyzers"].is_array()) {
    for(uint n = 0; n < ConfigIN["Analyzers"].size(); n++) {
      if(ConfigIN["Analyzers"][n].is_string()) ConfigOUT["Analyzers"].push_back(ConfigIN["Analyzers"][n]);
    }
  }
  ConfigOUT["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, ConfigOUT);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pAnalyzersLock.WriteUnlock();
      return true;
    }
  }
  LOGFile::Info("Starting 'Remote' driver\n");
  HTTPServer::RegisterCallback("setRemote",  SetRemoteCallback);
  HTTPServer::RegisterCallback("getRemote",  GetRemoteCallback);
  for(uint n = 0; n < ConfigOUT["Analyzers"].size(); n++) pAnalyzers.push_back(new AnalyzerRemote(ConfigOUT["Analyzers"][n]));
  pState = RUNNING;
  pAnalyzersLock.WriteUnlock();
  return true;
}
bool DeviceRemote::StopDevice() {
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterCallback("getRemote");
  pAnalyzersLock.WriteLock();
  if(pState != RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Stopping 'Remote' driver...\n");
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pState = STOPPED;  
  pAnalyzersLock.WriteUnlock();
  return true;
}
void DeviceRemote::GetDevice(HTTPRequest &req) {
}
void DeviceRemote::SetDevice(HTTPRequest &req) {
}
void DeviceRemote::GetRemoteCallback(HTTPRequest &req) {
  pInstance->Reference();
  pInstance->GetRemote(req);
  pInstance->Dereference();
}
void DeviceRemote::SetRemoteCallback(HTTPRequest &req) {
  pInstance->Reference();
  pInstance->SetRemote(req);
  pInstance->Dereference();
}
void DeviceRemote::GetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->GetRemote(req);
    break;
  }
  pAnalyzersLock.ReadUnlock();
}
void DeviceRemote::SetRemote(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    ((AnalyzerRemote*)pAnalyzers[n])->SetRemote(req);
    break;
  }
  pAnalyzersLock.ReadUnlock();
}