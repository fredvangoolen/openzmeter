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
#include "nlohmann/json.h"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class TariffYear final {
  public :
    uint16_t     Year;
    uint16_t     Days[366];
  public :
    static json CheckConfig(const json &config, const uint16_t dayTypes);
  public :
    TariffYear(const json &config);
};
// ---------------------------------------------------------------------------------------------------------------------