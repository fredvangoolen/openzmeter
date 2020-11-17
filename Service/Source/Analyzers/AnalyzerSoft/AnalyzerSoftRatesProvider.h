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
#include <stdint.h>
#include <string>
#include <map>
#include "muParser/muParser.h"
#include "Database.h"
#include "Rate.h"
#include "PriceSource.h"
#include "RatesEvalProvider.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace mu;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftRatesProvider final : public RatesEvalProvider {
  private :
    Database   *pRateDB;
    Database   *pDataDB;
    uint8_t     pGroup;
  public :
    AnalyzerSoftRatesProvider(const string &serial, const time_zone &tz, const uint8_t group);
    ~AnalyzerSoftRatesProvider();
    Rate     *InitRates(const uint64_t from, const uint64_t to)   override;
    Sample_t *InitSamples(const uint64_t from, const uint64_t to) override;
    Rate     *NextRate()   override;
    Sample_t *NextSample() override;
};
// ---------------------------------------------------------------------------------------------------------------------