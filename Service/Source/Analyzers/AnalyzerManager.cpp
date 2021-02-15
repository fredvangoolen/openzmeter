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
pthread_rwlock_t        AnalyzerManager::pAnalyzersLock = PTHREAD_RWLOCK_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerManager::AnalyzerManager() {
  HTTPServer::RegisterCallback("getAnalyzers",    GetAnalyzersCallback);
  HTTPServer::RegisterCallback("getAnalyzer",     GetAnalyzerCallback);
  HTTPServer::RegisterCallback("setAnalyzer",     SetAnalyzerCallback);
  HTTPServer::RegisterCallback("getStatus",       GetStatusCallback);
  HTTPServer::RegisterCallback("getSeries",       GetSeriesCallback);
  HTTPServer::RegisterCallback("getSeriesNow",    GetSeriesNowCallback);
  HTTPServer::RegisterCallback("getCosts",        GetCostsCallback);
  HTTPServer::RegisterCallback("getBills",        GetBillsCallback);
  HTTPServer::RegisterCallback("getWaveStream",   GetWaveStream);
  HTTPServer::RegisterCallback("getSeriesRange",  GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("delSeriesRange",  DelSeriesRangeCallback);
  HTTPServer::RegisterCallback("getEvents",       GetEventsCallback);
  HTTPServer::RegisterCallback("getEventsRange",  GetEventsRangeCallback);
  HTTPServer::RegisterCallback("delEventsRange",  DelEventsRangeCallback);
  HTTPServer::RegisterCallback("getEvent",        GetEventCallback);
  HTTPServer::RegisterCallback("setEvent",        SetEventCallback);
  HTTPServer::RegisterCallback("delEvent",        DelEventCallback);
  HTTPServer::RegisterCallback("testAlarm",       TestAlarmCallback);
  HTTPServer::RegisterCallback("getAlarms",       GetAlarmsCallback);
  HTTPServer::RegisterCallback("getAlarmsRange",  GetAlarmsRangeCallback);
  HTTPServer::RegisterCallback("delAlarm",        DelAlarmCallback);
  HTTPServer::RegisterCallback("delAlarmsRange",  DelAlarmsRangeCallback);
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
  HTTPServer::UnregisterCallback("delAlarmsRange");
  LogFile::Debug("AnalyzerManager stopped");
}
bool AnalyzerManager::AddAnalyzer(Analyzer *analyzer, const string &user) {
  pthread_rwlock_wrlock(&pAnalyzersLock);
  auto Result = pAnalyzers.insert({analyzer->Serial(), analyzer});
  pthread_rwlock_unlock(&pAnalyzersLock);
  return Result.second;
}
bool AnalyzerManager::DelAnalyzer(const string &serial) {
  pthread_rwlock_wrlock(&pAnalyzersLock);
  uint32_t Erased = pAnalyzers.erase(serial);
  pthread_rwlock_unlock(&pAnalyzersLock);
  return (Erased == 1);
}
bool AnalyzerManager::HandleModbus(uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len) {
  pthread_rwlock_rdlock(&pAnalyzersLock);
  auto Analyzer = pAnalyzers.find(serial);
  bool Ret = (Analyzer != pAnalyzers.end()) ? Analyzer->second->HandleModbus(buff, start, len) : false;
  pthread_rwlock_unlock(&pAnalyzersLock);
  return Ret;
}
void AnalyzerManager::CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response)) {
  if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Analyzer"];
  pthread_rwlock_rdlock(&pAnalyzersLock);
  auto Analyzer = pAnalyzers.find(Serial);
  if(Analyzer != pAnalyzers.end()) {
    (Analyzer->second->*method)(params, response);
  } else {
    HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
  }
  pthread_rwlock_unlock(&pAnalyzersLock);
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
        pthread_rwlock_rdlock(&pAnalyzersLock);
        auto Tmp = pAnalyzers.find(DB.ResultString("serial"));
        if(Tmp != pAnalyzers.end()) sink.os << ((Items++ != 0) ? ",\n  " : "") + Tmp->second->GetJSONDescriptor().dump();
        pthread_rwlock_unlock(&pAnalyzersLock);
      }
    } else {
      pthread_rwlock_rdlock(&pAnalyzersLock);
      for(auto Tmp : pAnalyzers) {
        if(sink.is_writable() == false) break;
        sink.os << ((Items++ != 0) ? ",\n  " : "") + Tmp.second->GetJSONDescriptor().dump();
      }
      pthread_rwlock_unlock(&pAnalyzersLock);
    }
    sink.os << "\n]";
    sink.done();
    return true;
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
void AnalyzerManager::TestAlarmCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Analyzer::TestAlarm);
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
//----------------------------------------------------------------------------------------------------------------------