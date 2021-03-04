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
#include "MessageLoop.h"
#include "Tools.h"
#include "AnalyzerRemote.h"
#include "AnalyzerSoft.h"
#include "LogFile.h"
#include "PriceSource.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemote::AnalyzerRemote(const string &id, const AnalyzerMode mode, const string &user) : Analyzer(id, mode, user), pAccess(id, user), pRates(id), pSeries(id), pAlarms(id), pEvents(id) {
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(id, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
  pLastClientReq = 0;
  pSamplerate    = NAN;
  pConfig        = nullptr;
  pNow           = nullptr;
  pStatus        = nullptr;
  pLock          = PTHREAD_RWLOCK_INITIALIZER;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerRemote::~AnalyzerRemote() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerRemote::SetClientConnected() {
  pthread_rwlock_wrlock(&pLock);
  pLastClientReq = Tools::GetTime64();
  pthread_rwlock_unlock(&pLock);
}
bool AnalyzerRemote::isClientConnected() {
  uint64_t Now = Tools::GetTime64() - 60000;
  pthread_rwlock_rdlock(&pLock);
  bool Result = (pLastClientReq > Now);
  pthread_rwlock_unlock(&pLock);
  return Result;
}
void AnalyzerRemote::PushAction(const string name, const json &params) {
  json Action = json::object();
  Action["Type"] = name;
  Action["Params"] = params;
  if(Action["Params"].contains("UserName")) Action["Params"].erase("UserName");
  pthread_mutex_lock(&pMutex);
  pPendingActions.push(Action);
  pthread_mutex_unlock(&pMutex);
}
json AnalyzerRemote::GetActions(const uint16_t max) {
  json Result = json::array();
  pthread_mutex_lock(&pMutex);
  while((Result.size() < max) && (pPendingActions.empty() == false)) {
    Result.push_back(pPendingActions.front());
    pPendingActions.pop();
  }
  pthread_mutex_unlock(&pMutex);
  return Result;
}
uint16_t AnalyzerRemote::PendingActions() {
  pthread_mutex_lock(&pMutex);
  uint16_t Size = pPendingActions.size();
  pthread_mutex_unlock(&pMutex);
  return Size;
}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pthread_rwlock_rdlock(&pLock);
  json Response = pConfig;
  pthread_rwlock_unlock(&pLock);
  Response["Driver"]       = "Remote";
  Response["Online"]       = Online();
  Response["OnlineTime"]   = OnlineTime();
  Response["User"]         = pOwner;
  Response["Mode"]         = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"]   = pSamplerate;
  Response["PriceModules"] = PriceSource::VarsInfo();
  Response["Users"]        = pAccess.Users();
  return HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::CONFIG, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(params.contains("Users")) pAccess.Config(AnalyzerAccess::CheckConfig(params["Users"]));
  if(params.contains("ClearCache") && params["ClearCache"].is_boolean() && (params["ClearCache"] == true)) {
    Database DB(pSerial);
    DB.ExecSentenceNoResult("TRUNCATE aggreg_200ms, aggreg_3s, aggreg_1m, aggreg_10m, aggreg_15m, aggreg_1h, events, alarms CASCADE");
  }
  SetClientConnected();
  json Config;
  try {
    Config = AnalyzerSoft::CheckConfig(params, pMode, true);
  } catch(const string &err) {
    HTTPServer::FillResponse(response, err, "text/plain", HTTP_BAD_REQUEST);
  }
  pthread_rwlock_wrlock(&pLock);
  pConfig = Config;
  pthread_rwlock_unlock(&pLock);
  PushAction("SetAnalyzer", Config);
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerRemote::GetStatus(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  SetClientConnected();
  pthread_rwlock_rdlock(&pLock);
  if(pStatus == nullptr) {
    pthread_rwlock_unlock(&pLock);
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  }
  json Result = json::object();
  if(pStatus.contains("Time")) Result["Time"] = pStatus["Time"];
  for(auto &Serie : params["Series"]) {
    if((Serie.is_string() == false) || (Serie == "Time")) {
      pthread_rwlock_unlock(&pLock);
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    if(pStatus.contains(Serie)) Result[(string)Serie] = pStatus[(string)Serie];
  }
  pthread_rwlock_unlock(&pLock);
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.GetSeries(params, response);
}
void AnalyzerRemote::SetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.SetSeries(params, response);
  if(response.status == HTTP_OK) PushAction("SetSeries", params);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  SetClientConnected();
  pthread_rwlock_rdlock(&pLock);
  if(pNow == nullptr) {
    pthread_rwlock_unlock(&pLock);
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  }
  json Result = json::object();
  if(pNow.contains("Time")) Result["Time"] = pNow["Time"];
  for(auto &Serie : params["Series"]) {
    if((Serie.is_string() == false) || (Serie == "Time")) {
      pthread_rwlock_unlock(&pLock);
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    if(pNow.contains(Serie)) Result[(string)Serie] = pNow[(string)Serie];
  }
  pthread_rwlock_unlock(&pLock);
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::GetCosts(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pRates.GetCosts(params, response);
}
void AnalyzerRemote::GetBills(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pRates.GetBills(params, response);
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {
  return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.GetSeriesRange(params, response);
}
void AnalyzerRemote::DelSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.DelSeriesRange(params, response);
  if(response.status == HTTP_OK) PushAction("DelSeriesRange", params);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEvents(params, response);
}
void AnalyzerRemote::SetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.SetEvents(params, response);
  if(response.status == HTTP_OK) PushAction("SetEvents", params);
}
void AnalyzerRemote::GetEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEventsRange(params, response);
}
void AnalyzerRemote::DelEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.DelEventsRange(params, response);
  if(response.status == HTTP_OK) PushAction("DelEventsRange", params);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEvent(params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.SetEvent(params, response);
  if(response.status == HTTP_OK) PushAction("SetEvent", params);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.DelEventsRange(params, response);
  if(response.status == HTTP_OK) PushAction("DelEvent", params);
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.GetAlarms(params, response);
}
void AnalyzerRemote::SetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.SetAlarms(params, response);
  if(response.status == HTTP_OK) PushAction("SetAlarms", params);
}
void AnalyzerRemote::SetAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.SetAlarm(params, response);
  if(response.status == HTTP_OK) PushAction("SetAlarm", params);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.DelAlarm(params, response);
  if(response.status == HTTP_OK) PushAction("DelAlarm", params);
}
void AnalyzerRemote::GetAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.GetAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(PendingActions() > pMaxActions) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.DelAlarmsRange(params, response);
  if(response.status == HTTP_OK) PushAction("DelAlarmsRange", params);
}
void AnalyzerRemote::SetRemote(const json &params, Response &response) {
  if(params["UserName"] != pOwner) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((Configured() == false) && (params.contains("Config") == false)) return HTTPServer::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params.contains("Config") && params["Config"].is_object()) {
    if((params.contains("SampleRate") == false) || (params["SampleRate"].is_number() == false)) return HTTPServer::FillResponse(response, "'SampleRate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    if((params.contains("Mode") == false) || (params["Mode"].is_string() == false)) return HTTPServer::FillResponse(response, "'SampleRate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    string Mode = (pMode == Analyzer::PHASE1) ? "1PHASE" : (pMode == Analyzer::PHASE3) ? "3PHASE" : "3PHASE+N";
    if(Mode != params["Mode"]) return HTTPServer::FillResponse(response, "Analyzer mode do not match", "text/plain", HTTP_BAD_REQUEST);
    pthread_rwlock_wrlock(&pLock);
    pSamplerate = params["SampleRate"];
    pConfig = AnalyzerSoft::CheckConfig(params["Config"], pMode, false);
    pRates.Config(pConfig["TimeZone"], pConfig["Rates"]);
    pthread_rwlock_unlock(&pLock);
    Configured(true);
  }
  Online(true);
  if(params.contains("Now") && params["Now"].is_object()) {
    pthread_rwlock_wrlock(&pLock);
    pNow = params["Now"];
    pthread_rwlock_unlock(&pLock);
  }
  if(params.contains("Status") && params["Status"].is_object()) {
    pthread_rwlock_wrlock(&pLock);
    pNow = params["Now"];
    pthread_rwlock_unlock(&pLock);
  }
  if(params.contains("Actions") && params["Actions"].is_array()) {
    for(auto &Action : params["Actions"]) {
      Response Resp;
      if((Action.contains("Type") == false) || (Action["Type"].is_string() == false) || Action.contains("Params")) continue;
      string Type = Action["Type"];
      if(Type == "SetSeries") {
        pSeries.SetSeries(Action["Params"], Resp);
      } else if(Type == "DelSeriesRange") {
        pSeries.DelSeriesRange(Action["Params"], Resp);
      } else if(Type == "SetEvents") {
        pEvents.SetEvents(Action["Params"], Resp);
      } else if(Type == "DelEventsRange") {
        pEvents.DelEventsRange(Action["Params"], Resp);
      } else if(Type == "DelEvent") {
        pEvents.DelEvent(Action["Params"], Resp);
      } else if(Type == "SetAlarms") {
        pAlarms.SetAlarms(Action["Params"], Resp);
      } else if(Type == "SetAlarm") {
        pAlarms.SetAlarm(Action["Params"], Resp);
      } else if(Type == "DelAlarm") {
        pAlarms.DelAlarm(Action["Params"], Resp);
      } else if(Type == "DelAlarmsRange") {
        pAlarms.DelAlarmsRange(Action["Params"], Resp);
      } else {
        LogFile::Warning("AnayzerRemote receive invalid action from remote analyzer '%s' (%s)", pSerial.c_str(), Type.c_str());
        continue;
      }
      if(Resp.status != HTTP_OK) LogFile::Warning("AnayzerRemote action from remote analyzer '%s' (%s) fails", pSerial.c_str(), Type.c_str());
    }
  }
  Database DB(pSerial);
  json     Response       = json::object();
  Response["Now"]         = isClientConnected();
  Response["Status"]      = isClientConnected();
  Response["Time_200MS"]  = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_200ms") && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_3S"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_3s")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1M"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1m")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_10M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_10m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_15M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_15m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1H"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1h")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Alarms"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from alarms")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Events"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from events")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Actions"]     = GetActions(10);
  printf("-> %s\n", Response.dump().c_str());
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
bool AnalyzerRemote::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  return false;
}
//----------------------------------------------------------------------------------------------------------------------