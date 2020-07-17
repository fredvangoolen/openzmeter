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
    AggregType               Type;                               //Aggregation type
    bool                     Flag;                               //Flagged period
    float                    Frequency;                          //Frecuency
    float                    Unbalance;                          //Unbalance (only in 3phase)
    float                    Voltage[3];                         //Aggregation average RMS voltage
    float                    Current[4];                         //Aggregation average RMS current
    uint64_t                 Timestamp;                          //Aggregation timestampt
    uint16_t                 AggregSamples;                      //Number of samples accumulated in aggreg
    uint16_t                 StatsFrequency[FrequencyRangesLen]; //Distribucion de frecuencias
    uint16_t                 StatsVoltage[VoltageRangesLen];     //Distribucion de tensiones
    uint16_t                 StatsTHD[THDRangesLen];             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
    uint16_t                 StatsHarmonics[2];                  //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    uint16_t                 StatsUnbalance[UnbalanceRangesLen]; //Distribucion de unbalance
    float                    Active_Power[3];                    //Aggregation average Active Power (P)
    float                    Reactive_Power[3];                  //Reactive power
    float                    Active_Energy[3];                   //Energia consumida
    float                    Reactive_Energy[3];                 //Reactive energy
    float                    Apparent_Power[3];                  //Potencia aparente (S)
    float                    Power_Factor[3];                    //Factor de potencia (cos(PHI))
    float                    Voltage_THD[3];                     //Distorsion harmonica de la tension
    float                    Current_THD[4];                     //Distorsion harmonica de la corriente
    float                    Voltage_Phase[3];                   //Angulo entre canales de tension
    float                    Phi[3];                             //Angulo de Phi
    float                    Voltage_Harmonics[150];             //Harmonicos del canal de tension
    float                    Current_Harmonics[200];             //Harmonicos del canal de corriente
    float                    Power_Harmonics[150];               //Power harmonics



  private :
//      void InitAggreg(Params_t &params, AggregType type);
//    void AccumulateAggreg(Params_t &src, const Params_t &dst);
//    void NormalizeAggreg(Params_t &src);
//    void SaveAggreg(Params_t &params);

  public :
    AnalyzerSoftAggreg(const string &serial, Analyzer::AnalyzerMode mode);
        void InitAggreg(Params_t &params, AggregType type);
    void AccumulateAggreg(Params_t &src, const Params_t &dst);
    void NormalizeAggreg(Params_t &src);
    void SaveAggreg(Params_t &params);

};
// ---------------------------------------------------------------------------------------------------------------------