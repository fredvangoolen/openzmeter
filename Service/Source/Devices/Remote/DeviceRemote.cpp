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
#include <regex>
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
bool DeviceRemote::pRegistered = DeviceRemote::Register();
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
  LogFile::Info("Starting DeviceRemote...");
  pMutex = PTHREAD_MUTEX_INITIALIZER;
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  LogFile::Info("DeviceManager started");
}
void DeviceRemote::ClearAnalyzers() {
  pthread_mutex_lock(&pMutex);
  while(pAnalyzers.size() > 0) {
    AnalyzerManager::DelAnalyzer(pAnalyzers.back()->Serial());
    delete pAnalyzers.back();
    pAnalyzers.pop_back();
  }
  pthread_mutex_unlock(&pMutex);
}
DeviceRemote::~DeviceRemote() {
  LogFile::Info("Stopping DeviceRemote");
  ClearAnalyzers();
  LogFile::Info("DeviceRemote stopped");
}
uint32_t DeviceRemote::AnalyzersCount() {
  return pAnalyzers.size();
}
json DeviceRemote::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Analyzers"] = json::array();
  if(config.is_object() == false) return ConfigOUT;
  if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  if(config.contains("Analyzers") && config["Analyzers"].is_array()) {
    set<string> Analyzers;
    regex Validator("^[0-9A-F]{24}(_CAL|_CH1|_CH2|_CH3|_CH4)?(_1P|_3P|_3PN)$");
    for(auto &Analyzer : config["Analyzers"]) {
      if(Analyzer.is_object() == false) continue;
      if((Analyzer.contains("User") == false) || (Analyzer["User"].is_string() == false)) continue;
      if((Analyzer.contains("Analyzer") == false) || (Analyzer["Analyzer"].is_string() == false)) continue;
      string UserName = Analyzer["User"];
      string Serial   = Analyzer["Analyzer"];
      json   Tmp      = json::object();
      Tmp["User"]     = UserName;
      Tmp["Analyzer"] = Serial;
      if((regex_match(Serial, Validator) == false) || (Analyzers.find(Serial) != Analyzers.end())) continue;
      ConfigOUT["Analyzers"].push_back(Tmp);
      Analyzers.insert(Serial);
    }
  }
  return ConfigOUT;
}
void DeviceRemote::Config(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  ClearAnalyzers();
  pthread_mutex_lock(&pMutex);
  pEnabled = config["Enabled"];
  if(pEnabled == true) {
    for(auto &Analyzer : config["Analyzers"]) {
      string                 Serial = Analyzer["Analyzer"];
      Analyzer::AnalyzerMode Mode = (Serial.find("_1P") != string::npos) ? Analyzer::PHASE1 : (Serial.find("_3PN") != string::npos) ? Analyzer::PHASE3N : Analyzer::PHASE3;
      pAnalyzers.push_back(new AnalyzerRemote(Serial, Mode, Analyzer["User"]));
      AnalyzerManager::AddAnalyzer(pAnalyzers.back());
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void DeviceRemote::GetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  Database DB;
  for(auto &Analyzer : Config["Analyzers"]) {
    string Serial = Analyzer["Analyzer"];
    DB.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename)))::bigint FROM pg_tables WHERE schemaname = " + DB.Bind(Serial));
    Analyzer["Size"] = (DB.FecthRow() == true) ? DB.ResultUInt64(0) : 0;
  }
  req.Success(Config);
}
void DeviceRemote::SetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  Config(CheckConfig(req.Params()));
  req.Success(json::object());
}
//----------------------------------------------------------------------------------------------------------------------