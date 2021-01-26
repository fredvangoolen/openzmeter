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
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class TariffDay {
  private :
    string    pName;
    string    pColor;
    uint16_t  pQuarters[96];
  public :
    static json CheckConfig(const json &config, const uint16_t periods);
  public :
    TariffDay(const json &config);
    json GetConfig() const;
  friend class RatesEval;
};
// ---------------------------------------------------------------------------------------------------------------------