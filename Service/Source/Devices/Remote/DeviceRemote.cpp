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
pthread_rwlock_t             DeviceRemote::pLocks = PTHREAD_RWLOCK_INITIALIZER;
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
  pthread_rwlock_wrlock(&pLocks);
  while(pAnalyzers.size() > 0) {
    auto Analyzer = pAnalyzers.begin();
    AnalyzerManager::DelAnalyzer(Analyzer->second->Serial());
    delete Analyzer->second;
    pAnalyzers.erase(Analyzer);
  }
  pthread_rwlock_unlock(&pLocks);
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
  pthread_rwlock_wrlock(&pLocks);
  bool Enabled = config["Enabled"];
  if((pEnabled == false) && (Enabled == true)) {
    LogFile::Info("REMOTE", "Starting 'Remote' driver");
    HTTPServer::RegisterCallback("setRemote", SetRemoteCallback, { "X-Sequence", "X-Analyzer", "X-Status", "Content-Type" });
    HTTPServer::RegisterCallback("getRemote", GetRemoteCallback);
  } else if((pEnabled == true) && (Enabled == false)) {
    LogFile::Info("REMOTE", "Stopping 'Remote' driver...");
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
  pthread_rwlock_unlock(&pLocks);
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
  pthread_rwlock_rdlock(&pLocks);
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->GetRemote(params, response);
    pthread_rwlock_unlock(&pLocks);
    return;
  }
  pthread_rwlock_unlock(&pLocks);
  AnalyzerRemote *Analyzer = new AnalyzerRemote(Serial);
  pthread_rwlock_wrlock(&pLocks);
  if(AnalyzerManager::AddAnalyzer(Analyzer, params["UserName"]) == false) {
    delete Analyzer;
    pthread_rwlock_unlock(&pLocks);
    return HTTPServer::FillResponse(response, "'Analyzer' not allowed on remote system", "text/plain", HTTP_BAD_REQUEST);
  }
  pAnalyzers.insert( { Serial, Analyzer } );
  pthread_rwlock_unlock(&pLocks);
  LogFile::Info("REMOTE", "Added remote analyzer '%s'", Serial.c_str());
  pthread_rwlock_rdlock(&pLocks);
  Analyzer->GetRemote(params, response);
  pthread_rwlock_unlock(&pLocks);
}
void DeviceRemote::SetRemoteCallback(const map<string, string> &params, Response &response, const ContentReader &content_reader) {
  string Serial = params.find("X-Analyzer")->second;
  pthread_rwlock_rdlock(&pLocks);
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->SetRemote(params, response, content_reader);
    pthread_rwlock_unlock(&pLocks);
    return;
  }
  pthread_rwlock_unlock(&pLocks);
  HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
}
//----------------------------------------------------------------------------------------------------------------------