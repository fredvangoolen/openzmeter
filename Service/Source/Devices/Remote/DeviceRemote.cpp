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
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "DeviceRemote.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::pRegistered = DeviceRemote::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
vector<Device*> DeviceRemote::GetInstances() {
  LOGFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'\n");
  vector<Device*> Result;
  Result.push_back(new DeviceRemote());
  return Result;
}
DeviceRemote::DeviceRemote() : Device("REMOTE", "Remote", "Allow repplication of remote openZmeter devices in local machine") {
  Configure(CheckConfig(ConfigManager::ReadConfig("/Devices/" + Serial)));
}
DeviceRemote::~DeviceRemote() {
}
json DeviceRemote::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
//  ConfigOUT["Analyzers"] = json::array();
  ConfigOUT["Enabled"] = false;
  if(config.is_object()) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
//    if(config.contains("Analyzers") && config["Analyzers"].is_array()) {
//      for(uint n = 0; n < config["Analyzers"].size(); n++) {
//        if(config["Analyzers"][n].is_string()) ConfigOUT["Analyzers"].push_back(config["Analyzers"][n]);
//      }
//    }
  }
  return ConfigOUT;
}
void DeviceRemote::Configure(const json &config) {
  ConfigManager::WriteConfig("/Devices/" + Serial, config);
  pLocks.WriteLock();
  pEnabled = config["Enabled"];
  pLocks.Unlock();
//  if(pState == RUNNING) {
//    pMembersLock.Unlock();
//    return false;
//  }
//  LOGFile::Info("Loading 'Remote' driver configuration\n");
//  json Config = CheckConfig(ConfigManager::ReadConfig("/Devices/" + pSerial));
//  if(pState == UNCONFIGURED) {
//    pState = STOPPED;
//    if(AutoStart == false) {
//      pMembersLock.Unlock();
//      return true;
//    }
//  }
//  LOGFile::Info("Starting 'Remote' driver\n");
//  while(pAnalyzers.size() > 0) {
//    Analyzer *Analyzer = pAnalyzers.back();
//    pAnalyzers.pop_back();
//    delete Analyzer;
//  }
//  for(uint n = 0; n < Config["Analyzers"].size(); n++) pAnalyzers.push_back(new AnalyzerRemote(Config["Analyzers"][n]));
//  pState = RUNNING;
//  HTTPServer::RegisterCallback("setRemote",  SetRemoteCallback);
//  HTTPServer::RegisterCallback("getRemote",  GetRemoteCallback);
//  pMembersLock.Unlock();
//  return (pState == RUNNING);
}
//bool DeviceRemote::StopDevice() {
//  pMembersLock.WriteLock();
//  if(pState != RUNNING) {
//    pMembersLock.Unlock();
//    return false;
//  }
//  LOGFile::Info("Stopping 'Remote' driver...\n");
//  HTTPServer::UnregisterCallback("setRemote");
//  HTTPServer::UnregisterCallback("getRemote");
//  while(pAnalyzers.size() > 0) {
//    Analyzer *Analyzer = pAnalyzers.back();
//    pAnalyzers.pop_back();
//    delete Analyzer;
//  }
//  pState = STOPPED;
//  pMembersLock.Unlock();
//  return true;
//}
void DeviceRemote::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/Devices/" + Serial);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceRemote::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Configure(params);
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
//void DeviceRemote::GetRemote(const json &params, Response &response) {
//  if((params.contains("Analyzers") == false) || (params["Analyzers"].is_array() == false)) return HTTPServer::FillResponse(response, "'Analyzers' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  for(auto Analyzer : params["Analyzers"]) {
//    if(Analyzer.is_object() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_BAD_REQUEST);
//    if((Analyzer.contains("Serial") == false) || (Analyzer["Serial"].is_string() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_BAD_REQUEST);
//    if((Analyzer.contains("Name") == false) || (Analyzer["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_BAD_REQUEST);


    //Busco analyzer si tiene algo lo cojo y relleno campos
    //si no tiene nada guardo




//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["Name"].is_string()) return req.FromString("'Analyzer' and 'Name' params are requiered", MHD_HTTP_BAD_REQUEST);
//  pInstance->GetRemote(req);
//  json Params = req.GetParams();
//  string Serial = Params["Analyzer"];
//  pMembersLock.ReadLock();
//  for(uint n = 0; n != pAnalyzers.size(); n++) {
//    if(pAnalyzers[n]->Serial() != Serial) continue;
//    ((AnalyzerRemote*)pAnalyzers[n])->GetRemote(req);
//    pMembersLock.ReadUnlock();
//    return;
//  }
//  pMembersLock.ReadUnlock();
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);


  //}





//S}
//void DeviceRemote::SetRemote(const json &params, Response &response) {
//if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  pInstance->SetRemote(req);
//  json Params = req.GetParams();
//  string Serial = Params["Analyzer"];
//  pMembersLock.ReadLock();
//  for(uint n = 0; n != pAnalyzers.size(); n++) {
//    if(pAnalyzers[n]->Serial() != Serial) continue;
//    ((AnalyzerRemote*)pAnalyzers[n])->SetRemote(req);
//    pMembersLock.ReadUnlock();
//    return;
//  }
//  pMembersLock.ReadUnlock();
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//}