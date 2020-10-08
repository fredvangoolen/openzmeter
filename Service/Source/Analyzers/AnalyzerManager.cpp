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
#include "AnalyzerManager.h"
#include "HTTPServer.h"
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
map<string, Analyzer*>  AnalyzerManager::pAnalyzers;
RWLocks                 AnalyzerManager::pAnalyzersLock;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerManager::AnalyzerManager() {
  HTTPServer::RegisterJSONCallback("getAnalyzers",    GetAnalyzersCallback);
  HTTPServer::RegisterJSONCallback("getAnalyzer",     GetAnalyzerCallback);
  HTTPServer::RegisterJSONCallback("setAnalyzer",     SetAnalyzerCallback);
  HTTPServer::RegisterJSONCallback("getStatus",       GetStatusCallback);
  HTTPServer::RegisterJSONCallback("getSeries",       GetSeriesCallback);
  HTTPServer::RegisterJSONCallback("getSeriesNow",    GetSeriesNowCallback);
  HTTPServer::RegisterJSONCallback("getCosts",        GetCostsCallback);
  HTTPServer::RegisterJSONCallback("getBills",        GetBillsCallback);
  HTTPServer::RegisterJSONCallback("getWaveStream",   GetWaveStream);
  HTTPServer::RegisterJSONCallback("getSeriesRange",  GetSeriesRangeCallback);
  HTTPServer::RegisterJSONCallback("delSeriesRange",  DelSeriesRangeCallback);
  HTTPServer::RegisterJSONCallback("getEvents",       GetEventsCallback);
  HTTPServer::RegisterJSONCallback("getEventsRange",  GetEventsRangeCallback);
  HTTPServer::RegisterJSONCallback("delEventsRange",  DelEventsRangeCallback);
  HTTPServer::RegisterJSONCallback("getEvent",        GetEventCallback);
  HTTPServer::RegisterJSONCallback("setEvent",        SetEventCallback);
  HTTPServer::RegisterJSONCallback("delEvent",        DelEventCallback);
  HTTPServer::RegisterJSONCallback("getAlarmsDef",    GetAlarmsDefCallback);
  HTTPServer::RegisterJSONCallback("testAlarm",       TestAlarmCallback);
  HTTPServer::RegisterJSONCallback("addAlarmDef",     AddAlarmDefCallback);
  HTTPServer::RegisterJSONCallback("getAlarmDef",     GetAlarmDefCallback);
  HTTPServer::RegisterJSONCallback("setAlarmDef",     SetAlarmDefCallback);
  HTTPServer::RegisterJSONCallback("delAlarmDef",     DelAlarmDefCallback);
  HTTPServer::RegisterJSONCallback("getAlarms",       GetAlarmsCallback);
  HTTPServer::RegisterJSONCallback("getAlarmsRange",  GetAlarmsRangeCallback);
  HTTPServer::RegisterJSONCallback("delAlarm",        DelAlarmCallback);
  HTTPServer::RegisterJSONCallback("delAlarmsRange",  DelAlarmsRangeCallback);
  HTTPServer::RegisterJSONCallback("getRates",        GetRatesCallback);
  HTTPServer::RegisterJSONCallback("getRate",         GetRateCallback);
  HTTPServer::RegisterJSONCallback("addRate",         AddRateCallback);
  HTTPServer::RegisterJSONCallback("setRate",         SetRateCallback);
  HTTPServer::RegisterJSONCallback("delRate",         DelRateCallback);
  HTTPServer::RegisterJSONCallback("getPermissions",  GetPermissionsCallback);
  HTTPServer::RegisterJSONCallback("addPermission",   AddPermissionCallback);
  HTTPServer::RegisterJSONCallback("setPermission",   SetPermissionCallback);
  HTTPServer::RegisterJSONCallback("delPermission",   DelPermissionCallback);
}
AnalyzerManager::~AnalyzerManager() {
  LogFile::Debug("Stopping AnalyzerManager");
  HTTPServer::UnregisterJSONCallback("getAnalyzers");
  HTTPServer::UnregisterJSONCallback("getAnalyzer");
  HTTPServer::UnregisterJSONCallback("setAnalyzer");
  HTTPServer::UnregisterJSONCallback("getStatus");
  HTTPServer::UnregisterJSONCallback("getSeries");
  HTTPServer::UnregisterJSONCallback("getSeriesNow");
  HTTPServer::UnregisterJSONCallback("getCosts");
  HTTPServer::UnregisterJSONCallback("getBills");
  HTTPServer::UnregisterJSONCallback("getWaveStream");
  HTTPServer::UnregisterJSONCallback("getSeriesRange");
  HTTPServer::UnregisterJSONCallback("delSeriesRange");
  HTTPServer::UnregisterJSONCallback("getEvents");
  HTTPServer::UnregisterJSONCallback("getEventsRange");
  HTTPServer::UnregisterJSONCallback("delEventsRange");
  HTTPServer::UnregisterJSONCallback("getEvent");
  HTTPServer::UnregisterJSONCallback("setEvent");
  HTTPServer::UnregisterJSONCallback("delEvent");
  HTTPServer::UnregisterJSONCallback("getAlarmsDef");
  HTTPServer::UnregisterJSONCallback("testAlarm");
  HTTPServer::UnregisterJSONCallback("addAlarmDef");
  HTTPServer::UnregisterJSONCallback("getAlarmDef");
  HTTPServer::UnregisterJSONCallback("setAlarmDef");
  HTTPServer::UnregisterJSONCallback("delAlarmDef");
  HTTPServer::UnregisterJSONCallback("getAlarms");
  HTTPServer::UnregisterJSONCallback("getAlarmsRange");
  HTTPServer::UnregisterJSONCallback("delAlarm");
  HTTPServer::UnregisterJSONCallback("delAlarmsRange");
  HTTPServer::UnregisterJSONCallback("getRates");
  HTTPServer::UnregisterJSONCallback("getRate");
  HTTPServer::UnregisterJSONCallback("addRate");
  HTTPServer::UnregisterJSONCallback("setRate");
  HTTPServer::UnregisterJSONCallback("delRate");
  HTTPServer::UnregisterJSONCallback("getPermissions");
  HTTPServer::UnregisterJSONCallback("addPermission");
  HTTPServer::UnregisterJSONCallback("setPermission");
  HTTPServer::UnregisterJSONCallback("delPermission");
  LogFile::Debug("AnalyzerManager stopped");
}
bool AnalyzerManager::AddAnalyzer(Analyzer *analyzer, const string &user) {
  pAnalyzersLock.WriteLock();
  auto Result = pAnalyzers.insert({analyzer->Serial(), analyzer});
  pAnalyzersLock.Unlock();
  if(Result.second == true) {
    Database DB;
    if(user != "admin") DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(user) + ", " + DB.Bind(analyzer->Serial()) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
    DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES('admin', " + DB.Bind(analyzer->Serial()) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
  }
  return Result.second;
}
bool AnalyzerManager::DelAnalyzer(const string &serial) {
  pAnalyzersLock.WriteLock();
  uint32_t Erased = pAnalyzers.erase(serial);
  pAnalyzersLock.Unlock();
  return (Erased == 1);
}
bool AnalyzerManager::HandleModbus(uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len) {
  pAnalyzersLock.ReadLock();
  auto Analyzer = pAnalyzers.find(serial);
  bool Ret = (Analyzer != pAnalyzers.end()) ? Analyzer->second->HandleModbus(buff, start, len) : false;
  pAnalyzersLock.Unlock();
  return Ret;
}
void AnalyzerManager::CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response)) {
  if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Analyzer"];
  pAnalyzersLock.ReadLock();
  auto Analyzer = pAnalyzers.find(Serial);
  if(Analyzer != pAnalyzers.end()) {
    (Analyzer->second->*method)(params, response);
  } else {
    HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
  }
  pAnalyzersLock.Unlock();
}
void AnalyzerManager::GetAnalyzersCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  auto Func = [UserName](uint64_t offset, DataSink &sink) -> bool {
    uint32_t Items = 0;
    sink.os << "[\n  ";
    if(UserName != "admin") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT serial FROM permissions WHERE username = " + DB.Bind(UserName) + " AND (read = true OR write = true OR config = true)");
      while(DB.FecthRow() && sink.is_writable()) {
        pAnalyzersLock.ReadLock();
        auto Tmp = pAnalyzers.find(DB.ResultString("serial"));
        if(Tmp != pAnalyzers.end()) {
          if(Items++ != 0) sink.os << ",\n  ";
          sink.os << Tmp->second->GetJSONDescriptor().dump();
        }
        pAnalyzersLock.Unlock();
      }
    } else {
      pAnalyzersLock.ReadLock();
      for(auto Tmp : pAnalyzers) {
        if(sink.is_writable() == false) break;
        if(Items++ != 0) sink.os << ",\n  ";
        sink.os << Tmp.second->GetJSONDescriptor().dump();
      }
      pAnalyzersLock.Unlock();
    }
    sink.os << "\n]";
    sink.done();
    return false;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerManager::GetAnalyzerCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetAnalyzer);
}
void AnalyzerManager::SetAnalyzerCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::SetAnalyzer);
}
void AnalyzerManager::GetStatusCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetStatus);
}
void AnalyzerManager::GetSeriesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetSeries);
}
void AnalyzerManager::GetSeriesNowCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetSeriesNow);
}
void AnalyzerManager::GetCostsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetCosts);
}
void AnalyzerManager::GetBillsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetBills);
}
void AnalyzerManager::GetWaveStream(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetWaveStream);
}
void AnalyzerManager::GetSeriesRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetSeriesRange);
}
void AnalyzerManager::DelSeriesRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelSeriesRange);
}
void AnalyzerManager::GetEventsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetEvents);
}
void AnalyzerManager::GetEventsRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetEventsRange);
}
void AnalyzerManager::DelEventsRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelEventsRange);
}
void AnalyzerManager::GetEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetEvent);
}
void AnalyzerManager::SetEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::SetEvent);
}
void AnalyzerManager::DelEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelEvent);
}
void AnalyzerManager::GetAlarmsDefCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetAlarmsDef);
}
void AnalyzerManager::TestAlarmCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::TestAlarm);
}
void AnalyzerManager::AddAlarmDefCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::AddAlarmDef);
}
void AnalyzerManager::GetAlarmDefCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetAlarmDef);
}
void AnalyzerManager::SetAlarmDefCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::SetAlarmDef);
}
void AnalyzerManager::DelAlarmDefCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelAlarmDef);
}
void AnalyzerManager::GetAlarmsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetAlarms);
}
void AnalyzerManager::GetAlarmsRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetAlarmsRange);
}
void AnalyzerManager::DelAlarmCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelAlarm);
}
void AnalyzerManager::DelAlarmsRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelAlarmsRange);
}
void AnalyzerManager::GetRatesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetRates);
}
void AnalyzerManager::GetRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetRate);
}
void AnalyzerManager::AddRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::AddRate);
}
void AnalyzerManager::SetRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::SetRate);
}
void AnalyzerManager::DelRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelRate);
}
void AnalyzerManager::GetPermissionsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::GetPermissions);
}
void AnalyzerManager::AddPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::AddPermission);
}
void AnalyzerManager::SetPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::SetPermission);
}
void AnalyzerManager::DelPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::DelPermission);
}
//----------------------------------------------------------------------------------------------------------------------