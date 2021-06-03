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
//----------------------------------------------------------------------------------------------------------------------
class FilterDC final {
  private :
    const uint8_t  pChannels;       //Channels count
  private :
    float          pCoeff;          //Filter coeff
    float         *pZ1;             //DC block filter coeff
    float         *pZ2;             //DC block filter coeff
  public :
    FilterDC(const uint8_t channels, const float samplerate, const float setTime);
    ~FilterDC();
    void Push(float *values);
};
//----------------------------------------------------------------------------------------------------------------------