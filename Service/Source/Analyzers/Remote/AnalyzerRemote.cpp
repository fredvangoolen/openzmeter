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
  pDB            = new DatabaseQueue(id, true);
  pDefStorage    = new AnalyzerDefaultStorage(id);
  pActions       = new AnalyzerRemoteActions(id);
  pDefRates      = new AnalyzerDefaultRates(id);
  pLastClientReq = 0;
  pConfig        = nullptr;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
  MessageLoop::RegisterListener("AnalyzerRemote_" + pSerial + "_PeriodicTasks", [&]() { this->pDefStorage->CleanOldSamples(); }, 60000);
}
AnalyzerRemote::~AnalyzerRemote() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  MessageLoop::UnregisterListener("AnalyzerRemote_" + pSerial + "_PeriodicTasks");
  delete pActions;
  delete pDefRates;
  delete pDefStorage;
  delete pDB;
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
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
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
  Response["Users"]        = GetUsers();
  return HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemote::GetSeries(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetSeries(params, response);
}
void AnalyzerRemote::GetSeriesRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetSeriesRange(params, response);
}
void AnalyzerRemote::GetEvents(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetEvents(params, response);
}
void AnalyzerRemote::GetEventsRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetEventsRange(params, response);
}
void AnalyzerRemote::GetEvent(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetEvent(params, response);
}
void AnalyzerRemote::GetCosts(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefRates->GetCosts(params, response);
}
void AnalyzerRemote::GetBills(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefRates->GetBills(params, response);
}
void AnalyzerRemote::GetAlarms(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetAlarms(params, response);
}
void AnalyzerRemote::GetAlarmsRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pDefStorage->GetAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarmsRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelAlarmsRange(params, response);
}
void AnalyzerRemote::DelAlarm(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelAlarm(params, response);
}
void AnalyzerRemote::DelSeriesRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelSeriesRange(params, response);
}
void AnalyzerRemote::DelEventsRange(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelEventsRange(params, response);
}
void AnalyzerRemote::DelEvent(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  pActions->DelEventsRange(params, response);
}



void AnalyzerRemote::SetAnalyzer(const json &params, Response &response) {
  if(TestUser(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
  //Tengo que flushear datos del local?
//  PushAction("SetAnalyzer", AnalyzerSoft::CheckConfig(params, , false));
  //Valido datos para no mandar peticiones erroneas

//  HandleRequest(AnalyzerAccess::CONFIG, "setAnalyzer", params, response);
}
void AnalyzerRemote::GetSeriesNow(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::GetStatus(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::GetWaveStream(const json &params, Response &response) {
//  HandleRequest(AnalyzerAccess::READ, "getWaveStream", params, response);
}
void AnalyzerRemote::SetEvent(const json &params, Response &response) {
  if(TestUser(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(Configured() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  SetClientConnected();
//HACER
}
void AnalyzerRemote::TestAlarm(const json &params, Response &response) {
  if(TestUser(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
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
    pDefRates->Config(params["Config"]["TimeZone"], params["Config"]["Rates"]);
    //Leo samplerate y compruebo tambien el modo, si el modo no coincide tengo que devolver un error
    Configured(true);
  }
  if(params.contains("Now") && params["Now"].is_object()) {
    pthread_mutex_lock(&pMutex);
    pNow = params["Now"];
    pthread_mutex_unlock(&pMutex);
  }
  if(params.contains("Actions") && params["Action"].is_array()) {


    if(params.contains("Samples") && params["Samples"].is_object()) {
  //    "Samples": {
  //     "200MS": [{...}, {...}],
  //     "3S": [{...}, {...}],
  //     ...     //Se envian muestras nuevas que haya desde la ultima conexion, limitar cantidad para que no sea una burrada de consulta
    }
    if(params.contains("Alarms") && params["Alarms"].is_array()) {
  //    "Alarms": [{...}],  //Si hay algun cambio en alguna alarma lo envio ,limitar cantidad
    }
    if(params.contains("Events") && params["Events"].is_array()) {
  //    "Events": [{...}],  //Si hay algun cambio en un evento lo mando, limitar cantidad
    }

  }
  Database DB(pSerial);
  json     Response          = json::object();
  Response["Now"]            = isClientConnected();
  Response["Last"]           = json::object();
  Response["Last"]["200MS"]  = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_200ms") && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["3S"]     = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_3s")    && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["1M"]     = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_1m")    && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["10M"]    = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_10m")   && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["15M"]    = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_15m")   && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["1H"]     = (DB.ExecSentenceAsyncResult("SELECT max(sampletime) from aggreg_1h")    && DB.FecthRow()) ? DB.ResultUInt64(0) + 1 : UINT64_MAX;
  Response["Last"]["Alarms"] = UINT64_MAX;
  Response["Last"]["Events"] = UINT64_MAX;
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