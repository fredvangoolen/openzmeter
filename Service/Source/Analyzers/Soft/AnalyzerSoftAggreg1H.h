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
#include "AnalyzerSoftAggreg3S.h"
#include "AnalyzerSoftAggreg10M.h"
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAggreg1H final : public AnalyzerSoftAggreg {
  private :
    pthread_mutex_t pMutex;
    float           pNominalVoltage;
    float           pNominalFrequency;
  private :
    uint16_t        pSamples;
    uint16_t        pStatsFrequency[Analyzer::FrequencyRangesLen]; //Distribucion de frecuencias
    uint16_t        pStatsVoltage[Analyzer::VoltageRangesLen];     //Distribucion de tensiones
    uint16_t        pStatsTHD[Analyzer::THDRangesLen];             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
    uint16_t        pStatsHarmonics[2];                            //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    uint16_t        pStatsUnbalance[Analyzer::UnbalanceRangesLen]; //Distribucion de unbalance
  private :
    float CalcChange(const float in, const float reference);
  public :
    AnalyzerSoftAggreg1H(const string &serial, const Analyzer::AnalyzerMode mode);
    ~AnalyzerSoftAggreg1H();
    void Config(const float nominalVoltage, const float nominalFreq);
    void Accumulate(const AnalyzerSoftAggreg200MS *src);
    void Save() const;
    void Reset();
    bool Normalize();
    void AccumulateStats(const AnalyzerSoftAggreg3S *params);
    void AccumulateStats(const AnalyzerSoftAggreg10M *params);
};
// ---------------------------------------------------------------------------------------------------------------------