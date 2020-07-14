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
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftZeroCrossing {
  private :
    typedef struct {
      float                    A0;
      float                    A1;
      float                    A2;
      float                    B1;
      float                    B2;
      float                    ChannelsZ1[3];
      float                    ChannelsZ2[3];
    } Filter_t;
  public :
    AnalyzerSoftZeroCrossing(const AnalyzerSoft::AnalyzerMode mode, const float samplerate, const float nominal);
    void Clear();
    bool PushSample(float *sample);
};
// ---------------------------------------------------------------------------------------------------------------------