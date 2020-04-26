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
#include <string.h>
#include <errno.h>
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
Tariff::Tariff(const json &t) {
  if((t.contains("Description") == false) || (t["Description"].is_string() == false)) {
    pParseError = "'Description' param is required";
    return;
  }
  pDescription = t["Description"];
  if((t.contains("Currency") == false) || (t["Currency"].is_string() == false)) {
    pParseError = "'Currency' param is required";
    return;
  }
  pCurrency = t["Currency"];
  if((t.contains("PowerUnits") == false) || (t["PowerUnits"].is_string() == false)) {
    pParseError = "'PowerUnits' param is required";
    return;
  }
  pPowerUnits = t["PowerUnits"];
  if((t.contains("EnergyUnits") == false) || (t["EnergyUnits"].is_string() == false)) {
    pParseError = "'EnergyUnits' param is required";
    return;
  }
  pEnergyUnits = t["EnergyUnits"];
  if((t.contains("Periods") == false) || (t["Periods"].is_array() == false) || (t["Periods"].size() ==0)) {
    pParseError = "'Periods' param is required";
    return;
  }
  pPeriods.resize(t["Periods"].size());
  for(uint8_t P = 0; P < t["Periods"].size(); P++) {
    if((t["Periods"][P].contains("Color") == false) || (t["Periods"][P]["Color"].is_string() == false)) {
      pParseError = "'Periods/P" + to_string(P + 1) + "/Color' param is required";
      return;
    }
    pPeriods[P].Color = t["Periods"][P]["Color"];
    if((t["Periods"][P].contains("DefaultPowerCost") == false) || (t["Periods"][P]["DefaultPowerCost"].is_number() == false)) {
      pParseError = "'Periods/P" + to_string(P + 1) + "/DefaultPowerCost' param is required";
      return;
    }
    pPeriods[P].DefaultPowerCost = t["Periods"][P]["DefaultPowerCost"];
    if((t["Periods"][P].contains("DefaultEnergyCost") == false) || (t["Periods"][P]["DefaultEnergyCost"].is_number() == false)) {
      pParseError = "'Periods/P" + to_string(P + 1) + "/DefaultEnergyCost' param is required";
      return;
    }
    pPeriods[P].DefaultEnergyCost = t["Periods"][P]["DefaultEnergyCost"];
    if((t["Periods"][P].contains("Name") == false) || (t["Periods"][P]["Name"].is_string() == false)) {
      pParseError = "'Periods/P" + to_string(P + 1) + "/Name' param is required";
      return;
    }
    pPeriods[P].Name = t["Periods"][P]["Name"];
  }
  if((t.contains("Script") == false) || (t["Script"].is_string() == false)) {
    pParseError = "'Script' param is required";
    return;
  }
  pScript.SetVarFactory(VariableFactory, &pVariables);
  try {
    pScript.DefineVar("Contracted_Power", &pEvalParams.Contracted_Power);
    pScript.DefineVar("Power_Cost",       &pEvalParams.Power_Cost);
    pScript.DefineVar("Energy_Cost",      &pEvalParams.Energy_Cost);
    pScript.DefineVar("Active_Power",     &pEvalParams.Active_Power);
    pScript.DefineVar("Active_Energy",    &pEvalParams.Active_Energy);
    pScript.DefineVar("Reactive_Power",   &pEvalParams.Reactive_Power);
    pScript.DefineVar("Reactive_Energy",  &pEvalParams.Reactive_Energy);
    ///TODO METER VARIABLES DE LOS PERIODOS
    pScript.SetExpr(t["Script"]);
    pScript.Eval();
  } catch(Parser::exception_type &e) {
    pParseError = "'Script' parse failed (" + e.GetMsg() + ")";
    return;
  }
  if((t.contains("Concepts") == false) || (t["Concepts"].is_array() == false)) {
    pParseError = "'Concepts' param is required";
    return;
  }
  pConcepts.resize(t["Concepts"].size());
  for(uint8_t C = 0; C < t["Concepts"].size(); C++) {
    if((t["Concepts"][C].contains("Description") == false) || (t["Concepts"][C]["Description"].is_string() == false)) {
      pParseError = "'Concepts/" + to_string(C) + "/Description' param is required";
      return;
    }
    pConcepts[C].Description = t["Concepts"][C]["Description"];
    if((t["Concepts"][C].contains("Variable") == false) || (t["Concepts"][C]["Variable"].is_string() == false)) {
      pParseError = "'Concepts/" + to_string(C) + "/Variable' param is required";
      return;
    }
    pConcepts[C].Variable = t["Concepts"][C]["Variable"];
    if((t["Concepts"][C].contains("Type") == false) || (t["Concepts"][C]["Type"].is_string() == false) || ((t["Concepts"][C]["Type"] != "INFO") && (t["Concepts"][C]["Type"] != "INFO_POWER") && (t["Concepts"][C]["Type"] != "INFO_ENERGY") && (t["Concepts"][C]["Type"] != "COST_POWER") && (t["Concepts"][C]["Type"] != "COST_ENERGY") && (t["Concepts"][C]["Type"] != "COST"))) {
      pParseError = "'Concepts/" + to_string(C) + "/Type' param is required";
      return;
    }
    pConcepts[C].Type = (t["Concepts"][C]["Type"] == "INFO") ? CONCEPT_INFO : (t["Concepts"][C]["Type"] == "INFO_POWER") ? CONCEPT_INFO_POWER : (t["Concepts"][C]["Type"] == "INFO_ENERGY") ? CONCEPT_INFO_ENERGY : (t["Concepts"][C]["Type"] == "COST_POWER") ? CONCEPT_COST_POWER : (t["Concepts"][C]["Type"] == "COST_ENERGY") ? CONCEPT_COST_ENERGY : CONCEPT_COST;
    if(pVariables.find(pConcepts[C].Variable) == pVariables.end()) {
      pParseError = "'" + pConcepts[C].Variable + "' is not defined in 'Script'";
      return;
    }
  }
  if((t.contains("Days") == false) || (t["Days"].is_array() == false) || (t["Days"].size() == 0)) {
    pParseError = "'Days' param is required";
    return;
  }
  pDays.resize(t["Days"].size());
  for(uint8_t D = 0; D < t["Days"].size(); D++) {
    if((t["Days"][D].contains("Name") == false) || (t["Days"][D]["Name"].is_string() == false)) {
      pParseError = "'Days/D" + to_string(D + 1) + "/Name' param is required";
      return;
    }
    pDays[D].Name = t["Days"][D]["Name"];
    if((t["Days"][D].contains("Color") == false) || (t["Days"][D]["Color"].is_string() == false)) {
      pParseError = "'Days/D" + to_string(D + 1) + "/Color' param is required";
      return;
    }
    pDays[D].Color = t["Days"][D]["Color"];
    if((t["Days"][D].contains("Quarters") == false) || (t["Days"][D]["Quarters"].is_array() == false) || (t["Days"][D]["Quarters"].size() != 96)) {
      pParseError = "'Days/D" + to_string(D + 1) + "/Quarters' param is required";
      return;
    };
    for(uint8_t Q = 0; Q < 96; Q++) {
      if((t["Days"][D]["Quarters"][Q].is_number_unsigned() == false) || (t["Days"][D]["Quarters"][Q] >= pPeriods.size())) {
        pParseError = "'Days/D" + to_string(D + 1) + "/Quarters/" + to_string(Q) + "' param is required";
        return;
      };
      pDays[D].Quarters[Q] = t["Days"][D]["Quarters"][Q];
    }
  }
  if((t.contains("Years") == false) || (t["Years"].is_array() == false) || (t["Years"].size() == 0)) {
    pParseError = "'Years' param is required";
    return;
  }
  for(uint8_t Y = 0; Y < t["Years"].size(); Y++) {
    if((t["Years"][Y].contains("Year") == false) || (t["Years"][Y]["Year"].is_number_integer() == false) || (t["Years"][Y]["Year"] < 2000) || (t["Years"][Y]["Year"] > 2100)) {
      pParseError = "'Years/" + to_string(Y) + "/Year' param is required";
      return;
    }
    uint16_t Year = t["Years"][Y]["Year"];
    uint16_t YearDays = get_yearday(civil_day(Year, 12, 31));
    pYears.emplace(Year, vector<uint8_t>(YearDays, 0));
    if((t["Years"][Y].contains("Days") == false) || (t["Years"][Y]["Days"].is_array() == false) || (t["Years"][Y]["Days"].size() != YearDays)) {
      pParseError = "'Years/" + to_string(Y) + "/Days' param is required";
      return;
    }
    for(uint16_t D = 0; D < YearDays; D++) {
      if((t["Years"][Y]["Days"][D].is_number_unsigned() == false) || (t["Years"][Y]["Days"][D] >= pDays.size())) {
        pParseError = "'Years/" + to_string(Y) + "/Days/" + to_string(D) + "' param is required";
        return;
      };
      pYears[Year][D] = t["Years"][Y]["Days"][D];
    }
  }
}
Tariff::~Tariff() {
//  for(uint32_t n = 0; n < pPeriods.size(); n++) {
//    delete pPeriods[n].PowerModule;
//    delete pPeriods[n].EnergyModule;
//  }
}
string Tariff::ParseError() {
  return pParseError;
}
bool Tariff::IsValid() {
  return pParseError.empty();
}
json Tariff::GetTemplate() {
  json T;
  T["Description"] = pDescription;
  T["Periods"] = json::array();
  T["EnergyUnits"] = pEnergyUnits;
  T["PowerUnits"] = pPowerUnits;
  T["Currency"] = pCurrency;
  T["Script"] = Tools::StrTrim(pScript.GetExpr());
  for(uint8_t P = 0; P < pPeriods.size(); P++) {
    T["Periods"].push_back(json::object());
    T["Periods"][P]["Color"] = pPeriods[P].Color;
    T["Periods"][P]["DefaultPowerCost"] = pPeriods[P].DefaultPowerCost;
    T["Periods"][P]["DefaultEnergyCost"] = pPeriods[P].DefaultEnergyCost;
    T["Periods"][P]["Name"] = pPeriods[P].Name;
  }
  T["Concepts"] = json::array();
  for(uint8_t C = 0; C < pConcepts.size(); C++) {
    T["Concepts"].push_back(json::object());
    T["Concepts"][C]["Description"] = pConcepts[C].Description;
    T["Concepts"][C]["Variable"] = pConcepts[C].Variable;
    T["Concepts"][C]["Type"] = (pConcepts[C].Type == CONCEPT_INFO) ? "INFO" : (pConcepts[C].Type == CONCEPT_INFO_POWER) ? "INFO_POWER" : (pConcepts[C].Type == CONCEPT_INFO_ENERGY) ? "INFO_ENERGY" : (pConcepts[C].Type == CONCEPT_COST_POWER) ? "COST_POWER" : (pConcepts[C].Type == CONCEPT_COST_ENERGY) ? "COST_ENERGY" : "COST";
  }
  T["Days"] = json::array();
  for(uint8_t D = 0; D < pDays.size(); D++) {
    T["Days"].push_back(json::object());
    T["Days"][D]["Color"] = pDays[D].Color;
    T["Days"][D]["Name"] = pDays[D].Name;
    T["Days"][D]["Quarters"] = json::array();
    for(uint8_t Q = 0; Q < 96; Q++) T["Days"][D]["Quarters"].push_back(pDays[D].Quarters[Q]);
  }
  T["Years"] = json::array();
  for(auto Y : pYears) {
    T["Years"].push_back(json::object());
    T["Years"].back()["Year"] = Y.first;
    T["Years"].back()["Days"] = Y.second;
  }
  return T;
}















