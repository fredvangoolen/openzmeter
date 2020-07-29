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
#include "Rate.h"
#include "PriceSource.h"
// ---------------------------------------------------------------------------------------------------------------------
Rate::Rate(const json &config, const time_zone &tz) : Tariff(config) {
  Name = config["Name"];
  for(uint16_t T = 0; T < config["Taxs"].size(); T++) Taxs.emplace_back(config["Taxs"][T]);
  Script = config["Script"];
  if(config["ContractedPower"].is_number()) {
    Contracted = vector<float>(Periods.size(), config["ContractedPower"]);
  } else if(config["ContractedPower"].is_array()) {
    for(uint16_t P = 0; P < Periods.size(); P++) Contracted.emplace_back(config["ContractedPower"][P]);
  }
  if(config["PowerCost"].is_string()) {
    PowerCostScript = config["PowerCost"];
  } else {
    for(uint16_t P = 0; P < Periods.size(); P++) PowerCost.emplace_back(config["PowerCost"][P]);
  }
  if(config["EnergyCost"].is_string()) {
    EnergyCostScript = config["EnergyCost"];
  } else {
    for(uint16_t P = 0; P < Periods.size(); P++) EnergyCost.emplace_back(config["EnergyCost"][P]);
  }
  chrono::system_clock::time_point TimePoint;
  parse("%d/%m/%Y", config["StartDate"], tz, &TimePoint);
  StartDate = TimePoint.time_since_epoch().count() / 1000000;
  parse("%d/%m/%Y", config["EndDate"], tz, &TimePoint);
  EndDate = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
  for(uint16_t B = 0; B < config["Billing"].size(); B++) {
    parse("%d/%m/%Y", config["Billing"][B], tz, &TimePoint);
    uint64_t Time = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
    if((Time > StartDate) && (Time < EndDate)) BillingCycles.emplace_back(Time);
  }
  sort(BillingCycles.begin(), BillingCycles.end());
  PriceSrc = config["Module"];
}
json Rate::CheckConfig(const json &config, const bool primary) {
  json ConfigOUT = Tariff::CheckConfig(config);
  if(ConfigOUT.is_string()) return ConfigOUT;
  if((config.contains("Name") == false) || (config["Name"].is_string() == false)) return "'Name' param is required";
  ConfigOUT["Name"] = config["Name"];
  if((config.contains("Taxs") == false) || (config["Taxs"].is_array() == false)) return "'Taxs' param is required";
  ConfigOUT["Taxs"] = json::array();
  for(uint16_t T = 0; T < config["Taxs"].size(); T++) {
    json Tax = RateTax::CheckConfig(config["Taxs"][T]);
    if(Tax.is_string()) return Tax;
    ConfigOUT["Taxs"].push_back(Tax);
  }
  if((config.contains("ContractedPower") == true) && (config["ContractedPower"].is_array() == true) && (config["ContractedPower"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["ContractedPower"] = json::array();
    for(uint16_t P = 0; P < config["ContractedPower"].size(); P++) {
      if(config["ContractedPower"][P].is_number() == false) return "'ContractedPower' param is required";
      ConfigOUT["ContractedPower"].push_back(config["ContractedPower"][P]);
    }
  } else if((config.contains("ContractedPower") == true) && (config["ContractedPower"].is_number() == true)) {
    ConfigOUT["ContractedPower"] = config["ContractedPower"];
  } else {
    return "'ContractedPower' param is required";
  }
  if((config.contains("Module") == false) || (config["Module"].is_string() == false)) return "'Module' param is required";
  PriceSource *Module = PriceSource::CreateInstance(config["Module"]);
  if(Module == NULL) return "'Module' param is invalid";
  ConfigOUT["Module"] = config["Module"];
  map<string, float> Variables;
  Module->GetParams(0, Variables);
  delete Module;
  Variables.insert( {"Period", 0.0 });
  if((config.contains("PowerCost") == true) && (config["PowerCost"].is_array() == true) && (config["PowerCost"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["PowerCost"] = json::array();
    for(uint16_t P = 0; P < config["PowerCost"].size(); P++) {
      if(config["PowerCost"][P].is_number() == false) return "'PowerCost' param is required";
      ConfigOUT["PowerCost"].push_back(config["PowerCost"][P]);
    }
  } else if((config.contains("PowerCost") == true) && (config["PowerCost"].is_string() == true)) {
    Parser TmpParser;
    ConfigOUT["PowerCost"] = config["PowerCost"];
    for(auto &Var : Variables) TmpParser.DefineVar(Var.first, &Var.second);
    try {
      TmpParser.SetExpr(config["PowerCost"]);
      TmpParser.Eval();
    } catch(Parser::exception_type &e) {
      return "'PowerCost' parse failed (" + e.GetMsg() + ")";
    }
  } else {
    return "'PowerCost' param is required";
  }
  if((config.contains("EnergyCost") == true) && (config["EnergyCost"].is_array() == true) && (config["EnergyCost"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["EnergyCost"] = json::array();
    for(uint16_t P = 0; P < config["EnergyCost"].size(); P++) {
      if(config["EnergyCost"][P].is_number() == false) return "'EnergyCost' param is required";
      ConfigOUT["EnergyCost"].push_back(config["EnergyCost"][P]);
    }
  } else if((config.contains("EnergyCost") == true) && (config["EnergyCost"].is_string() == true)) {
    Parser TmpParser;
    ConfigOUT["EnergyCost"] = config["EnergyCost"];
    for(auto &Var : Variables) TmpParser.DefineVar(Var.first, &Var.second);
    try {
      TmpParser.SetExpr(config["EnergyCost"]);
      TmpParser.Eval();
    } catch(Parser::exception_type &e) {
      return "'PowerCost' parse failed (" + e.GetMsg() + ")";
    }
  } else {
    return "'EnergyCost' param is required";
  }
  chrono::system_clock::time_point TimePoint;
  if((config.contains("StartDate") == false) || (config["StartDate"].is_string() == false) || (parse("%d/%m/%Y", config["StartDate"], utc_time_zone(), &TimePoint) == false)) return "'StartDate' param is required";
  ConfigOUT["StartDate"] = format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day StartDate = civil_day(convert(TimePoint, utc_time_zone()));
  if((config.contains("EndDate") == false) || (config["EndDate"].is_string() == false) || (parse("%d/%m/%Y", config["EndDate"], utc_time_zone(), &TimePoint) == false)) return "'EndDate' param is required";
  ConfigOUT["EndDate"] = format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day EndDate = civil_day(convert(TimePoint, utc_time_zone()));
  for(uint16_t Y = StartDate.year(); Y <= EndDate.year(); Y++) {
    if(find_if(ConfigOUT["Years"].begin(), ConfigOUT["Years"].end(), [&](const json &a) -> bool { return (a["Year"] == Y); }) == ConfigOUT["Years"].end()) return "Missing year(s) in defined range";
  }
  if(primary == true) {
    if((config.contains("Billing") == false) || (config["Billing"].is_array() == false)) return "'Billing' param is required";
    ConfigOUT["Billing"] = json::array();
    vector<civil_day> Billing;
    for(uint16_t B = 0; B < config["Billing"].size(); B++) {
      if((config["Billing"][B].is_string() == false) || (parse("%d/%m/%Y", config["Billing"][B], utc_time_zone(), &TimePoint) == false)) return "'Billing' param is required";
      civil_day BillDate = civil_day(convert(TimePoint, utc_time_zone()));
      if((BillDate <= StartDate) || (BillDate >= EndDate) || (find_if(Billing.begin(), Billing.end(), [&](const civil_day &day) -> bool { return day == BillDate; }) != Billing.end())) return "'Billing' param is invalid or repeated";
      ConfigOUT["Billing"].push_back(format("%d/%m/%Y", TimePoint, utc_time_zone()));
    }
  }
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------