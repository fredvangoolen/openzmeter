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
  HTTPServer::RegisterDoc("Analyzer", "API/Analyzer/Analyzer.json");
  HTTPServer::RegisterCallback("getAnalyzers",    GetAnalyzersCallback,   "API/Analyzer/GetAnalyzers.json");
  HTTPServer::RegisterCallback("getAnalyzer",     GetAnalyzerCallback,    "API/Analyzer/GetAnalyzer.json");
  HTTPServer::RegisterCallback("setAnalyzer",     SetAnalyzerCallback);
  HTTPServer::RegisterCallback("getStatus",       GetStatusCallback,      "API/Analyzer/GetStatus.json");
  HTTPServer::RegisterCallback("getSeries",       GetSeriesCallback,      "API/Analyzer/GetSeries.json");
  HTTPServer::RegisterCallback("getSeriesNow",    GetSeriesNowCallback,   "API/Analyzer/GetSeriesNow.json");
  HTTPServer::RegisterCallback("getCosts",        GetCostsCallback,       "API/Analyzer/GetCosts.json");
  HTTPServer::RegisterCallback("getBills",        GetBillsCallback,       "API/Analyzer/GetBills.json");
  HTTPServer::RegisterCallback("getWaveStream",   GetWaveStream);
  HTTPServer::RegisterCallback("getSeriesRange",  GetSeriesRangeCallback, "API/Analyzer/GetSeriesRange.json");
  HTTPServer::RegisterCallback("delSeriesRange",  DelSeriesRangeCallback, "API/Analyzer/DelSeriesRange.json");
  HTTPServer::RegisterCallback("getEvents",       GetEventsCallback,      "API/Analyzer/GetEvents.json");
  HTTPServer::RegisterCallback("getEventsRange",  GetEventsRangeCallback, "API/Analyzer/GetEventsRange.json");
  HTTPServer::RegisterCallback("delEventsRange",  DelEventsRangeCallback, "API/Analyzer/DelEventsRange.json");
  HTTPServer::RegisterCallback("getEvent",        GetEventCallback,       "API/Analyzer/GetEvent.json");
  HTTPServer::RegisterCallback("setEvent",        SetEventCallback,       "API/Analyzer/SetEvent.json");
  HTTPServer::RegisterCallback("delEvent",        DelEventCallback,       "API/Analyzer/DelEvent.json");
  HTTPServer::RegisterCallback("testAlarm",       TestAlarmCallback);
  HTTPServer::RegisterCallback("getAlarms",       GetAlarmsCallback,      "API/Analyzer/GetAlarms.json");
  HTTPServer::RegisterCallback("getAlarmsRange",  GetAlarmsRangeCallback, "API/Analyzer/GetAlarmsRange.json");
  HTTPServer::RegisterCallback("setAlarm",        SetAlarmCallback,       "API/Analyzer/SetAlarm.json");
  HTTPServer::RegisterCallback("delAlarm",        DelAlarmCallback,       "API/Analyzer/DelAlarm.json");
  HTTPServer::RegisterCallback("delAlarmsRange",  DelAlarmsRangeCallback, "API/Analyzer/DelAlarmsRange.json");
  HTTPServer::RegisterCallback("setRemote",       SetRemoteCallback);
}
AnalyzerManager::~AnalyzerManager() {
  LogFile::Debug("Stopping AnalyzerManager");
  HTTPServer::UnregisterCallback("getAnalyzers");
  HTTPServer::UnregisterCallback("getAnalyzer");
  HTTPServer::UnregisterCallback("setAnalyzer");
  HTTPServer::UnregisterCallback("getStatus");
  HTTPServer::UnregisterCallback("getSeries");
  HTTPServer::UnregisterCallback("getSeriesNow");
  HTTPServer::UnregisterCallback("getCosts");
  HTTPServer::UnregisterCallback("getBills");
  HTTPServer::UnregisterCallback("getWaveStream");
  HTTPServer::UnregisterCallback("getSeriesRange");
  HTTPServer::UnregisterCallback("delSeriesRange");
  HTTPServer::UnregisterCallback("getEvents");
  HTTPServer::UnregisterCallback("getEventsRange");
  HTTPServer::UnregisterCallback("delEventsRange");
  HTTPServer::UnregisterCallback("getEvent");
  HTTPServer::UnregisterCallback("setEvent");
  HTTPServer::UnregisterCallback("delEvent");
  HTTPServer::UnregisterCallback("testAlarm");
  HTTPServer::UnregisterCallback("getAlarms");
  HTTPServer::UnregisterCallback("getAlarmsRange");
  HTTPServer::UnregisterCallback("delAlarm");
  HTTPServer::UnregisterCallback("setAlarm");
  HTTPServer::UnregisterCallback("delAlarmsRange");
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterDoc("Analyzer");
  LogFile::Debug("AnalyzerManager stopped");
}
bool AnalyzerManager::AddAnalyzer(Analyzer *analyzer) {
  pAnalyzersLock.WriteLock();
  auto Result = pAnalyzers.insert({analyzer->Serial(), analyzer});
  pAnalyzersLock.Unlock();
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
void AnalyzerManager::CallAnalyzerFunction(HTTPRequest &req, void (Analyzer::*method)(HTTPRequest &req)) {
  if(!req.CheckParamString("Analyzer")) return;
  string Serial = req.Param("Analyzer");
  pAnalyzersLock.ReadLock();
  auto Analyzer = pAnalyzers.find(Serial);
  if(Analyzer != pAnalyzers.end()) {
    (Analyzer->second->*method)(req);
  } else {
    string UserName = req.UserName();
    LogFile::Warning("User '%s' request access to not defined analyzer '%s'", UserName.c_str(), Serial.c_str());
    req.Error("Analyzer not found", HTTP_BAD_REQUEST);
  }
  pAnalyzersLock.Unlock();
}
void AnalyzerManager::GetAnalyzersCallback(HTTPRequest &req) {
  string UserName = req.UserName();
  auto Func = [UserName](function<bool(const json&)> Write) {
    Write({"Analyzer", "User", "Name", "State", "Read", "Write", "Config"});
    auto StateToString = [](const Analyzer::AnalyzerState state) {
      return (state == Analyzer::UNCONFIGURED) ? "UNCONFIGURED" : (state == Analyzer::OFFLINE) ? "OFFLINE" : (state == Analyzer::SYNCING) ? "SYNCING" : "ONLINE";
    };
    if(UserName != "admin") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT serial, read, write, config FROM permissions WHERE username = " + DB.Bind(UserName) + " AND (read = true OR write = true OR config = true)");
      while(DB.FecthRow()) {
        pAnalyzersLock.ReadLock();
        auto Tmp = pAnalyzers.find(DB.ResultString("serial"));
        if((Tmp == pAnalyzers.end()) || (Tmp->second->State() == Analyzer::UNCONFIGURED)) continue;
        json Analyzer = { Tmp->second->Serial(), Tmp->second->Owner(), Tmp->second->Name(), StateToString(Tmp->second->State()), DB.ResultBool("read"), DB.ResultBool("write"), DB.ResultBool("config") };
        pAnalyzersLock.Unlock();
        if(Write(Analyzer) == false) break;
      }
    } else {
      pAnalyzersLock.ReadLock();
      for(auto &Tmp : pAnalyzers) {
        if(Tmp.second->State() == Analyzer::UNCONFIGURED) continue;
        json Analyzer = { Tmp.second->Serial(), Tmp.second->Owner(), Tmp.second->Name(), StateToString(Tmp.second->State()), true, true, true };
        if(Write(Analyzer) == false) break;
      }
      pAnalyzersLock.Unlock();
    }
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerManager::GetAnalyzerCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetAnalyzer);
}
void AnalyzerManager::SetAnalyzerCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::SetAnalyzer);
}
void AnalyzerManager::GetStatusCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetStatus);
}
void AnalyzerManager::GetSeriesCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetSeries);
}
void AnalyzerManager::GetSeriesNowCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetSeriesNow);
}
void AnalyzerManager::GetCostsCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetCosts);
}
void AnalyzerManager::GetBillsCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetBills);
}
void AnalyzerManager::GetWaveStream(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetWaveStream);
}
void AnalyzerManager::GetSeriesRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetSeriesRange);
}
void AnalyzerManager::DelSeriesRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::DelSeriesRange);
}
void AnalyzerManager::GetEventsCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetEvents);
}
void AnalyzerManager::GetEventsRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetEventsRange);
}
void AnalyzerManager::DelEventsRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::DelEventsRange);
}
void AnalyzerManager::GetEventCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetEvent);
}
void AnalyzerManager::SetEventCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::SetEvent);
}
void AnalyzerManager::DelEventCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::DelEvent);
}
void AnalyzerManager::TestAlarmCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::TestAlarm);
}
void AnalyzerManager::GetAlarmsCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetAlarms);
}
void AnalyzerManager::GetAlarmsRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::GetAlarmsRange);
}
void AnalyzerManager::DelAlarmCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::DelAlarm);
}
void AnalyzerManager::SetAlarmCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::SetAlarm);
}
void AnalyzerManager::DelAlarmsRangeCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::DelAlarmsRange);
}
void AnalyzerManager::SetRemoteCallback(HTTPRequest &req) {
  CallAnalyzerFunction(req, &Analyzer::SetRemote);
}
//----------------------------------------------------------------------------------------------------------------------