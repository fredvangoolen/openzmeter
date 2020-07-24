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
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg::AnalyzerSoftAggreg(const string &serial, const Analyzer::AnalyzerMode mode) {
  pSerial = serial;
  pVoltageChannels = (mode == Analyzer::PHASE1) ? 1 : 3;
  pCurrentChannels = (mode == Analyzer::PHASE1) ? 1 : (mode == Analyzer::PHASE3) ? 3 : 4;
}
AnalyzerSoftAggreg::~AnalyzerSoftAggreg() {
}
uint64_t AnalyzerSoftAggreg::Timestamp() const {
  return pTimestamp;
}
float AnalyzerSoftAggreg::Frequency() const {
  return pFrequency;
}
bool AnalyzerSoftAggreg::Flag() const {
  return pFlag;
}
float AnalyzerSoftAggreg::Unbalance() const {
  return pUnbalance;
}
float AnalyzerSoftAggreg::Voltage(const uint8_t channel) const {
  return pVoltage[channel];
}
float AnalyzerSoftAggreg::Current(const uint8_t channel) const {
  return pCurrent[channel];
}
float AnalyzerSoftAggreg::ActivePower(const uint8_t channel) const {
  return pActive_Power[channel];
}
float AnalyzerSoftAggreg::ApparentPower(const uint8_t channel) const {
  return pApparent_Power[channel];
}
float AnalyzerSoftAggreg::ReactivePower(const uint8_t channel) const {
  return pReactive_Power[channel];
}
float AnalyzerSoftAggreg::ActiveEnergy(const uint8_t channel) const {
  return pActive_Energy[channel];
}
float AnalyzerSoftAggreg::ReactiveEnergy(const uint8_t channel) const {
  return pReactive_Energy[channel];
}
float AnalyzerSoftAggreg::VoltageHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pVoltage_Harmonics[channel * 50 + harmonic];
}
float AnalyzerSoftAggreg::CurrentHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pCurrent_Harmonics[channel * 50 + harmonic];
}
float AnalyzerSoftAggreg::PowerHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pPower_Harmonics[channel * 50 + harmonic];
}
float AnalyzerSoftAggreg::CurrentTHD(const uint8_t channel) const {
  return pCurrent_THD[channel];
}
float AnalyzerSoftAggreg::VoltageTHD(const uint8_t channel) const {
  return pVoltage_THD[channel];
}
float AnalyzerSoftAggreg::VoltagePhase(const uint8_t channel) const {
  return pVoltage_Phase[channel];
}
float AnalyzerSoftAggreg::Phi(const uint8_t channel) const {
  return pPhi[channel];
}
float AnalyzerSoftAggreg::PowerFactor(const uint8_t channel) const {
  return pPower_Factor[channel];
}
void AnalyzerSoftAggreg::Reset() {
  pFlag      = false;
  pFrequency = 0;
  pUnbalance = 0;
  pTimestamp = 0;
  memset(pVoltage, 0, sizeof(pVoltage));
  memset(pCurrent, 0, sizeof(pCurrent));
  memset(pActive_Power, 0, sizeof(pActive_Power));
  memset(pReactive_Power, 0, sizeof(pReactive_Power));
  memset(pActive_Energy, 0, sizeof(pActive_Energy));
  memset(pReactive_Energy, 0, sizeof(pReactive_Energy));
  memset(pApparent_Power, 0, sizeof(pApparent_Power));
  memset(pPower_Factor, 0, sizeof(pPower_Factor));
  memset(pVoltage_THD, 0, sizeof(pVoltage_THD));
  memset(pCurrent_THD, 0, sizeof(pCurrent_THD));
  memset(pVoltage_Phase, 0, sizeof(pVoltage_Phase));
  memset(pPhi, 0, sizeof(pPhi));
  memset(pVoltage_Harmonics, 0, sizeof(pVoltage_Harmonics));
  memset(pCurrent_Harmonics, 0, sizeof(pCurrent_Harmonics));
  memset(pPower_Harmonics, 0, sizeof(pPower_Harmonics));
}
void AnalyzerSoftAggreg::Accumulate(const AnalyzerSoftAggreg *src) {
  pFrequency += src->pFrequency;
  pUnbalance += src->pUnbalance;
  pFlag      |= src->pFlag;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage[n] += pow(src->pVoltage[n], 2.0);
  for(uint8_t n = 0; n < pCurrentChannels; n++)        pCurrent[n] += pow(src->pCurrent[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pActive_Power[n] += src->pActive_Power[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pReactive_Power[n] += src->pReactive_Power[n];
  for(uint8_t n = 0; n < pCurrentChannels; n++)        pCurrent_THD[n] += pow(src->pCurrent_THD[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pPhi[n] += src->pPhi[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pPower_Factor[n] += src->pPower_Factor[n];
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) pCurrent_Harmonics[n] += pow(src->pCurrent_Harmonics[n], 2.0);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) pPower_Harmonics[n] += src->pPower_Harmonics[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage_THD[n] += pow(src->pVoltage_THD[n], 2.0);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) pVoltage_Harmonics[n] += pow(src->pVoltage_Harmonics[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pActive_Energy[n] += src->pActive_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage_Phase[n] += src->pVoltage_Phase[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pReactive_Energy[n] += src->pReactive_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pApparent_Power[n] += src->pApparent_Power[n];
}
void AnalyzerSoftAggreg::Normalize(const uint16_t samples) {
  pFrequency = pFrequency / samples;
  pUnbalance = pUnbalance / samples;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage[n] = sqrt(pVoltage[n] / samples);
  for(uint8_t n = 0; n < pCurrentChannels; n++)        pCurrent[n] = sqrt(pCurrent[n] / samples);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pActive_Power[n] = pActive_Power[n] / samples;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pReactive_Power[n] = pReactive_Power[n] / samples;
  for(uint8_t n = 0; n < pCurrentChannels; n++)        pCurrent_THD[n] = sqrt(pCurrent_THD[n] / samples);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pPhi[n] = pPhi[n] / samples;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pPower_Factor[n] = pPower_Factor[n] / samples;
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) pCurrent_Harmonics[n] = sqrt(pCurrent_Harmonics[n] / samples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) pPower_Harmonics[n] = pPower_Harmonics[n] / samples;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage_THD[n] = sqrt(pVoltage_THD[n] / samples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) pVoltage_Harmonics[n] = sqrt(pVoltage_Harmonics[n] / samples);
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pVoltage_Phase[n] = pVoltage_Phase[n] / samples;
  for(uint8_t n = 0; n < pVoltageChannels; n++)        pApparent_Power[n] = pApparent_Power[n] / samples;
}
// ---------------------------------------------------------------------------------------------------------------------