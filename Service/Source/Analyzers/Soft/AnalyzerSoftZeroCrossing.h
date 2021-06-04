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
#include <pthread.h>
#include "FilterBandPass.h"
#include "Analyzer.h"
#include "AnalyzerSoftBlock.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftZeroCrossing final {
  private :
    static const uint8_t FILTERSTAGES = 5; //Filter stages for zerocrossing
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
    const float        pSamplerate;                 //Velocidad de muestreo
    const uint8_t      pVoltageChannels;            //Number of voltage channels
    const uint8_t      pCurrentChannels;            //Number of current channels
  private :
    pthread_mutex_t    pMutex;                      //Configuration mutex
    uint64_t           pSequence;                   //Sequence number of analyzed blocks
    Filter_t           pFilters[FILTERSTAGES];      //Filter bank
    uint8_t            pMaxCycles;                  //Allowed cicles for configured frequency
    uint16_t           pMaxSamples;                 //Max samples for configured frequency
    uint16_t           pDelaySize;                  //Delay samples
    uint16_t           pDelayPos;                   //Delay pos
    uint16_t           pCycleLen;                   //Current cicle length
    float             *pDelayVoltage;               //Delay line for voltage
    float             *pDelayCurrent;               //Delay line for current
    float              pLastSamples[3];             //Filter samples backup
    float              pNominalFrequency;           //Configured grid frequency
    float              pNominalVoltage;             //Configured grid voltage
    AnalyzerSoftBlock *pWorkingBlock;               //Current filling block
  public :
    AnalyzerSoftZeroCrossing(const Analyzer::AnalyzerMode mode, const float samplerate);
    ~AnalyzerSoftZeroCrossing();
    void               Config(const float nominalVolt, const float nominalFreq);
    AnalyzerSoftBlock *PushSample(const float *voltageSamples, const float *currentSamples);
};
// ---------------------------------------------------------------------------------------------------------------------