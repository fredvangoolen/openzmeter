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
#include <string>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class RateTax final {
  public :
    enum Tax_Type { TAX_PERCENT, TAX_PERCENT_POWER, TAX_PERCENT_ENERGY, TAX_PERCENT_ENERGY_POWER, TAX_FIXED };
  public :
    Tax_Type    Type;
    float       Value;
    string      Description;
  public :
    static json CheckConfig(const json &config);
  public :
    RateTax(const json &config);
};
// ---------------------------------------------------------------------------------------------------------------------