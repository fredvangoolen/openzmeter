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
#include <math.h>
#include "Device.h"
#include "HTTPServer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
Device::Device(const string &serial, const string &driver, const string &name) : Serial(serial), Driver(driver), Name(name) {
  pEnabled = false;
}
Device::~Device() {
  ClearAnalyzers();
}
void Device::AddAnalyzer(Analyzer *analyzer) {
  pAnalyzersLock.WriteLock();
  auto Tmp = pAnalyzers.insert(analyzer);
  if(Tmp.second == false) delete analyzer;
  pAnalyzersLock.Unlock();
}
void Device::ClearAnalyzers() {
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    auto Tmp = pAnalyzers.begin();
    delete *Tmp;
    pAnalyzers.erase(Tmp);
  }
  pAnalyzersLock.Unlock();
}
json Device::GetJSONDescriptor() {
  json Device = json::object();
  Device["Driver"]    = Driver;
  Device["Name"]      = Name;
  Device["Serial"]    = Serial;
  Device["Enabled"]   = pEnabled;
  pAnalyzersLock.ReadLock();
  Device["Analyzers"] = pAnalyzers.size();
  pAnalyzersLock.Unlock();
  return Device;
}
json Device::GetJSONAnalyzers(const string &user) {
  json Result = json::array();
  pAnalyzersLock.ReadLock();
  for(auto &Tmp : pAnalyzers) {
    if(Tmp->CheckRights(user, Analyzer::READ)) Result.push_back(Tmp->GetJSONDescriptor());
  }
  pAnalyzersLock.Unlock();
  return Result;
}
void Device::GetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_SERVICE_UNAVAILABLE);
}
void Device::SetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_SERVICE_UNAVAILABLE);
}
bool Device::CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response)) {
  string Serial = params["Analyzer"];
  pAnalyzersLock.ReadLock();
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp == pAnalyzers.end()) {
    pAnalyzersLock.Unlock();
    return false;
  } else {
    ((*Tmp)->*method)(params, response);
    pAnalyzersLock.Unlock();
    return true;
  }
}
bool Device::GetAnalyzer(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetAnalyzer);
}
bool Device::SetAnalyzer(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetAnalyzer);
}
bool Device::GetSeries(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeries);
}
bool Device::GetSeriesNow(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeriesNow);
}
bool Device::GetSeriesRange(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeriesRange);
}
bool Device::GetEvents(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetEvents);
}
bool Device::GetEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetEvent);
}
bool Device::SetEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetEvent);
}
bool Device::DelEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelEvent);
}
bool Device::GetRates(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetRates);
}
bool Device::GetRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetRate);
}
bool Device::SetRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetRate);
}
bool Device::DelRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelRate);
}
bool Device::AddRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::AddRate);
}
bool Device::GetPrices(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetPrices);
}
bool Device::GetBills(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetBills);
}
bool Device::TestAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::TestAlarm);
}
bool Device::GetAlarms(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetAlarms);
}
bool Device::GetPermissions(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetPermissions);
}
bool Device::AddPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::AddPermission);
}
bool Device::SetPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetPermission);
}
bool Device::DelPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelPermission);
}
