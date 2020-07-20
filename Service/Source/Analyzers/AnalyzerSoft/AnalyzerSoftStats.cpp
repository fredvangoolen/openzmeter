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
#include "AnalyzerSoftStats.h"
// ---------------------------------------------------------------------------------------------------------------------
constexpr float   AnalyzerSoftStats::FrequencyRanges[];
const     uint8_t AnalyzerSoftStats::FrequencyRangesLen;
constexpr float   AnalyzerSoftStats::VoltageRanges[];
const     uint8_t AnalyzerSoftStats::VoltageRangesLen;
constexpr float   AnalyzerSoftStats::THDRanges[];
const     uint8_t AnalyzerSoftStats::THDRangesLen;
constexpr float   AnalyzerSoftStats::HarmonicsAllowed[];
const     uint8_t AnalyzerSoftStats::HarmonicsAllowedLen;
constexpr float   AnalyzerSoftStats::UnbalanceRanges[];
const     uint8_t AnalyzerSoftStats::UnbalanceRangesLen;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftStats::AnalyzerSoftStats(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode, AGGREG_1H) {
  //Limpio todos los rangos
}
AnalyzerSoftStats::~AnalyzerSoftStats() {
}
void AnalyzerSoftStats::AccumulateStats(const AnalyzerSoftAggreg *params) {
  if((params->Type() == AGGREG_3S) && (params->Flag() == false)) {
//      float Variation = CalcChange(pAggreg3S.Frequency, pNominalFrequency);
//      for(uint8_t Range = 0; Range < FrequencyRangesLen; Range++) {
//        if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) pAggreg1H.StatsFrequency[Range]++;
//      }
  }
  if((params->Type() == AGGREG_3S) && (params->Flag() == false)) {
//      for(uint8_t Range = 0; Range < UnbalanceRangesLen; Range++) {
//        if((pAggreg10M.Unbalance >= UnbalanceRanges[Range]) && (pAggreg10M.Unbalance < UnbalanceRanges[Range + 1])) pAggreg1H.StatsUnbalance[Range]++;
//      }
//      for(uint8_t i = 0; i < pVoltageChannels; i++) {
//        float Variation = CalcChange(pAggreg10M.Voltage[i], pNominalVoltage);
//        for(uint8_t Range = 0; Range < VoltageRangesLen; Range++) {
//          if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) pAggreg1H.StatsVoltage[Range]++;
//        }
//        for(uint8_t Range = 0; Range < THDRangesLen; Range++) {
//          if((pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) pAggreg1H.StatsTHD[Range]++;
//        }
//        bool HarmonicsInRange = true;
//        for(uint8_t n = 0; n < HarmonicsAllowedLen; n++) {
//          float Variation = CalcChange(pAggreg10M.Voltage_Harmonics[i * 50 + n + 1], pNominalVoltage);
//            if(Variation > HarmonicsAllowed[n]) {
//              HarmonicsInRange = false;
//              break;
//            }
//        }
//        if(HarmonicsInRange == true) {
//          pAggreg1H.StatsHarmonics[0]++;
//        } else {
//          pAggreg1H.StatsHarmonics[1]++;
//        }
//      }
  }

}
// ---------------------------------------------------------------------------------------------------------------------
