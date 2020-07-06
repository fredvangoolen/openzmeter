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
#include "RatePeriod.h"
// ---------------------------------------------------------------------------------------------------------------------
RatePeriod::RatePeriod(const json &config) {
  Color = config["Color"];
  DefaultPowerCost = config["DefaultPowerCost"];
  DefaultEnergyCost = config["DefaultEnergyCost"];
  Name = config["Name"];
  Script = config["Script"];
  for(uint16_t C = 0; C < config["Concepts"].size(); C++) Concepts.emplace_back(config["Concepts"][C]);
}
// ---------------------------------------------------------------------------------------------------------------------