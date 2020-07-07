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
#include "TariffConcept.h"
// ---------------------------------------------------------------------------------------------------------------------
TariffConcept::TariffConcept(const json &config) {
  Description = config["Description"];
  Variable = config["Variable"];
  Type = (config["Type"] == "INFO") ? CONCEPT_INFO : (config["Type"] == "COST_POWER") ? CONCEPT_COST_POWER : (config["Type"] == "COST_ENERGY") ? CONCEPT_COST_ENERGY : CONCEPT_COST;
}
// ---------------------------------------------------------------------------------------------------------------------