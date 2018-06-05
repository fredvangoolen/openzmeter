
#include "Devices.h"

//  pPrevCyclesLenght = 0;
//  for(int i = 0; i < ANALYZER_FREQCYCLES; i++) {
//    uint32_t Samples = (pMaxSamples * i / 100);
//    pLastCycles[i] = Samples - pLastCyclesLenght;
//    pLastCyclesLenght += pLastCycles[i]; 
//  }
//  pPrevCyclesPos    = 0;

ZeroCrossing::ZeroCrossing(bool phase3, float samplerate) {
}
bool  ZeroCrossing::ProcessSample(float *samples) {
  return false;
}
float ZeroCrossing::Frequency() {
  Devices::GetNominalFrequency();
}