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
#include "TariffYear.h"
#include "cctz/time_zone.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
// ---------------------------------------------------------------------------------------------------------------------
TariffYear::TariffYear(const json &config) {
  Year = config["Year"];
  for(uint16_t D = 0; D < config["Days"].size(); D++) Days[D] = config["Days"][D];
}
json TariffYear::CheckConfig(const json &config, const uint8_t dayTypes) {
  json ConfigOUT = json::object();
  if((config.contains("Year") == false) || (config["Year"].is_number_integer() == false) || (config["Year"] < 2000) || (config["Year"] > 2100)) return "'Year' param is required";
  ConfigOUT["Year"] = config["Year"];
  if((config.contains("Days") == false) || (config["Days"].is_array() == false) || (config["Days"].size() != (size_t)get_yearday(civil_day(config["Year"], 12, 31)))) return "'Days' param is required";
  ConfigOUT["Days"] = json::array();
  for(uint16_t D = 0; D < config["Days"].size(); D++) {
    if((config["Days"][D].is_number_unsigned() == false) || (config["Days"][D] >= dayTypes)) return "'Days' param is required";
    ConfigOUT["Days"].push_back(config["Days"][D]);
  }
  return ConfigOUT;
}
