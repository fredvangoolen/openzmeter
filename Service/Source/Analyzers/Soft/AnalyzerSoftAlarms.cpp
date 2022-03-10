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
AnalyzerSoftAlarms::AnalyzerSoftAlarms(const string &serial, AnalyzerSoftMQTT *mqtt) : AnalyzerAlarms(serial) {
  pNominalVoltage   = 230.0;
  pNominalFrequency = 50.0;
  pEnabled          = false;
  pMQTT             = mqtt;
  pTask             = NULL;
}
AnalyzerSoftAlarms::~AnalyzerSoftAlarms() {
  pLock.WriteLock();
  if(pTask != NULL) TaskPool::Wait(pTask);
  pLock.Unlock();
}
json AnalyzerSoftAlarms::CheckConfig(const json &config) {
  json ConfigOUT = json::array();
  if(config.is_array() == false) return ConfigOUT;
  for(auto &Alarm : config) ConfigOUT.push_back(AnalyzerSoftAlarm::CheckConfig(Alarm));
  return ConfigOUT;
}
void AnalyzerSoftAlarms::Config(const bool enabled, const string &tz, const json &alarms, const float nominalVoltage, const float nominalFreq) {
  pLock.WriteLock();
  pNominalVoltage   = nominalVoltage;
  pNominalFrequency = nominalFreq;
  cctz::load_time_zone(tz, &pTimeZone);
  pEnabled = enabled;
  for(auto &List : pAlarms) {
    for(auto &Alarm : List.second) delete Alarm;
  }
  pAlarms.clear();
  Database DB(pSerial);
  string   IDs;
  for(auto &Alarm : alarms) {
    string Aggreg = Alarm["Aggreg"];
    string ID     = Alarm["AlarmID"];
    pAlarms.emplace(std::piecewise_construct, std::make_tuple(Aggreg), std::make_tuple()).first->second.emplace_back(new AnalyzerSoftAlarm(pSerial, pNominalVoltage, pNominalFrequency, Alarm));
    IDs.append((IDs.size() == 0) ? DB.Bind(ID) : ", " + DB.Bind(ID));
  }
  if(alarms.size() == 0) {
    DB.ExecSentenceNoResult("DELETE FROM alarms");
  } else {
    DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id NOT IN(" + IDs + ")");
  }
  pLock.Unlock();
}
void AnalyzerSoftAlarms::Alarms_Func(const string &name, AnalyzerSoftAggreg *params) {
  pLock.ReadLock();
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
  pLock.Unlock();
}
void AnalyzerSoftAlarms::Run(const string &aggreg, AnalyzerSoftAggreg *params) {
  pLock.ReadLock();
  if(pEnabled) pTask = TaskPool::Run([&, aggreg, params]() { this->Alarms_Func(aggreg, params); }, TaskPool::ABOUTNORMAL);
  pLock.Unlock();
}
void AnalyzerSoftAlarms::Wait() {
  pLock.ReadLock();
  if(pTask != NULL) TaskPool::Wait(pTask);
  pLock.Unlock();
}
void AnalyzerSoftAlarms::TestAlarm(HTTPRequest &req, AnalyzerSoftAggreg200MS *aggreg) const {
  if(!req.CheckParamString("Expression")) return;
  Parser Evaluator;
  json   Result = json::object();
  float  NominalVoltage   = pNominalVoltage;
  float  NominalFrequency = pNominalFrequency;
  Evaluator.DefineVar("NominalVoltage", &NominalVoltage);
  Evaluator.DefineVar("NominalFrequency", &NominalFrequency);
  aggreg->muParserInit(Evaluator, req.Param("Expression"));
  Result["Variables"] = json::object();
  for(auto Var : Evaluator.GetVar()) Result["Variables"][Var.first] = *Var.second;
  try {
    Result["Result"] = Evaluator.Eval();
    return req.Success(Result);
  } catch (Parser::exception_type &e) {
    return req.Error(e.GetMsg(), HTTP_BAD_REQUEST);
  }
}
// ---------------------------------------------------------------------------------------------------------------------