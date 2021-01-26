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
  pDescription = config["Description"];
  pCurrency = config["Currency"];
  pPowerUnits = config["PowerUnits"];
  pEnergyUnits = config["EnergyUnits"];
  pScript = config["Script"];
  for(uint16_t P = 0; P < config["Periods"].size(); P++) pPeriods.emplace_back(config["Periods"][P]);
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) pConcepts.emplace_back(config["Concepts"][C]);
  for(uint16_t D = 0; D < config["Days"].size(); D++) pDays.emplace_back(config["Days"][D]);
  for(uint16_t Y = 0; Y < config["Years"].size(); Y++) pYears.emplace_back(config["Years"][Y]);
  sort(pYears.begin(), pYears.end(), [](TariffYear &a, TariffYear &b) { return a.Year() < b.Year(); });
}
json Tariff::GetConfig() const {
  json Result = json::object();
  Result["Description"] = pDescription;
  Result["Currency"] = pCurrency;
  Result["PowerUnits"] = pPowerUnits;
  Result["EnergyUnits"] = pEnergyUnits;
  Result["Script"] = pScript;
  Result["Periods"] = json::array();
  for(uint16_t P = 0; P < pPeriods.size(); P++) Result["Periods"].push_back(pPeriods[P].GetConfig());
  Result["Concepts"] = json::array();
  for(uint16_t C = 0; C < pConcepts.size(); C++) Result["Concepts"].push_back(pConcepts[C].GetConfig());
  Result["Days"] = json::array();
  for(uint16_t D = 0; D < pDays.size(); D++) Result["Days"].push_back(pDays[D].GetConfig());
  Result["Years"] = json::array();
  for(uint16_t Y = 0; Y < pYears.size(); Y++) Result["Years"].push_back(pYears[Y].GetConfig());
  return Result;
}
json Tariff::CheckConfig(const json &config) {
  if(config.is_object() == false) throw string("Tariff template is not an object");
  json ConfigOUT = json::object();
  if((config.contains("Description") == false) || (config["Description"].is_string() == false)) throw string("'Description' param is required");
  ConfigOUT["Description"] = config["Description"];
  if((config.contains("Currency") == false) || (config["Currency"].is_string() == false)) throw string("'Currency' param is required");
  ConfigOUT["Currency"] = config["Currency"];
  if((config.contains("PowerUnits") == false) || (config["PowerUnits"].is_string() == false) || ((config["PowerUnits"] != "kW") && (config["PowerUnits"] != "kVA"))) throw string("'PowerUnits' param is required");
  ConfigOUT["PowerUnits"] = config["PowerUnits"];
  if((config.contains("EnergyUnits") == false) || (config["EnergyUnits"].is_string() == false) || (config["EnergyUnits"] != "kWh")) throw string("'EnergyUnits' param is required");
  ConfigOUT["EnergyUnits"] = config["EnergyUnits"];
  if((config.contains("Periods") == false) || (config["Periods"].is_array() == false)) throw string("'Periods' param is required");
  ConfigOUT["Periods"] = json::array();
  for(uint16_t P = 0; P < config["Periods"].size(); P++) ConfigOUT["Periods"].push_back(TariffPeriod::CheckConfig(config["Periods"][P]));
  set<string> Variables = {"Period", "ActivePower", "ActiveEnergy", "ReactivePower", "ReactiveEnergy"};
  for(uint16_t P = 0; P < ConfigOUT["Periods"].size(); P++) {
    Variables.insert("ContractedPower_P" + to_string(P + 1));
    Variables.insert("PowerCost_P" + to_string(P + 1));
    Variables.insert("EnergyCost_P" + to_string(P + 1));
  }
  if((config.contains("Script") == false) || (config["Script"].is_string() == false)) throw string("'Script' param is required");
  TariffScript::CheckConfig(config["Script"], Variables);
  if(Variables.find("Cost") == Variables.end()) throw string("'Cost' is not defined in 'Script'");
  ConfigOUT["Script"] = config["Script"];
  if((config.contains("Concepts") == false) || (config["Concepts"].is_array() == false)) throw string("'Concepts' param is required");
  ConfigOUT["Concepts"] = json::array();
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) ConfigOUT["Concepts"].push_back(TariffConcept::CheckConfig(config["Concepts"][C], Variables));
  if((config.contains("Days") == false) || (config["Days"].is_array() == false)) throw string("'Days' param is required");
  ConfigOUT["Days"] = json::array();
  for(uint16_t D = 0; D < config["Days"].size(); D++) ConfigOUT["Days"].push_back(TariffDay::CheckConfig(config["Days"][D], ConfigOUT["Periods"].size()));
  if((config.contains("Years") == false) || (config["Years"].is_array() == false) || (config["Years"].size() == 0)) throw string("'Years' param is required");
  set<uint16_t> DefYears;
  ConfigOUT["Years"] = json::array();
  for(uint16_t Y = 0; Y < config["Years"].size(); Y++) {
    json Year = TariffYear::CheckConfig(config["Years"][Y], ConfigOUT["Days"].size());
    ConfigOUT["Years"].push_back(Year);
    if(DefYears.insert(Year["Year"].get<uint16_t>()).second == false) throw string("'Year' is duplicated");
  }
  return ConfigOUT;
}
//----------------------------------------------------------------------------------------------------------------------