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
#include <set>
#include "cctz/time_zone.h"
#include "muParser/muParser.h"
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "Analyzer.h"
#include "PriceSource.h"

#include "TariffConcept.h"
#include "TariffPeriod.h"
#include "TariffDay.h"
#include "TariffYear.h"




//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
using namespace mu;
//----------------------------------------------------------------------------------------------------------------------
class Tariff {
  public :
    string                        Name;
    string                        Description;
    string                        Currency;
    string                        PowerUnits;
    string                        EnergyUnits;


        vector<TariffConcept>           Concepts;
    vector<TariffPeriod>            Periods;
    vector<TariffDay>               Days;
    vector<TariffYear>              Years;

  public :
    static json CheckConfig(const json &config);
  public :
    Tariff(const json &config);
};




//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------