float *Tariff::VariableFactory(const char_type *name, void *params) {
  map<string, float> *Tmp = (map<string, float>*)params;
  Tmp->emplace(name, 0.0);
  return &Tmp->find(name)->second;
}
//string Tariff::SetTemplate(const json &t) {






//  if(pSeasons.back().EndDate < pEndDate) return false;
//  if(!t["Holidays"].is_array()) return false;
//  for(uint32_t n = 0; n < t["Holidays"].size(); n++) {
//    Holiday_t Holiday;
//    if(!t["Holidays"][n]["Name"].is_string()) return false;
//    Holiday.Name = t["Holidays"][n]["Name"];
//    if(!t["Holidays"][n]["StartDate"].is_number_unsigned()) return false;
//    Holiday.StartDate = t["Holidays"][n]["StartDate"];
//    if(Holiday.StartDate < pStartDate) return "3";
//    if(!t["Holidays"][n]["EndDate"].is_number_unsigned()) return false;
//    Holiday.EndDate = t["Holidays"][n]["EndDate"];
//    if((Holiday.StartDate % 900000) != 0) return false;
//    if((Holiday.EndDate % 900000) != 899999) return false;
//    if((Holiday.EndDate <= Holiday.StartDate) || (Holiday.EndDate > pEndDate)) return false;
//    pHolidays.push_back(Holiday);
//  }

