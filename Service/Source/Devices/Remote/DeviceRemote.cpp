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
#include "AnalyzerManager.h"
#include "LogFile.h"
#include "ConfigFile.h"
//----------------------------------------------------------------------------------------------------------------------
bool                         DeviceRemote::pRegistered = DeviceRemote::Register();
RWLocks                      DeviceRemote::pLocks;
map<string, AnalyzerRemote*> DeviceRemote::pAnalyzers;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceRemote::GetInstances() {
  LogFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'");
  DeviceManager::AddDevice(new DeviceRemote());
}
DeviceRemote::DeviceRemote() : Device("REMOTE", "Remote", "Allow repplication of remote openZmeter devices in local machine") {
  Configure(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
}
DeviceRemote::~DeviceRemote() {
  LogFile::Info("Stopping device with serial '%s'", pSerial.c_str());
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterCallback("getRemote");
  pLocks.WriteLock();
  while(pAnalyzers.size() > 0) {
    auto Analyzer = pAnalyzers.begin();
    AnalyzerManager::DelAnalyzer(Analyzer->second->Serial());
    delete Analyzer->second;
    pAnalyzers.erase(Analyzer);
  }
  pLocks.Unlock();
  LogFile::Info("Device with serial '%s' stopped", pSerial.c_str());
}
uint32_t DeviceRemote::AnalyzersCount() {
  return pAnalyzers.size();
}
json DeviceRemote::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  if(config.is_object()) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void DeviceRemote::Configure(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  pLocks.WriteLock();
  bool Enabled = config["Enabled"];
  if((pEnabled == false) && (Enabled == true)) {
    LogFile::Info("Starting 'Remote' driver");
    HTTPServer::RegisterCallback("setRemote", SetRemoteCallback, { "X-Sequence", "X-Analyzer", "X-Status", "Content-Type" });
    HTTPServer::RegisterCallback("getRemote", GetRemoteCallback);
  } else if((pEnabled == true) && (Enabled == false)) {
    LogFile::Info("Stopping 'Remote' driver...");
    HTTPServer::UnregisterCallback("setRemote");
    HTTPServer::UnregisterCallback("getRemote");
    while(pAnalyzers.size() > 0) {
      auto Analyzer = pAnalyzers.begin();
      AnalyzerManager::DelAnalyzer(Analyzer->second->Serial());
      delete Analyzer->second;
      pAnalyzers.erase(Analyzer);
    }
  }
  pEnabled = Enabled;
  pLocks.Unlock();
}
void DeviceRemote::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceRemote::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void DeviceRemote::GetRemoteCallback(const json &params, Response &response) {
  if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Analyzer"];
  pLocks.ReadLock();
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->GetRemote(params, response);
    pLocks.Unlock();
    return;
  }
  pLocks.Unlock();
  AnalyzerRemote *Analyzer = new AnalyzerRemote(Serial);
  if(AnalyzerManager::AddAnalyzer(Analyzer, params["UserName"]) == false) {
    delete Analyzer;
    return HTTPServer::FillResponse(response, "'Analyzer' not allowed on remote system", "text/plain", HTTP_BAD_REQUEST);
  }
  pLocks.WriteLock();
  LogFile::Info("Added remote analyzer '%s'", Serial.c_str());
  pAnalyzers.insert( { Serial, Analyzer } );
  pLocks.Unlock();
  pLocks.ReadLock();
  Analyzer->GetRemote(params, response);
  pLocks.Unlock();
}
void DeviceRemote::SetRemoteCallback(const map<string, string> &params, Response &response, const ContentReader &content_reader) {
  string Serial = params.find("X-Analyzer")->second;
  pLocks.ReadLock();
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->SetRemote(params, response, content_reader);
    pLocks.Unlock();
    return;
  }
  pLocks.Unlock();
  HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
}
//----------------------------------------------------------------------------------------------------------------------