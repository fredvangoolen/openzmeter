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
#pragma once
#include <inttypes.h>
#include "AnalyzerSoftBlock.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftFrequency final {
  private :
    static const uint8_t       SECONDS = 10;             //Used seconds to calc frequency
  private :
    float                      pNominalFrequency;        //Defined nominal frequency
    float                      pSamplerate;              //input samplerate
    uint16_t                   pFreqCyclesPos;           //Frequency cycles write position
    uint16_t                   pFreqCyclesLen;           //Frequency cycles buffer len
    uint16_t                  *pFreqCycles;              //Frequency calc cycles lenghts
    uint16_t                   pInvalidValues;           //Invalid samples in buffer
    uint16_t                   pMinCycle;                //Minimum cycle len
    uint16_t                   pMaxCycle;                //Maximun cycle len
    uint32_t                   pBufferLen;               //Total length of valid cycles
  private :
    void  PushCycle(const uint16_t len);
  public :
    AnalyzerSoftFrequency(const float samplerate);
    ~AnalyzerSoftFrequency();
    void  Config(const float freq);
    float Frequency(const AnalyzerSoftBlock *block);
    float NominalFrequency();
};
// ---------------------------------------------------------------------------------------------------------------------