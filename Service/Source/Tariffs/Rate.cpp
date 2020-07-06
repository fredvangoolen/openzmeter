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
Rate::Rate(const json &config) {
  Name = config["Name"];
  Description = config["Description"];
  Currency = config["Currency"];
  PowerUnits = config["PowerUnits"];
  EnergyUnits = config["EnergyUnits"];
  for(uint16_t T = 0; T < config["Taxs"].size(); T++) Taxs.emplace_back(config["Taxs"][T]);
  for(uint16_t P = 0; P < config["Periods"].size(); P++) Periods.emplace_back(config["Periods"][P]);
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) Concepts.emplace_back(config["Concepts"][C]);
  for(uint16_t D = 0; D < config["Days"].size(); D++) Days.emplace_back(config["Days"][D]);
  for(uint16_t Y = 0; Y < config["Years"].size(); Y++) Years.emplace_back(config["Years"][Y]);
  sort(Years.begin(), Years.end(), [](RateYear &a, RateYear &b) { return a.Year < b.Year; });

  Script = config["Script"];







  if(config["ContractedPower"].is_number()) {
    Contracted = vector<float>(Periods.size(), config["ContractedPower"]);
  } else if(r[R]["ContractedPower"].is_array()) {
    pRates[R].Contracted.resize(pRates[R].Periods.size());
    for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].Contracted[P] = r[R]["ContractedPower"][P];
  }

    if(r[R]["PowerCost"].is_string()) {
      pRates[R].PowerCostScript = r[R]["PowerCost"];
    } else {
      pRates[R].PowerCost.resize(pRates[R].Periods.size());
      for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].PowerCost[P] = r[R]["PowerCost"][P];
    }

    if(r[R]["EnergyCost"].is_string()) {
      pRates[R].EnergyCostScript = r[R]["EnergyCost"];
    } else {
      pRates[R].EnergyCost.resize(pRates[R].Periods.size());
      for(uint16_t P = 0; P < pRates[R].Periods.size(); P++) pRates[R].EnergyCost[P] = r[R]["EnergyCost"][P];
    }
    chrono::system_clock::time_point TimePoint;
    parse("%d/%m/%Y", r[R]["StartDate"], *pTimeZone, &TimePoint);
    pRates[R].StartDate = TimePoint.time_since_epoch().count() / 1000000;
    parse("%d/%m/%Y", r[R]["EndDate"], *pTimeZone, &TimePoint);
    pRates[R].EndDate = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
    for(uint16_t B = 0; B < r[R]["Billing"].size(); B++) {
      parse("%d/%m/%Y", r[R]["Billing"][B], *pTimeZone, &TimePoint);
      uint64_t Time = TimePoint.time_since_epoch().count() / 1000000 + 86399999;
      if((Time > pRates[R].StartDate) && (Time < pRates[R].EndDate)) pRates[R].BillingCycles.push_back(Time);
    }
    sort(pRates[R].BillingCycles.begin(), pRates[R].BillingCycles.end());
    pRates[R].PriceSource = r[R]["Module"];
  }