//  pDescription = t["Description"];
//  if(t["HolidayPeriod"].is_null()) {
//    pHolidayPeriod = UINT8_MAX;
//  } else if(t["HolidayPeriod"].is_number_unsigned()) {
//    pHolidayPeriod = t["HolidayPeriod"];
//    if((pHolidayPeriod < 0) || (pHolidayPeriod >= pPeriods.size())) return false;
//  } else {
//    return false;
//  }
//  if(t["WeekendPeriod"].is_null()) {
//    pWeekendPeriod = UINT8_MAX;
//  } else if(t["WeekendPeriod"].is_number_unsigned()) {
//    pWeekendPeriod = t["WeekendPeriod"];
//    if((pWeekendPeriod < 0) || (pWeekendPeriod >= pPeriods.size())) return false;
//  } else {
//    return false;
//  }
//  return "";
//}


uint8_t Tariff::FindPeriod(uint64_t timestamp) {
  uint8_t Period = UINT8_MAX;
//  if(pHolidayPeriod != UINT8_MAX) {
//    for(vector<Holiday_t>::iterator it = pHolidays.begin(); it != pHolidays.end(); it++) {
//      if((timestamp >= it->StartDate) && (timestamp <= it->EndDate)) return pHolidayPeriod;
//    }
//  }
//  for(uint8_t m = 0; m < pSeasons.size(); m++) {
//    if((pSeasons[m].StartDate >= timestamp) || (pSeasons[m].EndDate < timestamp)) continue;
//    cctz::civil_second Time = cctz::convert(chrono::system_clock::from_time_t(timestamp / 1000), pTimeZone);
//    if((pWeekendPeriod != UINT8_MAX) && ((get_weekday(civil_day(Time)) == weekday::saturday) || (get_weekday(civil_day(Time)) == weekday::sunday))) return pWeekendPeriod;
//    return pSeasons[m].DayPeriods[Time.hour()];
//  }
  return Period;
}
//string Tariff::SetRate(const json &t) {
//  if(pValidTimeZone == false) return false;
//  if(!t["Taxs"].is_array()) return false;
//  for(uint32_t n = 0; n < t["Taxs"].size(); n++) {
//    Tax_t Tax;
//    if(!t["Taxs"][n]["Name"].is_string()) return false;
//    Tax.Name = t["Taxs"][n]["Name"];
//    if(!t["Taxs"][n]["Value"].is_number()) return false;
//    Tax.Value = t["Taxs"][n]["Value"];
//    if(!t["Taxs"][n]["Type"].is_string()) return false;
//    Tax.Type = t["Taxs"][n]["Type"];
//    if((Tax.Type != "PERCENT_TOTAL") && (Tax.Type != "PERCENT_ENERGY") && (Tax.Type != "PERCENT_POWER") && (Tax.Type != "PERCENT_ENERGY_POWER") && (Tax.Type != "FIXED_QUANTITY")) return false;
//    pTaxs.push_back(Tax);
//  }
//  if(!t["Periods"].is_array()) return false;
  //This requiere a previous template is loaded with same periods number
