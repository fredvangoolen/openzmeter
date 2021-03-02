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
#include "AnalyzerSoftAggreg1H.h"
// ---------------------------------------------------------------------------------------------------------------------
constexpr float   AnalyzerSoftAggreg1H::FrequencyRanges[];
const     uint8_t AnalyzerSoftAggreg1H::FrequencyRangesLen;
constexpr float   AnalyzerSoftAggreg1H::VoltageRanges[];
const     uint8_t AnalyzerSoftAggreg1H::VoltageRangesLen;
constexpr float   AnalyzerSoftAggreg1H::THDRanges[];
const     uint8_t AnalyzerSoftAggreg1H::THDRangesLen;
constexpr float   AnalyzerSoftAggreg1H::HarmonicsAllowed[];
const     uint8_t AnalyzerSoftAggreg1H::HarmonicsAllowedLen;
constexpr float   AnalyzerSoftAggreg1H::UnbalanceRanges[];
const     uint8_t AnalyzerSoftAggreg1H::UnbalanceRangesLen;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg1H::AnalyzerSoftAggreg1H(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg1H::~AnalyzerSoftAggreg1H() {
}
void AnalyzerSoftAggreg1H::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pVoltage, pVoltageChannels) + ", " +
  DB.Bind(pCurrent, pCurrentChannels) + ", " +
  DB.Bind(pFrequency) + ", " +
  DB.Bind(pFlag) + ", " +
  DB.Bind(pActive_Power, pVoltageChannels) + ", " +
  DB.Bind(pReactive_Power, pVoltageChannels) + ", " +
  DB.Bind(pStatsVoltage, VoltageRangesLen) + ", " +
  DB.Bind(pStatsFrequency, FrequencyRangesLen) + ", " +
  DB.Bind(pStatsTHD, THDRangesLen) + ", " +
  DB.Bind(pStatsHarmonics, 2) + ", " +
  DB.Bind(pStatsUnbalance, UnbalanceRangesLen) + ")");
}
void AnalyzerSoftAggreg1H::Reset() {
  pSamples = 0;
  memset(pStatsFrequency, 0, sizeof(pStatsFrequency));
  memset(pStatsVoltage, 0, sizeof(pStatsVoltage));
  memset(pStatsTHD, 0, sizeof(pStatsTHD));
  memset(pStatsHarmonics, 0, sizeof(pStatsHarmonics));
  memset(pStatsUnbalance, 0, sizeof(pStatsUnbalance));
  AnalyzerSoftAggreg::Reset();
}
float AnalyzerSoftAggreg1H::CalcChange(const float in, const float reference) {
  return ((in - reference) / reference) * 100;
}
void AnalyzerSoftAggreg1H::AccumulateStats(const AnalyzerSoftAggreg3S &params, const float nominalFreq) {
  if(params.Flag() == true) return;
  float Variation = CalcChange(params.Frequency(), nominalFreq);
  for(uint8_t Range = 0; Range < FrequencyRangesLen; Range++) {
    if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) pStatsFrequency[Range]++;
  }
}
void AnalyzerSoftAggreg1H::AccumulateStats(const AnalyzerSoftAggreg10M &params, const float nominalVolt) {
  if(params.Flag() == true) return;
  for(uint8_t Range = 0; Range < UnbalanceRangesLen; Range++) {
    if((params.Unbalance() >= UnbalanceRanges[Range]) && (params.Unbalance() < UnbalanceRanges[Range + 1])) pStatsUnbalance[Range]++;
  }
  for(uint8_t i = 0; i < pVoltageChannels; i++) {
    float Variation = CalcChange(params.Voltage(i), nominalVolt);
    for(uint8_t Range = 0; Range < VoltageRangesLen; Range++) {
      if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) pStatsVoltage[Range]++;
    }
    for(uint8_t Range = 0; Range < THDRangesLen; Range++) {
      if((params.VoltageTHD(i) >= THDRanges[Range]) && (params.VoltageTHD(i) < THDRanges[Range + 1])) pStatsTHD[Range]++;
    }
    bool HarmonicsInRange = true;
    for(uint8_t n = 0; n < HarmonicsAllowedLen; n++) {
      float Variation = CalcChange(params.VoltageHarmonic(i, n + 1), nominalVolt);
      if(Variation > HarmonicsAllowed[n]) {
        HarmonicsInRange = false;
        break;
      }
    }
    if(HarmonicsInRange == true) {
      pStatsHarmonics[0]++;
    } else {
      pStatsHarmonics[1]++;
    }
  }
}
bool AnalyzerSoftAggreg1H::Normalize() {
  if(pSamples < 16000) return false;
  AnalyzerSoftAggreg::Normalize(pSamples);
  pSamples = 1;
  return true;
}
void AnalyzerSoftAggreg1H::Accumulate(const AnalyzerSoftAggreg200MS &src) {
  pSamples++;
  if(pTimestamp == 0) pTimestamp = src.Timestamp() / 3600000 * 3600000 + 3599999;
  AnalyzerSoftAggreg::Accumulate(src);
}
// ---------------------------------------------------------------------------------------------------------------------