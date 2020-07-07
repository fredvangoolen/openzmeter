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
json RatePeriod::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
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
// ---------------------------------------------------------------------------------------------------------------------