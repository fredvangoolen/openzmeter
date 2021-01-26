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
#include "TariffPeriod.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffPeriod::TariffPeriod(const json &config) {
  pColor = config["Color"];
  pDefaultPowerCost = config["DefaultPowerCost"];
  pDefaultEnergyCost = config["DefaultEnergyCost"];
  pName = config["Name"];
  pScript = config["Script"];
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) pConcepts.emplace_back(config["Concepts"][C]);
}
json TariffPeriod::GetConfig() const {
  json Result = json::object();
  Result["Color"] = pColor;
  Result["DefaultPowerCost"] = pDefaultPowerCost;
  Result["DefaultEnergyCost"] = pDefaultEnergyCost;
  Result["Name"] = pName;
  Result["Script"] = pScript;
  Result["Concepts"] = json::array();
  for(uint16_t C = 0; C < pConcepts.size(); C++) Result["Concepts"].push_back(pConcepts[C].GetConfig());
  return Result;
}
json TariffPeriod::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  if(config.is_object() == false) throw string("'Concept' must be an object");
  if((config.contains("Color") == false) || (config["Color"].is_string() == false)) throw string("'Color' param is required");
  ConfigOUT["Color"] = config["Color"];
  if((config.contains("DefaultPowerCost") == false) || (config["DefaultPowerCost"].is_number() == false)) throw string("'DefaultPowerCost' param is required");
  ConfigOUT["DefaultPowerCost"] = config["DefaultPowerCost"];
  if((config.contains("DefaultEnergyCost") == false) || (config["DefaultEnergyCost"].is_number() == false)) throw string("'DefaultEnergyCost' param is required");
  ConfigOUT["DefaultEnergyCost"] = config["DefaultEnergyCost"];
  if((config.contains("Name") == false) || (config["Name"].is_string() == false)) throw string("'Name' param is required");
  ConfigOUT["Name"] = config["Name"];
  if((config.contains("Script") == false) || (config["Script"].is_string() == false)) throw string("'Script' param is required");
  set<string> Variables = {"ContractedPower", "PowerCost", "EnergyCost", "ActivePower", "ActiveEnergy", "ReactivePower", "ReactiveEnergy"};
  TariffScript::CheckConfig(config["Script"], Variables);
  if(Variables.find("Cost") == Variables.end()) throw string("'Cost' is not defined in 'Script'");
  ConfigOUT["Script"] = config["Script"];
  if((config.contains("Concepts") == false) || (config["Concepts"].is_array() == false)) throw string("'Concepts' param is required");
  ConfigOUT["Concepts"] = json::array();
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) ConfigOUT["Concepts"].push_back(TariffConcept::CheckConfig(config["Concepts"][C], Variables));
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------