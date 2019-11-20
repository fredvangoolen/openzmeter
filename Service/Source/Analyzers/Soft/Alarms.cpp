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
  Result["Variables"] = json::object();
  json Query  = nullptr;
  Result["NominalVoltage"] = pParent->pNominalVoltage;
  Result["NominalFreq"] = pParent->pNominalFrequency;
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
      uint8_t Phases = Query["Voltage"].size();
      for(uint8_t n = 0; n < Phases; n++) {
        float Tmp = 0.0;
        if(Query["Voltage"][n].is_number()) Tmp = Query["Voltage"][n];
        Result["Variables"][(Phases > 1) ? "Voltage_" + to_string(n + 1) : "Voltage"] = Tmp;
      }
    }  
    if(Query["Current"].is_array()) {
      for(uint8_t n = 0; n < Query["Current"].size(); n++) {
        float Tmp = 0.0;
        if(Query["Current"][n].is_number()) Tmp = Query["Current"][n];
        Result["Variables"]["Current_" + to_string(n + 1)] = Tmp;
      }
    }
    if(Query["Active_Power"].is_array()) {
      for(uint8_t n = 0; n < Query["Active_Power"].size(); n++) {
        float Tmp = 0.0;
        if(Query["Active_Power"][n].is_number()) Tmp = Query["Active_Power"][n];
        Result["Variables"]["Active_Power_" + to_string(n + 1)] = Tmp;
      }
    }
    if(Query["Reactive_Power"].is_array()) {
      for(uint8_t n = 0; n < Query["Reactive_Power"].size(); n++) {
        float Tmp = 0.0;
        if(Query["Reactive_Power"][n].is_number()) Tmp = Query["Reactive_Power"][n];
        Result["Variables"]["Reactive_Power_" + to_string(n + 1)] = Tmp;
      }
    }  
    if(Query["Frequency"].is_number()) {
      float Tmp = 0.0;
      if(Query["Frequency"].is_number()) Tmp = Query["Frequency"];
      Result["Variables"]["Frequency"] = Tmp;
    }
    try {
      Parser Exp;
      for(auto it = Result["Variables"].begin(); it != Result["Variables"].end(); ++it) Exp.DefineConst(it.key(), it.value());
      Exp.SetExpr(Expression);
      float Tmp = Exp.Eval();
      Result["Result"] = Tmp;
    } catch (Parser::exception_type &e) {
      Result["Error"] = e.GetMsg();
    }
  }
  req.FromJSON(Result);
}