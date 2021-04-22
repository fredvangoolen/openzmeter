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
#include "AnalyzerSoftZeroCrossing.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftZeroCrossing::AnalyzerSoftZeroCrossing(const Analyzer::AnalyzerMode mode, const float samplerate) : pSamplerate(samplerate),
  pVoltageChannels((mode == Analyzer::PHASE1) ? 1 : 3), pCurrentChannels((mode == Analyzer::PHASE1) ? 1 : (mode == Analyzer::PHASE3) ? 3 : 4) {
  LogFile::Debug("Starting AnalyzerSoftZeroCrosing...");
  pWorkingBlock      = NULL;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pDelayVoltage      = NULL;
  pDelayCurrent      = NULL;
  pDelaySize         = 0;
  pSequence          = 0;
  pCycleLen          = 0;
  pDelayPos          = 0;
  pMaxCycles         = 1;
  pMaxSamples        = pSamplerate * 0.2;
  memset(pFilters, 0, sizeof(pFilters));
  LogFile::Debug("AnalyzerSoftZeroCrosing started");
}
AnalyzerSoftZeroCrossing::~AnalyzerSoftZeroCrossing() {
  LogFile::Debug("Stoping AnalyzerSoftZeroCrosing...");
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  if(pWorkingBlock != NULL) delete pWorkingBlock;
  LogFile::Debug("AnalyzerSoftZeroCrosing stoped");
}
void AnalyzerSoftZeroCrossing::Config(const float nominalVolt, const float nominalFreq) {
  LogFile::Debug("Configuring AnalyzerSoftZeroCrosing...");
  pthread_mutex_lock(&pMutex);
  pNominalVoltage   = nominalVolt;
  if(pNominalFrequency != nominalFreq) {
    if(pWorkingBlock != NULL) delete pWorkingBlock;
    pWorkingBlock     = NULL;
    if(pDelayVoltage != NULL) free(pDelayVoltage);
    pDelayVoltage     = NULL;
    if(pDelayCurrent != NULL) free(pDelayCurrent);
    pDelayCurrent     = NULL;
    pSequence         = 0;
    pDelayPos         = 0;
    pCycleLen         = 0;
    pNominalFrequency = nominalFreq;
    pMaxCycles        = (pNominalFrequency == 0.0) ? 1 : (pNominalFrequency == 50.0) ? 20 : 24;
    pMaxSamples       = (pNominalFrequency == 0.0) ? pSamplerate * 0.2 : (pMaxCycles * pSamplerate * 1.15 * 0.5) / pNominalFrequency;
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
      pDelaySize = ((pSamplerate / pNominalFrequency) / 2.0) * FILTERSTAGES;
      pDelayVoltage = (float*)calloc(sizeof(float), pDelaySize * pVoltageChannels);
      pDelayCurrent = (float*)calloc(sizeof(float), pDelaySize * pCurrentChannels);
    }
    memset(pLastSamples, 0, sizeof(pLastSamples));
  }
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("AnalyzerSoftZeroCrosing configured");
}
AnalyzerSoftBlock *AnalyzerSoftZeroCrossing::PushSample(float *voltageSamples, float *currentSamples) {
  pthread_mutex_lock(&pMutex);
  if(pWorkingBlock == NULL) {
    pWorkingBlock = new AnalyzerSoftBlock(pVoltageChannels, pCurrentChannels, pSequence, pMaxSamples, pNominalVoltage, pNominalFrequency);
    pSequence++;
  }
  if(pNominalFrequency == 0) {
    pWorkingBlock->PushSample(voltageSamples, currentSamples);
  } else {
    pWorkingBlock->PushSample(&pDelayVoltage[pVoltageChannels * pDelayPos], &pDelayCurrent[pCurrentChannels * pDelayPos]);
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
    pCycleLen++;
    if((((pLastSamples[0] >= 0.0) && (voltageSamples[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (voltageSamples[0] > 0.0))) && (pCycleLen > ((pSamplerate * 0.85) / (2.0 * pNominalFrequency)))) {
      pWorkingBlock->PushCycle(pCycleLen);
      pCycleLen = 0;
    }
    memcpy(pLastSamples, voltageSamples, sizeof(pLastSamples));
  }
  AnalyzerSoftBlock *Result = NULL;
  if((pWorkingBlock->Samples() == pMaxSamples) || (pWorkingBlock->Cycles() == pMaxCycles)) {
    Result = pWorkingBlock;
    pWorkingBlock = NULL;
  }
  pthread_mutex_unlock(&pMutex);
  return Result;
}
// ---------------------------------------------------------------------------------------------------------------------