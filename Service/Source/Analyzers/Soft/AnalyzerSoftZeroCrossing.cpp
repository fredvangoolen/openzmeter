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
AnalyzerSoftZeroCrossing::AnalyzerSoftZeroCrossing(const Analyzer::AnalyzerMode mode, const float samplerate) : pSamplerate(samplerate), pVoltageChannels(Analyzer::VoltageChannels(mode)), pCurrentChannels(Analyzer::CurrentChannels(mode)) {
  LogFile::Debug("Starting AnalyzerSoftZeroCrosing...");
  pWorkingBlock      = NULL;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pDelayVoltage      = NULL;
  pDelayCurrent      = NULL;
  pFilter            = NULL;
  pDelaySize         = 0;
  pSequence          = 0;
  pCycleLen          = 0;
  pDelayPos          = 0;
  pMaxCycles         = 1;
  pMaxSamples        = pSamplerate * 0.2;
  pNominalVoltage    = 230.0;
  pNominalFrequency  = 0;
  LogFile::Debug("AnalyzerSoftZeroCrosing started");
}
AnalyzerSoftZeroCrossing::~AnalyzerSoftZeroCrossing() {
  LogFile::Debug("Stoping AnalyzerSoftZeroCrosing...");
  if(pFilter != NULL) delete pFilter;
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
    if(pFilter != NULL) delete pFilter;
    pFilter           = NULL;
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
      pFilter = new FilterBiquad(1, 5, pSamplerate, pNominalFrequency, pNominalFrequency * 0.1);
      pDelaySize = ((pSamplerate / pNominalFrequency) / 2.0) * FILTERSTAGES;
      pDelayVoltage = (float*)calloc(sizeof(float), pDelaySize * pVoltageChannels);
      pDelayCurrent = (float*)calloc(sizeof(float), pDelaySize * pCurrentChannels);
    }
    pLastSample = 0.0;
  }
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("AnalyzerSoftZeroCrosing configured");
}
AnalyzerSoftBlock *AnalyzerSoftZeroCrossing::PushSample(const float *voltageSamples, const float *currentSamples) {
  pthread_mutex_lock(&pMutex);
  if(pWorkingBlock == NULL) {
    pWorkingBlock = new AnalyzerSoftBlock(pVoltageChannels, pCurrentChannels, pSequence, pMaxSamples, pNominalVoltage, pNominalFrequency);
    pSequence++;
  }
  if(pNominalFrequency == 0) {
    pWorkingBlock->PushSample(voltageSamples, currentSamples);
  } else {
    float VoltageSample;
    pWorkingBlock->PushSample(&pDelayVoltage[pVoltageChannels * pDelayPos], &pDelayCurrent[pCurrentChannels * pDelayPos]);
    memcpy(&pDelayVoltage[pDelayPos * pVoltageChannels], voltageSamples, sizeof(float) * pVoltageChannels);
    memcpy(&pDelayCurrent[pDelayPos * pCurrentChannels], currentSamples, sizeof(float) * pCurrentChannels);
    pDelayPos = (pDelayPos + 1) % pDelaySize;
    VoltageSample = voltageSamples[0];
    pFilter->Push(&VoltageSample);
    pCycleLen++;
    if(((pLastSample >= 0.0) && (VoltageSample < 0.0)) || ((pLastSample <= 0.0) && (VoltageSample > 0.0))) {
      pWorkingBlock->PushCycle(pCycleLen);
      pCycleLen = 0;
    }
    pLastSample = VoltageSample;
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