//  if(pPeriods.size() != t["Periods"].size()) return false;
//  for(uint32_t n = 0; n < t["Periods"].size(); n++) {
//    if(!t["Periods"][n]["Power_Cost"].is_number()) return false;
//    pPeriods[n].PowerCost = t["Periods"][n]["Power_Cost"];
//    if(!t["Periods"][n]["Energy_Cost"].is_number()) return false;
//    pPeriods[n].EnergyCost = t["Periods"][n]["Energy_Cost"];
//    if(!t["Periods"][n]["Contracted_Power"].is_number()) return false;
//    pPeriods[n].ContractedPower = t["Periods"][n]["Contracted_Power"];
//    pPeriods[n].PowerModule->Config(pPeriods[n].ContractedPower, pPeriods[n].PowerCost);
//    pPeriods[n].EnergyModule->Config(pPeriods[n].EnergyCost);
//  }
//  if(!t["Billing"].is_array()) return false;
//  for(uint32_t n = 0; n < t["Billing"].size(); n++) {
//    if(!t["Billing"][n].is_number_unsigned()) return false;
//    uint64_t Time = t["Billing"][n];
//    if((Time % 900000) != 899999) return false;
//    pBillingCycles.push_back(Time);
//  }
//  return "";
//}
//json Tariff::GetConfig() {
//  json T;
//  T["Taxs"] = json::array();
//  for(uint32_t n = 0; n < pTaxs.size(); n++) {
//    json Tax;
//    Tax["Name"] = pTaxs[n].Name;
//    Tax["Value"] = pTaxs[n].Value;
//    Tax["Type"] = pTaxs[n].Type;
//    T["Taxs"].push_back(Tax);
//  }
//  T["Periods"] = json::array();
//  for(uint32_t n = 0; n < pPeriods.size(); n++) {
//    json Period;
//    Period["Power_Cost"] = pPeriods[n].PowerCost;
//    Period["Energy_Cost"] = pPeriods[n].EnergyCost;
//    Period["Contracted_Power"] = pPeriods[n].ContractedPower;
//    T["Periods"].push_back(Period);
//  }
//  T["Billing"] = pBillingCycles;
//  return T;
//}
json Tariff::GetResume(uint64_t now) {
  json Result;
//  Result["From"] = pFirstSampleTime;
//  Result["To"] = pLastSampletime;
//  Result["Periods"] = json::array();
//  Result["Complete"] = (now > pLastSampletime);
//  for(vector<Period_t>::iterator it = pPeriods.begin(); it != pPeriods.end(); it++) {
//    json Period, Energy, Power;
//    TariffEnergyPlugin::CycleStats_t EnergyStats = it->EnergyModule->CycleStats();
//    Energy["Consumed"]   = EnergyStats.Consumed;
//    Energy["Cost"]       = EnergyStats.Cost;
//    Energy["Other"]      = EnergyStats.Other;
//    Energy["OtherCosts"] = EnergyStats.OtherCosts;
//    Period["Energy"]     = Energy;
//    TariffPowerPlugin::CycleStats_t PowerStats = it->PowerModule->CycleStats();
//    Power["Contracted"]      = PowerStats.Contracted;
//    Power["Days"]            = PowerStats.Days;
//    Power["Cost"]            = PowerStats.Cost;
//    Power["Other"]           = PowerStats.Other;
//    Power["OtherCosts"]      = PowerStats.OtherCosts;
//    Period["Power"]          = Power;
//    Result["Periods"].push_back(Period);
//  }
//  Result["Taxes"] = json::array();
//  float Total = pSumEnergyCost + pSumPowerCost;
//  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
//    json Tax;
//    float Value = NAN;
//    if((it->Type == "PERCENT_ENERGY") || (it->Type == "PERCENT_POWER") || (it->Type == "PERCENT_ENERGY_POWER") || (it->Type == "FIXED_QUANTITY")) {
//      Tax["Name"]  = it->Name;
//      Tax["Type"]  = it->Type;
//      Tax["Value"] = it->Value;
//    }
//    if(it->Type == "PERCENT_ENERGY") Value = pSumEnergyCost * (it->Value / 100.0);
//    if(it->Type == "PERCENT_POWER") Value = pSumPowerCost * (it->Value / 100.0);
//    if(it->Type == "PERCENT_ENERGY_POWER") Value = (pSumEnergyCost + pSumPowerCost) * (it->Value / 100.0);
//    if(it->Type == "FIXED_QUANTITY") Value = it->Value;
//    if(isnan(Value) == false) {
//      Tax["Total"] = Value;
//      Total += Value;
//      Result["Taxes"].push_back(Tax);
//    }
//  }
//  float Tmp = 0.0;
//  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
//    if(it->Type == "PERCENT_TOTAL") {
//      json Tax;
//      Tax["Name"]  = it->Name;
//      Tax["Type"]  = it->Type;
//      Tax["Value"] = it->Value;
//      Tax["Total"] = Total * (it->Value / 100.0);
//      Tmp += Total * (it->Value / 100.0);
//      Result["Taxes"].push_back(Tax);
//    }
//  }
//  Result["Total"] = Total + Tmp;
  return Result;
}
bool Tariff::PushSample(const uint64_t time, const float active, const float reactive) {
//  if(pSamples.size() == 0) {
//    pSumEnergyCost = 0.0;
//    pSumPowerCost  = 0.0;
//    pFirstSampleTime = time;
//  }
//  Sample_t Sample;
//  Sample.Active   = isnan(active) ? 0.0 : active;
//  Sample.Reactive = isnan(reactive) ? 0.0 : reactive;
//  Sample.Period   = FindPeriod(time);
//  pSamples.push(Sample);
//  pPeriods[Sample.Period].PowerModule->PushSample(Sample.Active, Sample.Reactive);
//  pPeriods[Sample.Period].EnergyModule->PushSample(Sample.Active, Sample.Reactive);
//  pLastSampletime = time;
//  if(time == pEndDate) return false;
//  for(vector<uint64_t>::iterator it = pBillingCycles.begin(); it != pBillingCycles.end(); it++) {
//    if(time == *it) return false;
//  }
  return true;
}
bool Tariff::PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower) {
//  if(pSamples.size() == 0) return false;
//  Sample_t Sample = pSamples.front();
//  pSamples.pop();
//  time             = pLastSampletime - (900000ULL * pSamples.size());
//  active           = Sample.Active;
//  reactive         = Sample.Reactive;
//  period           = "P" + to_string(Sample.Period + 1);
//  color            = pPeriods[Sample.Period].Color;
//  contractedPower  = pPeriods[Sample.Period].ContractedPower;
//  float EnergyCost = pPeriods[Sample.Period].EnergyModule->PopSample(Sample.Active, Sample.Reactive);
//  float PowerCost  = pPeriods[Sample.Period].PowerModule->PopSample(Sample.Active, Sample.Reactive);
//  pSumEnergyCost += EnergyCost;
//  pSumPowerCost  += PowerCost;
//  float Cost = EnergyCost + PowerCost;
//  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
//    if(it->Type == "PERCENT_ENERGY") Cost += EnergyCost * (it->Value / 100.0);
//    if(it->Type == "PERCENT_POWER") Cost += PowerCost * (it->Value / 100.0);
//    if(it->Type == "PERCENT_ENERGY_POWER") Cost += (EnergyCost + PowerCost) * (it->Value / 100.0);
//    if(it->Type == "FIXED_QUANTITY") Cost += it->Value / 96.0;
//  }
//  float Tmp = 0.0;
//  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
//    if(it->Type == "PERCENT_TOTAL") Tmp += Cost * (it->Value / 100.0);
//  }
//  cost = Cost + Tmp;
  return true;
}
//----------------------------------------------------------------------------------------------------------------------



