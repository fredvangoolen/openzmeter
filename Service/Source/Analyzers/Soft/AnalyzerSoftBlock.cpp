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
#include "AnalyzerSoftBlock.h"
#include "Tools.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftBlock::AnalyzerSoftBlock(const uint8_t vChanels, const uint8_t cChannels, const uint64_t seq, const uint16_t maxLen, const float nominalVolt, const float nominalFreq) : pSequence(seq), pVoltageChannels(vChanels), pCurrentChannels(cChannels), pNominalFrequency(nominalFreq), pNominalVoltage(nominalVolt) {
  pBlockCyclesCount = 0;
  pBlockSamples     = 0;
  pVoltageSamples   = (float*)malloc(sizeof(float) * pVoltageChannels * maxLen);
  pCurrentSamples   = (float*)malloc(sizeof(float) * pCurrentChannels * maxLen);
  pTimestamp        = Tools::GetTime64();
  memset(pBlockCycles, 0, sizeof(pBlockCycles));
}
AnalyzerSoftBlock::~AnalyzerSoftBlock() {
  if(pVoltageSamples != NULL) free(pVoltageSamples);
  if(pCurrentSamples != NULL) free(pCurrentSamples);
}
void AnalyzerSoftBlock::PushSample(const float *voltageSamples, const float *currentSamples) {
  memcpy(&pVoltageSamples[pVoltageChannels * pBlockSamples], voltageSamples, sizeof(float) * pVoltageChannels);
  memcpy(&pCurrentSamples[pCurrentChannels * pBlockSamples], currentSamples, sizeof(float) * pCurrentChannels);
  pBlockSamples++;
}
void AnalyzerSoftBlock::PushCycle(const uint16_t len) {
  pBlockCycles[pBlockCyclesCount] = len;
  pBlockCyclesCount++;
}
uint64_t AnalyzerSoftBlock::Timestamp() const {
  return pTimestamp;
}
uint16_t AnalyzerSoftBlock::Samples() const {
  return pBlockSamples;
}
uint8_t AnalyzerSoftBlock::Cycles() const {
  return pBlockCyclesCount;
}
uint16_t AnalyzerSoftBlock::CycleLen(const uint8_t cycle) const {
  return pBlockCycles[cycle];
}
float AnalyzerSoftBlock::NominalFrequency() const {
  return pNominalFrequency;
}
float AnalyzerSoftBlock::NominalVoltage() const {
  return pNominalVoltage;
}
float AnalyzerSoftBlock::VoltageSample(const uint8_t ch, const uint16_t sample) const {
  return pVoltageSamples[pVoltageChannels * sample + ch];
}
float AnalyzerSoftBlock::CurrentSample(const uint8_t ch, const uint16_t sample) const {
  return pCurrentSamples[pCurrentChannels * sample + ch];
}
uint64_t AnalyzerSoftBlock::Sequence() const {
  return pSequence;
}
bool AnalyzerSoftBlock::GetSeriesJSON(const string &serie, json &result) {
  if(serie == "Voltage_Samples") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      json Tmp2 = json::array();
      for(uint16_t i = 0; i < pBlockSamples; i++) Tmp2.push_back(VoltageSample(n, i));
      Tmp1.push_back(Tmp2);
    }
    result["Voltage_Samples"] = Tmp1;
  } else if(serie == "Current_Samples") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) {
      json Tmp2 = json::array();
      for(uint16_t i = 0; i < pBlockSamples; i++) Tmp2.push_back(CurrentSample(n, i));
      Tmp1.push_back(Tmp2);
    }
    result["Current_Samples"] = Tmp1;
  } else {
    return false;
  }
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------