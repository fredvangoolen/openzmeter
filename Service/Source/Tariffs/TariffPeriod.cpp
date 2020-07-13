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
  Color = config["Color"];
  DefaultPowerCost = config["DefaultPowerCost"];
  DefaultEnergyCost = config["DefaultEnergyCost"];
  Name = config["Name"];
  Script = config["Script"];
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) Concepts.emplace_back(config["Concepts"][C]);
}
json TariffPeriod::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  if(config.is_object() == false) return "'Concept' must be an object";
  if((config.contains("Color") == false) || (config["Color"].is_string() == false)) return "'Color' param is required";
  ConfigOUT["Color"] = config["Color"];
  if((config.contains("DefaultPowerCost") == false) || (config["DefaultPowerCost"].is_number() == false)) return "'DefaultPowerCost' param is required";
  ConfigOUT["DefaultPowerCost"] = config["DefaultPowerCost"];
  if((config.contains("DefaultEnergyCost") == false) || (config["DefaultEnergyCost"].is_number() == false)) return "'DefaultEnergyCost' param is required";
  ConfigOUT["DefaultEnergyCost"] = config["DefaultEnergyCost"];
  if((config.contains("Name") == false) || (config["Name"].is_string() == false)) return "'Name' param is required";
  ConfigOUT["Name"] = config["Name"];
  if((config.contains("Script") == false) || (config["Script"].is_string() == false)) return "'Script' param is required";
  set<string> Variables = {"ContractedPower", "PowerCost", "EnergyCost", "ActivePower", "ActiveEnergy", "ReactivePower", "ReactiveEnergy"};
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
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------