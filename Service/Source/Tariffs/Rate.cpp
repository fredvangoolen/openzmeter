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
  pName  = config["Name"];
  pID    = config["RateID"];
  pGroup = config["Group"];
  for(uint16_t T = 0; T < config["Taxes"].size(); T++) pTaxes.emplace_back(config["Taxes"][T]);
  if(config["ContractedPower"].is_number()) {
    pContracted = vector<float>(pPeriods.size(), config["ContractedPower"]);
    pContractedEqual = true;
  } else if(config["ContractedPower"].is_array()) {
    for(uint16_t P = 0; P < pPeriods.size(); P++) pContracted.emplace_back(config["ContractedPower"][P]);
    pContractedEqual = false;
  }
  if(config["PowerCost"].is_string()) {
    pPowerCostScript = config["PowerCost"];
  } else {
    for(uint16_t P = 0; P < pPeriods.size(); P++) pPowerCost.emplace_back(config["PowerCost"][P]);
  }
  if(config["EnergyCost"].is_string()) {
    pEnergyCostScript = config["EnergyCost"];
  } else {
    for(uint16_t P = 0; P < pPeriods.size(); P++) pEnergyCost.emplace_back(config["EnergyCost"][P]);
  }
  chrono::system_clock::time_point TimePoint;
  parse("%d/%m/%Y", config["StartDate"], tz, &TimePoint);
  pStartDate = TimePoint.time_since_epoch().count() / 1000000;
  parse("%d/%m/%Y", config["EndDate"], tz, &TimePoint);
  pEndDate = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
  for(uint16_t B = 0; B < config["Billing"].size(); B++) {
    parse("%d/%m/%Y", config["Billing"][B], tz, &TimePoint);
    uint64_t Time = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
    if((Time > pStartDate) && (Time < pEndDate)) pBillingCycles.emplace_back(Time);
  }
  sort(pBillingCycles.begin(), pBillingCycles.end());
  pPriceSrc = config["Module"];
}
uint64_t Rate::EndDate() const {
  return pEndDate;
}
uint64_t Rate::StartDate() const {
  return pStartDate;
}
uint8_t Rate::Group() const {
  return pGroup;
}
json Rate::CheckConfig(const json &config) {
  json ConfigOUT = Tariff::CheckConfig(config);
  if((config.contains("Name") == false) || (config["Name"].is_string() == false)) throw string("'Name' param is required");
  ConfigOUT["Name"] = config["Name"];
  if((config.contains("Group") == false) || (config["Group"].is_number_integer() == false) || (config["Group"] < 0) || (config["Group"] > 10)) throw string("'Group' param is required");
  ConfigOUT["Group"] = config["Group"];
  if((config.contains("RateID") == false) || (config["RateID"].is_string() == false)) throw string("'RateID' param is required");
  ConfigOUT["RateID"] = config["RateID"];
  if((config.contains("Taxes") == false) || (config["Taxes"].is_array() == false)) throw string("'Taxes' param is required");
  ConfigOUT["Taxes"] = json::array();
  for(uint16_t T = 0; T < config["Taxes"].size(); T++) ConfigOUT["Taxes"].push_back(RateTax::CheckConfig(config["Taxes"][T]));
  if((config.contains("ContractedPower") == true) && (config["ContractedPower"].is_array() == true) && (config["ContractedPower"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["ContractedPower"] = json::array();
    for(uint16_t P = 0; P < config["ContractedPower"].size(); P++) {
      if(config["ContractedPower"][P].is_number() == false) throw string("'ContractedPower' param is required");
      ConfigOUT["ContractedPower"].push_back(config["ContractedPower"][P]);
    }
  } else if((config.contains("ContractedPower") == true) && (config["ContractedPower"].is_number() == true)) {
    ConfigOUT["ContractedPower"] = config["ContractedPower"];
  } else {
    throw string("'ContractedPower' param is required");
  }
  if((config.contains("Module") == false) || (config["Module"].is_string() == false)) throw string("'Module' param is required");
  map<string, float> Variables;
  if(PriceSource::CheckConfig(config["Module"], Variables) == false) throw string("'Module' param is invalid");
  ConfigOUT["Module"] = config["Module"];
  Variables.insert( {"Period", 0.0 });
  if((config.contains("PowerCost") == true) && (config["PowerCost"].is_array() == true) && (config["PowerCost"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["PowerCost"] = json::array();
    for(uint16_t P = 0; P < config["PowerCost"].size(); P++) {
      if(config["PowerCost"][P].is_number() == false) throw string("'PowerCost' param is required");
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
      throw string("'PowerCost' parse failed (" + e.GetMsg() + ")");
    }
  } else {
    throw string("'PowerCost' param is required");
  }
  if((config.contains("EnergyCost") == true) && (config["EnergyCost"].is_array() == true) && (config["EnergyCost"].size() == ConfigOUT["Periods"].size())) {
    ConfigOUT["EnergyCost"] = json::array();
    for(uint16_t P = 0; P < config["EnergyCost"].size(); P++) {
      if(config["EnergyCost"][P].is_number() == false) throw string("'EnergyCost' param is required");
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
      throw string("'PowerCost' parse failed (" + e.GetMsg() + ")");
    }
  } else {
    throw string("'EnergyCost' param is required");
  }
  chrono::system_clock::time_point TimePoint;
  if((config.contains("StartDate") == false) || (config["StartDate"].is_string() == false) || (parse("%d/%m/%Y", config["StartDate"], utc_time_zone(), &TimePoint) == false)) throw string("'StartDate' param is required");
  ConfigOUT["StartDate"] = cctz::format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day StartDate = civil_day(convert(TimePoint, utc_time_zone()));
  if((config.contains("EndDate") == false) || (config["EndDate"].is_string() == false) || (parse("%d/%m/%Y", config["EndDate"], utc_time_zone(), &TimePoint) == false)) throw string("'EndDate' param is required");
  ConfigOUT["EndDate"] = cctz::format("%d/%m/%Y", TimePoint, utc_time_zone());
  civil_day EndDate = civil_day(convert(TimePoint, utc_time_zone()));
  for(uint16_t Y = StartDate.year(); Y <= EndDate.year(); Y++) {
    if(find_if(ConfigOUT["Years"].begin(), ConfigOUT["Years"].end(), [&](const json &a) -> bool { return (a["Year"] == Y); }) == ConfigOUT["Years"].end()) throw string("Missing year(s) in defined range");
  }
  if((config.contains("Billing") == false) || (config["Billing"].is_array() == false)) throw string("'Billing' param is required");
  ConfigOUT["Billing"] = json::array();
  vector<civil_day> Billing;
  for(uint16_t B = 0; B < config["Billing"].size(); B++) {
    if((config["Billing"][B].is_string() == false) || (parse("%d/%m/%Y", config["Billing"][B], utc_time_zone(), &TimePoint) == false)) throw string("'Billing' param is required");
    civil_day BillDate = civil_day(convert(TimePoint, utc_time_zone()));
    if((BillDate <= StartDate) || (BillDate >= EndDate) || (find_if(Billing.begin(), Billing.end(), [&](const civil_day &day) -> bool { return day == BillDate; }) != Billing.end())) throw string("'Billing' param is invalid or repeated");
    ConfigOUT["Billing"].push_back(cctz::format("%d/%m/%Y", TimePoint, utc_time_zone()));
  }
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------