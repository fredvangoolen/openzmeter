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
#include "RateTax.h"
#include "RateConcept.h"
#include "RatePeriod.h"
#include "RateDay.h"
#include "RateYear.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class Rate final {
  private :
    string                        Name;
    string                        Description;
    string                        Currency;
    string                        PowerUnits;
    string                        EnergyUnits;
    string                        Script;
    string                        PowerCostScript;
    vector<float>                 PowerCost;
    string                        EnergyCostScript;
    vector<float>                 EnergyCost;
    uint64_t                      StartDate;
    string                        PriceSource;
    uint64_t                      EndDate;
    vector<uint64_t>              BillingCycles;
    vector<RateTax>               Taxs;
    vector<RateConcept>           Concepts;
    vector<RatePeriod>            Periods;
    vector<RateDay>               Days;
    vector<RateYear>              Years;
    vector<float>                 Contracted;
  public :
    static json CheckConfig(const json &config);
  public :
    Rate(const json &config);
    ~Rate();
};
// ---------------------------------------------------------------------------------------------------------------------