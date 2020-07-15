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
#include "Worker.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftFFT final {
  public :
    typedef struct {
      float Re;
      float Im;
    } Complex_t;
  private :
    static pthread_mutex_t     pFFTMutex;                //Planer mutex
    static uint32_t            pFFTRunning;              //Currently running FFTs
  private :
    Worker                    *pWorker;
    const float                     *pInPtr;
    Complex_t                 *pOutPtr;
    uint32_t                   pInLen;
    uint8_t                    pCycles;
//    uint32_t                   pOutLen;
//    uint32_t                   pReservedLen;
  private :
    void FFT_Func();
  public :
   static float CalcAngle(const Complex_t &a, const Complex_t &b);
  public :
    AnalyzerSoftFFT(const string &name);
    ~AnalyzerSoftFFT();
    void      Config(const float *src, Complex_t *dst, const uint8_t inCycles);
    void      Run(const uint32_t samples);
    void      Wait();
    Complex_t Harmonic(const uint8_t index) const;
    Complex_t Bin(const uint32_t index) const;
    float     HarmonicPower(const uint8_t index) const;
    float     BinPower(const uint32_t index) const;
};
// ---------------------------------------------------------------------------------------------------------------------