//----------------------------------------------------------------------------------------------------------------------
TariffList::TariffList(const json &list, const string &timezone) {
//  cctz::load_time_zone(timezone, &pTimeZone);
//  if(list.is_array() == false) return;
//  for(uint32_t n = 0; n < list.size(); n++) PushTariff(list[n]);
//  std::sort(pList.begin(), pList.end(), [](Tariff *a, Tariff *b) { return a->pStartDate > b->pStartDate; });
}
bool TariffList::PushTariff(const json &tariff) {
//  if(!tariff["StartDate"].is_number()) return false;
//  if(!tariff["EndDate"].is_number()) return false;
//  if(!tariff["Template"].is_object()) return false;
//  if(!tariff["Config"].is_object()) return false;
//  Tariff *Tmp = new Tariff(tariff["StartDate"], tariff["EndDate"], pTimeZone.name());
//  if((Tmp->SetTemplate(tariff["Template"]) == false) || (Tmp->SetConfig(tariff["Config"]) == false)) {
//    delete Tmp;
//    return false;
//  }
//  pList.push_back(Tmp);
  return true;
}
uint64_t TariffList::BillFrom(const uint64_t from) {
  uint64_t BillingFrom = (from / 3600000) * 3600000;
//  Tariff *Tariff = Find(BillingFrom);
//  if(Tariff != NULL) {
//    BillingFrom = Tariff->pStartDate;
//    for(vector<uint64_t>::iterator c = Tariff->pBillingCycles.begin(); c < Tariff->pBillingCycles.end(); c++) {
//      if(*c > from) break;
//      BillingFrom = *c + 1;
//    }
//  }
  return BillingFrom;
}
uint64_t TariffList::BillTo(const uint64_t to) {
  uint64_t BillingTo = (to / 3600000) * 3600000 + 3600000 - 1;
//  Tariff *Tariff = Find(BillingTo);
//  if(Tariff != NULL) {
//    BillingTo = Tariff->pEndDate;
//    for(vector<uint64_t>::reverse_iterator c = Tariff->pBillingCycles.rbegin(); c < Tariff->pBillingCycles.rend(); c++) {
//      if(*c < to) break;
//      BillingTo = *c;
//    }
//  }
  return BillingTo;
}
Tariff *TariffList::Find(const uint64_t time) {
//  for(uint32_t i = 0; i < pList.size(); i++) {
//    if((pList[i]->pStartDate <= time) && (pList[i]->pEndDate >= time)) return pList[i];
//  }
  return NULL;
}
TariffList::~TariffList() {
//  for(uint32_t i = 0; i < pList.size(); i++) delete pList[i];
}


