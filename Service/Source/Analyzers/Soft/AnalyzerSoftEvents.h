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
#include "Worker.h"
#include "AnalyzerSoftBlock.h"
#include "AnalyzerEvents.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftEvents final : public AnalyzerEvents {
  private :
    enum EventType { EVT_SWELL, EVT_DIP, EVT_INTERRUPTION, EVT_RVC };
    enum EventSamples { SAMPLES_RAW_PRESTART = 0x01,
                        SAMPLES_RAW_POSSTART = 0x02,
                        SAMPLES_RAW_PREEND   = 0x04,
                        SAMPLES_RAW_POSEND   = 0x08,
                        SAMPLES_RMS_PRESTART = 0x10,
                        SAMPLES_RMS_POSSTART = 0x20,
                        SAMPLES_RMS_PREEND   = 0x40,
                        SAMPLES_RMS_POSEND   = 0x80
                      };
    typedef struct {
      EventType      Type;                      //Running event type
      uint64_t       Start;                     //Event start timestamp
      uint64_t       SamplesDuration;           //Event duration in samples
      float          Reference[3];              //Voltage reference for event (Stationary for RVC, nominal for others)
      float          PeakValue[3];              //max/min voltage variation along event
      float          ChangeValue[3];            //diference voltage of event
      uint32_t       CyclesDuration;            //Event duration in samples
    } Event_t;
  private :
    static constexpr float Event_RVC        =  5.0;   //Variation in % to start a RVC event
    static constexpr float Event_DIP_SWELL  = 10.0;   //Variation in % to start a dip/swell event
    static constexpr float Event_INT        = 90.0;   //Variation in % to start a interruption
    static constexpr float Event_Hysteresis =  2.0;   //Hysteresis in % to end and event
    static constexpr float Event_RAW_Time   =  0.5;   //Time in seconds before and after event to store RAW samples
  private :
    pthread_mutex_t  pMutex;                    //Configuration lock
    uint8_t          pVoltageChannels;          //Input channels
    float            pSamplerate;               //Input samplerate
    float            pNominalVoltage;           //Nominal voltage
    float            pNominalFrequency;         //Nominal frequency
    Worker          *pWorker;                   //Working thread
    Event_t         *pEvent;                    //Running event
    list<Event_t*>   pEvents;                   //Close pending events
    bool             pPrevStationary;           //Steady state flag
    uint16_t         pPrevSamplesLen;           //Previous samples count
    uint16_t         pPrevSamplesPos;           //Previous samples write pos
    float           *pPrevSamples;              //Previous samples
    uint16_t         pPrevCyclesLen;            //Previous cycles count
    uint16_t         pPrevCyclesPos;            //Previous cycles write pos
    float           *pPrevCycles;               //Previous cycles
    bool             pHaveEvent;                //Flag to block event
  private :
    Event_t *InitEvent();
    void     Events_Func(const AnalyzerSoftBlock *block, const uint64_t timestamp);
    bool     FindRunningEvent(const uint32_t mask) const;
    float    CalcChange(const float in, const float reference) const;
    string   EventTypeToString(const EventType type) const;
    void     SaveSamples(const uint64_t &start, const uint8_t type, const uint16_t len);
  public :
    AnalyzerSoftEvents(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate);
    ~AnalyzerSoftEvents();
    void Config(const bool enabled);
    void Run(const AnalyzerSoftBlock *block, const uint64_t timestamp, const float nominalVoltage, const float nominalFreq);
    bool Wait();

};
// ---------------------------------------------------------------------------------------------------------------------