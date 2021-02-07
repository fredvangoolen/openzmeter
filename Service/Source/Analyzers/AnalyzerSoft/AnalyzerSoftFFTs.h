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
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftBlock;
class AnalyzerSoftFFTs final {
  public :
    typedef struct {
      float Re;
      float Im;
    } Complex_t;
    typedef struct {
      Complex_t           *DataPtr;               //in/out data samples
      uint16_t             ReservedLen;           //Buffer size
    } FFT_t;
  private :
    static pthread_mutex_t pFFTMutex;             //Planer mutex
    static uint32_t        pFFTRunning;           //Currently running FFTs
  private :
    string                 pSerial;               //Parent serial number
    uint8_t                pVoltageChannels;      //Avaliable voltage channels
    uint8_t                pCurrentChannels;      //Avaliable current channels
    Worker                *pVoltageFFT[3];        //Voltage FFT worker
    Worker                *pCurrentFFT[4];        //Current FFT worker
    FFT_t                  pFFTVoltageData[3];    //Worker data
    FFT_t                  pFFTCurrentData[4];    //Worker data
    uint16_t               pInLen;                //Input length
    uint16_t               pOutLen;               //Output length
    uint8_t                pCycles;               //Number of full cycles
  private :
    void  FFT_Func(const FFT_t *data);
    float VoltageBinPower(const uint8_t channel, const uint16_t index) const;
    float CurrentBinPower(const uint8_t channel, const uint16_t index) const;
  public :
   static float CalcAngle(const Complex_t &a, const Complex_t &b);
  public :
    AnalyzerSoftFFTs(const string &serial, const Analyzer::AnalyzerMode mode);
    ~AnalyzerSoftFFTs();
    void      Run(const AnalyzerSoftBlock *block, const uint8_t inCycles);
    void      Wait();
    uint16_t  OutLen() const;
    Complex_t VoltageHarmonic(const uint8_t channel, const uint8_t index) const;
    Complex_t CurrentHarmonic(const uint8_t channel, const uint8_t index) const;
    float     VoltageHarmonicPower(const uint8_t channel, const uint8_t index) const;
    float     CurrentHarmonicPower(const uint8_t channel, const uint8_t index) const;
    bool      GetSeriesJSON(const string &serie, json &result);
};
// ---------------------------------------------------------------------------------------------------------------------