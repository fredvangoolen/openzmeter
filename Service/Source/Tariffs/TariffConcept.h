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
#include <set>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class TariffConcept final {
  public :
    enum Concept_Type { CONCEPT_INFO, CONCEPT_COST_ENERGY, CONCEPT_COST_POWER, CONCEPT_COST };
  private :
    string        pDescription;
    string        pVariable;
    Concept_Type  pType;
  public :
    static json CheckConfig(const json &config, const set<string> &variables);
  public :
    TariffConcept(const json &config);
  friend class RatesEval;
};
// ---------------------------------------------------------------------------------------------------------------------