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
#include "RateTax.h"
// ---------------------------------------------------------------------------------------------------------------------
RateTax::RateTax(const json &config) {
  Description = config["Description"];
  Value = config["Value"];
  Type = (config["Type"] == "PERCENT_TOTAL") ? TAX_PERCENT : (config["Type"] == "PERCENT_ENERGY") ? TAX_PERCENT_ENERGY : (config["Type"] == "PERCENT_POWER") ? TAX_PERCENT_POWER : (config["Type"] == "PERCENT_ENERGY_POWER") ? TAX_PERCENT_ENERGY_POWER : TAX_FIXED;
}
// ---------------------------------------------------------------------------------------------------------------------