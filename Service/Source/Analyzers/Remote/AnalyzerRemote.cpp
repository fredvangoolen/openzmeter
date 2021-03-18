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
AnalyzerRemote::AnalyzerRemote(const string &id, const AnalyzerMode mode, const string &user) : Analyzer(id, mode, user),
  pAccess(id, user), pRates(id), pSeries(id), pAlarms(id), pEvents(id), pNow(id), pActions([&](const string &n, const json &p) { return HandleAction(n, p); }) {
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(id, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
  pSamplerate    = NAN;
  pConfig        = nullptr;
  pLock          = PTHREAD_RWLOCK_INITIALIZER;
}
AnalyzerRemote::~AnalyzerRemote() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerRemote::HandleAction(const string &name, const json &params) {
  Response Resp;
  if(name == "SetSeries") {
    pSeries.SetSeries(params, Resp);
  } else if(name == "DelSeriesRange") {
    pSeries.DelSeriesRange(params, Resp);
  } else if(name == "SetEvents") {
    pEvents.SetEvents(params, Resp);
  } else if(name == "DelEventsRange") {
    pEvents.DelEventsRange(params, Resp);
  } else if(name == "DelEvent") {
    pEvents.DelEvent(params, Resp);
  } else if(name == "SetEvent") {
    pEvents.SetEvent(params, Resp);
  } else if(name == "SetAlarms") {
    pAlarms.SetAlarms(params, Resp);
  } else if(name == "SetAlarm") {
    pAlarms.SetAlarm(params, Resp);
  } else if(name == "DelAlarm") {
    pAlarms.DelAlarm(params, Resp);
  } else if(name == "DelAlarmsRange") {
    pAlarms.DelAlarmsRange(params, Resp);
  } else {
    LogFile::Warning("AnayzerRemote receive invalid action from AnalyzerSoft '%s' (%s)", pSerial.c_str(), name.c_str());
    return;
  }
  if(Resp.status != HTTP_OK) LogFile::Warning("AnayzerRemote action from remote AnalyzerSoft '%s' (%s) fails", pSerial.c_str(), name.c_str());
}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pthread_rwlock_rdlock(&pLock);
  json Response = pConfig;
  pthread_rwlock_unlock(&pLock);
  Response["User"]           = pOwner;
  Response["Mode"]           = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"]     = pSamplerate;
  Response["PriceModules"]   = PriceSource::VarsInfo();
  Response["Users"]          = pAccess.Users();
  return HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::CONFIG, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(params.contains("Users")) pAccess.Config(AnalyzerAccess::CheckConfig(params["Users"]));
  if(params.contains("ClearCache") && params["ClearCache"].is_boolean() && (params["ClearCache"] == true)) {
    Database DB(pSerial);
    DB.ExecSentenceNoResult("TRUNCATE aggreg_200ms, aggreg_3s, aggreg_1m, aggreg_10m, aggreg_15m, aggreg_1h, events, alarms CASCADE");
  }
  json Config;
  try {
    Config = AnalyzerSoft::CheckConfig(params, pMode, true);
  } catch(const string &err) {
    HTTPServer::FillResponse(response, err, "text/plain", HTTP_BAD_REQUEST);
  }
  pthread_rwlock_wrlock(&pLock);
  pConfig = Config;
  pthread_rwlock_unlock(&pLock);
  pActions.Push("SetAnalyzer", Config);
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerRemote::GetStatus(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  json Extra = json::object();
  Extra["PendingActions"] = pActions.Count();
  Extra["State"] = (State() == UNCONFIGURED) ? "UNCONFIGURED" : (State() == OFFLINE) ? "OFFLINE" : (State() == SYNCING) ? "SYNCING" : "ONLINE";
  pNow.GetStatus(params, response, Extra);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.GetSeries(params, response);
}
void AnalyzerRemote::SetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.SetSeries(params, response);
  if(response.status == HTTP_OK) pActions.Push("SetSeries", params);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pNow.GetSeriesNow(params, response);
}
void AnalyzerRemote::GetCosts(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pRates.GetCosts(params, response);
}
void AnalyzerRemote::GetBills(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pRates.GetBills(params, response);
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {
  return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.GetSeriesRange(params, response);
}
void AnalyzerRemote::DelSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.DelSeriesRange(params, response);
  if(response.status == HTTP_OK) pActions.Push("DelSeriesRange", params);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.GetEvents(params, response);
}
void AnalyzerRemote::SetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.SetEvents(params, response);
  if(response.status == HTTP_OK) pActions.Push("SetEvents", params);
}
void AnalyzerRemote::GetEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.GetEventsRange(params, response);
}
void AnalyzerRemote::DelEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.DelEventsRange(params, response);
  if(response.status == HTTP_OK) pActions.Push("DelEventsRange", params);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.GetEvent(params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.SetEvent(params, response);
  if(response.status == HTTP_OK) pActions.Push("SetEvent", params);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.DelEventsRange(params, response);
  if(response.status == HTTP_OK) pActions.Push("DelEvent", params);
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
//HACER
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.GetAlarms(params, response);
}
void AnalyzerRemote::SetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.SetAlarms(params, response);
  if(response.status == HTTP_OK) pActions.Push("SetAlarms", params);
}
void AnalyzerRemote::SetAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.SetAlarm(params, response);
  if(response.status == HTTP_OK) pActions.Push("SetAlarm", params);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.DelAlarm(params, response);
  if(response.status == HTTP_OK) pActions.Push("DelAlarm", params);
}
void AnalyzerRemote::GetAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.GetAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  if(pActions.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.DelAlarmsRange(params, response);
  if(response.status == HTTP_OK) pActions.Push("DelAlarmsRange", params);
}
void AnalyzerRemote::SetRemote(const json &params, Response &response) {
  if(params["UserName"] != pOwner) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((State() == UNCONFIGURED) && (params.contains("Config") == false)) return HTTPServer::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
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
    State(SYNCING);
  }
  Database DB(pSerial);
  json     Response = json::object();
  pNow.SetRemote(params, Response);
  pActions.SetRemote(params, Response);
  Response["Time_200MS"]  = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_200ms") && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_3S"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_3s")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1M"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1m")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_10M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_10m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_15M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_15m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1H"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1h")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Alarms"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from alarms")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Events"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from events")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  State((params.contains("Synced") && params["Synced"].is_boolean() && (params["Synced"] == true)) ? ONLINE : SYNCING);
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
bool AnalyzerRemote::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  return false;
}
//----------------------------------------------------------------------------------------------------------------------