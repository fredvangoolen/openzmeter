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
// Measures the real sample rate of the S1 stream. The meter announces its rate with a resolution of only 100 Hz, but it
// sends exactly one telegram per sample (e-MUCS H ed. 1.3, 4.2.3 and 4.2.4), so counting telegrams against the system
// clock gives the real rate. A straight line is fitted through one point per second, which averages out the arrival
// jitter of the USB adapter. Self contained (no openZmeter dependencies) so it can be unit tested.
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <stdint.h>
#include <deque>
#include <utility>
// ---------------------------------------------------------------------------------------------------------------------
class DeviceS1Rate final {
  public :
    static constexpr double SETTLE     = 2.0;              //Seconds ignored after a restart (buffered data arrives late)
    static constexpr double MAX_SILENCE = 0.1;             //Longer pauses may hide lost telegrams (sequence wraps at 256)
    static const uint16_t   MIN_POINTS = 15;               //Points (seconds) needed to give a result
  private :
    std::deque<std::pair<double, uint64_t>> pPoints;       //Time, samples
    uint16_t                                pMaxPoints;    //Fit window length
    double                                  pNominal;      //Announced rate
    double                                  pTolerance;    //Max accepted difference with announced rate
    double                                  pRate;         //Last result, 0 if none
    double                                  pStart;        //Time of first data after restart, negative if none
    double                                  pLastTime;     //Time of last data
    uint64_t                                pCount;        //Samples since restart
  public :
    DeviceS1Rate();
    void   Config(const double nominal, const double tolerance, const uint16_t maxPoints);  //New stream, forgets everything
    void   Restart();                                                  //Stream interrupted, keeps last result
    void   Push(const double time, const uint32_t samples);            //Samples received (lost ones included) at 'time'
    bool   Valid() const;
    double Rate() const;                                               //Measured rate, or announced one if not known yet
};
// ---------------------------------------------------------------------------------------------------------------------
