#include <math.h>
#include <string.h>
#include "ZeroCrossing.h"
//ZeroCrossing::ZeroCrossing(uint8_t channels, float samplerate, float freq) {
//  pSamplerate       = samplerate;
//  pNominalFrequency = freq;
//  pMaxSamples       = ceil(15.0 * samplerate / freq);
//  float Q = 1.0;
//  float K = tan(M_PI * (freq / pSamplerate));
//  float Norm = 1 / (1 + K / Q + K * K);
//  pFilters.A0 = K / Q * Norm;
//  pFilters.A1 = 0;
//  pFilters.A2 = -pFilters.A0;
//  pFilters.B1 = 2 * (K * K - 1) * Norm;
//  pFilters.B2 = (1 - K / Q + K * K) * Norm;
//  pLastSamples.resize(channels);
//  pFilters.Channels.resize(channels);
//  pCycles.reserve(ZEROCROSSING_SECONDS * freq);
//  pBlockCycles.reserve(20);
//  pCyclesPos = 0;
//  pBlockLen  = 0;
//  pCycleLen  = 0;
//}
//ZeroCrossing::~ZeroCrossing() {
//}
//bool ZeroCrossing::ProcessSample(const vector<float> &samples) {
//  vector<float> Outs = samples;
//  for(uint8_t i = 0; i < samples.size(); i++) {
//    for(int j = 0; j < ZEROCROSSING_FILTERSTAGES; j++) {
//      float Out = Outs[i] * pFilters.A0 + pFilters.Channels[i].Z1[j];
//      pFilters.Channels[i].Z1[j] = Outs[i] * pFilters.A1 + pFilters.Channels[i].Z2[j] - pFilters.B1 * Out;
//      pFilters.Channels[i].Z2[j] = Outs[i] * pFilters.A2 - pFilters.B2 * Out;
//      Outs[i] = Out;
//    }
//  }
//  if(pBlockCycles.size() == pBlockCycles.capacity()) {
//    pBlockCycles.clear();
//    pBlockLen = 0;
//  }
//  pCycleLen++;
//  pBlockLen++;
//  if((((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) && (pBlockLen < pMaxSamples)) {
//    pBlockCycles.push_back(pCycleLen);
//    if(pCycles.size() < pCycles.capacity()) {
//      pCycles.push_back(pCycleLen);
//    } else {
//      pCycles[pCyclesPos] = pCycleLen;
//      pCyclesPos = (pCyclesPos + 1) % pCycles.capacity();
//    }
//    pCycleLen = 0;
//  }
//  pLastSamples = Outs;
//  return (pBlockCycles.size() == pBlockCycles.capacity());
//}
//float ZeroCrossing::Frequency() {
//  uint32_t Samples = 0;
//  uint32_t MinCycle = pSamplerate / (pNominalFrequency * 1.15 * 2.0);
//  uint32_t MaxCycle = pSamplerate / (pNominalFrequency * 0.85 * 2.0);
//  for(uint i = 0; i < pCycles.size(); i++) {
//    if((pCycles[i] < MinCycle) || (pCycles[i] > MaxCycle)) return NAN;
//    Samples += pCycles[i];
//  } 
//  return (Samples / pSamplerate) * 10.0;
//}