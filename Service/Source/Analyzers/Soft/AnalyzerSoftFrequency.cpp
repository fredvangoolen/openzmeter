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
  pSamplerate       = samplerate;
  pFreqCycles       = NULL;
  pNominalFrequency = 0.0;
  pBufferLen        = 0;
  pFreqCyclesPos    = 0;
  pFreqCyclesLen    = 0;
  pCyclesCount      = 0;
}
AnalyzerSoftFrequency::~AnalyzerSoftFrequency() {
  if(pFreqCycles != NULL) free(pFreqCycles);
}
void AnalyzerSoftFrequency::PushCycle(const uint16_t len) {
  if(pNominalFrequency == 0) return;
  pBufferLen = pBufferLen - pFreqCycles[pFreqCyclesPos] + len;
  pFreqCycles[pFreqCyclesPos] = len;
  pFreqCyclesPos = (pFreqCyclesPos + 1) % pFreqCyclesLen;
  if(pCyclesCount < pFreqCyclesLen) pCyclesCount++;
}
float AnalyzerSoftFrequency::Frequency(const AnalyzerSoftBlock *block) {
  if(block->NominalFrequency() != pNominalFrequency) {
    pBufferLen        = 0;
    pFreqCyclesPos    = 0;
    pNominalFrequency = block->NominalFrequency();
    pFreqCyclesLen    = SECONDS * pNominalFrequency * 2;
    pCyclesCount      = 0;
    if(pFreqCyclesLen != 0) {
      pFreqCycles = (uint16_t*)realloc(pFreqCycles, pFreqCyclesLen * sizeof(uint16_t));
      memset(pFreqCycles, 0, pFreqCyclesLen * sizeof(uint16_t));
    } else {
      if(pFreqCycles != NULL) free(pFreqCycles);
      pFreqCycles = NULL;
    }
  }
  for(uint8_t n = 0; n < block->Cycles(); n++) PushCycle(block->CycleLen(n));
  float Val = (pNominalFrequency == 0) ? 0.0 : pCyclesCount / (2.0 * pBufferLen / pSamplerate);
  return Val;
}
// ---------------------------------------------------------------------------------------------------------------------