#include <math.h>
#include <string.h>

#include "ZeroCrossing.h"
//#include "Capture.h"
//#include "Tools.h"
//  pPrevCyclesLenght = 0;
//  for(int i = 0; i < ANALYZER_FREQCYCLES; i++) {
//    uint32_t Samples = (pMaxSamples * i / 100);
//    pLastCycles[i] = Samples - pLastCyclesLenght;
//    pLastCyclesLenght += pLastCycles[i]; 
//  }
//  pPrevCyclesPos    = 0;

//// ceil(12.0 * pSamplerate / Devices::GetNominalFrequency());
//  pSamples = 0;
//  pVoltageSamples = voltageSamples;
//  pVoltageFFT     = voltageFFT;
//  pCurrentSamples = currentSamples;
//  pCurrentFFT     = currentFFT;
//  if(cycles == 20) {
//    for(int i = 0; i < cycles; i++) {
//      pPrevCyclesLenght = pPrevCyclesLenght - pPrevCycles[pPrevCyclesPos];
//      pPrevCycles[pPrevCyclesPos] = cyclesLen[i];
//      pPrevCyclesLenght = pPrevCyclesLenght + pPrevCycles[pPrevCyclesPos];
//      pPrevCyclesPos = (pPrevCyclesPos + 1) % ANALYZER_FREQCYCLES;
//    }
//    pAggreg10C.Frequency = ANALYZER_FREQCYCLES / (pPrevCyclesLenght / pSamplerate) / 2.0;
//  } else {
//    for(uint8_t i = 0; i < cycles; i++) pSamples = pSamples + cyclesLen[i];
//    pAggreg10C.Frequency = NAN;
//  }
ZeroCrossing::ZeroCrossing(uint8_t channels, float samplerate, float freq) {
  pSamplerate = samplerate;
  pChannels = channels;
  pNominalFrequency = freq;
  float Q = 1.0;
  float K = tan(M_PI * (freq / pSamplerate));
  float Norm = 1 / (1 + K / Q + K * K);
  pFilters.A0 = K / Q * Norm;
  pFilters.A1 = 0;
  pFilters.A2 = -pFilters.A0;
  pFilters.B1 = 2 * (K * K - 1) * Norm;
  pFilters.B2 = (1 - K / Q + K * K) * Norm;
  pLastSamples = (float*)calloc(sizeof(float), channels);
  pFilters.Channels = (FilterStage_t*)calloc(sizeof(FilterStage_t), channels);
  pCyclesCount = ZEROCROSSING_SECONDS * freq;
  pCyclesLen = (uint32_t*)calloc(sizeof(uint32_t), pCyclesCount);
  pCyclesPos = 0;
  pFrequency = NAN;
}
ZeroCrossing::~ZeroCrossing() {
  free(pLastSamples);
  free(pFilters.Channels);
  free(pCyclesLen);
}
bool ZeroCrossing::ProcessSample(float *samples) {
  float Outs[3];
  for(uint8_t i = 0; i < pChannels; i++) {
    Outs[i] = samples[i];
    for(int j = 0; j < ZEROCROSSING_FILTERSTAGES; j++) {
      float Out = Outs[i] * pFilters.A0 + pFilters.Channels[i].Z1[j];
      pFilters.Channels[i].Z1[j] = Outs[i] * pFilters.A1 + pFilters.Channels[i].Z2[j] - pFilters.B1 * Out;
      pFilters.Channels[i].Z2[j] = Outs[i] * pFilters.A2 - pFilters.B2 * Out;
      Outs[i] = Out;
    }
  }
  pCyclesLen[pCyclesPos]++;
  if(((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) {
    for(int i = 0; i < 3; i++) pLastSamples[i] = Outs[i];
    pCyclesPos = (pCyclesPos + 1) % pCyclesCount;
    uint32_t Samples = 0;
    uint32_t MinCycle = pSamplerate / (pNominalFrequency * 1.15 * 2.0);
    uint32_t MaxCycle = pSamplerate / (pNominalFrequency * 0.85 * 2.0);
    for(int i = 0; i < pCyclesCount; i++) {
      if((pCyclesLen[i] < MinCycle) || (pCyclesLen[i] > MaxCycle)) {
        pFrequency = NAN;
        Samples = 0;
        break;
      }
      Samples += pCyclesLen[i];
    } 
    if(Samples > 0) pFrequency = (Samples / pSamplerate) * 10.0;
    pCyclesLen[pCyclesPos] = 0;
    return true;
  } else {
    for(int i = 0; i < 3; i++) pLastSamples[i] = Outs[i];
    return false;
  }
}
float ZeroCrossing::Frequency() {
  return pFrequency;
}