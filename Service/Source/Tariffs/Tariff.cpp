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
Tariff::Tariff(const json &config) {
  Name = config["Name"];
  Description = config["Description"];
  Currency = config["Currency"];
  PowerUnits = config["PowerUnits"];
  EnergyUnits = config["EnergyUnits"];


  for(uint16_t P = 0; P < config["Periods"].size(); P++) Periods.emplace_back(config["Periods"][P]);
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) Concepts.emplace_back(config["Concepts"][C]);
  for(uint16_t D = 0; D < config["Days"].size(); D++) Days.emplace_back(config["Days"][D]);
  for(uint16_t Y = 0; Y < config["Years"].size(); Y++) Years.emplace_back(config["Years"][Y]);
  sort(Years.begin(), Years.end(), [](TariffYear &a, TariffYear &b) { return a.Year < b.Year; });

}
json Tariff::CheckConfig(const json &config) {
  auto VariableFactory = [](const char_type *name, void *params) -> float* {
    set<string> *Tmp = (set<string>*)params;
    Tmp->emplace(name);
    return &pTmpVar;
  };
  if(config.is_object() == false) return "Tariff template is not an object";
  json Out = json::object();
  if((config.contains("Description") == false) || (config["Description"].is_string() == false)) return "'Description' param is required";
  Out["Description"] = config["Description"];
  if((config.contains("Currency") == false) || (config["Currency"].is_string() == false)) return "'Currency' param is required";
  Out["Currency"] = config["Currency"];
  if((config.contains("PowerUnits") == false) || (config["PowerUnits"].is_string() == false) || ((config["PowerUnits"] != "kW") && (t["PowerUnits"] != "kVA"))) return "'PowerUnits' param is required";
  Out["PowerUnits"] = config["PowerUnits"];
  if((config.contains("EnergyUnits") == false) || (config["EnergyUnits"].is_string() == false) || (config["EnergyUnits"] != "kWh")) return "'EnergyUnits' param is required";
  Out["EnergyUnits"] = config["EnergyUnits"];

  if((t.contains("Periods") == false) || (t["Periods"].is_array() == false) || (t["Periods"].size() == 0)) return "'Periods' param is required";
  Out["Periods"] = json::array();
  for(uint16_t P = 0; P < t["Periods"].size(); P++) {
    json Period = RatePeriod::CheckConfig(t["Periods"][P]);
    if(Period.is_string()) return Period;
    Out["Periods"].emplace_back(RatePeriod(Period));
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
  if((config.contains("Years") == false) || (config["Years"].is_array() == false) || (config["Years"].size() == 0)) return "'Years' param is required";
  set<uint16_t> DefYears;
  Out["Years"] = json::array();
  for(uint16_t Y = 0; Y < t["Years"].size(); Y++) {
    json Year = RateYear::CheckConfig(config["Years"][Y]);
    if(Year.is_string()) return Year;
    Out["Years"].push_back(Year);
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






float Rates::pTmpVar = 0.0;
//----------------------------------------------------------------------------------------------------------------------
Rates::Rates(const json &r, const time_zone &timezone) {
  pTimeZone = &timezone;
  for(uint16_t R = 0; R < r.size(); R++) pRates.emplace_back(r[R], timezone);
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
  json Out = Tariff::CheckConfig(t);
  if(Out.is_string()) return Out;
  if((t.contains("Name") == false) || (t["Name"].is_string() == false)) return "'Name' param is required";
  Out["Name"] = t["Name"];
  if((t.contains("Taxs") == false) || (t["Taxs"].is_array() == false)) return "'Taxs' param is required";
  Out["Taxs"] = json::array();
  for(uint16_t T = 0; T < t["Taxs"].size(); T++) {
    json Tax = RateTax::CheckConfig(t["Taxs"][T]);
    if(Tax.is_string()) return Tax;
    Out["Taxs"].emplace_back(Tax);
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
  PriceSource *Module = PriceSource::CreateInstance(t["Module"]);
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


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
//650