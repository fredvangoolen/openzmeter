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
#include <stdint.h>
// ---------------------------------------------------------------------------------------------------------------------
class FilterBandPass final {
  private :
    typedef struct {
      float  CA0;
      float  CA1;
      float  CA2;
      float  CB0;
      float  CB1;
      float  CB2;
      float *Z1;
      float *Z2;
    } Stage_t;
  private :
    const uint8_t  pChannels;       //Channels count
    const uint8_t  pStageCount;
  private :
    Stage_t *pStages;
  public :
    FilterBandPass(const uint8_t channels, const uint8_t order, const float samplerate, const float freq, const float bandwidth);
    ~FilterBandPass();
    void Push(float *values);
};
// ---------------------------------------------------------------------------------------------------------------------