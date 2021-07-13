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
  private :
    const uint64_t  pSequence;                    //Sequence number of analyzed blocks
    const uint8_t   pVoltageChannels;             //Number of voltage channels
    const uint8_t   pCurrentChannels;             //Number of current channels
    const float     pNominalFrequency;            //Configured grid frequency
    const float     pNominalVoltage;              //Configured grid voltage
  private :
    uint64_t        pTimestamp;                   //Current block timestamp
    uint8_t         pBlockCyclesCount;            //Block cycles count
    uint16_t        pBlockSamples;                //Number of samples in block
    uint16_t        pBlockCycles[24];             //Max block sizes
    float          *pVoltageSamples;              //Output voltage signal
    float          *pCurrentSamples;              //Output current signal
  public :
    AnalyzerSoftBlock(const uint8_t vChanels, const uint8_t cChannels, const uint64_t seq, const uint16_t maxLen, const float nominalVolt, const float nominalFreq);
    ~AnalyzerSoftBlock();
    void      PushSample(const float *voltageSamples, const float *currentSamples);
    void      PushCycle(const uint16_t len);
    uint16_t  Samples() const;
    uint8_t   Cycles() const;
    uint16_t  CycleLen(const uint8_t cycle) const;
    uint64_t  Sequence() const;
    float     NominalFrequency() const;
    float     NominalVoltage() const;
    uint64_t  Timestamp() const;
    float     VoltageSample(const uint8_t ch, const uint16_t sample) const;
    float     CurrentSample(const uint8_t ch, const uint16_t sample) const;
    bool      GetSeriesJSON(const string &serie, json &result);
};
// ---------------------------------------------------------------------------------------------------------------------