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
#include "AnalyzerSoftAlarms.h"
#include "TelegramManager.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAlarms::AnalyzerSoftAlarms(const string &serial, AnalyzerSoftMQTT *mqtt, DatabaseQueue *db) {
  pSerial           = serial;
  pNominalVoltage   = 230.0;
  pNominalFrequency = 50.0;
  pWorker           = NULL;
  pMQTT             = mqtt;
  pDB               = db;
  pLock             = PTHREAD_RWLOCK_INITIALIZER;
}
AnalyzerSoftAlarms::~AnalyzerSoftAlarms() {
  pthread_rwlock_wrlock(&pLock);
  if(pWorker != NULL) delete pWorker;
  pthread_rwlock_unlock(&pLock);
}
json AnalyzerSoftAlarms::CheckConfig(const json &config) {
  json ConfigOUT = json::array();
  if(config.is_array() == false) return ConfigOUT;
  for(auto &Alarm : config) ConfigOUT.push_back(AnalyzerSoftAlarm::CheckConfig(Alarm));
  return ConfigOUT;
}
void AnalyzerSoftAlarms::ConfigNominals(const float nominalVoltage, const float nominalFreq) {
  pNominalVoltage   = nominalVoltage;
  pNominalFrequency = nominalFreq;
}
void AnalyzerSoftAlarms::Config(const bool enabled, const string &tz, const json &alarms) {
  pthread_rwlock_wrlock(&pLock);
  cctz::load_time_zone(tz, &pTimeZone);
  if((enabled == false) && (pWorker != NULL)) {
    delete pWorker;
    pWorker = NULL;
  } else if((enabled == true) && (pWorker == NULL)) {
    pWorker = new Worker("AlarmsDet", Worker::ABOUTNORMAL);
  }
  for(auto &List : pAlarms) {
    for(auto &Alarm : List.second) delete Alarm;
  }
  pAlarms.clear();
  string IDs;
  for(auto &Alarm : alarms) {
    string Aggreg = Alarm["Aggreg"];
    string ID     = Alarm["AlarmID"];
    pAlarms.emplace(std::piecewise_construct, std::make_tuple(Aggreg), std::make_tuple()).first->second.emplace_back(new AnalyzerSoftAlarm(pSerial, pNominalVoltage, pNominalFrequency, Alarm, pDB));
    IDs = (IDs.size() == 0) ? IDs + pDB->Bind(ID) : IDs + ", " + pDB->Bind(ID);
  }
  if(alarms.size() == 0) {
    pDB->ExecSentenceNoResult("DELETE FROM alarms");
  } else {
    pDB->ExecSentenceNoResult("DELETE FROM alarms WHERE id NOT IN(" + IDs + ")");
  }
  pthread_rwlock_unlock(&pLock);
}
void AnalyzerSoftAlarms::Alarms_Func(const string &name, AnalyzerSoftAggreg *params) {
  pthread_rwlock_rdlock(&pLock);
  auto List = pAlarms.find(name);
  if(List != pAlarms.end()) {
    for(auto &Alarm : List->second) {
      auto Result = Alarm->PushAggreg(params);
      if(Result == AnalyzerSoftAlarm::TRIGGERED) {
        pMQTT->PublishAlarm(params->Timestamp(), Alarm->Name(), Alarm->Priority(), true);
        TelegramManager::SendAlarm(pSerial, pTimeZone, params->Timestamp(), Alarm->Name(), Alarm->Priority(), true);
      } else if(Result == AnalyzerSoftAlarm::RELEASED) {
        pMQTT->PublishAlarm(params->Timestamp(), Alarm->Name(), Alarm->Priority(), false);
        TelegramManager::SendAlarm(pSerial, pTimeZone, params->Timestamp(), Alarm->Name(), Alarm->Priority(), false);
      }
    }
  }
  pthread_rwlock_unlock(&pLock);
}
void AnalyzerSoftAlarms::Run(const string &aggreg, AnalyzerSoftAggreg *params) {
  pthread_rwlock_rdlock(&pLock);
  if(pWorker != NULL) pWorker->Run([&, aggreg, params]() { this->Alarms_Func(aggreg, params); });
  pthread_rwlock_unlock(&pLock);
}
void AnalyzerSoftAlarms::Wait() {
  pthread_rwlock_rdlock(&pLock);
  if(pWorker != NULL) pWorker->Wait();
  pthread_rwlock_unlock(&pLock);
}
json AnalyzerSoftAlarms::GetConfig() {
  json Result = json::array();
  pthread_rwlock_rdlock(&pLock);
  for(auto &List : pAlarms) {
    for(auto &Alarm : List.second)
    Result.push_back(Alarm->GetConfig());
  }
  pthread_rwlock_unlock(&pLock);
  return Result;
}
void AnalyzerSoftAlarms::TestAlarm(const json &params, Response &response, AnalyzerSoftAggreg200MS *aggreg) {
  if((params.contains("Expression") == false) || (params["Expression"].is_string() == false)) return HTTPServer::FillResponse(response, "'Expression' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Parser Evaluator;
  json   Result = json::object();
  Evaluator.DefineVar("NominalVoltage", &pNominalVoltage);
  Evaluator.DefineVar("NominalFrequency", &pNominalFrequency);
  aggreg->muParserInit(Evaluator, params["Expression"]);
  Result["Variables"] = json::object();
  for(auto Var : Evaluator.GetVar()) Result["Variables"][Var.first] = *Var.second;
  try {
    Result["Result"] = Evaluator.Eval();
  } catch (Parser::exception_type &e) {
    Result["Error"] = e.GetMsg();
  }
  return HTTPServer::FillResponse(response, Result.dump(), "application/json", (Result.contains("Error")) ? HTTP_BAD_REQUEST : HTTP_OK);
}
void AnalyzerSoftAlarms::GetAlarms(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To   = params["To"];
  uint64_t From = params["From"];
  Database DB(pSerial);
  if((params.contains("Update") == true) && (params["Update"].is_boolean() == true) && (params["Update"] == true)) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET alarmsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  auto Func = [&, To, From](uint64_t offset, DataSink &sink) -> bool {
    json Headers = {"AlarmID", "Trigger", "Release"};
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(id, trigger, release) as result FROM alarms WHERE trigger >= " + DB.Bind(From) + " AND trigger <= " + DB.Bind(To));
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftAlarms::GetAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(trigger), 'From', min(trigger)) AS ranges FROM alarms");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftAlarms::DelAlarm(const json &params, Response &response) {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Trigger") == false) || (params["Trigger"].is_number() == false)) return HTTPServer::FillResponse(response, "'Trigger' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   ID       = params["AlarmID"];
  uint64_t Trigger = params["Trigger"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id = " + DB.Bind(ID) + " AND trigger = " + DB.Bind(Trigger)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftAlarms::DelAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE trigger >= " + DB.Bind(From) + " AND trigger <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------