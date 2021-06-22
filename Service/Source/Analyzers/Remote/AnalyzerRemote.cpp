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
AnalyzerRemote::AnalyzerRemote(const string &id, const AnalyzerMode mode, const string &user) : Analyzer(id, mode, user) {
  LogFile::Info("Starting AnalyzerRemote with serial '%s'...", pSerial.c_str());
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(id, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
  pSamplerate    = NAN;
  pConfig        = nullptr;
  pLock          = PTHREAD_RWLOCK_INITIALIZER;
  pAccess        = new AnalyzerAccess(id, user);
  pRates         = new AnalyzerRates(id);
  pSeries        = new AnalyzerSeries(id, mode);
  pAlarms        = new AnalyzerAlarms(id);
  pEvents        = new AnalyzerEvents(id, mode);
  pNow           = new AnalyzerRemoteNow(id);
  pActions       = new AnalyzerRemoteActions([&](const string &n, const json &p) { return HandleAction(n, p); });
  LogFile::Info("AnalyzerRemote with serial '%s' started", pSerial.c_str());
}
AnalyzerRemote::~AnalyzerRemote() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  delete pActions;
  delete pNow;
  delete pEvents;
  delete pAlarms;
  delete pSeries;
  delete pRates;
  delete pAccess;
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerRemote::HandleAction(const string &name, const json &params) {
  HTTPRequest Req(params);
  if(name == "SetSeries")           pSeries->SetSeries(Req);
  else if(name == "DelSeriesRange") pSeries->DelSeriesRange(Req);
  else if(name == "SetEvents")      pEvents->SetEvents(Req);
  else if(name == "DelEventsRange") pEvents->DelEventsRange(Req);
  else if(name == "DelEvent")       pEvents->DelEvent(Req);
  else if(name == "SetEvent")       pEvents->SetEvent(Req);
  else if(name == "SetAlarms")      pAlarms->SetAlarms(Req);
  else if(name == "SetAlarm")       pAlarms->SetAlarm(Req);
  else if(name == "DelAlarm")       pAlarms->DelAlarm(Req);
  else if(name == "DelAlarmsRange") pAlarms->DelAlarmsRange(Req);
  else {
    LogFile::Warning("AnayzerRemote receive invalid action from AnalyzerSoft '%s' (%s)", pSerial.c_str(), name.c_str());
    return;
  }
  if(Req.Status() != HTTP_OK) {
    if(Req.ErrorBody().empty() == false)
      LogFile::Warning("AnayzerRemote action from remote AnalyzerSoft '%s' (%s -> (%u) %s) fails", pSerial.c_str(), name.c_str(), Req.Status(), Req.ErrorBody().c_str());
    else
      LogFile::Warning("AnayzerRemote action from remote AnalyzerSoft '%s' (%s -> %u) fails", pSerial.c_str(), name.c_str(), Req.Status());
  }
}
void AnalyzerRemote::GetAnalyzer(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pthread_rwlock_rdlock(&pLock);
  json Response = pConfig;
  pthread_rwlock_unlock(&pLock);
  Response["User"]           = pOwner;
  Response["Mode"]           = Analyzer::ModeToString(pMode);
  Response["SampleRate"]     = pSamplerate;
  Response["PriceModules"]   = PriceSource::VarsInfo();
  Response["Users"]          = pAccess->Users();
  req.Success(Response);
}
void AnalyzerRemote::SetAnalyzer(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::CONFIG, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  if(req.CheckParamArray("Users")) pAccess->Config(AnalyzerAccess::CheckConfig(req.Param("Users")));
  if(req.CheckParamBool("ClearCache") && (req.Param("ClearCache") == true)) {
    Database DB(pSerial);
    DB.ExecSentenceNoResult("TRUNCATE aggreg_200ms, aggreg_3s, aggreg_1m, aggreg_10m, aggreg_15m, aggreg_1h, events, alarms CASCADE");
  }
  try {
    pthread_rwlock_wrlock(&pLock);
    pConfig = AnalyzerSoft::CheckConfig(req.Params(), pMode, true);
    Name(pConfig["Name"]);
    pRates->Config(pConfig["TimeZone"], pConfig["Rates"]);
    pActions->Push("SetAnalyzer", pConfig);
    pthread_rwlock_unlock(&pLock);
    req.Success(json::object());
  } catch(const string &err) {
    req.Error(err, HTTP_BAD_REQUEST);
  }
}
void AnalyzerRemote::GetStatus(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(!req.CheckParamSet("Series", {"Version", "Log", "System", "SystemLog", "Events", "Alarms", "Storage"})) return;
  json Extra = json::object();
  Extra["PendingActions"] = pActions->Count();
  Extra["State"] = (State() == UNCONFIGURED) ? "UNCONFIGURED" : (State() == OFFLINE) ? "OFFLINE" : (State() == SYNCING) ? "SYNCING" : "ONLINE";
  pNow->GetStatus(req, Extra);
}
void AnalyzerRemote::GetSeries(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pSeries->GetSeries(req);
}
void AnalyzerRemote::SetSeries(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pSeries->SetSeries(req);
  if(req.Status() == HTTP_OK) pActions->Push("SetSeries", req.Params());
}
void AnalyzerRemote::GetSeriesNow(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(!req.CheckParamSet("Series", {"Frequency", "Flag", "Unbalance", "Voltage", "Current", "Active_Power", "Reactive_Power", "Active_Energy", "Reactive_Energy", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Phase", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Voltage_FFT", "Current_FFT", "Voltage_Samples", "Current_Samples"})) return;
  pNow->GetSeriesNow(req);
}
void AnalyzerRemote::GetCosts(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pRates->GetCosts(req);
}
void AnalyzerRemote::GetBills(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pRates->GetBills(req);
}
void AnalyzerRemote::GetWaveStream(HTTPRequest &req) {
  return req.Error("", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerRemote::GetSeriesRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pSeries->GetSeriesRange(req);
}
void AnalyzerRemote::DelSeriesRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pSeries->DelSeriesRange(req);
  if(req.Status() == HTTP_OK) pActions->Push("DelSeriesRange", req.Params());
}
void AnalyzerRemote::GetEvents(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pEvents->GetEvents(req);
}
void AnalyzerRemote::SetEvents(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->SetEvents(req);
  if(req.Status() == HTTP_OK) pActions->Push("SetEvents", req.Params());
}
void AnalyzerRemote::GetEventsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pEvents->GetEventsRange(req);
}
void AnalyzerRemote::DelEventsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->DelEventsRange(req);
  if(req.Status() == HTTP_OK) pActions->Push("DelEventsRange", req.Params());
}
void AnalyzerRemote::GetEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pEvents->GetEvent(req);
}
void AnalyzerRemote::SetEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->SetEvent(req);
  if(req.Status() == HTTP_OK) pActions->Push("SetEvent", req.Params());
}
void AnalyzerRemote::DelEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->DelEvent(req);
  if(req.Status() == HTTP_OK) pActions->Push("DelEvent", req.Params());
}
void AnalyzerRemote::TestAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
//HACER
}
void AnalyzerRemote::GetAlarms(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->GetAlarms(req);
}
void AnalyzerRemote::SetAlarms(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->SetAlarms(req);
  if(req.Status() == HTTP_OK) pActions->Push("SetAlarms", req.Params());
}
void AnalyzerRemote::SetAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->SetAlarm(req);
  if(req.Status() == HTTP_OK) pActions->Push("SetAlarm", req.Params());
}
void AnalyzerRemote::DelAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->DelAlarm(req);
  if(req.Status() == HTTP_OK) pActions->Push("DelAlarm", req.Params());
}
void AnalyzerRemote::GetAlarmsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->GetAlarmsRange(req);
}
void AnalyzerRemote::DelAlarmsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(State() == UNCONFIGURED) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  if(pActions->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->DelAlarmsRange(req);
  if(req.Status() == HTTP_OK) pActions->Push("DelAlarmsRange", req.Params());
}
void AnalyzerRemote::SetRemote(HTTPRequest &req) {
  if(req.UserName() != pOwner) return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Synced")) return;
  if(State() == UNCONFIGURED) {
    if(!req.CheckParamObject("Config") || !req.CheckParamNumber("SampleRate") || !req.CheckParamString("Mode")) return;
  }
  if(req.CheckParamString("Mode")) {
    if(Analyzer::ModeToString(pMode) != req.Param("Mode")) return req.Error("Analyzer mode do not match", HTTP_BAD_REQUEST);
  }
  if(req.CheckParamNumber("SampleRate")) {
    pthread_rwlock_wrlock(&pLock);
    pSamplerate = req.Param("SampleRate");
    pthread_rwlock_unlock(&pLock);
  }
  if(req.CheckParamObject("Config")) {
    pthread_rwlock_wrlock(&pLock);
    pConfig = AnalyzerSoft::CheckConfig(req.Param("Config"), pMode, false);
    pRates->Config(pConfig["TimeZone"], pConfig["Rates"]);
    Name(pConfig["Name"]);
    pthread_rwlock_unlock(&pLock);
    State(SYNCING);
  }
  Database DB(pSerial);
  json     Response = json::object();
  pNow->SetRemote(req.Params(), Response);
  pActions->SetRemote(req.Params(), Response);
  Response["Time_200MS"]  = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_200ms") && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_3S"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_3s")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1M"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1m")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_10M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_10m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_15M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_15m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1H"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1h")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Alarms"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from alarms")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Events"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from events")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  State((req.Param("Synced") == true) ? ONLINE : SYNCING);
  req.Success(Response);
}
bool AnalyzerRemote::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  return false;
}
//----------------------------------------------------------------------------------------------------------------------