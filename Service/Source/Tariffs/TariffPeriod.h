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
#include "nlohmann/json.hpp"
#include "TariffConcept.h"
#include "TariffScript.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class TariffPeriod final {
  private :
    string                 pName;
    float                  pDefaultPowerCost;
    float                  pDefaultEnergyCost;
    string                 pColor;
    string                 pScript;
    vector<TariffConcept>  pConcepts;
  public :
    static json CheckConfig(const json &config);
  public :
    TariffPeriod(const json &config);
    json GetConfig() const;
  friend class RatesEval;
};
// ---------------------------------------------------------------------------------------------------------------------