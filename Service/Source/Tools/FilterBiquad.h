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
#include <complex>
#include <stdint.h>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class FilterBiquad final {
  private :
    const uint8_t  pChannelsCount;       //Channels count
    const uint8_t  pOrder;
  private :
    float     *pA;
    float     *pB;
    float     *pD;
  private :
    complex<float> BilinearTransform(complex<float> &s, float ts);
    void           SetBandPass(const float samplerate, const float freq, const float bandwidth);
    void           SetLowPass(const float samplerate, const float freq);
  public :
    FilterBiquad(const uint8_t channels, const uint8_t order, const float samplerate, const float freq, const float bandwidth);
    FilterBiquad(const uint8_t channels, const uint8_t order, const float samplerate, const float freq);
    ~FilterBiquad();
    void Push(float *values);
};
// ---------------------------------------------------------------------------------------------------------------------