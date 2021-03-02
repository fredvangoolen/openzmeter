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
AnalyzerSoftAlarm::AnalyzerSoftAlarm(const string &serial, float &voltage, float &freq, const json &config) {
  pSerial      = serial;
  pUID         = config["AlarmID"];
  pExpression  = config["Expression"];
  pName        = config["Name"];
  pPriority    = config["Priority"];
  pSamples     = config["Samples"];
  pWindow      = config["Window"];
  pAggreg      = config["Aggreg"];
  pEnabled     = config["Enabled"];
  pResultPos   = 0;
  pResultCount = 0;
  pTriggerTime = 0;
  pInitialized = false;
  pEvaluator   = Parser();
  pResults     = (uint8_t*)calloc(ceil(pWindow / 8), 1);
  pEvaluator.DefineVar("NominalVoltage", &voltage);
  pEvaluator.DefineVar("NominalFrequency", &freq);
}
AnalyzerSoftAlarm::~AnalyzerSoftAlarm() {
  if(pResults != NULL) free(pResults);
}
json AnalyzerSoftAlarm::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["AlarmID"]    = Tools::GetUUID();
  ConfigOUT["Name"]       = "No name";
  ConfigOUT["Priority"]   = 0;
  ConfigOUT["Expression"] = "";
  ConfigOUT["Aggreg"]     = "3S";
  ConfigOUT["Samples"]    = 10;
  ConfigOUT["Window"]     = 12;
  ConfigOUT["Enabled"]    = true;
  if((config.contains("AlarmID") == true) && (config["AlarmID"].is_string() == true) && (Tools::ValidUUID(config["AlarmID"]) == true)) ConfigOUT["AlarmID"] = config["AlarmID"];
  if((config.contains("Name") == true) && (config["Name"].is_string() == true)) ConfigOUT["Name"] = config["Name"];
  if((config.contains("Priority") == true) && (config["Priority"].is_number_unsigned() == false) && (config["Priority"] < 3)) ConfigOUT["Priority"] = config["Priority"];
  if((config.contains("Expression") == true) && (config["Expression"].is_string() == true)) ConfigOUT["Expression"] = config["Expression"];
  if((config.contains("Aggreg") == true) && (config["Aggreg"].is_string() == true) && ((config["Aggreg"] == "200MS") || (config["Aggreg"] == "3S") || (config["Aggreg"] == "1M") || (config["Aggreg"] == "10M") || (config["Aggreg"] == "15M") || (config["Aggreg"] == "1H"))) ConfigOUT["Aggreg"] = config["Aggreg"];
  if((config.contains("Samples") == true) && (config["Samples"].is_number_unsigned() == true) && (config["Samples"] <= 1000)) ConfigOUT["Samples"] = config["Samples"];
  if((config.contains("Window") == true) && (config["Window"].is_number_unsigned() == true) && (config["Window"] > ConfigOUT["Samples"])) ConfigOUT["Window"] = config["Window"];
  if((config.contains("Enabled") == true) && (config["Enabled"].is_boolean() == true)) ConfigOUT["Enabled"] = config["Enabled"];
  return ConfigOUT;
}
AnalyzerSoftAlarm::Alarm_Result AnalyzerSoftAlarm::PushAggreg(AnalyzerSoftAggreg &data) {
  if(pEnabled == false) return NONE;
  if(pInitialized == false) {
    data.muParserInit(pEvaluator, pExpression);
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
    LogFile::Debug("Alarm expresion evaluation failed on analyzer '%s' (%s)", pSerial.c_str(), e.GetMsg().c_str());
  }
  pResultPos = (pResultPos + 1) % pWindow;
  if((pResultCount == pSamples) && (pTriggerTime == 0)) {
    Database DB(pSerial);
    pTriggerTime = data.Timestamp();
    LogFile::Debug("Alarm triggered on analyzer '%s'", pSerial.c_str());
    DB.ExecSentenceNoResult("INSERT INTO alarms(id, trigger, release) VALUES(" + DB.Bind(pUID) + ", " + DB.Bind(pTriggerTime) + ", null)");
    return TRIGGERED;
  } else if((pResultCount == 0) && (pTriggerTime != 0)) {
    Database DB(pSerial);
    LogFile::Debug("Alarm reseted on analyzer '%s'", pSerial.c_str());
    DB.ExecSentenceNoResult("UPDATE alarms SET release = " + DB.Bind(data.Timestamp()) + " WHERE id = " + DB.Bind(pUID) + " AND trigger = " + DB.Bind(pTriggerTime));
    pTriggerTime = 0;
    return RELEASED;
  }
  return NONE;
}
string AnalyzerSoftAlarm::Name() const {
  return pName;
}
uint8_t AnalyzerSoftAlarm::Priority() const {
  return pPriority;
}
// ---------------------------------------------------------------------------------------------------------------------