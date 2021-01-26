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
#include "TariffDay.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffDay::TariffDay(const json &config) {
  pName = config["Name"];
  pColor = config["Color"];
  for(uint8_t Q = 0; Q < 96; Q++) pQuarters[Q] = config["Quarters"][Q];
}
json TariffDay::GetConfig() const {
  json Result = json::object();
  Result["Name"] = pName;
  Result["Color"] = pColor;
  Result["Quarters"] = json::array();
  for(uint8_t Q = 0; Q < 96; Q++) Result["Quarters"].push_back(pQuarters[Q]);
  return Result;
}
json TariffDay::CheckConfig(const json &config, const uint16_t periods) {
  json ConfigOUT = json::object();
  if(config.is_object() == false) throw string("''Day' param is required");
  if((config.contains("Name") == false) || (config["Name"].is_string() == false)) throw string("'Name' param is required");
  ConfigOUT["Name"] = config["Name"];
  if((config.contains("Color") == false) || (config["Color"].is_string() == false)) throw string("'Color' param is required");
  ConfigOUT["Color"] = config["Color"];
  if((config.contains("Quarters") == false) || (config["Quarters"].is_array() == false) || (config["Quarters"].size() != 96)) throw string("'Quarters' param is required");
  ConfigOUT["Quarters"] = json::array();
  for(uint8_t Q = 0; Q < 96; Q++) {
    if((config["Quarters"][Q].is_number_unsigned() == false) || (config["Quarters"][Q] >= periods)) throw string("'Quarters' param is required");
    ConfigOUT["Quarters"].push_back(config["Quarters"][Q]);
  }
  return ConfigOUT;
}
// ---------------------------------------------------------------------------------------------------------------------