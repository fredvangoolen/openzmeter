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
// ---------------------------------------------------------------------------------------------------------------------
Rate::Rate(const json &config, const time_zone &tz) : Tariff(config) {

  for(uint16_t T = 0; T < config["Taxs"].size(); T++) Taxs.emplace_back(config["Taxs"][T]);

  Script = config["Script"];
  if(config["ContractedPower"].is_number()) {
    Contracted = vector<float>(Periods.size(), config["ContractedPower"]);
  } else if(config["ContractedPower"].is_array()) {
    for(uint16_t P = 0; P < Periods.size(); P++) Contracted.emplace_back(config["ContractedPower"][P]);
  }
  if(config["PowerCost"].is_string()) {
    PowerCostScript = config["PowerCost"];
  } else {
    for(uint16_t P = 0; P < Periods.size(); P++) PowerCost.emplace_back(config["PowerCost"][P]);
  }
  if(config["EnergyCost"].is_string()) {
    EnergyCostScript = config["EnergyCost"];
  } else {
    for(uint16_t P = 0; P < Periods.size(); P++) EnergyCost.emplace_back(config["EnergyCost"][P]);
  }
  chrono::system_clock::time_point TimePoint;
  parse("%d/%m/%Y", config["StartDate"], tz, &TimePoint);
  StartDate = TimePoint.time_since_epoch().count() / 1000000;
  parse("%d/%m/%Y", config["EndDate"], tz, &TimePoint);
  EndDate = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
  for(uint16_t B = 0; B < config["Billing"].size(); B++) {
    parse("%d/%m/%Y", config["Billing"][B], tz, &TimePoint);
    uint64_t Time = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
    if((Time > StartDate) && (Time < EndDate)) BillingCycles.emplace_back(Time);
  }
  sort(BillingCycles.begin(), BillingCycles.end());
  PriceSource = config["Module"];
}
json Rate::CheckConfig(const json &config) {

}
// ---------------------------------------------------------------------------------------------------------------------