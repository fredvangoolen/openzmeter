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
float Rates::pTmpVar = 0.0;
//----------------------------------------------------------------------------------------------------------------------
Rates::Rates(const json &r, const time_zone &timezone) {
  pTimeZone = &timezone;
  pRates.resize(r.size());
  for(uint16_t R = 0; R < r.size(); R++) {
    pRates[R].Name = r[R]["Name"];
    pRates[R].Description = r[R]["Description"];
    pRates[R].Currency = r[R]["Currency"];
    pRates[R].PowerUnits = r[R]["PowerUnits"];
    pRates[R].EnergyUnits = r[R]["EnergyUnits"];
    pRates[R].Taxs.resize(r[R]["Taxs"].size());
    for(uint16_t T = 0; T < r[R]["Taxs"].size(); T++) {
      pRates[R].Taxs[T].Description = r[R]["Taxs"][T]["Description"];
      pRates[R].Taxs[T].Value = r[R]["Taxs"][T]["Value"];
      pRates[R].Taxs[T].Type = (r[R]["Taxs"][T]["Type"] == "PERCENT_TOTAL") ? TAX_PERCENT : (r[R]["Taxs"][T]["Type"] == "PERCENT_ENERGY") ? TAX_PERCENT_ENERGY : (r[R]["Taxs"][T]["Type"] == "PERCENT_POWER") ? TAX_PERCENT_POWER : (r[R]["Taxs"][T]["Type"] == "PERCENT_ENERGY_POWER") ? TAX_PERCENT_ENERGY_POWER : TAX_FIXED;
    }
    pRates[R].Periods.resize(r[R]["Periods"].size());
    for(uint16_t P = 0; P < r[R]["Periods"].size(); P++) {
      pRates[R].Periods[P].Color = r[R]["Periods"][P]["Color"];
      pRates[R].Periods[P].DefaultPowerCost = r[R]["Periods"][P]["DefaultPowerCost"];
      pRates[R].Periods[P].DefaultEnergyCost = r[R]["Periods"][P]["DefaultEnergyCost"];
      pRates[R].Periods[P].Name = r[R]["Periods"][P]["Name"];
      pRates[R].Periods[P].Script = r[R]["Periods"][P]["Script"];
      pRates[R].Periods[P].Concepts.resize(r[R]["Periods"][P]["Concepts"].size());
      for(uint16_t C = 0; C < r[R]["Periods"][P]["Concepts"].size(); C++) {
        pRates[R].Periods[P].Concepts[C].Description = r[R]["Periods"][P]["Concepts"][C]["Description"];
        pRates[R].Periods[P].Concepts[C].Variable = r[R]["Periods"][P]["Concepts"][C]["Variable"];
        pRates[R].Periods[P].Concepts[C].Type = (r[R]["Periods"][P]["Concepts"][C]["Type"] == "INFO") ? CONCEPT_INFO : (r[R]["Periods"][P]["Concepts"][C]["Type"] == "COST_POWER") ? CONCEPT_COST_POWER : (r[R]["Periods"][P]["Concepts"][C]["Type"] == "COST_ENERGY") ? CONCEPT_COST_ENERGY : CONCEPT_COST;
      }
    }
    pRates[R].Script = r[R]["Script"];
    pRates[R].Concepts.resize(r[R]["Concepts"].size());
    for(uint16_t C = 0; C < r[R]["Concepts"].size(); C++) {
      pRates[R].Concepts[C].Description = r[R]["Concepts"][C]["Description"];
      pRates[R].Concepts[C].Variable = r[R]["Concepts"][C]["Variable"];
      pRates[R].Concepts[C].Type = (r[R]["Concepts"][C]["Type"] == "INFO") ? CONCEPT_INFO : (r[R]["Concepts"][C]["Type"] == "COST_POWER") ? CONCEPT_COST_POWER : (r[R]["Concepts"][C]["Type"] == "COST_ENERGY") ? CONCEPT_COST_ENERGY : CONCEPT_COST;
    }
    pRates[R].Days.resize(r[R]["Days"].size());
    for(uint16_t D = 0; D < r[R]["Days"].size(); D++) {
      pRates[R].Days[D].Name = r[R]["Days"][D]["Name"];
      pRates[R].Days[D].Color = r[R]["Days"][D]["Color"];
      for(uint8_t Q = 0; Q < 96; Q++) pRates[R].Days[D].Quarters[Q] = r[R]["Days"][D]["Quarters"][Q];
    }
    pRates[R].Years.resize(r[R]["Years"].size());
    for(uint16_t Y = 0; Y < r[R]["Years"].size(); Y++) {
      pRates[R].Years[Y].Year = r[R]["Years"][Y]["Year"];
      for(uint16_t D = 0; D < r[R]["Years"][Y]["Days"].size(); D++) pRates[R].Years[Y].Days[D] = r[R]["Years"][Y]["Days"][D];
    }
    sort(pRates[R].Years.begin(), pRates[R].Years.end(), [](Year_t &a, Year_t &b) { return a.Year < b.Year; });
    if(r[R]["ContractedPower"].is_number()) {
      pRates[R].Contracted = vector<float>(pRates[R].Periods.size(), r[R]["ContractedPower"]);
    } else if(r[R]["ContractedPower"].is_array()) {
      for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].Contracted[P] = r[R]["ContractedPower"][P];
    }
    if(r[R]["PowerCost"].is_string()) {
      pRates[R].PowerCostScript = r[R]["PowerCost"];
    } else {
      pRates[R].PowerCost.resize(pRates[R].Periods.size());
      for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].PowerCost[P] = r[R]["PowerCost"][P];
    }
    if(r[R]["EnergyCost"].is_string()) {
      pRates[R].EnergyCostScript = r[R]["EnergyCost"];
    } else {
      pRates[R].EnergyCost.resize(pRates[R].Periods.size());
      for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].EnergyCost[P] = r[R]["EnergyCost"][P];
    }
    chrono::system_clock::time_point TimePoint;
    parse("%d/%m/%Y", r[R]["StartDate"], *pTimeZone, &TimePoint);
    pRates[R].StartDate = TimePoint.time_since_epoch().count() / 1000000;
    parse("%d/%m/%Y", r[R]["EndDate"], *pTimeZone, &TimePoint);
    pRates[R].EndDate = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
    for(uint16_t B = 0; B < r[R]["Billing"].size(); B++) {
      parse("%d/%m/%Y", r[R]["Billing"][B], *pTimeZone, &TimePoint);
      uint64_t Time = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
      if((Time > pRates[R].StartDate) && (Time < pRates[R].EndDate)) pRates[R].BillingCycles.push_back(Time);
    }
    sort(pRates[R].BillingCycles.begin(), pRates[R].BillingCycles.end());
    pRates[R].PriceSource = r[R]["Module"];
  }
}
json Rates::CheckConfig(const json &r) {
  json Out = json::array();
  if(r.is_array() == false) return "Rate list is not an array";
  for(uint16_t R = 0; R < r.size(); R++) {
    json Tmp = CheckRateConfig(r[R]);
    if(Tmp.is_string() == true) return Tmp;
    Out.push_back(Tmp);
  }
  return Out;
}
json Rates::CheckRateConfig(const json &t) {
  json Out = CheckTariffConfig(t);
  if(Out.is_string()) return Out;
  if((t.contains("Name") == false) || (t["Name"].is_string() == false)) return "'Name' param is required";
  Out["Name"] = t["Name"];
  if((t.contains("Taxs") == false) || (t["Taxs"].is_array() == false)) return "'Taxs' param is required";
  Out["Taxs"] = json::array();
  for(uint16_t T = 0; T < t["Taxs"].size(); T++) {
    Out["Taxs"].push_back(json::object());
    if((t["Taxs"][T].contains("Description") == false) || (t["Taxs"][T]["Description"].is_string() == false)) return "'Taxs/" + to_string(T) + "/Description' param is required";
    Out["Taxs"][T]["Description"] = t["Taxs"][T]["Description"];
    if((t["Taxs"][T].contains("Value") == false) || (t["Taxs"][T]["Value"].is_number() == false)) return "'Taxs/" + to_string(T) + "/Value' param is required";
    Out["Taxs"][T]["Value"] = t["Taxs"][T]["Value"];
    if((t["Taxs"][T].contains("Type") == false) || (t["Taxs"][T]["Type"].is_string() == false) || ((t["Taxs"][T]["Type"] != "PERCENT_TOTAL") && (t["Taxs"][T]["Type"] != "PERCENT_ENERGY") && (t["Taxs"][T]["Type"] != "PERCENT_POWER") && (t["Taxs"][T]["Type"] != "PERCENT_ENERGY_POWER") && (t["Taxs"][T]["Type"] != "FIXED_QUANTITY"))) return "'Taxs/" + to_string(T) + "/Type' param is required";
    Out["Taxs"][T]["Type"] = t["Taxs"][T]["Type"];
  }
  if((t.contains("ContractedPower") == true) && (t["ContractedPower"].is_array() == true) && (t["ContractedPower"].size() == Out["Periods"].size())) {
    Out["ContractedPower"] = json::array();
    for(uint16_t P = 0; P < t["ContractedPower"].size(); P++) {
      if(t["ContractedPower"][P].is_number() == false) return "'ContractedPower/" + to_string(P) + "' param is required";
      Out["ContractedPower"].push_back(t["ContractedPower"][P]);
    }
  } else if((t.contains("ContractedPower") == true) && (t["ContractedPower"].is_number() == true)) {
    Out["ContractedPower"] = t["ContractedPower"];
  } else {
    return "'ContractedPower' param is required";
  }
  if((t.contains("Module") == false) || (t["Module"].is_string() == false)) return "'Module' param is required";
  PriceModule *Module = PriceModule::CreateInstance(t["Module"]);
  if(Module == NULL) return "'Module' param is invalid";
  Out["Module"] = t["Module"];
  map<string, float> Variables;
  Module->GetParams(0, Variables);
  delete Module;
  Variables.insert( {"Period", 0.0 });
  if((t.contains("PowerCost") == true) && (t["PowerCost"].is_array() == true) && (t["PowerCost"].size() == Out["Periods"].size())) {
    Out["PowerCost"] = json::array();
    for(uint16_t P = 0; P < t["PowerCost"].size(); P++) {
      if(t["PowerCost"][P].is_number() == false) return "'PowerCost/" + to_string(P) + "' param is required";
      Out["PowerCost"].push_back(t["PowerCost"][P]);
    }
  } else if((t.contains("PowerCost") == true) && (t["PowerCost"].is_string() == true)) {
    Parser TmpParser;
    Out["PowerCost"] = t["PowerCost"];
    for(auto &Var : Variables) TmpParser.DefineVar(Var.first, &Var.second);
    try {
      TmpParser.SetExpr(t["PowerCost"]);
      TmpParser.Eval();
    } catch(Parser::exception_type &e) {
      return "'PowerCost' parse failed (" + e.GetMsg() + ")";
    }
  } else {
    return "'PowerCost' param is required";
  }
  if((t.contains("EnergyCost") == true) && (t["EnergyCost"].is_array() == true) && (t["EnergyCost"].size() == Out["Periods"].size())) {
    Out["EnergyCost"] = json::array();
    for(uint16_t P = 0; P < t["EnergyCost"].size(); P++) {
      if(t["EnergyCost"][P].is_number() == false) return "'EnergyCost/" + to_string(P) + "' param is required";
      Out["EnergyCost"].push_back(t["EnergyCost"][P]);
    }
  } else if((t.contains("EnergyCost") == true) && (t["EnergyCost"].is_string() == true)) {
    Parser TmpParser;
    Out["EnergyCost"] = t["EnergyCost"];
    for(auto &Var : Variables) TmpParser.DefineVar(Var.first, &Var.second);
    try {
      TmpParser.SetExpr(t["EnergyCost"]);
      TmpParser.Eval();
    } catch(Parser::exception_type &e) {
      return "'PowerCost' parse failed (" + e.GetMsg() + ")";
    }
  } else {
    return "'EnergyCost' param is required";
  }
  chrono::system_clock::time_point TimePoint;
  if((t.contains("StartDate") == false) || (t["StartDate"].is_string() == false) || (parse("%d/%m/%Y", t["StartDate"], utc_time_zone(), &TimePoint) == false)) return "'StartDate' param is required";
  Out["StartDate"] = format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day StartDate = civil_day(convert(TimePoint, utc_time_zone()));
  if((t.contains("EndDate") == false) || (t["EndDate"].is_string() == false) || (parse("%d/%m/%Y", t["EndDate"], utc_time_zone(), &TimePoint) == false)) return "'EndDate' param is required";
  Out["EndDate"] = format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day EndDate = civil_day(convert(TimePoint, utc_time_zone()));
  for(uint16_t Y = StartDate.year(); Y <= EndDate.year(); Y++) {
    if(find_if(Out["Years"].begin(), Out["Years"].end(), [&](const json &a) -> bool { return (a["Year"] == Y); }) == Out["Years"].end()) return "Missing year(s) in defined range";
  }
  if((t.contains("Billing") == false) || (t["Billing"].is_array() == false)) return "'Billing' param is required";
  Out["Billing"] = json::array();
  vector<civil_day> Billing;
  for(uint16_t B = 0; B < t["Billing"].size(); B++) {
    if((t["Billing"][B].is_string() == false) || (parse("%d/%m/%Y", t["Billing"][B], utc_time_zone(), &TimePoint) == false)) return "'Billing/" + to_string(B) + "' param is required";
    civil_day BillDate = civil_day(convert(TimePoint, utc_time_zone()));
    if((BillDate <= StartDate) || (BillDate >= EndDate) || (find_if(Billing.begin(), Billing.end(), [&](const civil_day &day) -> bool { return day == BillDate; }) != Billing.end())) return "'Billing/" + to_string(B) + "' param is invalid or repeated";
    Out["Billing"].push_back(format("%d/%m/%Y", TimePoint, utc_time_zone()));
  }
  return Out;
}
json Rates::CheckTariffConfig(const json &t) {
  auto VariableFactory = [](const char_type *name, void *params) -> float* {
    set<string> *Tmp = (set<string>*)params;
    Tmp->emplace(name);
    return &pTmpVar;
  };
  if(t.is_object() == false) return "Tariff template is not an object";
  json Out = json::object();
  if((t.contains("Description") == false) || (t["Description"].is_string() == false)) return "'Description' param is required";
  Out["Description"] = t["Description"];
  if((t.contains("Currency") == false) || (t["Currency"].is_string() == false)) return "'Currency' param is required";
  Out["Currency"] = t["Currency"];
  if((t.contains("PowerUnits") == false) || (t["PowerUnits"].is_string() == false) || ((t["PowerUnits"] != "kW") && (t["PowerUnits"] != "kvar"))) return "'PowerUnits' param is required";
  Out["PowerUnits"] = t["PowerUnits"];
  if((t.contains("EnergyUnits") == false) || (t["EnergyUnits"].is_string() == false) || (t["EnergyUnits"] != "kWh")) return "'EnergyUnits' param is required";
  Out["EnergyUnits"] = t["EnergyUnits"];
  if((t.contains("Periods") == false) || (t["Periods"].is_array() == false) || (t["Periods"].size() == 0)) return "'Periods' param is required";
  Out["Periods"] = json::array();
  for(uint16_t P = 0; P < t["Periods"].size(); P++) {
    Parser      TmpParser;
    set<string> TmpVariables;
    Out["Periods"].push_back(json::object());
    if((t["Periods"][P].contains("Color") == false) || (t["Periods"][P]["Color"].is_string() == false)) return "'Periods/P" + to_string(P + 1) + "/Color' param is required";
    Out["Periods"][P]["Color"] = t["Periods"][P]["Color"];
    if((t["Periods"][P].contains("DefaultPowerCost") == false) || (t["Periods"][P]["DefaultPowerCost"].is_number() == false)) return "'Periods/P" + to_string(P + 1) + "/DefaultPowerCost' param is required";
    Out["Periods"][P]["DefaultPowerCost"] = t["Periods"][P]["DefaultPowerCost"];
    if((t["Periods"][P].contains("DefaultEnergyCost") == false) || (t["Periods"][P]["DefaultEnergyCost"].is_number() == false)) return "'Periods/P" + to_string(P + 1) + "/DefaultEnergyCost' param is required";
    Out["Periods"][P]["DefaultEnergyCost"] = t["Periods"][P]["DefaultEnergyCost"];
    if((t["Periods"][P].contains("Name") == false) || (t["Periods"][P]["Name"].is_string() == false)) return "'Periods/P" + to_string(P + 1) + "/Name' param is required";
    Out["Periods"][P]["Name"] = t["Periods"][P]["Name"];
    if((t["Periods"][P].contains("Script") == false) || (t["Periods"][P]["Script"].is_string() == false)) return "'Periods/P" + to_string(P + 1) + "/Script' param is required";
    Out["Periods"][P]["Script"] = t["Periods"][P]["Script"];
    TmpParser.SetVarFactory(VariableFactory, &TmpVariables);
    TmpParser.DefineVar("ContractedPower",  &pTmpVar);
    TmpParser.DefineVar("PowerCost",        &pTmpVar);
    TmpParser.DefineVar("EnergyCost",       &pTmpVar);
    TmpParser.DefineVar("ActivePower",      &pTmpVar);
    TmpParser.DefineVar("ActiveEnergy",     &pTmpVar);
    TmpParser.DefineVar("ReactivePower",    &pTmpVar);
    TmpParser.DefineVar("ReactiveEnergy",   &pTmpVar);
    try {
      TmpParser.SetExpr(t["Periods"][P]["Script"]);
      TmpParser.Eval();
    } catch(Parser::exception_type &e) {
      return "'Periods/P" + to_string(P + 1) + "/Script' parse failed (" + e.GetMsg() + ")";
    }
    if(TmpVariables.find("Cost") == TmpVariables.end()) return "'Cost' is not defined in 'Periods/P" + to_string(P + 1) + "/Script'";
    if((t["Periods"][P].contains("Concepts") == false) || (t["Periods"][P]["Concepts"].is_array() == false)) return "'Periods/P" + to_string(P + 1) + "/Concepts' param is required";
    Out["Periods"][P]["Concepts"] = json::array();
    for(uint16_t C = 0; C < t["Periods"][P]["Concepts"].size(); C++) {
      if((t["Periods"][P]["Concepts"][C].contains("Description") == false) || (t["Periods"][P]["Concepts"][C]["Description"].is_string() == false)) return "'Periods/P" + to_string(P + 1) + "/Concepts/" + to_string(C) + "/Description' param is required";
      Out["Periods"][P]["Concepts"][C]["Description"] = t["Periods"][P]["Concepts"][C]["Description"];
      if((t["Periods"][P]["Concepts"][C].contains("Variable") == false) || (t["Periods"][P]["Concepts"][C]["Variable"].is_string() == false)) return "'Periods/P" + to_string(P + 1) + "/Concepts/" + to_string(C) + "/Variable' param is required";
      Out["Periods"][P]["Concepts"][C]["Variable"] = t["Periods"][P]["Concepts"][C]["Variable"];
      if((t["Periods"][P]["Concepts"][C].contains("Type") == false) || (t["Periods"][P]["Concepts"][C]["Type"].is_string() == false) || ((t["Periods"][P]["Concepts"][C]["Type"] != "INFO") && (t["Periods"][P]["Concepts"][C]["Type"] != "COST_POWER") && (t["Periods"][P]["Concepts"][C]["Type"] != "COST_ENERGY") && (t["Periods"][P]["Concepts"][C]["Type"] != "COST"))) return "'Periods/P" + to_string(P + 1) + "/Concepts/" + to_string(C) + "/Type' param is required";
      Out["Periods"][P]["Concepts"][C]["Type"] = t["Periods"][P]["Concepts"][C]["Type"];
      if(TmpVariables.find(t["Periods"][P]["Concepts"][C]["Variable"]) == TmpVariables.end()) return "'" + t["Periods"][P]["Concepts"][C]["Variable"].get<string>() + "' is not defined in 'Periods/P" + to_string(P + 1) + "/Script'";
    }
  }
  Parser      TmpParser;
  set<string> TmpVariables;
  if((t.contains("Script") == false) || (t["Script"].is_string() == false)) return "'Script' param is required";
  Out["Script"] = t["Script"];
  TmpParser.SetVarFactory(VariableFactory, &TmpVariables);
  for(uint16_t P = 0; P < t["Periods"].size(); P++) {
    TmpParser.DefineVar("ContractedPower_P" + to_string(P + 1),  &pTmpVar);
    TmpParser.DefineVar("PowerCost_P" + to_string(P + 1),        &pTmpVar);
    TmpParser.DefineVar("EnergyCost_P" + to_string(P + 1),       &pTmpVar);
  }
  TmpParser.DefineVar("Period",           &pTmpVar);
  TmpParser.DefineVar("ActivePower",      &pTmpVar);
  TmpParser.DefineVar("ActiveEnergy",     &pTmpVar);
  TmpParser.DefineVar("ReactivePower",    &pTmpVar);
  TmpParser.DefineVar("ReactiveEnergy",   &pTmpVar);
  try {
    TmpParser.SetExpr(t["Script"]);
    TmpParser.Eval();
  } catch(Parser::exception_type &e) {
    return "'Script' parse failed (" + e.GetMsg() + ")";
  }
  if(TmpVariables.find("Cost") == TmpVariables.end()) return "'Cost' is not defined in 'Script'";
  if((t.contains("Concepts") == false) || (t["Concepts"].is_array() == false)) return "'Concepts' param is required";
  Out["Concepts"] = json::array();
  for(uint16_t C = 0; C < t["Concepts"].size(); C++) {
    Out["Concepts"].push_back(json::object());
    if((t["Concepts"][C].contains("Description") == false) || (t["Concepts"][C]["Description"].is_string() == false)) return "'Concepts/" + to_string(C) + "/Description' param is required";
    Out["Concepts"][C]["Description"] = t["Concepts"][C]["Description"];
    if((t["Concepts"][C].contains("Variable") == false) || (t["Concepts"][C]["Variable"].is_string() == false)) return "'Concepts/" + to_string(C) + "/Variable' param is required";
    Out["Concepts"][C]["Variable"] = t["Concepts"][C]["Variable"];
    if((t["Concepts"][C].contains("Type") == false) || (t["Concepts"][C]["Type"].is_string() == false)) return "'Concepts/" + to_string(C) + "/Type' param is required";
    Out["Concepts"][C]["Type"] = t["Concepts"][C]["Type"];
    if(TmpVariables.find(t["Concepts"][C]["Variable"]) == TmpVariables.end()) return "'" + t["Concepts"][C]["Variable"].get<string>() + "' is not defined in 'Script'";
  }
  if((t.contains("Days") == false) || (t["Days"].is_array() == false) || (t["Days"].size() == 0)) return "'Days' param is required";
  Out["Days"] = json::array();
  for(uint16_t D = 0; D < t["Days"].size(); D++) {
    Out["Days"].push_back(json::object());
    if((t["Days"][D].contains("Name") == false) || (t["Days"][D]["Name"].is_string() == false)) return "'Days/D" + to_string(D + 1) + "/Name' param is required";
    Out["Days"][D]["Name"] = t["Days"][D]["Name"];
    if((t["Days"][D].contains("Color") == false) || (t["Days"][D]["Color"].is_string() == false)) return "'Days/D" + to_string(D + 1) + "/Color' param is required";
    Out["Days"][D]["Color"] = t["Days"][D]["Color"];
    if((t["Days"][D].contains("Quarters") == false) || (t["Days"][D]["Quarters"].is_array() == false) || (t["Days"][D]["Quarters"].size() != 96)) return "'Days/D" + to_string(D + 1) + "/Quarters' param is required";
    Out["Days"][D]["Quarters"] = json::array();
    for(uint8_t Q = 0; Q < 96; Q++) {
      if((t["Days"][D]["Quarters"][Q].is_number_unsigned() == false) || (t["Days"][D]["Quarters"][Q] >= t["Periods"].size())) return "'Days/D" + to_string(D + 1) + "/Quarters/" + to_string(Q) + "' param is required";
      Out["Days"][D]["Quarters"].push_back(t["Days"][D]["Quarters"][Q]);
    }
  }
  if((t.contains("Years") == false) || (t["Years"].is_array() == false) || (t["Years"].size() == 0)) return "'Years' param is required";
  set<uint16_t> DefYears;
  Out["Years"] = json::array();
  for(uint16_t Y = 0; Y < t["Years"].size(); Y++) {
    Out["Years"].push_back(json::object());
    if((t["Years"][Y].contains("Year") == false) || (t["Years"][Y]["Year"].is_number_integer() == false) || (t["Years"][Y]["Year"] < 2000) || (t["Years"][Y]["Year"] > 2100)) return "'Years/" + to_string(Y) + "/Year' param is required";
    if(DefYears.insert(t["Years"][Y]["Year"].get<uint16_t>()).second == false) return "'Years/" + to_string(Y) + "' duplicated";
    Out["Years"][Y]["Year"] = t["Years"][Y]["Year"];
    if((t["Years"][Y].contains("Days") == false) || (t["Years"][Y]["Days"].is_array() == false) || (t["Years"][Y]["Days"].size() != (size_t)get_yearday(civil_day(t["Years"][Y]["Year"], 12, 31)))) return "'Years/" + to_string(Y) + "/Days' param is required";
    Out["Years"][Y]["Days"] = json::array();
    for(uint16_t D = 0; D < t["Years"][Y]["Days"].size(); D++) {
      if((t["Years"][Y]["Days"][D].is_number_unsigned() == false) || (t["Years"][Y]["Days"][D] >= t["Days"].size())) return "'Years/" + to_string(Y) + "/Days/" + to_string(D) + "' param is required";
      Out["Years"][Y]["Days"].push_back(t["Years"][Y]["Days"][D]);
    }
  }
  return Out;
}
//----------------------------------------------------------------------------------------------------------------------
map<string, PriceModule::Module_t>  __attribute__((init_priority(200))) PriceModule::pModules;
//----------------------------------------------------------------------------------------------------------------------
PriceModule *PriceModule::CreateInstance(const string &name) {
  auto Tmp = pModules.find(name);
  if(Tmp == pModules.end()) return NULL;
  return Tmp->second.Constructor();
}
json PriceModule::GetVarsInfo() {
  json Result = json::object();
  for(auto M : pModules) Result[M.first] = M.second.Info();
  return Result;
}
PriceModule::PriceModule() {
}
PriceModule::~PriceModule() {
}
//----------------------------------------------------------------------------------------------------------------------
bool  None_PriceModule::pRegistered = Register();
//----------------------------------------------------------------------------------------------------------------------
bool None_PriceModule::Register() {
  pModules.insert( { "NONE", { CreateInstance, GetVarsInfo } } );
  return true;
}
PriceModule *None_PriceModule::CreateInstance() {
  return new None_PriceModule();
}
json None_PriceModule::GetVarsInfo() {
  json Result = json::array();
  return Result;
}
void None_PriceModule::GetParams(const uint64_t sampletime, map<string, float> &vars) {
}
void None_PriceModule::PrepareRange(const uint64_t from, const uint64_t to) {
}
//----------------------------------------------------------------------------------------------------------------------
RatesEval::RatesEval(Rates *parent, const uint64_t from, const uint64_t to, const bool precise) {
  pParent                = parent;
  pFrom                  = FindStart(from);
  pTo                    = FindEnd(to);
  pCurrent               = pFrom;
  pRatePtr               = pParent->pRates.begin();
  pPriceModule           = NULL;
  pNowDate               = Tools::GetTime64();
}
RatesEval::~RatesEval() {
  UnloadRate();
}
uint64_t RatesEval::StartDate() {
  return pFrom;
}
uint64_t RatesEval::EndDate() {
  return pTo;
}
uint64_t RatesEval::FindStart(const uint64_t from) {
  uint64_t Time = (from / 3600000) * 3600000 + 899999;
  for(uint16_t R = 0; R < pParent->pRates.size(); R++) {
    if((Time >= pParent->pRates[R].StartDate) && (Time <= pParent->pRates[R].EndDate)) {
      Time = pParent->pRates[R].StartDate + 899999;
      for(uint16_t B = 0; B < pParent->pRates[R].BillingCycles.size(); B++) {
        if(pParent->pRates[R].BillingCycles[B] > Time) return Time;
        Time = pParent->pRates[R].BillingCycles[B] + 900000;
      }
    }
  }
  return Time;
}
uint64_t RatesEval::FindEnd(const uint64_t to) {
  uint64_t Time = (to / 3600000) * 3600000 + 899999;
  for(uint16_t R = 0; R < pParent->pRates.size(); R++) {
    if((pParent->pRates[R].StartDate <= Time) && (pParent->pRates[R].EndDate >= Time)) {
      Time = pParent->pRates[R].EndDate;
      for(uint16_t B = pParent->pRates[R].BillingCycles.size(); B > 0; B--) {
        if(pParent->pRates[R].BillingCycles[B - 1] < Time) return Time;
        Time = pParent->pRates[R].BillingCycles[B - 1];
      }
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
    pPeriods[P].Script.DefineVar("ContractedPower",   &pPeriods[P].Variables.emplace("ContractedPower", 0.0).first->second);
    pPeriods[P].Script.DefineVar("PowerCost",         &pPeriods[P].Variables.emplace("PowerCost",       0.0).first->second);
    pPeriods[P].Script.DefineVar("EnergyCost",        &pPeriods[P].Variables.emplace("EnergyCost",      0.0).first->second);
    pPeriods[P].Script.DefineVar("ActivePower",       &pPeriods[P].Variables.emplace("ActivePower",     0.0).first->second);
    pPeriods[P].Script.DefineVar("ActiveEnergy",      &pPeriods[P].Variables.emplace("ActiveEnergy",    0.0).first->second);
    pPeriods[P].Script.DefineVar("ReactivePower",     &pPeriods[P].Variables.emplace("ReactivePower",   0.0).first->second);
    pPeriods[P].Script.DefineVar("ReactiveEnergy",    &pPeriods[P].Variables.emplace("ReactiveEnergy",  0.0).first->second);
    pPeriods[P].Script.SetExpr(pRatePtr->Periods[P].Script);
    pScript.DefineVar("ContractedPower_P" + to_string(P + 1),  &pVariables.emplace("ContractedPower_P" + to_string(P + 1), 0.0).first->second);
    pScript.DefineVar("PowerCost_P" + to_string(P + 1),        &pVariables.emplace("PowerCost_P" + to_string(P + 1),       0.0).first->second);
    pScript.DefineVar("EnergyCost_P" + to_string(P + 1),       &pVariables.emplace("EnergyCost_P" + to_string(P + 1),      0.0).first->second);
  }
  pScript.DefineVar("Period",         &pVariables.emplace("Period",          1.0).first->second);
  pScript.DefineVar("ActivePower",    &pVariables.emplace("ActivePower",     0.0).first->second);
  pScript.DefineVar("ActiveEnergy",   &pVariables.emplace("ActiveEnergy",    0.0).first->second);
  pScript.DefineVar("ReactivePower",  &pVariables.emplace("ReactivePower",   0.0).first->second);
  pScript.DefineVar("ReactiveEnergy", &pVariables.emplace("ReactiveEnergy",  0.0).first->second);
  pScript.SetExpr(pRatePtr->Script);
  pPriceModule = PriceModule::CreateInstance(pRatePtr->PriceSource);
  pPriceModule->PrepareRange(pCurrent, (pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate);
  pPriceModule->GetParams(0, pPriceVariables);
  pPriceVariables.insert( { "Period", 0.0 } );
  if(pRatePtr->EnergyCost.size() == 0) {
    for(auto &Var : pPriceVariables) pEnergyScript.DefineVar(Var.first, &Var.second);
    pEnergyScript.SetExpr(pRatePtr->EnergyCostScript);
  }
  if(pRatePtr->PowerCost.size() == 0) {
    for(auto &Var : pPriceVariables) pPowerScript.DefineVar(Var.first, &Var.second);
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
  while(pRatePtr != pParent->pRates.end()) {
    if(pCurrent < pRatePtr->StartDate) break;
    if((pCurrent >= pRatePtr->StartDate) && (pCurrent <= pRatePtr->EndDate)) {
      LoadRate();
      break;
    }
//    UnloadRate();
    pRatePtr++;
  }
  civil_second Time = convert(chrono::system_clock::from_time_t(pCurrent / 1000), *(pParent->pTimeZone));
  if(pPeriods.size() > 0) {
    while(Time.year() > pYearPtr->Year) pYearPtr++;
    uint16_t PeriodIdx = pRatePtr->Days[pYearPtr->Days[get_yearday(civil_day(Time)) - 1]].Quarters[Time.hour() * 4 + Time.minute() / 15];
    if(pPeriodCallback) {
      PeriodOut_t PeriodOut = { .SampleTime = pCurrent, .Period = PeriodIdx, .Color = pRatePtr->Periods[PeriodIdx].Color, .ContractedPower = pRatePtr->Contracted[PeriodIdx] };
      pPeriodCallback(PeriodOut);
    }
    if(pSampleCallback || pBillCallback) {
      SampleOut_t SampleOut = { .SampleTime = pCurrent, .ActivePower = active, .ReactivePower = reactive, .Cost = 0.0 };
      pPeriods[PeriodIdx].Variables["ContractedPower"] = pRatePtr->Contracted[PeriodIdx];
      pPeriods[PeriodIdx].Variables["ActivePower"]     = active;
      pPeriods[PeriodIdx].Variables["ActiveEnergy"]    = active / 4.0;
      pPeriods[PeriodIdx].Variables["ReactivePower"]   = reactive;
      pPeriods[PeriodIdx].Variables["ReactiveEnergy"]  = reactive / 4.0;
      pVariables["ActivePower"]    = active;
      pVariables["ActiveEnergy"]   = active / 4.0;
      pVariables["ReactivePower"]  = reactive;
      pVariables["ReactiveEnergy"] = reactive / 4.0;
      pVariables["Period"]         = PeriodIdx + 1;
      for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) pVariables["ContractedPower_P" + to_string(P + 1)] = pRatePtr->Contracted[P];
      if((pRatePtr->EnergyCost.size() == 0) || (pRatePtr->PowerCost.size() == 0)) {
        pPriceVariables["Period"] = PeriodIdx + 1;
        pPriceModule->GetParams(pCurrent, pPriceVariables);
      }
      if(pRatePtr->EnergyCost.size() == 0) {
        float EnergyCost = pEnergyScript.Eval();
        pPeriods[PeriodIdx].Variables["EnergyCost"] = EnergyCost;
        for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) pVariables["EnergyCost_P" + to_string(P + 1)] = EnergyCost;
      } else {
        pPeriods[PeriodIdx].Variables["EnergyCost"] = pRatePtr->EnergyCost[PeriodIdx];
        for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) pVariables["EnergyCost_P" + to_string(P + 1)] = pRatePtr->EnergyCost[P];
      }
      if(pRatePtr->PowerCost.size() == 0) {
        float PowerCost = pPowerScript.Eval();
        pPeriods[PeriodIdx].Variables["PowerCost"] = PowerCost;
        for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) pVariables["PowerCost_P" + to_string(P + 1)] = PowerCost;
      } else {
        pPeriods[PeriodIdx].Variables["PowerCost"] = pRatePtr->PowerCost[PeriodIdx];
        for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) pVariables["PowerCost_P" + to_string(P + 1)] = pRatePtr->PowerCost[P];
      }
      pScript.Eval();
      pPeriods[PeriodIdx].Script.Eval();
      SampleOut.Cost = pVariables["Cost"] + pPeriods[PeriodIdx].Variables["Cost"];

//      printf("Global - ");
//      for(auto Var : pVariables) printf("%s: %.10f, ", Var.first.c_str(), Var.second);
//      printf("\nPeriod - ");
//      for(auto Var : pPeriods[PeriodIdx].Variables) printf("%s: %.10f, ", Var.first.c_str(), Var.second);
//      printf("\n");

      if(pSampleCallback) pSampleCallback(SampleOut);
      if(pCurrent == ((pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate)) {
        if(pBillCallback) pBillCallback(BuildBill(true));
//        if(pBillPtr != pRatePtr->BillingCycles.end()) pBillPtr++;
        UnloadRate();
      }
    }
  } else {
    if(pSampleCallback) {
      SampleOut_t SampleOut = { .SampleTime = pCurrent, .ActivePower = active, .ReactivePower = reactive, .Cost = 0.0 };
      pSampleCallback(SampleOut);
    }
    if(pPeriodCallback) {
      PeriodOut_t PeriodOut = { .SampleTime = pCurrent, .Period = UINT16_MAX, .Color = "", .ContractedPower = NAN };
      pPeriodCallback(PeriodOut);
    }
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
      if(pRatePtr->Periods[P].Concepts[C].Type == Rates::CONCEPT_COST) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == Rates::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == Rates::CONCEPT_COST_POWER) {
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
    if(pRatePtr->Concepts[C].Type == Rates::CONCEPT_COST) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == Rates::CONCEPT_COST_ENERGY) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == Rates::CONCEPT_COST_POWER) {
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
    if(pRatePtr->Taxs[T].Type == Rates::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == Rates::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == Rates::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == Rates::TAX_FIXED) {
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
    if(pRatePtr->Taxs[T].Type == Rates::TAX_PERCENT) {
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
void RatesEval::PushSample(const uint64_t time, const float active, const float reactive) {
  uint64_t Time = (time / 900000) * 900000 + 899999;
  while(pCurrent < Time) HandleSample(0.0, 0.0);
  HandleSample(active, reactive);
}
void RatesEval::Finish() {
  while(pCurrent < pTo) HandleSample(0.0, 0.0);
  if(pPeriods.size() > 0) {
    if(pBillCallback) pBillCallback(BuildBill(true));
    UnloadRate();
  }
}
void RatesEval::SetPeriodCallback(function<void(const PeriodOut_t &)> periodOut) {
  pPeriodCallback = periodOut;
}
void RatesEval::SetSampleCallback(function<void(const SampleOut_t &)> sampleOut) {
  pSampleCallback = sampleOut;
}
void RatesEval::SetBillCallback(function<void(const json &)> billOut) {
  pBillCallback = billOut;
}
//----------------------------------------------------------------------------------------------------------------------