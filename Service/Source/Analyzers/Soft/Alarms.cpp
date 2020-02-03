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
#include <condition_variable>

#include "Alarms.h"
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::Alarms::Alarm::Alarm(const uint32_t id, const json &config) {
  pAlarmID = id;
  Configure(config);
}
void AnalyzerSoft::Alarms::Alarm::Configure(const json &config) {
  pMemberLocks.WriteLock();
  pName = config["Name"];
  pPriority = config["Priority"];
  if(pPriority > 2) pPriority = 2;
  try {
    pEvaluator.SetExpr(config["Expression"]);
  } catch (Parser::exception_type &e) {
  }
  pType = (config["Aggreg"] == "3S") ? AnalyzerSoft::AGGREG_3S : (config["Aggreg"] == "1M") ? AnalyzerSoft::AGGREG_1M : (config["Aggreg"] == "10M") ? AnalyzerSoft::AGGREG_10M : (config["Aggreg"] == "15M") ? AnalyzerSoft::AGGREG_15M : AnalyzerSoft::AGGREG_1H;
  pMessage = config["Message"];
  pSamples = config["Samples"];
  pWindow = config["Window"];
  pResults = queue<bool>();
  pMemberLocks.WriteUnlock();
}
json AnalyzerSoft::Alarms::Alarm::CheckConfig(const json& config) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed alarm";
  ConfigOUT["Priority"] = 0;
  ConfigOUT["Expression"] = "";
  ConfigOUT["Aggreg"] = "3S";
  ConfigOUT["Message"] = "Alarm triggered";
  ConfigOUT["Samples"] = 1;
  ConfigOUT["Window"] = 10;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("Priority") && config["Priority"].is_number_unsigned() && config["Priority"] < 3) ConfigOUT["Priority"] = config["Priority"];
  if(config.contains("Expression") && config["Expression"].is_string()) ConfigOUT["Expression"] = config["Expression"];
  if(config.contains("Aggreg") && config["Aggreg"].is_string() && ((config["Aggreg"] == "3S") || (config["Aggreg"] == "1M") || (config["Aggreg"] == "10M") || (config["Aggreg"] == "15M") || (config["Aggreg"] == "1H"))) ConfigOUT["Aggreg"] = config["Aggreg"];
  if(config.contains("Message") && config["Message"].is_string()) ConfigOUT["Message"] = config["Message"];
  if(config.contains("Samples") && config["Samples"].is_number_unsigned() && config["Samples"] < 1000) ConfigOUT["Samples"] = config["Samples"];
  if(config.contains("Window") && config["Window"].is_number_unsigned()) ConfigOUT["Window"] = config["Window"];
  if(ConfigOUT["Window"] < ConfigOUT["Samples"]) ConfigOUT["Window"] = ConfigOUT["Samples"];
  return ConfigOUT;
}
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::Alarms::Alarms(AnalyzerSoft *parent, const json &config) {
  pLastAlarmID = 0;
  pParent = parent;
//  pConfigPath = configPath;
  pMemberLocks.WriteLock();
  pTerminate = false;
//  json Config = ConfigManager::ReadConfig(configPath);
  for(auto &Tmp : config) pAlarms.emplace_back(pLastAlarmID++, Tmp);
  pMemberLocks.WriteUnlock();
  if(pthread_create(&pAlarmThread, NULL, Alarms_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pAlarmThread = 0;
    return;
  }
  pthread_setname_np(pAlarmThread, "AlarmsDet");
}
void *AnalyzerSoft::Alarms::Alarms_Thread(void* args) {
  Alarms *This = (Alarms*)args;
  while(This->pTerminate == false) {
    if(This->pWorkerLocks.WorkerWaitTask() == false) break;
    This->pMemberLocks.ReadLock();
    printf("chech alarms\n");
    for(auto Alarm : This->pAlarms) {
    }
    This->pMemberLocks.ReadUnlock();
    This->pWorkerLocks.WorkerEndsTask();
  }
  return NULL;
}
AnalyzerSoft::Alarms::~Alarms() {
  pTerminate = true;
  pWorkerLocks.Abort(&pAlarmThread);
  pMemberLocks.WriteLock();
  pAlarms.clear();
  pMemberLocks.WriteUnlock();
}
json AnalyzerSoft::Alarms::CheckConfig(const json &config) {
  json ConfigOUT = json::array();
  if(!config.is_array()) return ConfigOUT;
  for(auto Tmp : config) {
    if(!Tmp.is_object()) continue;
    ConfigOUT.push_back(Alarm::CheckConfig(Tmp));
  }
  return ConfigOUT;
}
void AnalyzerSoft::Alarms::Start(AnalyzerSoft::Params_t &aggreg) {
  pParams = &aggreg;
  pWorkerLocks.WakeWorker();
}
void AnalyzerSoft::Alarms::Wait() {
  pWorkerLocks.WaitWorker();
}
void AnalyzerSoft::Alarms::TestAlarm(const json &params, Response &response) {
  //Parse, check params and prepare result struct
  json Result = json::object();
  Result["Variables"] = json::array();
  json NominalVoltage = json::object();
  NominalVoltage["Name"] = "NominalVoltage";
  NominalVoltage["Value"] = pParent->pNominalVoltage;
  NominalVoltage["Units"] = "V";
  Result["Variables"].push_back(NominalVoltage);
  json NominalFreq = json::object();
  NominalFreq["Name"] = "NominalFrequency";
  NominalFreq["Value"] = pParent->pNominalFrequency;
  NominalFreq["Units"] = "Hz";
  Result["Variables"].push_back(NominalFreq);
  string   Aggreg     = params["Aggreg"];
  string   Expression = params["Expression"];
  Database DB(pParent->Serial);
  if(Aggreg == "3S") {
    DB.ExecSentenceAsyncResult("SELECT json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag, 'Active_Power', active_power, 'Reactive_Power', reactive_power) as result FROM aggreg_3s ORDER BY sampletime DESC LIMIT 1");
    Result["Aggregation_Source"] = "3S";
  } else if(Aggreg == "1M") {
    DB.ExecSentenceAsyncResult("SELECT * FROM aggreg_3s ORDER BY sampletime ASC LIMIT 1");
  }
  if(DB.FecthRow() == true) {
    json Query = DB.ResultJSON("result");
    if(Query["Voltage"].is_array()) {
      unsigned Phases = Query["Voltage"].size();
      for(unsigned n = 0; n < Phases; n++) {
        float Tmp = 0.0;
        if(Query["Voltage"][n].is_number()) Tmp = Query["Voltage"][n];
        json Voltage = json::object();
        Voltage["Name"] = (Phases > 1) ? "Voltage_" + to_string(n + 1) : "Voltage";
        Voltage["Value"] = Tmp;
        Voltage["Units"] = "V";
        Result["Variables"].push_back(Voltage);
      }
    }
    if(Query["Current"].is_array()) {
      unsigned Phases = Query["Current"].size();
      for(unsigned n = 0; n < Phases; n++) {
        float Tmp = 0.0;
        if(Query["Current"][n].is_number()) Tmp = Query["Current"][n];
        json Current = json::object();
        Current["Name"] = (Phases > 1) ? "Current_" + to_string(n + 1) : "Current";
        Current["Value"] = Tmp;
        Current["Units"] = "A";
        Result["Variables"].push_back(Current);
      }
    }
    if(Query["Active_Power"].is_array()) {
      unsigned Phases = Query["Active_Power"].size();
      for(unsigned n = 0; n < Phases; n++) {
        float Tmp = 0.0;
        if(Query["Active_Power"][n].is_number()) Tmp = Query["Active_Power"][n];
        json Active_Power = json::object();
        Active_Power["Name"] = (Phases > 1) ? "Active_Power_" + to_string(n + 1) : "Active_Power";
        Active_Power["Value"] = Tmp;
        Active_Power["Units"] = "W";
        Result["Variables"].push_back(Active_Power);
      }
    }
    if(Query["Reactive_Power"].is_array()) {
      unsigned Phases = Query["Reactive_Power"].size();
      for(unsigned n = 0; n < Phases; n++) {
        float Tmp = 0.0;
        if(Query["Reactive_Power"][n].is_number()) Tmp = Query["Reactive_Power"][n];
        json Reactive_Power = json::object();
        Reactive_Power["Name"] = (Phases > 1) ? "Reactive_Power_" + to_string(n + 1) : "Reactive_Power";
        Reactive_Power["Value"] = Tmp;
        Reactive_Power["Units"] = "var";
        Result["Variables"].push_back(Reactive_Power);
      }
    }
    if(Query["Frequency"].is_number()) {
      float Tmp = 0.0;
      if(Query["Frequency"].is_number()) Tmp = Query["Frequency"];
      json Frequency = json::object();
      Frequency["Name"] = "Frequency";
      Frequency["Value"] = Tmp;
      Frequency["Units"] = "Hz";
      Result["Variables"].push_back(Frequency);
    }
  }
  try {
    Parser Exp;
    for(auto &Var : Result["Variables"]) Exp.DefineConst(Var["Name"], Var["Value"]);
    Exp.SetExpr(Expression);
    float Tmp = Exp.Eval();
    Result["Result"] = Tmp;
  } catch (Parser::exception_type &e) {
    Result["Error"] = e.GetMsg();
  }
  response.set_content(Result.dump(), "application/json");
  response.status = HTTP_OK;
}
void AnalyzerSoft::Alarms::SetAlarm(const json &params, Response &response) {
  json Result = json::object();
  uint32_t AlarmID = params["AlarmID"];
  pMemberLocks.WriteLock();
  for(auto &Alarm : pAlarms) {
    if(Alarm.GetID() != AlarmID) continue;
    Alarm.Configure(Alarm::CheckConfig(params));
    json Config = ConfigManager::ReadConfig(pParent->pConfigPath);
    Config["Alarms"] = json::array();
    pMemberLocks.ReadLock();
    for(auto Alarm : pAlarms) Config["Alarms"].push_back(Alarm.GetFullDescriptor());
    pMemberLocks.ReadUnlock();
    ConfigManager::WriteConfig(pParent->pConfigPath, Config);
    response.set_content("", "text/plain");
    response.status = HTTP_OK;
    return;
  }
  pMemberLocks.WriteUnlock();
  response.set_content("AlarmID not found", "text/plain");
  response.status = HTTP_BAD_REQUEST;
}
