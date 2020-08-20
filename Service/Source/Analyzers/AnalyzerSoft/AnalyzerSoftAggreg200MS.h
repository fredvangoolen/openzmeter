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
#include "AnalyzerSoftAggreg.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAggreg200MS final : public AnalyzerSoftAggreg {
  public :
    AnalyzerSoftAggreg200MS(const string &serial, const Analyzer::AnalyzerMode mode);
    ~AnalyzerSoftAggreg200MS();
    void  Save() const;
    void  Reset();
    void  setFlag(const bool val);
    void  setTimestamp(const uint64_t time);
    void  setFrequency(const float val);
    void  setUnbalance(const float val);
    void  setVoltage(const uint8_t channel, const float val);
    void  setCurrent(const uint8_t channel, const float val);
    void  setActivePower(const uint8_t channel, const float val);
    void  setApparentPower(const uint8_t channel, const float val);
    void  setReactivePower(const uint8_t channel, const float val);
    void  setActiveEnergy(const uint8_t channel, const float val);
    void  setReactiveEnergy(const uint8_t channel, const float val);
    void  setVoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    void  setCurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    void  setPowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    void  setCurrentTHD(const uint8_t channel, const float val);
    void  setVoltageTHD(const uint8_t channel, const float val);
    void  setVoltagePhase(const uint8_t channel, const float val);
    void  setPhi(const uint8_t channel, const float val);
    void  setPowerFactor(const uint8_t channel, const float val);
};
// ---------------------------------------------------------------------------------------------------------------------