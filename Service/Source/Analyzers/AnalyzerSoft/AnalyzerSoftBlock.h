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
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftBlock {
  public :
    enum AppendResult { NONE, FULL_CYCLE, FULL_BLOCK};
  private :
    static const uint8_t FILTERSTAGES =  5; //Filter stages for zerocrossing
  private :
    typedef struct {
      float  A0;
      float  A1;
      float  A2;
      float  B1;
      float  B2;
      float  ChannelsZ1[3];
      float  ChannelsZ2[3];
    } Filter_t;
  private :
    Filter_t  pFilters[FILTERSTAGES];       //Filter bank
    uint8_t   pVoltageChannels;             //Number of voltage channels
    uint8_t   pCurrentChannels;             //Number of current channels
    uint8_t   pBlockCyclesCount;            //Block cycles count
    uint8_t   pMaxCycles;                   //Allowed cicles for configured frequency
    uint16_t  pMaxSamples;                  //Max samples for configured frequency
    uint16_t  pBlockSamples;                //Number of samples in block
    uint16_t  pDelaySize;                   //Delay samples
    uint16_t  pDelayPos;                    //Delay pos
    uint16_t  pBlockCycles[24];             //Max block sizes
    float    *pVoltageSamples;              //Output voltage signal
    float    *pCurrentSamples;              //Output current signal
    float    *pDelayVoltage;                //Delay line for voltage
    float    *pDelayCurrent;                //Delay line for current
    float     pLastSamples[3];              //Filter samples backup
    float     pSamplerate;                  //Velocidad de muestreo
    float     pNominalFrequency;            //Configured grid frequency
    bool      pNextReset;                   //Reset on next sample flag
  public :
    AnalyzerSoftBlock(const Analyzer::AnalyzerMode mode, const float samplerate);
    ~AnalyzerSoftBlock();
    void      Config(const float nominalFreq);
    bool      PushSample(float *voltageSamples, float *currentSamples);
    uint16_t  BlockSamples() const;
    uint8_t   Cycles() const;
    uint16_t  CycleLen(const uint8_t cycle) const;
    float     VoltageSample(const uint8_t ch, const uint16_t sample) const;
    float     CurrentSample(const uint8_t ch, const uint16_t sample) const;
    bool      GetSeriesNow(const string &serie, json &result);
};
// ---------------------------------------------------------------------------------------------------------------------