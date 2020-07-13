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
#pragma once
#include <stdlib.h>
#include <vector>
#include <stdint.h>
#include "nlohmann/json.h"
#include "cctz/time_zone.h"
#include "Tariff.h"
#include "RateTax.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace cctz;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class Rate final : public Tariff {
  public :
    string             Name;
    string             Script;
    string             PowerCostScript;
    vector<float>      PowerCost;
    string             EnergyCostScript;
    vector<float>      EnergyCost;
    uint64_t           StartDate;
    string             pPriceSource;
    uint64_t           EndDate;
    vector<uint64_t>   BillingCycles;
    vector<RateTax>    Taxs;
    vector<float>      Contracted;
  public :
    static json CheckConfig(const json &config);
  public :
    Rate(const json &config, const time_zone &tz);
};
// ---------------------------------------------------------------------------------------------------------------------