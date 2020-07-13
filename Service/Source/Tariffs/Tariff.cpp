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
#include "TariffScript.h"
//----------------------------------------------------------------------------------------------------------------------
Tariff::Tariff(const json &config) {
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
  if(config.is_object() == false) return "Tariff template is not an object";
  json ConfigOUT = json::object();
  if((config.contains("Description") == false) || (config["Description"].is_string() == false)) return "'Description' param is required";
  ConfigOUT["Description"] = config["Description"];
  if((config.contains("Currency") == false) || (config["Currency"].is_string() == false)) return "'Currency' param is required";
  ConfigOUT["Currency"] = config["Currency"];
  if((config.contains("PowerUnits") == false) || (config["PowerUnits"].is_string() == false) || ((config["PowerUnits"] != "kW") && (config["PowerUnits"] != "kVA"))) return "'PowerUnits' param is required";
  ConfigOUT["PowerUnits"] = config["PowerUnits"];
  if((config.contains("EnergyUnits") == false) || (config["EnergyUnits"].is_string() == false) || (config["EnergyUnits"] != "kWh")) return "'EnergyUnits' param is required";
  ConfigOUT["EnergyUnits"] = config["EnergyUnits"];
  if((config.contains("Periods") == false) || (config["Periods"].is_array() == false)) return "'Periods' param is required";
  ConfigOUT["Periods"] = json::array();
  for(uint16_t P = 0; P < config["Periods"].size(); P++) {
    json Period = TariffPeriod::CheckConfig(config["Periods"][P]);
    if(Period.is_string()) return Period;
    ConfigOUT["Periods"].push_back(Period);
  }
  set<string> Variables = {"Period", "ActivePower", "ActiveEnergy", "ReactivePower", "ReactiveEnergy"};
  for(uint16_t P = 0; P < ConfigOUT["Periods"].size(); P++) {
    Variables.insert("ContractedPower_P" + to_string(P + 1));
    Variables.insert("PowerCost_P" + to_string(P + 1));
    Variables.insert("EnergyCost_P" + to_string(P + 1));
  }
  if((config.contains("Script") == false) || (config["Script"].is_string() == false)) return "'Script' param is required";
  string Res = TariffScript::CheckConfig(config["Script"], Variables);
  if(Res.empty() == false) return Res;
  if(Variables.find("Cost") == Variables.end()) return "'Cost' is not defined in 'Script'";
  ConfigOUT["Script"] = config["Script"];
  if((config.contains("Concepts") == false) || (config["Concepts"].is_array() == false)) return "'Concepts' param is required";
  ConfigOUT["Concepts"] = json::array();
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) {
    json Concept = TariffConcept::CheckConfig(config["Concepts"][C], Variables);
    if(Concept.is_string()) return Concept;
    ConfigOUT["Concepts"].push_back(Concept);
  }
  if((config.contains("Days") == false) || (config["Days"].is_array() == false)) return "'Days' param is required";
  ConfigOUT["Days"] = json::array();
  for(uint16_t D = 0; D < config["Days"].size(); D++) {
    json Day = TariffDay::CheckConfig(config["Days"][D], ConfigOUT["Periods"].size());
    if(Day.is_string()) return Day;
    ConfigOUT["Days"].push_back(Day);
  }
  if((config.contains("Years") == false) || (config["Years"].is_array() == false) || (config["Years"].size() == 0)) return "'Years' param is required";
  set<uint16_t> DefYears;
  ConfigOUT["Years"] = json::array();
  for(uint16_t Y = 0; Y < config["Years"].size(); Y++) {
    json Year = TariffYear::CheckConfig(config["Years"][Y], ConfigOUT["Days"].size());
    if(Year.is_string()) return Year;
    ConfigOUT["Years"].push_back(Year);
    if(DefYears.insert(Year["Year"].get<uint16_t>()).second == false) return "'Year' is duplicated";
  }
  return ConfigOUT;
}
//----------------------------------------------------------------------------------------------------------------------