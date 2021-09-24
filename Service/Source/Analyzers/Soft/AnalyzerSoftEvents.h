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
#include <list>
#include <inttypes.h>
#include "TaskPool.h"
#include "AnalyzerSoftBlock.h"
#include "AnalyzerEvents.h"
#include "AnalyzerSoftCyclicHistogram.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftEvents final : public AnalyzerEvents {
  private :
    enum EventType { EVT_NONE, EVT_SWELL, EVT_DIP, EVT_INTERRUPTION, EVT_RVC };
    enum EventSamples { SAMPLES_RAW, SAMPLES_RMS };
    typedef struct {
      EventType      Type;                      //Running event type
      uint64_t       Start;                     //Event start timestamp
      uint64_t       SamplesDuration;           //Event duration in samples
      float          Reference[3];              //Voltage reference for event (Stationary for RVC, nominal for others)
      float          PeakValue[3];              //max/min voltage variation along event
      float          ChangeValue[3];            //diference voltage of event
      uint32_t       CyclesDuration;            //Event duration in samples
    } Event_t;
    typedef struct {
      bool  Enabled;
      float Threshold;
      float Hysteresis;
    } Levels_t;
  private :
    const float      pSamplerate;               //Input samplerate
    const uint8_t    pVoltageChannels;          //Input channels
  private :
    pthread_mutex_t  pMutex;                    //Configuration lock
    float            pNominalVoltage;           //Nominal voltage
    float            pNominalFrequency;         //Nominal frequency
    bool             pEnabled;                  //Events detection enabled
    TaskPool::Task   pTask;                     //Working thread
    Levels_t         pRVC;                      //RVC event config
    Levels_t         pDip;                      //Dip event config
    Levels_t         pSwell;                    //Swell event config
    Levels_t         pInterruption;             //Interruption event config
    Event_t         *pEvent;                    //Running event
    list<Event_t*>   pEvents;                   //Close pending events
    bool             pPrevStationary;           //Steady state flag
    uint16_t         pPrevSamplesLen;           //Previous samples count
    uint16_t         pPrevSamplesPos;           //Previous samples write pos
    float           *pPrevSamples;              //Previous samples
    uint16_t         pPrevCyclesSaveLen;        //Previous cycles to save
    uint16_t         pPrevCyclesAvgLen;         //Previous cycles for RVC calc
    uint16_t         pPrevCyclesLen;            //Previous cycles count
    uint16_t         pPrevCyclesPos;            //Previous cycles write pos
    float           *pPrevCycles;               //Previous cycles
    bool             pHaveEvent;                //Flag to block event
    AnalyzerSoftCyclicHistogram *pCyclicHistogram; //Cyclic histogram
  private :
    void     Events_Func(const AnalyzerSoftBlock *block);
    float    CalcChange(const float in, const float reference) const;
    string   EventTypeToString(const EventType type) const;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftEvents(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate, AnalyzerSoftCyclicHistogram *cyclicHistogram);
    ~AnalyzerSoftEvents();
    void Config(const json &config, const float nominalVoltage, const float nominalFreq);
    void Run(const AnalyzerSoftBlock *block);
    bool Wait();
};
// ---------------------------------------------------------------------------------------------------------------------