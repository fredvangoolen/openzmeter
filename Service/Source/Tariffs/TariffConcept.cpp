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
#include "TariffConcept.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffConcept::TariffConcept(const json &config) {
  Description = config["Description"];
  Variable = config["Variable"];
  Type = (config["Type"] == "INFO") ? CONCEPT_INFO : (config["Type"] == "COST_POWER") ? CONCEPT_COST_POWER : (config["Type"] == "COST_ENERGY") ? CONCEPT_COST_ENERGY : CONCEPT_COST;
}
json TariffConcept::CheckConfig(const json &config, const set<string> &variables) {
  json ConfigOUT = json::object();
  if((config.contains("Description") == false) || (config["Description"].is_string() == false)) return "'Description' param is required";
  ConfigOUT["Description"] = config["Description"];
  if((config.contains("Type") == false) || (config["Type"].is_string() == false) || ((config["Type"] != "INFO") && (config["Type"] != "COST_POWER") && (config["Type"] != "COST_ENERGY") && (config["Type"] != "COST"))) return "'Type' param is required";
  ConfigOUT["Type"] = config["Type"];
  if((config.contains("Variable") == false) || (config["Variable"].is_string() == false) || (variables.find(config["Variable"]) == variables.end())) return "'Variable' param is required";
  ConfigOUT["Variable"] = config["Variable"];
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------