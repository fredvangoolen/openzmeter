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
#include "muParser/muParser.h"
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace mu;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAggreg {
  protected :
    const string   pSerial;                           //Serial number of parent analyzer
    const uint8_t  pVoltageChannels;                  //Avaliable voltage channels
    const uint8_t  pCurrentChannels;                  //Avaliable current channels
  protected :
    bool           pFlag;                             //Flagged period
    float          pFrequency;                        //Frecuency
    float          pUnbalance;                        //Unbalance (only in 3phase)
    float          pVoltage[3];                       //Aggregation average RMS voltage
    float          pCurrent[4];                       //Aggregation average RMS current
    uint64_t       pTimestamp;                        //Aggregation timestampt
    float          pActive_Power[3];                  //Aggregation average Active Power (P)
    float          pReactive_Power[3];                //Reactive power
    float          pActive_Energy[3];                 //Energia consumida
    float          pReactive_Energy[3];               //Reactive energy
    float          pApparent_Power[3];                //Potencia aparente (S)
    float          pPower_Factor[3];                  //Factor de potencia (cos(Phi))
    float          pVoltage_THD[3];                   //Distorsion harmonica de la tension
    float          pCurrent_THD[4];                   //Distorsion harmonica de la corriente
    float          pVoltage_Phase[3];                 //Angulo entre canales de tension
    float          pPhi[3];                           //Angulo de Phi
    float          pVoltage_Harmonics[150];           //Harmonicos del canal de tension
    float          pCurrent_Harmonics[200];           //Harmonicos del canal de corriente
    float          pPower_Harmonics[150];             //Power harmonics
  public :
    void         muParserInit(Parser &parser, const string &expression);
    uint64_t     Timestamp() const;
    float        Frequency() const;
    bool         Flag() const;
    float        Unbalance() const;
    float        Voltage(const uint8_t channel) const;
    float        Current(const uint8_t channel) const;
    float        ActivePower(const uint8_t channel) const;
    float        ApparentPower(const uint8_t channel) const;
    float        ReactivePower(const uint8_t channel) const;
    float        ActiveEnergy(const uint8_t channel) const;
    float        ReactiveEnergy(const uint8_t channel) const;
    float        VoltageHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    float        CurrentHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    float        PowerHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    float        CurrentTHD(const uint8_t channel) const;
    float        VoltageTHD(const uint8_t channel) const;
    float        VoltagePhase(const uint8_t channel) const;
    float        Phi(const uint8_t channel) const;
    float        PowerFactor(const uint8_t channel) const;
    virtual bool GetSeriesJSON(const string &serie, json &result) const;
  protected :
    AnalyzerSoftAggreg(const string &serial, const Analyzer::AnalyzerMode mode);
    virtual ~AnalyzerSoftAggreg();
    void Normalize(const uint16_t samples);
    void Reset();
    void Accumulate(const AnalyzerSoftAggreg *src);
};
// ---------------------------------------------------------------------------------------------------------------------