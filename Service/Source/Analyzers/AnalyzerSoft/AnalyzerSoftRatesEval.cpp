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
#include "HTTPChunkedWriter.h"
#include "AnalyzerSoftRatesEval.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRatesEval::AnalyzerSoftRatesEval(const string &serial, const time_zone &timezone) {
  pSerial   = serial;
  pTimeZone = timezone;
  pRateDB   = new Database(serial);
  pDataDB   = new Database(serial);
}
AnalyzerSoftRatesEval::~AnalyzerSoftRatesEval() {
//  UnloadRate();
  delete pRateDB;
  delete pDataDB;
}
void AnalyzerSoftRatesEval::GetBills(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) return HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);

  //Tengo que buscar el momento inicial de facturacion. El final se hace conforme avance

/*  auto WriteFunc = [=](HTTPChunkedWriter *writer) -> bool {
    uint32_t Items = 0;
    writer->Write("[\n  ");
    Database DB(pSerial);
//    DB.ExecSentenceAsyncResult("SELECT json_build_object('RateID', id, 'Name', name, 'Group', ordering, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) AS result FROM rates");
//    while(DB.FecthRow()) writer->Write(((Items++ != 0) ? ",\n  " : "") + DB.ResultString("result"));
    writer->Write("\n]", true);
    return false;
  };
  new HTTPChunkedWriter(response, "application/json", WriteFunc);


*/


}
void AnalyzerSoftRatesEval::GetCosts(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Precise") == false) || (params["Precise"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Precise' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("Group") == false) || (params["Group"].is_number_integer() == false) || (params["Group"] < 0) || (params["Group"] > 10)) {
//    HTTPServer::FillResponse(response, "'Group' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//    return;
//  }
  json     Result  = json::object();
  uint64_t To      = params["To"];
  uint64_t From    = params["From"];
  string   Aggreg  = params["Aggreg"];
  uint8_t  Group   = 0;//params["Group"];
  bool     Precise = true;
//  pFrom            = FindStart(From, Precise);
//  pTo              = FindEnd(To, Precise);
  pCurrent         = pFrom;
  pPriceModule     = NULL;
  pNowDate         = Tools::GetTime64();
  pDataDB->ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime >= " + pDataDB->Bind(pFrom) + " AND sampletime <= " + pDataDB->Bind(pTo) + " ORDER BY sampletime ASC");
  while(pDataDB->FecthRow() == true) {
    uint64_t Time = (pDataDB->ResultUInt64("sampletime") / 900000) * 900000 + 899999;
//    while(pCurrent < Time) HandleSample(0.0, 0.0);
//    HandleSample(pDataDB->ResultFloat("active_power"), pDataDB->ResultFloat("reactive_power"));
  }
//  Finish();
}


//  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
//  pRatePtr     = pRates.begin();

//  bool     SaveActive      = false;
//  bool     SaveReactive    = false;
//  bool     SaveExpense     = false;
//  bool     SavePeriodName  = false;
//  bool     SavePeriodColor = false;
//  bool     SaveContracted  = false;
//  bool     SaveBills       = false;
//  bool     SaveEnergyCost  = false;
//  bool     SavePowerCost   = false;
//  bool     SaveSampleTime  = false;
//  bool     SavePeriodTime  = false;
//  float    AccActive       = 0.0;
//  float    AccReactive     = 0.0;
//  float    AccCost         = 0.0;
//  for(uint8_t i = 0; i < params["Series"].size(); i++) {
//    if(params["Series"][i].is_string() == false) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
//    if(params["Series"][i] == "ACTIVE_POWER") {
//      Result["Active_Power"] = json::array();
//      SaveActive = true;
//    } else if(params["Series"][i] == "REACTIVE_POWER") {
//      Result["Reactive_Power"] = json::array();
//      SaveReactive = true;
//    } else if(params["Series"][i] == "EXPENSE") {
//      Result["Expense"] = json::array();
//      SaveExpense = true;
//    } else if(params["Series"][i] == "PERIOD_POWER_COST") {
//      Result["Period_Power_Cost"] = json::array();
//      SavePowerCost = true;
//    } else if(params["Series"][i] == "PERIOD_ENERGY_COST") {
//      Result["Period_Energy_Cost"] = json::array();
//      SaveEnergyCost = true;
//    } else if(params["Series"][i] == "PERIOD_NUMBER") {
//      Result["Period_Number"] = json::array();
//      SavePeriodName = true;
//    } else if(params["Series"][i] == "PERIOD_COLOR") {
//      Result["Period_Color"] = json::array();
//      SavePeriodColor = true;
//    } else if(params["Series"][i] == "PERIOD_POWER_CONTRACTED") {
//      Result["Period_Power_Contracted"] = json::array();
//      SaveContracted = true;
//    } else if(params["Series"][i] == "BILLS") {
//      Result["Bills"] = json::array();
//      SaveBills = true;
//    } else {
//      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
//    }
//  }
//  if((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg != "15M")) {
//    Result["Period_Sampletime"] = json::array();
//    SavePeriodTime = true;
//  }
//  if((SaveActive || SaveReactive || SaveExpense) || ((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg == "15M"))) {
//    Result["Sampletime"] = json::array();
//    SaveSampleTime = true;
//  }








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




  //const json &t, const time_zone &timezone, const uint64_t from, const uint64_t to, const bool precise


