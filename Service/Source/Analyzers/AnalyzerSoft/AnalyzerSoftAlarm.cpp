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
#include "AnalyzerSoftAlarm.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAlarm::AnalyzerSoftAlarm(const string &serial, const json &config, float &voltage, float &freq) {
  pSerial      = serial;
  pExpression  = config["Expression"];
  pName        = config["Name"];
  pPriority    = config["Priority"];
  pSamples     = config["Samples"];
  pWindow      = config["Window"];
  pAggreg      = config["Aggreg"];
  pResultPos   = 0;
  pResultCount = 0;
  pTriggerTime = 0;
  pInitialized = false;
  pEvaluator   = Parser();
  pResults     = (uint8_t*)calloc(ceil(pWindow / 8), 1);
  pEvaluator.DefineVar("Nominal_Voltage", &voltage);
  pEvaluator.DefineVar("Nominal_Frequency", &freq);
}
AnalyzerSoftAlarm::~AnalyzerSoftAlarm() {
  if(pResults != NULL) free(pResults);
}
json AnalyzerSoftAlarm::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "No name";
  ConfigOUT["Priority"] = 0;
  ConfigOUT["Expression"] = "";
  ConfigOUT["Aggreg"] = "3S";
  ConfigOUT["Samples"] = 10;
  ConfigOUT["Window"] = 12;
  if((config.contains("Name") == true) && (config["Name"].is_string() == true)) ConfigOUT["Name"] = config["Name"];
  if((config.contains("Priority") == true) && (config["Priority"].is_number_unsigned() == false) && (config["Priority"] < 3)) ConfigOUT["Priority"] = config["Priority"];
  if((config.contains("Expression") == true) && (config["Expression"].is_string() == true)) ConfigOUT["Expression"] = config["Expression"];
  if((config.contains("Aggreg") == true) && (config["Aggreg"].is_string() == true) && ((config["Aggreg"] == "200MS") || (config["Aggreg"] == "3S") || (config["Aggreg"] == "1M") || (config["Aggreg"] == "10M") || (config["Aggreg"] == "15M") || (config["Aggreg"] == "1H"))) ConfigOUT["Aggreg"] = config["Aggreg"];
  if((config.contains("Samples") == true) && (config["Samples"].is_number_unsigned() == true) && (config["Samples"] <= 1000)) ConfigOUT["Samples"] = config["Samples"];
  if((config.contains("Window") == true) && (config["Window"].is_number_unsigned() == true) && (config["Window"] > ConfigOUT["Samples"])) ConfigOUT["Window"] = config["Window"];
  return ConfigOUT;
}
void AnalyzerSoftAlarm::PushAggreg(const string &aggreg, AnalyzerSoftAggreg *data) {
  if(pAggreg != aggreg) return;
  if(pInitialized == false) {
    data->muParserInit(pEvaluator, pExpression);
    pInitialized = true;
  }
  uint8_t *Byte   = &pResults[pResultPos / 8];
  uint8_t  Bit    = pResultPos % 8;
  if((*Byte) & (1 << Bit)) pResultCount--;
  *Byte &= ~(1 << Bit);
  try {
    if((pEvaluator.Eval() > 0.0) == true) {
      pResultCount++;
      *Byte |= (1 << Bit);
    }
  } catch (Parser::exception_type &e) {
    LogFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", pSerial.c_str());
  }
  pResultPos = (pResultPos + 1) % pWindow;
  if((pResultCount == pSamples) && (pTriggerTime == 0)) {
    pTriggerTime = data->Timestamp();
    LogFile::Debug("Alarm triggered on analyzer '%s'", pSerial.c_str());
    Database DB(pSerial);
//    DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", true, " + DB.Bind(Alarm.Priority) + ")");
//    if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
//    Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
  } else if((pResultCount == 0) && (pTriggerTime != 0)) {
    pTriggerTime = 0;
    LogFile::Debug("Alarm reseted on analyzer '%s'", pSerial.c_str());
    Database DB(pSerial);
//    DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", false, " + DB.Bind(Alarm.Priority) + ")");
//    if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
//    Telegram::SendAlarm(pSerial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
  }
}
/*void AnalyzerSoftAlarm::PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered) {
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
}*/
// ---------------------------------------------------------------------------------------------------------------------