//vector<TariffPowerPlugin::Type_t> __attribute__((init_priority(200))) TariffPowerPlugin::pPlugins;
//----------------------------------------------------------------------------------------------------------------------
//bool TariffPowerPlugin::Register(string name, Func_t instance) {
//  Type_t Type;
//  Type.Name = name;
//  Type.Instance = instance;
//  pPlugins.push_back(Type);
//  return true;
//}
//bool TariffPowerPlugin::Contains(string name) {
//  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
//    if((*it).Name == name) return true;
//  }
//  return false;
//}
//TariffPowerPlugin *TariffPowerPlugin::Instance(string name) {
//  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
//    if((*it).Name == name) {
//      TariffPowerPlugin *Instance = (*it).Instance();
//      Instance->pName = name;
//      return Instance;
//    }
//  }
//  return NULL;
//}
//TariffPowerPlugin::~TariffPowerPlugin() {
//}
//string TariffPowerPlugin::Name() {
//  return pName;
//}
//----------------------------------------------------------------------------------------------------------------------
//vector<TariffEnergyPlugin::Type_t> __attribute__((init_priority(200))) TariffEnergyPlugin::pPlugins;
//----------------------------------------------------------------------------------------------------------------------
//bool TariffEnergyPlugin::Register(string name, Func_t instance) {
//  Type_t Type;
//  Type.Name = name;
//  Type.Instance = instance;
//  pPlugins.push_back(Type);
//  return true;
//}
//bool TariffEnergyPlugin::Contains(string name) {
//  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
//    if((*it).Name == name) return true;
//  }
//  return false;
//}
//TariffEnergyPlugin *TariffEnergyPlugin::Instance(string name) {
//  for(vector<Type_t>::iterator it = pPlugins.begin() ; it != pPlugins.end(); ++it) {
//    if((*it).Name == name) {
//      TariffEnergyPlugin *Instance = (*it).Instance();
//      Instance->pName = name;
//      return Instance;
//    }
//  }
//  return NULL;
//}
//TariffEnergyPlugin::~TariffEnergyPlugin() {
//}
//string TariffEnergyPlugin::Name() {
//  return pName;
//}
//----------------------------------------------------------------------------------------------------------------------