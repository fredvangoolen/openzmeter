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
#include "AnalyzerSoftRates.h"
#include "HTTPChunkedWriter.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRates::AnalyzerSoftRates(const string &serial) {
  pSerial = serial;
}
AnalyzerSoftRates::~AnalyzerSoftRates() {
}
void AnalyzerSoftRates::GetRates(const json &params, Response &response) {
  auto WriteFunc = [=](HTTPChunkedWriter *writer) -> bool {
    uint32_t Items = 0;
    writer->Write("[\n  ");
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT coalesce(to_json(array_agg(json_build_object('RateID', id, 'Name', name, 'Order', ordering, 'StartDate', startdate, 'EndDate', enddate))),'[]') AS result FROM rates");
    while(DB.FecthRow()) writer->Write(((Items++ != 0) ? ",\n  " : "") + DB.ResultString("result"));
    writer->Write("\n]", true);
    return false;
  };
  new HTTPChunkedWriter(response, "application/json", WriteFunc);
}
void AnalyzerSoftRates::AddRate(const json &params, Response &response) {

}
void AnalyzerSoftRates::GetRate(const json &params, Response &response) {
  if((params.contains("RateID") == false) || (params["RateID"].is_string() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["RateID"];
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('Name', name, 'Order', ordering, 'StartDate', startdate, 'EndDate', enddate, 'Template', template) AS result FROM rates WHERE id = " + DB.Bind(ID));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);  
}
void AnalyzerSoftRates::SetRate(const json &params, Response &response) {}
void AnalyzerSoftRates::DelRate(const json &params, Response &response) {
  if((params.contains("RateID") == false) || (params["RateID"].is_string() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string ID = params["RateID"];
  Database DB(pSerial);
  bool Result = DB.ExecSentenceNoResult("DELETE FROM rates WHERE id = " + DB.Bind(ID));
  HTTPServer::FillResponse(response, "", "application/json", Result ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftRates::CalcRate(const json &params, Response &response) {

  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Precise") == false) || (params["Precise"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Precise' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json     Result          = json::object();
  uint64_t To              = params["To"];
  uint64_t From            = params["From"];
  string   Aggreg          = params["Aggreg"];
  bool     SaveActive      = false;
  bool     SaveReactive    = false;
  bool     SaveExpense     = false;
  bool     SavePeriodName  = false;
  bool     SavePeriodColor = false;
  bool     SaveContracted  = false;
  bool     SaveBills       = false;
  bool     SaveEnergyCost  = false;
  bool     SavePowerCost   = false;
  bool     SaveSampleTime  = false;
  bool     SavePeriodTime  = false;
  float    AccActive       = 0.0;
  float    AccReactive     = 0.0;
  float    AccCost         = 0.0;
  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if(((To - From) / ((Aggreg == "1H") ? 3600000 : 900000)) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    if(params["Series"][i] == "ACTIVE_POWER") {
      Result["Active_Power"] = json::array();
      SaveActive = true;
    } else if(params["Series"][i] == "REACTIVE_POWER") {
      Result["Reactive_Power"] = json::array();
      SaveReactive = true;
    } else if(params["Series"][i] == "EXPENSE") {
      Result["Expense"] = json::array();
      SaveExpense = true;
    } else if(params["Series"][i] == "PERIOD_POWER_COST") {
      Result["Period_Power_Cost"] = json::array();
      SavePowerCost = true;
    } else if(params["Series"][i] == "PERIOD_ENERGY_COST") {
      Result["Period_Energy_Cost"] = json::array();
      SaveEnergyCost = true;
    } else if(params["Series"][i] == "PERIOD_NUMBER") {
      Result["Period_Number"] = json::array();
      SavePeriodName = true;
    } else if(params["Series"][i] == "PERIOD_COLOR") {
      Result["Period_Color"] = json::array();
      SavePeriodColor = true;
    } else if(params["Series"][i] == "PERIOD_POWER_CONTRACTED") {
      Result["Period_Power_Contracted"] = json::array();
      SaveContracted = true;
    } else if(params["Series"][i] == "BILLS") {
      Result["Bills"] = json::array();
      SaveBills = true;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  if((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg != "15M")) {
    Result["Period_Sampletime"] = json::array();
    SavePeriodTime = true;
  }
  if((SaveActive || SaveReactive || SaveExpense) || ((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg == "15M"))) {
    Result["Sampletime"] = json::array();
    SaveSampleTime = true;
  }
//  pAnalyzerLock.ReadLock();
//  RatesEval Eval(pRates, pTimeZone, From, To, params["Precise"]);
//  Eval.SetSampleCallback([&](const RatesEval::SampleOut_t &out) {
//    AccActive   += out.ActivePower;
//    AccReactive += out.ReactivePower;
//    AccCost     += out.Cost;
//    if(SavePeriodTime) Result["Period_Sampletime"].push_back(out.SampleTime);
//    if(SaveContracted) Result["Period_Power_Contracted"].push_back(out.ContractedPower);
//    if(SavePowerCost)  Result["Period_Power_Cost"].push_back(out.PowerCost);
//    if(SaveEnergyCost) Result["Period_Energy_Cost"].push_back(out.EnergyCost);
//    if(SavePeriodName) {
//      if(out.Period == UINT16_MAX) {
//        Result["Period_Number"].push_back(nullptr);
//      } else {
//        Result["Period_Number"].push_back(out.Period);
//      }
//    }
//    if(SavePeriodColor) {
//      if(out.Color.empty() == true) {
//        Result["Period_Color"].push_back(nullptr);
//      } else {
//        Result["Period_Color"].push_back(out.Color);
//      }
//    }
//    if(Aggreg == "1H") {
//      if((out.SampleTime % 3600000) != 3599999) return;
//      AccActive = AccActive / 4.0;
//      AccReactive = AccReactive / 4.0;
//    }
//    if(SaveSampleTime) Result["Sampletime"].push_back(out.SampleTime);
//    if(SaveActive)     Result["Active_Power"].push_back(AccActive);
//    if(SaveReactive)   Result["Reactive_Power"].push_back(AccReactive);
//    if(SaveExpense)    Result["Expense"].push_back(AccCost);
//    AccActive   = 0.0;
//    AccReactive = 0.0;
//    AccCost     = 0.0;
//  });
//  Eval.SetBillCallback([&](const json &out) {
//    if(SaveBills == true) Result["Bills"].push_back(out);
//  });
//  Database DB(pSerial);
//  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime >= " + DB.Bind(Eval.StartDate()) + " AND sampletime <= " + DB.Bind(Eval.EndDate()) + " ORDER BY sampletime ASC");
//  while(DB.FecthRow() == true) Eval.PushSample(DB.ResultUInt64("sampletime"), DB.ResultFloat("active_power"), DB.ResultFloat("reactive_power"));
//  Eval.Finish();
//  pAnalyzerLock.Unlock();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);

}

// ---------------------------------------------------------------------------------------------------------------------