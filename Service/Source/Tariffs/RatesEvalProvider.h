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
#include "cctz/time_zone.h"
#include "nlohmann/json.hpp"
#include "Rate.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class RatesEvalProvider {
  public :
    typedef struct {
      uint64_t Time;
      float    Active;
      float    Reactive;
    } Sample_t;
  private :
    json       pRates;
    uint32_t   pRatesIdx;
    json       pSamples;
    uint32_t   pSamplesIdx;
  protected :
    Sample_t  *pLoadedSample;
    Rate      *pLoadedRate;
    time_zone  pTimeZone;
  public :
    virtual Rate     *InitRates(const uint64_t from, const uint64_t to);
    virtual Sample_t *InitSamples(const uint64_t from, const uint64_t to);
    virtual Rate     *NextRate();
    virtual Sample_t *NextSample();
  public :
    RatesEvalProvider(const time_zone &tz, const json &rates = nullptr, const json &samples = nullptr);
    virtual ~RatesEvalProvider();
};
// ---------------------------------------------------------------------------------------------------------------------