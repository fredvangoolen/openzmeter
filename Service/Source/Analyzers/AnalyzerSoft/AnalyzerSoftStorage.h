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
#include <string>
#include "HTTPServer.h"
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftStorage final {
  private :
    enum SeriesMask {
      SERIE_TIMESTAMP                 = 0x00000001,
      SERIE_FREQUENCY                 = 0x00000002,
      SERIE_UNBALANCE                 = 0x00000004,
      SERIE_FLAG                      = 0x00000008,
      SERIE_VOLTAGE                   = 0x00000010,
      SERIE_CURRENT                   = 0x00000020,
      SERIE_ACTIVE_POWER              = 0x00000040,
      SERIE_ACTIVE_ENERGY             = 0x00000080,
      SERIE_REACTIVE_POWER            = 0x00000100,
      SERIE_REACTIVE_ENERGY           = 0x00000200,
      SERIE_APPARENT_POWER            = 0x00000400,
      SERIE_POWER_FACTOR              = 0x00000800,
      SERIE_VOLTAGE_THD               = 0x00001000,
      SERIE_CURRENT_THD               = 0x00002000,
      SERIE_PHI                       = 0x00004000,
      SERIE_VOLTAGE_HARMONICS         = 0x00008000,
      SERIE_VOLTAGE_HARMONICS_COMPLEX = 0x00010000,
      SERIE_CURRENT_HARMONICS         = 0x00020000,
      SERIE_CURRENT_HARMONICS_COMPLEX = 0x00040000,
      SERIE_POWER_HARMONICS           = 0x00080000,
      SERIE_VOLTAGE_STAT              = 0x00100000,
      SERIE_FREQUENCY_STAT            = 0x00200000,
      SERIE_THD_STAT                  = 0x00400000,
      SERIE_HARMONIC_STAT             = 0x00800000,
      SERIE_UNBALANCE_STAT            = 0x01000000
    };
  private :
    string    pSerial;
    uint8_t   pVoltageChannels;
    uint8_t   pCurrentChannels;
  public :
    AnalyzerSoftStorage(const string &serial, const Analyzer::AnalyzerMode mode);
    ~AnalyzerSoftStorage();
    void GetStatus(const json &params, Response &response);
    void GetSeries(const json &params, Response &response) const;
    void GetSeriesRange(const json &params, Response &response) const;
    void DelSeriesRange(const json &params, Response &response) const;
};
// ---------------------------------------------------------------------------------------------------------------------