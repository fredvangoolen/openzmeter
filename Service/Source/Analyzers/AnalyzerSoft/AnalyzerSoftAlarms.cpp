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
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAlarms::AnalyzerSoftAlarms(const string &serial) {
  pSerial = serial;
    //Cargo las alarmas desde la DB
//  pWorker = new Worker("AlarmsDet", [&](void *a) { this->Alarms_Func((AnalyzerSoft::Params_t*)a); });
}
AnalyzerSoftAlarms::~AnalyzerSoftAlarms() {}
void AnalyzerSoftAlarms::GetAlarms(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To         = params["To"];
  uint64_t From       = params["From"];
  Database DB(pSerial);
  if((params.contains("Update") == true) && (params["Update"].is_boolean() == true) && (params["Update"] == true)) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET alarmsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  DB.ExecSentenceAsyncResult("SELECT json_build_object("
                              "'AlarmTime', COALESCE(ARRAY_AGG(alarmtime ORDER BY alarmtime ASC), '{}'), "
                              "'Name', COALESCE(ARRAY_AGG(name ORDER BY alarmtime ASC), '{}'), "
                              "'Priority', COALESCE(ARRAY_AGG(priority ORDER BY alarmtime ASC), '{}'), "
                              "'Triggered', COALESCE(ARRAY_AGG(trigger ORDER BY alarmtime ASC), '{}')) AS result "
                              "FROM alarms WHERE alarmtime >= " + DB.Bind(From) + " AND alarmtime <= " + DB.Bind(To));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoftAlarms::AddAlarm(const json &params, Response &response) {}
void AnalyzerSoftAlarms::GetAlarm(const json &params, Response &response) {}
void AnalyzerSoftAlarms::SetAlarm(const json &params, Response &response) {}
void AnalyzerSoftAlarms::DelAlarm(const json &params, Response &response) {}
void AnalyzerSoftAlarms::GetAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(alarmtime) AS max, min(alarmtime) AS min FROM alarms) AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftAlarms::DelAlarmsRange(const json &params, Response &response) {
  Database DB(pSerial);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE alarmtime >= " + DB.Bind(From) + " AND alarmtime <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}

/*

  if(config.contains("Alarms") && config["Alarms"].is_array()) {
    for(auto configAlarm : config["Alarms"]) {
      if((configAlarm.contains("Name") == false) || (configAlarm["Name"].is_string() == false)) continue;
      if((configAlarm.contains("Priority") == false) || (configAlarm["Priority"].is_number_unsigned() == false) || (configAlarm["Priority"] > 2)) continue;
      if((configAlarm.contains("Expression") == false) || (configAlarm["Expression"].is_string() == false)) continue;
      if((configAlarm.contains("Aggreg") == false) || (configAlarm["Aggreg"].is_string() == false) || ((configAlarm["Aggreg"] != "200MS") && (configAlarm["Aggreg"] != "3S") && (configAlarm["Aggreg"] != "1M") && (configAlarm["Aggreg"] != "10M") && (configAlarm["Aggreg"] != "15M") && (configAlarm["Aggreg"] != "1H"))) continue;
      if((configAlarm.contains("Samples") == false) || (configAlarm["Samples"].is_number_unsigned() == false) || (configAlarm["Samples"] > 1000)) continue;
      if((configAlarm.contains("Window") == false) || (configAlarm["Window"].is_number_unsigned() == false)) continue;
      if(configAlarm["Window"] < configAlarm["Samples"]) continue;
      ConfigOUT["Alarms"].push_back(configAlarm);
    }
  }



  if((AlarmsEnabled == false) && (pAlarms != NULL)) {
    delete pAlarms;
    pAlarms = NULL;
  } else if((AlarmsEnabled == true) && (pAlarms == NULL)) {
    pAlarms = new AnalyzerSoftAlarms(this);
  }*/

void AnalyzerSoftAlarms::TestAlarm(const json &params, Response &response) {
/*  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Expression") == false) || (params["Expression"].is_string() == false)) return HTTPServer::FillResponse(response, "'Expression' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Parser Evaluator;
  json   Result = json::object();
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  InitAlarmParser(Evaluator, params["Expression"], AGGREG_200MS);
  Result["Variables"] = json::object();
  for(auto Var : Evaluator.GetVar()) Result["Variables"][Var.first] = *Var.second;
  try {
    Result["Result"] = Evaluator.Eval();
  } catch (Parser::exception_type &e) {
    Result["Error"] = e.GetMsg();
  }
  pNowDataLock.ConsumerEnds();
  return HTTPServer::FillResponse(response, Result.dump(), "application/json", (Result.contains("Error")) ? HTTP_BAD_REQUEST : HTTP_OK);*/
}
void AnalyzerSoftAlarms::ListAlarms(const json &params, Response &response) {}


