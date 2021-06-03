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
#include "FilterDC.h"
// ---------------------------------------------------------------------------------------------------------------------
FilterDC::FilterDC(const uint8_t channels, const float samplerate, const float setTime) : pChannels(channels) {
  pZ1  = (float*)calloc(sizeof(float), channels);
  pZ2  = (float*)calloc(sizeof(float), channels);
  pCoeff = ((samplerate * setTime) - 1.0) / (samplerate * setTime);
}
FilterDC::~FilterDC() {
  free(pZ1);
  free(pZ2);
}
void FilterDC::Push(float *values) {
  for(uint8_t Ch = 0; Ch < pChannels; Ch++) {
    float In = values[Ch];
    values[Ch] = In - pZ1[Ch] + 0.99999 * pZ2[Ch];
    pZ1[Ch] = In;
    pZ2[Ch] = values[Ch];
  }
}
// ---------------------------------------------------------------------------------------------------------------------
