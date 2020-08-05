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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "AnalyzerSoftFrequency.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftFrequency::AnalyzerSoftFrequency(const float samplerate) {
 pNominalFrequency = 0.0;
 pSamplerate       = 10000.0;
 pFreqCyclesPos    = 0;
 pFreqCyclesLen    = 0;
 pInvalidValues    = 0;
 pSamplerate       = samplerate;
 pFreqCycles       = NULL;
 pMinCycle         = 0;
 pMaxCycle         = 0;
 pBufferLen        = 0;
}
AnalyzerSoftFrequency::~AnalyzerSoftFrequency() {
  if(pFreqCycles != NULL) free(pFreqCycles);
}
void AnalyzerSoftFrequency::Config(const float freq) {
  pFreqCyclesPos    = 0;
  pFreqCyclesLen    = SECONDS * freq;
  pNominalFrequency = freq;
  pInvalidValues    = SECONDS * freq;
  pMinCycle         = pSamplerate / (freq * 1.15 * 2.0);
  pMaxCycle         = pSamplerate / (freq * 0.85 * 2.0);
  if(pFreqCyclesLen != 0) {
    pFreqCycles = (uint16_t*)realloc(pFreqCycles, pFreqCyclesLen * sizeof(uint16_t));
    memset(pFreqCycles, 0, SECONDS * freq * sizeof(uint16_t));
  } else {
    if(pFreqCycles != NULL) free(pFreqCycles);
    pFreqCycles = NULL;
  }
}
void AnalyzerSoftFrequency::PushCycle(const uint16_t len) {
  if(pNominalFrequency == 0) return;
  if(pFreqCycles[pFreqCyclesPos] == 0) {
    pInvalidValues--;
  } else {
    pBufferLen = pBufferLen - pFreqCycles[pFreqCyclesPos];
  }
  if((len >= pMinCycle) && (len <= pMaxCycle)) {
    pFreqCycles[pFreqCyclesPos] = len;
    pBufferLen = pBufferLen + len;
  } else {
    pInvalidValues++;
    pFreqCycles[pFreqCyclesPos] = 0;
  }
  pFreqCyclesPos = (pFreqCyclesPos + 1) % pFreqCyclesLen;
}
float AnalyzerSoftFrequency::Frequency(const AnalyzerSoftBlock *block) {
  if(block != NULL) {
    for(uint8_t n = 0; n < block->Cycles(); n++) PushCycle(block->CycleLen(n));
  }
  return (pNominalFrequency == 0) ? 0.0 : (pInvalidValues != 0) ? NAN : pFreqCyclesLen / ((2.0 * pBufferLen) / pSamplerate);
}
// ---------------------------------------------------------------------------------------------------------------------