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
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAlarms::AnalyzerSoftAlarms(AnalyzerSoftMQTT *mqtt, const string &serial) {
  pSerial           = serial;
  pNominalVoltage   = 230.0;
  pNominalFrequency = 50.0;
  pWorker           = NULL;
  pMQTT             = mqtt;
  pConfigMutex      = PTHREAD_MUTEX_INITIALIZER;
  pAlarmsMutex      = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerSoftAlarms::~AnalyzerSoftAlarms() {
  Config(false, pNominalVoltage, pNominalFrequency);
}
void AnalyzerSoftAlarms::Config(const bool enabled, const float nominalVoltage, const float nominalFreq) {
  pthread_mutex_lock(&pConfigMutex);
  pNominalVoltage   = nominalVoltage;
  pNominalFrequency = nominalFreq;
  if((enabled == false) && (pWorker != NULL)) {
    pthread_mutex_lock(&pAlarmsMutex);
    pAlarms.clear();
    pthread_mutex_unlock(&pAlarmsMutex);
    pWorker->Wait();
    delete pWorker;
    pWorker = NULL;
  } else if((enabled == true) && (pWorker == NULL)) {
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT id, config::jsonb || jsonb_build_object('Name', name) as config FROM alarms");
    pthread_mutex_lock(&pAlarmsMutex);
    while(DB.FecthRow()) pAlarms.emplace(piecewise_construct, forward_as_tuple(DB.ResultString("id")), forward_as_tuple(pSerial, DB.ResultJSON("config"), pNominalVoltage, pNominalFrequency));
    pthread_mutex_unlock(&pAlarmsMutex);
    pWorker = new Worker("AlarmsDet", [&](void *a) { this->Alarms_Func((AnalyzerSoftAggreg*)a); });
  }
  pthread_mutex_unlock(&pConfigMutex);
}
void AnalyzerSoftAlarms::Alarms_Func(AnalyzerSoftAggreg *aggreg) {
  pthread_mutex_lock(&pAlarmsMutex);
  for(auto &Alarm : pAlarms) {
    auto Result = Alarm.second.PushAggreg(pRunningAggreg, aggreg);
    if(Result == AnalyzerSoftAlarm::TRIGGERED) {
      pMQTT->PublishAlarm(aggreg->Timestamp(), Alarm.second.Name(), Alarm.second.Priority(), true);
//      Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
    } else if(Result == AnalyzerSoftAlarm::RELEASED) {
      pMQTT->PublishAlarm(aggreg->Timestamp(), Alarm.second.Name(), Alarm.second.Priority(), false);
//    Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
    }
  }
  pthread_mutex_unlock(&pAlarmsMutex);
}
void AnalyzerSoftAlarms::Run(const string &aggreg, const AnalyzerSoftAggreg *params) {
  pthread_mutex_lock(&pConfigMutex);
  pRunningAggreg = aggreg;
  if(pWorker != NULL) pWorker->Run((void*)params);
  pthread_mutex_unlock(&pConfigMutex);
}
void AnalyzerSoftAlarms::Wait() {
  pthread_mutex_lock(&pConfigMutex);
  if(pWorker != NULL) pWorker->Wait();
  pthread_mutex_unlock(&pConfigMutex);
}
void AnalyzerSoftAlarms::GetAlarmsDef(const json &params, Response &response) {
  auto Func = [&](uint64_t offset, DataSink &sink) -> bool {
    uint32_t Items = 0;
    sink.os << "[\n  ";
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_object('AlarmID', id, 'Name', name) AS result FROM alarms");
    while(DB.FecthRow() && sink.is_writable()) {
      if(Items++ != 0) sink.os << ",\n  ";
      sink.os << DB.ResultString("result");
    }
    sink.os << "\n]";
    sink.done();
    return false;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftAlarms::TestAlarm(const json &params, Response &response, AnalyzerSoftAggreg200MS *aggreg) {
  if((params.contains("Expression") == false) || (params["Expression"].is_string() == false)) return HTTPServer::FillResponse(response, "'Expression' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Parser Evaluator;
  json   Result = json::object();
  Evaluator.DefineVar("Nominal_Voltage", &pNominalVoltage);
  Evaluator.DefineVar("Nominal_Frequency", &pNominalFrequency);
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
void AnalyzerSoftAlarms::AddAlarmDef(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID     = Tools::GetUUID();
  string Name   = params["Name"];
  json   Config = AnalyzerSoftAlarm::CheckConfig(params);
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("INSERT INTO alarms(id, name, config) VALUES(" + DB.Bind(ID) + ", " + DB.Bind(Name) + ", " + DB.Bind(Config.dump()) + ")") ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pAlarmsMutex);
  pAlarms.emplace(piecewise_construct, forward_as_tuple(DB.ResultString("id")), forward_as_tuple(pSerial, Config, pNominalVoltage, pNominalFrequency));
  pthread_mutex_unlock(&pAlarmsMutex);
}
void AnalyzerSoftAlarms::GetAlarmDef(const json &params, Response &response) {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["AlarmID"];
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT jsonb_build_object('Name', name) || config::jsonb AS result FROM alarms WHERE id = " + DB.Bind(ID));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoftAlarms::SetAlarmDef(const json &params, Response &response) {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID     = params["AlarmID"];
  string Name   = params["Name"];
  json   Config = AnalyzerSoftAlarm::CheckConfig(params);
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("UPDATE alarms SET name = " + DB.Bind(Name) + ", config = " + DB.Bind(Config.dump()) +" WHERE id = " + DB.Bind(ID)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pAlarmsMutex);
  auto Item = pAlarms.find(ID);
  if(Item != pAlarms.end()) pAlarms.erase(Item);
  pAlarms.emplace(piecewise_construct, forward_as_tuple(DB.ResultString("id")), forward_as_tuple(pSerial, Config, pNominalVoltage, pNominalFrequency));
  pthread_mutex_unlock(&pAlarmsMutex);
}
void AnalyzerSoftAlarms::DelAlarmDef(const json &params, Response &response) {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["AlarmID"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id = " + DB.Bind(ID)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pAlarmsMutex);
  auto Item = pAlarms.find(ID);
  if(Item != pAlarms.end()) pAlarms.erase(Item);
  pthread_mutex_unlock(&pAlarmsMutex);
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
    json Headers = {"Name", "Trigger", "Release"};
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(name, trigger, release) as result FROM alarms_ext WHERE trigger >= " + DB.Bind(From) + " AND trigger <= " + DB.Bind(To));
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " << DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return false;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftAlarms::GetAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(alarmtime) AS max, min(alarmtime) AS min FROM alarms) AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftAlarms::DelAlarm(const json &params, Response &response) {
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Trigger") == false) || (params["Trigger"].is_number() == false)) return HTTPServer::FillResponse(response, "'Trigger' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Name    = params["Name"];
  uint64_t Trigger = params["Trigger"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE name = " + DB.Bind(Name) + " AND trigger = " + DB.Bind(Trigger)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftAlarms::DelAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms_ext WHERE alarmtime >= " + DB.Bind(From) + " AND alarmtime <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------