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
#include "AnalyzerSoftAlarm.h"
// ---------------------------------------------------------------------------------------------------------------------
json AnalyzerSoftAlarm::CheckConfig(const json &config) {
//  if((configAlarm.contains("Name") == false) || (configAlarm["Name"].is_string() == false)) continue;
//      if((configAlarm.contains("Priority") == false) || (configAlarm["Priority"].is_number_unsigned() == false) || (configAlarm["Priority"] > 2)) continue;
//      if((configAlarm.contains("Expression") == false) || (configAlarm["Expression"].is_string() == false)) continue;
//      if((configAlarm.contains("Aggreg") == false) || (configAlarm["Aggreg"].is_string() == false) || ((configAlarm["Aggreg"] != "200MS") && (configAlarm["Aggreg"] != "3S") && (configAlarm["Aggreg"] != "1M") && (configAlarm["Aggreg"] != "10M") && (configAlarm["Aggreg"] != "15M") && (configAlarm["Aggreg"] != "1H"))) continue;
//      if((configAlarm.contains("Samples") == false) || (configAlarm["Samples"].is_number_unsigned() == false) || (configAlarm["Samples"] > 1000)) continue;
//      if((configAlarm.contains("Window") == false) || (configAlarm["Window"].is_number_unsigned() == false)) continue;
//      if(configAlarm["Window"] < configAlarm["Samples"]) continue;
//      ConfigOUT["Alarms"].push_back(configAlarm);
return json::object();
}