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
#include "AnalyzerSoftFFTs.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAggreg200MS final : public AnalyzerSoftAggreg {
  private :
    typedef AnalyzerSoftFFTs::Complex_t Complex_t;
  private :
    AnalyzerSoftFFTs::Complex_t pVoltage_HarmonicsComplex[150];             //Harmonicos del canal de tension
    AnalyzerSoftFFTs::Complex_t pCurrent_HarmonicsComplex[200];             //Harmonicos del canal de corriente
  public :
    AnalyzerSoftAggreg200MS(const string &serial, const Analyzer::AnalyzerMode mode, DatabaseQueue *db);
    ~AnalyzerSoftAggreg200MS();
    Complex_t VoltageHarmonicComplex(const uint8_t channel, const uint8_t harmonic) const;
    Complex_t CurrentHarmonicComplex(const uint8_t channel, const uint8_t harmonic) const;
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
    void  setVoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const Complex_t val);
    void  setCurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const Complex_t val);
    void  setPowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    void  setCurrentTHD(const uint8_t channel, const float val);
    void  setVoltageTHD(const uint8_t channel, const float val);
    void  setVoltagePhase(const uint8_t channel, const float val);
    void  setPhi(const uint8_t channel, const float val);
    void  setPowerFactor(const uint8_t channel, const float val);
    bool  GetSeriesJSON(const string &serie, json &result) const override;
    void  HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len);
};
// ---------------------------------------------------------------------------------------------------------------------