/*

json RatesEval::CheckConfig(const json &r) {
  json ConfigOUT = json::array();
  if(r.is_array() == false) return "Rate list is not an array";
  for(uint16_t R = 0; R < r.size(); R++) {
    json Tmp = Rate::CheckConfig(r[R], true);
    if(Tmp.is_string() == true) return Tmp;
    ConfigOUT.push_back(Tmp);
  }
  return ConfigOUT;
}




uint64_t RatesEval::FindStart(const uint64_t from, bool precise) {


  uint64_t CalcFrom = (From / 3600000) * 3600000 + 899999;
  Rate *CurrentRate = NULL;
  vector<uint64_t>::iterator  BillPtr;
  if(Precise == true) {
    RateDB.ExecSentenceAsyncResult("SELECT jsonb_build_object('Name', name, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) || template::jsonb AS result FROM rates WHERE ordering = " + RateDB.Bind(Group) + " AND " +
                                   "enddate >= " + RateDB.Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(From)), utc_time_zone())) + " " +
                                   "startdate <= " + RateDB.Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(To)), utc_time_zone())) + " " +
                                   "ORDER BY startdate ASC");
    while(RateDB.FecthRow()) {
      CurrentRate = new Rate(RateDB.ResultJSON("result"), pTimeZone);
      if(CurrentRate->EndDate > CalcFrom) {
        delete CurrentRate;
        CurrentRate = NULL;
        break;
      }
      CalcFrom = CurrentRate->StartDate + 899999;
      for(BillPtr = CurrentRate->BillingCycles.begin(); BillPtr != CurrentRate->BillingCycles.end(); BillPtr++) {
        if(*BillPtr > CalcFrom) break;
        CalcFrom = *BillPtr + 900000;
      }
      if(BillPtr != CurrentRate->BillingCycles.end()) break;
    }
  }






  uint64_t Time = (from / 3600000) * 3600000 + 899999;
  if(precise == false) return Time;
  for(uint16_t R = 0; R < pRates.size(); R++) {
    if((Time < pRates[R].StartDate) || (Time > pRates[R].EndDate)) continue;
    Time = pRates[R].StartDate + 899999;
    for(uint16_t B = 0; B < pRates[R].BillingCycles.size(); B++) {
      if(pRates[R].BillingCycles[B] > Time) return Time;
      Time = pRates[R].BillingCycles[B] + 900000;
    }
  }
  return Time;
}
uint64_t RatesEval::FindEnd(const uint64_t to, bool precise) {
  uint64_t Time = (to / 3600000) * 3600000 + 899999;
  if(precise == false) return Time;
  for(uint16_t R = 0; R < pRates.size(); R++) {
    if((Time < pRates[R].StartDate) || (Time > pRates[R].EndDate)) continue;
    Time = pRates[R].EndDate;
    for(uint16_t B = pRates[R].BillingCycles.size(); B > 0; B--) {
      if(pRates[R].BillingCycles[B - 1] < Time) return Time;
      Time = pRates[R].BillingCycles[B - 1];
    }
  }
  return Time;
}
float *RatesEval::VariableFactory(const char_type *name, void *params) {
  map<string, float> *Tmp = (map<string, float>*)params;
  return &Tmp->emplace(name, 0.0).first->second;
};
void RatesEval::LoadRate() {
  if(pPeriods.size() > 0) return;
  pYearPtr = pRatePtr->Years.begin();
  pBillPtr = pRatePtr->BillingCycles.begin();
  pLoadedPeriodDate = pRatePtr->StartDate;
  while((pBillPtr != pRatePtr->BillingCycles.end()) && (*pBillPtr < pCurrent)) {
    pLoadedPeriodDate = *pBillPtr;
    pBillPtr++;
  }
  pPeriods.resize(pRatePtr->Periods.size());
  pScript.SetVarFactory(VariableFactory, &pVariables);
  for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) {
    pPeriods[P].Script.SetVarFactory(VariableFactory, &pPeriods[P].Variables);
    pPeriods[P].Script.DefineVar("ContractedPower",   &pSampleOut.ContractedPower);
    pPeriods[P].Script.DefineVar("PowerCost",         &pSampleOut.PowerCost);
    pPeriods[P].Script.DefineVar("EnergyCost",        &pSampleOut.EnergyCost);
    pPeriods[P].Script.DefineVar("ActivePower",       &pSampleOut.ActivePower);
    pPeriods[P].Script.DefineVar("ActiveEnergy",      &pSampleOut.ActiveEnergy);
    pPeriods[P].Script.DefineVar("ReactivePower",     &pSampleOut.ReactivePower);
    pPeriods[P].Script.DefineVar("ReactiveEnergy",    &pSampleOut.ReactiveEnergy);
    pPeriods[P].Script.SetExpr(pRatePtr->Periods[P].Script);
    pScript.DefineVar("ContractedPower_P" + to_string(P + 1),  &pRatePtr->Contracted[P]);
    pScript.DefineVar("EnergyCost_P" + to_string(P + 1),       (pRatePtr->EnergyCost.size() == 0) ? &pSampleOut.EnergyCost : &pRatePtr->EnergyCost[P]);
    pScript.DefineVar("PowerCost_P" + to_string(P + 1),        (pRatePtr->PowerCost.size() == 0)  ? &pSampleOut.PowerCost  : &pRatePtr->PowerCost[P]);
  }
  pScript.DefineVar("Period",         &pSampleOut.PeriodF);
  pScript.DefineVar("ActivePower",    &pSampleOut.ActivePower);
  pScript.DefineVar("ActiveEnergy",   &pSampleOut.ActiveEnergy);
  pScript.DefineVar("ReactivePower",  &pSampleOut.ReactivePower);
  pScript.DefineVar("ReactiveEnergy", &pSampleOut.ReactiveEnergy);
  pScript.SetExpr(pRatePtr->Script);
  pPriceModule = PriceSource::CreateInstance(pRatePtr->pPriceSource);
  pPriceModule->PrepareRange(pCurrent, (pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate);
  pPriceModule->GetParams(0, pPriceVariables);
  if(pRatePtr->EnergyCost.size() == 0) {
    for(auto &Var : pPriceVariables) pEnergyScript.DefineVar(Var.first, &Var.second);
    pEnergyScript.DefineVar("Period", &pSampleOut.PeriodF);
    pEnergyScript.SetExpr(pRatePtr->EnergyCostScript);
  }
  if(pRatePtr->PowerCost.size() == 0) {
    for(auto &Var : pPriceVariables) pPowerScript.DefineVar(Var.first, &Var.second);
    pPowerScript.DefineVar("Period", &pSampleOut.PeriodF);
    pPowerScript.SetExpr(pRatePtr->PowerCostScript);
  }
}
void RatesEval::UnloadRate() {
  if(pPeriods.size() == 0) return;
  if(pPriceModule != NULL) {
    delete pPriceModule;
    pPriceModule = NULL;
  }
  pScript = Parser();
  pVariables.clear();
  pPeriods.clear();
}
void RatesEval::HandleSample(const float active, const float reactive) {
  while(pRatePtr != pRates.end()) {
    if(pCurrent < pRatePtr->StartDate) break;
    if((pCurrent >= pRatePtr->StartDate) && (pCurrent <= pRatePtr->EndDate)) {
      LoadRate();
      break;
    }
    pRatePtr++;
  }
  civil_second Time = convert(chrono::system_clock::from_time_t(pCurrent / 1000), pTimeZone);
  if(pPeriods.size() > 0) {
    while(Time.year() > pYearPtr->Year) pYearPtr++;
    pSampleOut.Period  = pRatePtr->Days[pYearPtr->Days[get_yearday(civil_day(Time)) - 1]].Quarters[Time.hour() * 4 + Time.minute() / 15];
    pSampleOut.PeriodF = (pSampleOut.Period + 1.0);
    if((pRatePtr->EnergyCost.size() == 0) || (pRatePtr->PowerCost.size() == 0)) pPriceModule->GetParams(pCurrent, pPriceVariables);
    pSampleOut.SampleTime      = pCurrent;
    pSampleOut.ActivePower     = active;
    pSampleOut.ActiveEnergy    = active / 4.0;
    pSampleOut.ReactivePower   = reactive;
    pSampleOut.ReactiveEnergy  = reactive / 4.0;
    pSampleOut.Color           = pRatePtr->Periods[pSampleOut.Period].Color;
    pSampleOut.ContractedPower = pRatePtr->Contracted[pSampleOut.Period];
    pSampleOut.EnergyCost      = (pRatePtr->EnergyCost.size() == 0) ? pEnergyScript.Eval() : pRatePtr->EnergyCost[pSampleOut.Period];
    pSampleOut.PowerCost       = (pRatePtr->PowerCost.size() == 0)  ? pPowerScript.Eval()  : pRatePtr->PowerCost[pSampleOut.Period];
    pScript.Eval();
    pPeriods[pSampleOut.Period].Script.Eval();
    pSampleOut.Cost = pVariables["Cost"] + pPeriods[pSampleOut.Period].Variables["Cost"];
    if(pSampleCallback) pSampleCallback(pSampleOut);
    if(pCurrent == ((pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate)) {
      if(pBillCallback) pBillCallback(BuildBill(true));
      UnloadRate();
    }
  } else if(pSampleCallback) {
    pSampleOut.Period = UINT16_MAX;
    pSampleOut.PeriodF = NAN;
    pSampleOut.SampleTime = pCurrent;
    pSampleOut.ActivePower = active;
    pSampleOut.ActiveEnergy = active / 4.0;
    pSampleOut.ReactivePower = reactive;
    pSampleOut.ReactiveEnergy = reactive / 4.0;
    pSampleOut.Color = "";
    pSampleOut.ContractedPower = NAN;
    pSampleOut.EnergyCost = NAN;
    pSampleOut.PowerCost = NAN;
    pSampleOut.Cost = NAN;
    pSampleCallback(pSampleOut);
  }
  pCurrent += 900000;
}
json RatesEval::BuildBill(bool complete) {
  float EnergyTerm      = 0.0;
  float PowerTerm       = 0.0;
  float GeneralTerm     = 0.0;
  json Result           = json::object();
  Result["From"]        = pLoadedPeriodDate;
  Result["To"]          = pCurrent;
  Result["Currency"]    = pRatePtr->Currency;
  Result["PowerUnits"]  = pRatePtr->PowerUnits;
  Result["EnergyUnits"] = pRatePtr->EnergyUnits;
  Result["Name"]        = pRatePtr->Name;
  Result["Description"] = pRatePtr->Description;
  Result["Periods"]     = json::array();
  Result["Complete"]    = (pNowDate > pCurrent) && complete;
  for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) {
    Result["Periods"].push_back(json::array());
    for(uint16_t C = 0; C < pRatePtr->Periods[P].Concepts.size(); C++) {
      Result["Periods"][P].push_back(json::object());
      Result["Periods"][P][C]["Description"] = pRatePtr->Periods[P].Concepts[C].Description;
      if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST_POWER";
        PowerTerm += Cost;
      } else {
        Result["Periods"][P][C]["Type"] = "INFO";
        Result["Periods"][P][C]["Value"] = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
      }
    }
  }
  Result["General"] = json::array();
  for(uint16_t C = 0; C < pRatePtr->Concepts.size(); C++) {
    Result["General"].push_back(json::object());
    Result["General"][C]["Description"] = pRatePtr->Concepts[C].Description;
    if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST_POWER";
      PowerTerm += Cost;
    } else {
      Result["General"][C]["Type"] = "INFO";
      Result["General"][C]["Value"] = pVariables[pRatePtr->Concepts[C].Variable];
    }
  }
  float Total = EnergyTerm + PowerTerm + GeneralTerm;
  Result["Taxes"] = json::array();
  for(uint16_t T = 0; T < pRatePtr->Taxs.size(); T++) {
    json Tax = json::object();
    if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_FIXED) {
      Tax["Type"] = "FIXED_QUANTITY";
      Tax["Value"] = pRatePtr->Taxs[T].Value;
      GeneralTerm += pRatePtr->Taxs[T].Value;
      Total += pRatePtr->Taxs[T].Value;
    } else {
      continue;
    }
    Tax["Description"] = pRatePtr->Taxs[T].Description;
    Result["Taxes"].push_back(Tax);
  }
  float Tmp = 0.0;
  for(uint16_t T = 0; T < pRatePtr->Taxs.size(); T++) {
    if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT) {
      json Tax = json::object();
      Tax["Description"] = pRatePtr->Taxs[T].Description;
      Tax["Type"] = "PERCENT_TOTAL";
      Tax["Value"] = Total * (pRatePtr->Taxs[T].Value / 100.0);
      Result["Taxes"].push_back(Tax);
      Tmp += Total * (pRatePtr->Taxs[T].Value / 100.0);
    }
  }
  Result["Total"] = Total + Tmp;
  return Result;
}
void RatesEval::Finish() {
  while(pCurrent < pTo) HandleSample(0.0, 0.0);
  if(pPeriods.size() > 0) {
    if(pBillCallback) pBillCallback(BuildBill(true));
    UnloadRate();
  }
}
//void RatesEval::SetPeriodCallback(function<void(const PeriodOut_t &)> periodOut) {
//  pPeriodCallback = periodOut;
//}
void RatesEval::SetSampleCallback(function<void(const SampleOut_t &)> sampleOut) {
  pSampleCallback = sampleOut;
}
void RatesEval::SetBillCallback(function<void(const json &)> billOut) {
  pBillCallback = billOut;
}
*/
// ---------------------------------------------------------------------------------------------------------------------