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
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAggreg final {
  public :
    enum AggregType  { AGGREG_200MS, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
  private :
    uint8_t                 pVoltageChannels;                    //Avaliable voltage channels
    uint8_t                 pCurrentChannels;                    //Avaliable current channels
    string                  pSerial;                             //Serial number of parent analyzer
    AggregType              pType;                               //Aggregation type
    bool                    pFlag;                               //Flagged period
    float                   pFrequency;                          //Frecuency
    float                   pUnbalance;                          //Unbalance (only in 3phase)
    float                   pVoltage[3];                         //Aggregation average RMS voltage
    float                   pCurrent[4];                         //Aggregation average RMS current
    uint64_t                pTimestamp;                          //Aggregation timestampt
//    uint16_t                pAggregSamples;                      //Number of samples accumulated in aggreg
//    uint16_t                pStatsFrequency[FrequencyRangesLen]; //Distribucion de frecuencias
//    uint16_t                pStatsVoltage[VoltageRangesLen];     //Distribucion de tensiones
//    uint16_t                pStatsTHD[THDRangesLen];             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
//    uint16_t                pStatsHarmonics[2];                  //Numero de muestras que cumplen los limites de harmonicos / no cumplen
//    uint16_t                pStatsUnbalance[UnbalanceRangesLen]; //Distribucion de unbalance
    float                   pActive_Power[3];                    //Aggregation average Active Power (P)
    float                   pReactive_Power[3];                  //Reactive power
    float                   pActive_Energy[3];                   //Energia consumida
    float                   pReactive_Energy[3];                 //Reactive energy
    float                   pApparent_Power[3];                  //Potencia aparente (S)
    float                   pPower_Factor[3];                    //Factor de potencia (cos(PHI))
    float                   pVoltage_THD[3];                     //Distorsion harmonica de la tension
    float                   pCurrent_THD[4];                     //Distorsion harmonica de la corriente
    float                   pVoltage_Phase[3];                   //Angulo entre canales de tension
    float                   pPhi[3];                             //Angulo de Phi
    float                   pVoltage_Harmonics[150];             //Harmonicos del canal de tension
    float                   pCurrent_Harmonics[200];             //Harmonicos del canal de corriente
    float                   pPower_Harmonics[150];               //Power harmonics
  public :
    AnalyzerSoftAggreg(const string &serial, const Analyzer::AnalyzerMode mode, const AggregType type);
    uint64_t Timestamp() const;
    void     Timestamp(const uint64_t time);
    float    Frequency() const;
    void     Frequency(const float val);
    float    Unbalance() const;
    void     Unbalance(const float val);
    float    Voltage(const uint8_t channel) const;
    void     Voltage(const uint8_t channel, const float val);
    float    Current(const uint8_t channel) const;
    void     Current(const uint8_t channel, const float val);
    float    ActivePower(const uint8_t channel) const;
    void     ActivePower(const uint8_t channel, const float val);
    float    ApparentPower(const uint8_t channel) const;
    void     ApparentPower(const uint8_t channel, const float val);
    float    ReactivePower(const uint8_t channel) const;
    void     ReactivePower(const uint8_t channel, const float val);
    float    ActiveEnergy(const uint8_t channel) const;
    void     ActiveEnergy(const uint8_t channel, const float val);
    float    ReactiveEnergy(const uint8_t channel) const;
    void     ReactiveEnergy(const uint8_t channel, const float val);
    float    VoltageHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    void     VoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    float    CurrentHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    void     CurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    float    PowerHarmonic(const uint8_t channel, const uint8_t harmonic) const;
    void     PowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val);
    float    CurrentTHD(const uint8_t channel) const;
    void     CurrentTHD(const uint8_t channel, const float val);
    float    VoltageTHD(const uint8_t channel) const;
    void     VoltageTHD(const uint8_t channel, const float val);
    float    VoltagePhase(const uint8_t channel) const;
    void     VoltagePhase(const uint8_t channel, const float val);
    float    Phi(const uint8_t channel) const;
    void     Phi(const uint8_t channel, const float val);
    float    PowerFactor(const uint8_t channel) const;
    void     PowerFactor(const uint8_t channel, const float val);




    bool AppendToJSON(const string &serie, json &result);


//        void InitAggreg(Params_t &params, AggregType type);
//    void AccumulateAggreg(Params_t &src, const Params_t &dst);
//    void NormalizeAggreg(Params_t &src);
//    void SaveAggreg(Params_t &params);

};
// ---------------------------------------------------------------------------------------------------------------------