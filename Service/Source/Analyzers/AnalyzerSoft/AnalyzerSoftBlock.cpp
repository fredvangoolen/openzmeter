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
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftBlock::AnalyzerSoftBlock(const Analyzer::AnalyzerMode mode, const float samplerate) {
  memset(pFilters, 0, sizeof(pFilters));
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pDelayVoltage      = NULL;
  pDelayCurrent      = NULL;
  pVoltageSamples    = NULL;
  pCurrentSamples    = NULL;
  pSequence          = 0;
  pSamplerate        = samplerate;
  if(mode == Analyzer::PHASE1) {
    pVoltageChannels = 1;
    pCurrentChannels = 1;
  } else if(mode == Analyzer::PHASE3) {
    pVoltageChannels = 3;
    pCurrentChannels = 3;
  } else {
    pVoltageChannels = 3;
    pCurrentChannels = 4;
  }
  Config(0.0);
}
AnalyzerSoftBlock::~AnalyzerSoftBlock() {
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  if(pVoltageSamples != NULL) free(pVoltageSamples);
  if(pCurrentSamples != NULL) free(pCurrentSamples);
}
void AnalyzerSoftBlock::Config(const float nominalFreq) {
  pthread_mutex_lock(&pMutex);
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  pDelayVoltage     = NULL;
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  pDelayCurrent     = NULL;
  pSequence         = 0;
  pBlockCyclesCount = 0;
  pBlockSamples     = 0;
  pDelayPos         = 0;
  pNominalFrequency = nominalFreq;
  pMaxCycles        = (pNominalFrequency == 0.0) ? 1 : (pNominalFrequency == 50.0) ? 20 : 24;
  pMaxSamples       = (pNominalFrequency == 0.0) ? pSamplerate * 0.2 : (pMaxCycles * pSamplerate * 1.15 * 0.5) / pNominalFrequency;
  pVoltageSamples   = (float*)realloc(pVoltageSamples, sizeof(float) * pVoltageChannels * pMaxSamples);
  pCurrentSamples   = (float*)realloc(pCurrentSamples, sizeof(float) * pCurrentChannels * pMaxSamples);
  if(pNominalFrequency != 0.0) {
    for(uint8_t n = 0; n < FILTERSTAGES; n++) {
      float w0 = 2 * M_PI * (pNominalFrequency / pSamplerate);
      float alpha = sin(w0) / 2.0;
      pFilters[n].A0 = alpha / (1.0 + alpha);
      pFilters[n].A1 = 0.0;
      pFilters[n].A2 = -alpha / (1.0 + alpha);
      pFilters[n].B1 = (-2.0 * cos(w0)) / (1.0 + alpha);
      pFilters[n].B2 = (1.0 - alpha) / (1.0 + alpha);
      memset(pFilters[n].ChannelsZ1, 0, sizeof(Filter_t::ChannelsZ1));
      memset(pFilters[n].ChannelsZ2, 0, sizeof(Filter_t::ChannelsZ2));
    }
    pDelaySize = ((pSamplerate / pNominalFrequency) / 2.0) * FILTERSTAGES + 2;
    pDelayVoltage = (float*)calloc(sizeof(float), pDelaySize * pVoltageChannels);
    pDelayCurrent = (float*)calloc(sizeof(float), pDelaySize * pCurrentChannels);
  }
  memset(pLastSamples, 0, sizeof(pLastSamples));
  memset(pBlockCycles, 0, sizeof(pBlockCycles));
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerSoftBlock::PushSample(float *voltageSamples, float *currentSamples) {
  if(pthread_mutex_trylock(&pMutex) != 0) return false;
  if((pBlockSamples == pMaxSamples) || (pBlockCyclesCount == pMaxCycles)) {
    memset(pBlockCycles, 0, sizeof(pBlockCycles));
    pBlockSamples     = 0;
    pBlockCyclesCount = 0;
    pSequence++;
  }
  if(pNominalFrequency == 0) {
    memcpy(&pVoltageSamples[pVoltageChannels * pBlockSamples], voltageSamples, sizeof(float) * pVoltageChannels);
    memcpy(&pCurrentSamples[pCurrentChannels * pBlockSamples], currentSamples, sizeof(float) * pCurrentChannels);
  } else {
    memcpy(&pVoltageSamples[pVoltageChannels * pBlockSamples], &pDelayVoltage[pVoltageChannels * pDelayPos], sizeof(float) * pVoltageChannels);
    memcpy(&pCurrentSamples[pCurrentChannels * pBlockSamples], &pDelayCurrent[pCurrentChannels * pDelayPos], sizeof(float) * pCurrentChannels);
    memcpy(&pDelayVoltage[pDelayPos * pVoltageChannels], voltageSamples, sizeof(float) * pVoltageChannels);
    memcpy(&pDelayCurrent[pDelayPos * pCurrentChannels], currentSamples, sizeof(float) * pCurrentChannels);
    pDelayPos = (pDelayPos + 1) % pDelaySize;
    for(uint8_t i = 0; i < pVoltageChannels; i++) {
      for(uint8_t n = 0; n < FILTERSTAGES; n++) {
        float Out = voltageSamples[i] * pFilters[n].A0 + pFilters[n].ChannelsZ1[i];
        pFilters[n].ChannelsZ1[i] = voltageSamples[i] * pFilters[n].A1 + pFilters[n].ChannelsZ2[i] - pFilters[n].B1 * Out;
        pFilters[n].ChannelsZ2[i] = voltageSamples[i] * pFilters[n].A2 - pFilters[n].B2 * Out;
        voltageSamples[i] = Out;
      }
    }
    pBlockCycles[pBlockCyclesCount]++;
    if((((pLastSamples[0] >= 0.0) && (voltageSamples[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (voltageSamples[0] > 0.0))) && (pBlockCycles[pBlockCyclesCount] > ((pSamplerate * 0.85) / (2.0 * pNominalFrequency)))) pBlockCyclesCount++;
    memcpy(pLastSamples, voltageSamples, sizeof(pLastSamples));
    memcpy(voltageSamples, &pVoltageSamples[pVoltageChannels * pBlockSamples], sizeof(float) * pVoltageChannels);
    memcpy(currentSamples, &pCurrentSamples[pCurrentChannels * pBlockSamples], sizeof(float) * pCurrentChannels);
  }
  pBlockSamples++;
  bool Complete = ((pBlockSamples == pMaxSamples) || (pBlockCyclesCount == pMaxCycles));
  pthread_mutex_unlock(&pMutex);
  return Complete;
}
uint16_t AnalyzerSoftBlock::BlockSamples() const {
  return pBlockSamples;
}
uint8_t AnalyzerSoftBlock::Cycles() const {
  return pBlockCyclesCount;
}
uint16_t AnalyzerSoftBlock::CycleLen(const uint8_t cycle) const {
  return pBlockCycles[cycle];
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
  if(serie == "VOLTAGE_SAMPLES") {
    json Tmp1 = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      json Tmp2 = json::array();
      for(uint16_t i = 0; i < pBlockSamples; i++) Tmp2.push_back(VoltageSample(n, i));
      Tmp1.push_back(Tmp2);
    }
    result["Voltage_Samples"] = Tmp1;
  } else if(serie == "CURRENT_SAMPLES") {
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