//  pAlarms = new Worker("AlarmsDet", [&](void *a) { this->Alarms_Func((Params_t*)a); });
//  if(config.contains("Alarms") && config["Alarms"].is_array()) {
//    for(auto configAlarm : config["Alarms"]) {
//      if((configAlarm.contains("Name") == false) || (configAlarm["Name"].is_string() == false)) continue;
//      if((configAlarm.contains("Priority") == false) || (configAlarm["Priority"].is_number_unsigned() == false) || (configAlarm["Priority"] > 2)) continue;
//      if((configAlarm.contains("Expression") == false) || (configAlarm["Expression"].is_string() == false)) continue;
//      if((configAlarm.contains("Aggreg") == false) || (configAlarm["Aggreg"].is_string() == false) || ((configAlarm["Aggreg"] != "200MS") && (configAlarm["Aggreg"] != "3S") && (configAlarm["Aggreg"] != "1M") && (configAlarm["Aggreg"] != "10M") && (configAlarm["Aggreg"] != "15M") && (configAlarm["Aggreg"] != "1H"))) continue;
//      if((configAlarm.contains("Samples") == false) || (configAlarm["Samples"].is_number_unsigned() == false) || (configAlarm["Samples"] > 1000)) continue;
//      if((configAlarm.contains("Window") == false) || (configAlarm["Window"].is_number_unsigned() == false)) continue;
//      if(configAlarm["Window"] < configAlarm["Samples"]) continue;
//      ConfigOUT["Alarms"].push_back(configAlarm);
//    }
//  }
/*

  for(Alarm_t &Alarm : pAlarms) {
    if(Alarm.Results != NULL) free(Alarm.Results);
  }
  pAlarms.clear();
  if(AlarmsEnabled == true) {
    pAlarms.resize(options["Alarms"].size());
    for(uint32_t n = 0; n < pAlarms.size(); n++) {
      pAlarms[n].Name        = options["Alarms"][n]["Name"];
      pAlarms[n].Priority    = options["Alarms"][n]["Priority"];
      pAlarms[n].Samples     = options["Alarms"][n]["Samples"];
      pAlarms[n].Window      = options["Alarms"][n]["Window"];
      pAlarms[n].ResultPos   = 0;
      pAlarms[n].ResultCount = 0;
      pAlarms[n].Aggreg      = (options["Alarms"][n]["Aggreg"] == "200MS") ? AnalyzerSoft::AGGREG_200MS :
                               (options["Alarms"][n]["Aggreg"] == "3S") ? AnalyzerSoft::AGGREG_3S :
                               (options["Alarms"][n]["Aggreg"] == "1M") ? AnalyzerSoft::AGGREG_1M :
                               (options["Alarms"][n]["Aggreg"] == "10M") ? AnalyzerSoft::AGGREG_10M :
                               (options["Alarms"][n]["Aggreg"] == "15M") ? AnalyzerSoft::AGGREG_15M : AnalyzerSoft::AGGREG_1H;
      pAlarms[n].Results     = (uint8_t*)calloc(ceil(pAlarms[n].Window / 8), 1);
      pAlarms[n].Triggered   = false;
      InitAlarmParser(pAlarms[n].Evaluator, options["Alarms"][n]["Expression"], pAlarms[n].Aggreg);
    }
  }



    pAlarms.resize(options["Alarms"].size());
    for(uint32_t n = 0; n < pAlarms.size(); n++) {
      pAlarms[n].Name        = options["Alarms"][n]["Name"];
      pAlarms[n].Priority    = options["Alarms"][n]["Priority"];
      pAlarms[n].Samples     = options["Alarms"][n]["Samples"];
      pAlarms[n].Window      = options["Alarms"][n]["Window"];
      pAlarms[n].ResultPos   = 0;
      pAlarms[n].ResultCount = 0;
      pAlarms[n].Aggreg      = (options["Alarms"][n]["Aggreg"] == "200MS") ? AnalyzerSoft::AGGREG_200MS :
                               (options["Alarms"][n]["Aggreg"] == "3S") ? AnalyzerSoft::AGGREG_3S :
                               (options["Alarms"][n]["Aggreg"] == "1M") ? AnalyzerSoft::AGGREG_1M :
                               (options["Alarms"][n]["Aggreg"] == "10M") ? AnalyzerSoft::AGGREG_10M :
                               (options["Alarms"][n]["Aggreg"] == "15M") ? AnalyzerSoft::AGGREG_15M : AnalyzerSoft::AGGREG_1H;
      pAlarms[n].Results     = (uint8_t*)calloc(ceil(pAlarms[n].Window / 8), 1);
      pAlarms[n].Triggered   = false;
      InitAlarmParser(pAlarms[n].Evaluator, options["Alarms"][n]["Expression"], pAlarms[n].Aggreg);
    }

    void AnalyzerSoft::PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered) {
  if(pMQTTClient.socketfd == -1) return;
  json Msg = json::object();
  Msg["AlarmTime"] = time;
  Msg["Name"] = name;
  Msg["Priority"] = prio;
  Msg["Triggered"] = triggered;
  string TextMsg = Msg.dump();
  string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + pSerial + "/Alarms" : pSerial + "/Alarms";
  mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1);
  LogFile::Debug("Analyzer '%s' publish message to MQTT broker", pSerial.c_str());
}
void AnalyzerSoftAlarms::InitAlarmParser(Parser &parser, const string &expr, const AggregType &type) {
  auto AppendArray = [&](const string &serie, float *data, const uint8_t len) {
    if(len == 1) {
      parser.DefineVar(serie, &data[0]);
    } else {
      for(uint8_t n = 0; n < len; n++) parser.DefineVar(serie + "_" + to_string(n + 1), &data[n]);
    }
  };
  Params_t *Params = NULL;
  if(type == AGGREG_200MS) Params = &pAggreg200MS;
  if(type == AGGREG_3S)    Params = &pAggreg3S;
  if(type == AGGREG_1M)    Params = &pAggreg1M;
  if(type == AGGREG_10M)   Params = &pAggreg10M;
  if(type == AGGREG_15M)   Params = &pAggreg15M;
  if(type == AGGREG_1H)    Params = &pAggreg1H;
  parser.DefineVar("Frequency", &Params->Frequency);
  parser.DefineVar("Nominal_Voltage", &pNominalVoltage);
  parser.DefineVar("Nominal_Frequency", &pNominalFrequency);
  if(pVoltageChannels > 1) parser.DefineVar("Unbalance", &Params->Unbalance);
  AppendArray("Voltage", Params->Voltage, pVoltageChannels);
  AppendArray("Current", Params->Current, pCurrentChannels);
  AppendArray("ActivePower", Params->Active_Power, pVoltageChannels);
  AppendArray("ReactivePower", Params->Reactive_Power, pVoltageChannels);
  AppendArray("ApparentPower", Params->Apparent_Power, pVoltageChannels);
  AppendArray("PowerFactor", Params->Power_Factor, pVoltageChannels);
  AppendArray("VoltageTHD", Params->Voltage_THD, pVoltageChannels);
  AppendArray("CurrentTHD", Params->Current_THD, pCurrentChannels);
  if(pVoltageChannels > 1) AppendArray("VoltagePhase", Params->Voltage_Phase, pVoltageChannels);
  AppendArray("Phi", Params->Phi, pVoltageChannels);
  try {
    parser.SetExpr(expr);
  } catch(Parser::exception_type &e) {
    LogFile::Error("Alarm expresion evaluation failed on analyzer '%s'", pSerial.c_str());
  }
}
void AnalyzerSoft::Alarms_Func(Params_t *aggreg) {
  for(Alarm_t &Alarm : pAlarms) {
    if(Alarm.Aggreg != aggreg->Type) continue;
    uint8_t *Byte   = &Alarm.Results[Alarm.ResultPos / 8];
    uint8_t  Bit    = Alarm.ResultPos % 8;
    if((*Byte) & (1 << Bit)) Alarm.ResultCount--;
    *Byte &= ~(1 << Bit);
    try {
      if((Alarm.Evaluator.Eval() > 0.0) == true) {
        Alarm.ResultCount++;
        *Byte |= (1 << Bit);
      }
    } catch (Parser::exception_type &e) {
      LogFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", pSerial.c_str());
    }
    Alarm.ResultPos = (Alarm.ResultPos + 1) % Alarm.Window;
    if((Alarm.ResultCount == Alarm.Samples) && (Alarm.Triggered == false)) {
      Alarm.Triggered = true;
      LogFile::Debug("Alarm triggered on analyzer '%s'", pSerial.c_str());
      Database DB(pSerial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", true, " + DB.Bind(Alarm.Priority) + ")");
      if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
      Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
    } else if((Alarm.ResultCount == 0) && (Alarm.Triggered == true)) {
      Alarm.Triggered = false;
      LogFile::Debug("Alarm reseted on analyzer '%s'", pSerial.c_str());
      Database DB(pSerial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", false, " + DB.Bind(Alarm.Priority) + ")");
      if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
      Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
    }
  }
}
void AnalyzerSoftAlarms::Alarms_Func(AnalyzerSoft::Params_t *aggreg) {}*/
void AnalyzerSoftAlarms::Run(const AnalyzerSoftAggreg *params) {

}
void AnalyzerSoftAlarms::Wait() {

}
// ---------------------------------------------------------------------------------------------------------------------