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
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftRates final {
  private :
    enum SeriesMask { SERIE_TIMESTAMP   = 0x0001,
                      SERIE_ACTIVE      = 0x0002,
                      SERIE_REACTIVE    = 0x0004,
                      SERIE_COST        = 0x0008,
                      SERIE_PERIOD      = 0x0010,
                      SERIE_COLOR       = 0x0020,
                      SERIE_CONTRACTED  = 0x0040,
                      SERIE_BILL        = 0x0080,
                      SERIE_ENERGY_COST = 0x0100,
                      SERIE_POWER_COST  = 0x0200 };
    typedef struct {
      time_zone   pTimeZone;
      Database   *RateDB;
      Database   *DataDB;
      uint16_t    SeriesMask;
      json        Row;
      json        Header;

    } Eval_t;
  private :
    string         pSerial;
    time_zone      pTimeZone;
  private :
    static void InitEval(const string &serial, const time_zone &tz, Eval_t &eval, const uint64_t from, const uint64_t to, const uint8_t group, const uint16_t serieMask = 0);
    static bool EvalFetchRow(Eval_t &eval);
//    static json PushSample(Eval_t &eval, const uint64_t timestamp, const float active, const float reactive);
    static void FreeEval(Eval_t &eval);
    //uint64_t FindStart(const uint64_t from, bool precise, const uint8_t order, Database &db) const;

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