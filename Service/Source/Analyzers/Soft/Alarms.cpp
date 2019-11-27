/* ================================== zMeter ===========================================================================
 * File:     Alarms.cpp
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Alarms.h"
AnalyzerSoft::Alarms::Alarms(AnalyzerSoft *parent) {
  pParent = parent;
}
AnalyzerSoft::Alarms::~Alarms() {
}
void AnalyzerSoft::Alarms::TestAlarm(HTTPRequest &req) {
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  json Result = json::object();
  Result["Variables"] = json::array();
  json Query  = nullptr;
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
  string   Aggreg     = Params["Aggreg"];
  string   Expression = Params["Expression"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  if(Aggreg == "3S") {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag, 'Active_Power', active_power, 'Reactive_Power', reactive_power) as result FROM aggreg_3s ORDER BY sampletime DESC LIMIT 1");
    Result["Aggregation_Source"] = "3S";
    if(req.FecthRow() == true) Query = req.ResultJSON("result");
  } else if(Aggreg == "1M") {
    req.ExecSentenceAsyncResult("SELECT * FROM aggreg_3s ORDER BY sampletime ASC LIMIT 1");
  }
  if(Query != nullptr) {
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
    try {
      Parser Exp;
      for(auto &Var : Result["Variables"]) Exp.DefineConst(Var["Name"], Var["Value"]);
      Exp.SetExpr(Expression);
      float Tmp = Exp.Eval();
      Result["Result"] = Tmp;
    } catch (Parser::exception_type &e) {
      Result["Error"] = e.GetMsg();
    }
  }
  req.FromJSON(Result);
}
void AnalyzerSoft::Alarms::GetAlarms(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT TO_JSON(COALESCE(ARRAY_AGG(JSON_BUILD_OBJECT('AlarmID', alarm, 'UserName', username, 'Name', name, 'Priority', priority)), '{}')) AS result FROM alarms");  
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("result"), MHD_HTTP_OK);
}
void AnalyzerSoft::Alarms::GetAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["AlarmID"].is_number_unsigned()) return req.FromString("'AlarmID' params required", MHD_HTTP_BAD_REQUEST);
  uint32_t AlarmID = Params["AlarmID"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT JSON_BUILD_OBJECT('AlarmID', alarm, 'UserName', username, 'Name', name, 'Priority', priority, 'Expression', formula, 'Description', description, 'Aggregation', aggregation, 'Message', message, 'Samples', samples, 'Window', window_length) AS result FROM alarms WHERE alarm = " + req.Bind(AlarmID));
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void AnalyzerSoft::Alarms::SetAlarm(HTTPRequest &req) {}
void AnalyzerSoft::Alarms::AddAlarm(HTTPRequest &req) {}
void AnalyzerSoft::Alarms::DelAlarm(HTTPRequest &req) {}