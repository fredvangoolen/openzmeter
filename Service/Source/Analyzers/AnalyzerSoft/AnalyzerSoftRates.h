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
#include "HTTPServer.h"
#include "Rate.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftRates final {
  private :
    string         pSerial;
    time_zone      pTimeZone;
  public :
    AnalyzerSoftRates(const string &serial);
    ~AnalyzerSoftRates();
    void Config(const time_zone &tz);
    void GetRates(const json &params, Response &response) const;
    void AddRate(const json &params, Response &response) const;
    void GetRate(const json &params, Response &response) const;
    void SetRate(const json &params, Response &response) const;
    void DelRate(const json &params, Response &response) const;
    void GetCosts(const json &params, Response &response) const;
    void GetBills(const json &params, Response &response) const;
};
// ---------------------------------------------------------------------------------------------------------------------