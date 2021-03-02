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

  pActions       = new AnalyzerRemoteActions(id);

  pLastClientReq = 0;
  pConfig        = nullptr;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerRemote::~AnalyzerRemote() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  delete pActions;
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerRemote::SetClientConnected() {
  pthread_mutex_lock(&pMutex);
  pLastClientReq = Tools::GetTime64();
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerRemote::isClientConnected() {
  uint64_t Now = Tools::GetTime64() - 60000;
  pthread_mutex_lock(&pMutex);
  bool Result = (pLastClientReq > Now);
  pthread_mutex_unlock(&pMutex);
  return Result;
}
void AnalyzerRemote::GetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pthread_mutex_lock(&pMutex);
  json Response = pConfig;
  pthread_mutex_unlock(&pMutex);
  Response["Online"]       = Online();
  Response["OnlineTime"]   = OnlineTime();
  Response["Mode"]         = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["User"]         = pOwner;
  Response["PriceModules"] = PriceSource::VarsInfo();
  Response["Users"]        = pAccess.Users();
  return HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.GetSeries(params, response);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pSeries.GetSeriesRange(params, response);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEvents(params, response);
}
void AnalyzerRemote::GetEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEventsRange(params, response);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pEvents.GetEvent(params, response);
}
void AnalyzerRemote::GetCosts(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pRates.GetCosts(params, response);
}
void AnalyzerRemote::GetBills(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pRates.GetBills(params, response);
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.GetAlarms(params, response);
}
void AnalyzerRemote::GetAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pAlarms.GetAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelAlarm(params, response);
}
void AnalyzerRemote::DelSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelSeriesRange(params, response);
}
void AnalyzerRemote::DelEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelEventsRange(params, response);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelEventsRange(params, response);
}



void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  //Tengo que flushear datos del local?
//  PushAction("SetAnalyzer", AnalyzerSoft::CheckConfig(params, , false));
  //Valido datos para no mandar peticiones erroneas

//  HandleRequest(AnalyzerAccess::CONFIG, "setAnalyzer", params, response);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::GetStatus(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {
//  HandleRequest(AnalyzerAccess::READ, "getWaveStream", params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(params["UserName"], AnalyzerAccess::READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::SetRemote(const json &params, Response &response) {
  if(params["UserName"] != pOwner) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  printf("<- %s\n", params.dump().c_str());
  Online(true);
  if(params.contains("Config") && params["Config"].is_object()) {
    pConfig = AnalyzerSoft::CheckConfig(params["Config"], pMode, false);
    pRates.Config(pConfig["TimeZone"], pConfig["Rates"]);
    //Leo samplerate y compruebo tambien el modo, si el modo no coincide tengo que devolver un error
    Configured(true);
  }
  if(params.contains("Now") && params["Now"].is_object()) {
    pthread_mutex_lock(&pMutex);
    pNow = params["Now"];
    pthread_mutex_unlock(&pMutex);
  }
  if(params.contains("Actions") && params["Actions"].is_array()) {
    for(auto &Action : params["Actions"]) {
      Response Resp;
      if((Action.contains("Type") == false) || (Action["Type"].is_string() == false)) continue;
      string Type = Action["Type"];
      if(Type == "SetSeries") {
        pSeries.SetSeries(Action["Params"], Resp);
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
  Response["Time_200MS"]  = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_200ms") && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_3S"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_3s")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1M"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1m")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_10M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_10m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_15M"]    = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_15m")   && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_1H"]     = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(sampletime), 0) from aggreg_1h")    && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Alarms"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from alarms")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
  Response["Time_Events"] = (DB.ExecSentenceAsyncResult("SELECT COALESCE(max(changetime), 0) from events")       && DB.FecthRow()) ? DB.ResultUInt64(0) : UINT64_MAX;
//  Response["Action"] =
//ActionResult



  printf("-> %s\n", Response.dump().c_str());
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::SetSeries(const json &params, Response &response) {}
void AnalyzerRemote::SetEvents(const json &params, Response &response) {}
void AnalyzerRemote::SetAlarms(const json &params, Response &response) {}
bool AnalyzerRemote::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  return false;
}
//----------------------------------------------------------------------------------------------------------------------