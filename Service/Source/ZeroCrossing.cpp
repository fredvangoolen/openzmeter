#include <math.h>
#include <string.h>
#include "Capture.h"
//  pPrevCyclesLenght = 0;
//  for(int i = 0; i < ANALYZER_FREQCYCLES; i++) {
//    uint32_t Samples = (pMaxSamples * i / 100);
//    pLastCycles[i] = Samples - pLastCyclesLenght;
//    pLastCyclesLenght += pLastCycles[i]; 
//  }
//  pPrevCyclesPos    = 0;

ZeroCrossing::ZeroCrossing(bool phase3, float samplerate) {
  pSamplerate = samplerate;
  p3Phase = phase3;
  float Q = 1.0;
  float K = tan(M_PI * (Capture::GetNominalFrequency() / pSamplerate));
  float Norm = 1 / (1 + K / Q + K * K);
  pFilters.A0 = K / Q * Norm;
  pFilters.A1 = 0;
  pFilters.A2 = -pFilters.A0;
  pFilters.B1 = 2 * (K * K - 1) * Norm;
  pFilters.B2 = (1 - K / Q + K * K) * Norm;
  pLastSamples[0] = 20;
  pLastSamples[1] = 20;
  pLastSamples[2] = 20;
  memset(pFilters.Channels, 0, sizeof(pFilters.Channels));  
}
bool ZeroCrossing::ProcessSample(float *samples) {
  float Outs[3];
  for(int i = 0; i < ((p3Phase == true) ? 3 : 1); i++) {
    Outs[i] = samples[i];
    for(int j = 0; j < ZEROCROSSING_FILTERSTAGES; j++) {
      float Out = Outs[i] * pFilters.A0 + pFilters.Channels[i][j].Z1;
      pFilters.Channels[i][j].Z1 = Outs[i] * pFilters.A1 + pFilters.Channels[i][j].Z2 - pFilters.B1 * Out;
      pFilters.Channels[i][j].Z2 = Outs[i] * pFilters.A2 - pFilters.B2 * Out;
      Outs[i] = Out;
    }
  }
  if(((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) {
    for(int i = 0; i < 3; i++) pLastSamples[i] = Outs[i];
    return true;
  } else {
    for(int i = 0; i < 3; i++) pLastSamples[i] = Outs[i];
    return false;
  }
}
float ZeroCrossing::Frequency() {
  Capture::GetNominalFrequency();
}