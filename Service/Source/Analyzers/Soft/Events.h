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
#include <vector>
#include <deque>
#include <list>
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
#define EVENT_RVC                        5.0
#define EVENT_DIP_SWELL                 10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION              90.0        //Bajada en % para generar interrupcion
#define EVENT_INTERRUPTION_HYSTERESIS   40.0        //Interruption hysteresis
#define EVENT_HYSTERESIS                02.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_RAWENVELOPETIME            0.5        //Time in seconds before and after event to store RAW samples
#define EVENT_RMSENVELOPETIME           10.0        //Time in seconds before and after event to store RMS samples
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft::Events final {
  private :
    enum EventType { EVT_NONE = 0x00, EVT_SWELL = 0x01, EVT_DIP = 0x02, EVT_INTERRUPTION = 0x04, EVT_RVC = 0x08 };
    enum EventState { EVT_RUNNING, EVT_CLOSING, EVT_CLOSED };
  private :
    typedef struct {
      vector<float> RMS;
      uint64_t      Timestamp;
    } EventCycle_t;
    typedef struct {
      enum EventType                 Type;                 //Running event type
      EventState                     State;                //Events state
      uint64_t                       Start;                //Event start timestamp
      uint64_t                       Duration;             //Event duration in samples
      vector<float>                  Reference;            //Voltage reference for event (Stationary for RVC, nominal for others)
      vector<float>                  PeakValue;            //max/min voltage variation along event
      vector<float>                  ChangeValue;          //diference voltage of event
      deque<vector<float> >          SamplesRAW_A;         //
      deque<vector<float> >          SamplesRMS_A;         //
      deque<vector<float> >          SamplesRAW_B;         //
      deque<vector<float> >          SamplesRMS_B;         //
    } Event_t;
  private :
    AnalyzerSoft              *pParent;                  //Parent analyzer
    pthread_t                  pEventThread;             //Hilo de ejecucion
    WorkerLocks                pWorkerLocks;             //Control access to event detection thread
    deque<vector<float> >      pPrevSamples;             //Previous samples
    deque<EventCycle_t>        pPrevCycles;              //Previous cycles
    bool                       pPrevStationary;          //Steady state flag
    list<Event_t>              pEvents;                  //Running events
    bool                       pTerminate;               //Signal to terminate
  private :
    static void *Events_Thread(void *args);
  private :
    void  SaveEvent(Event_t &evt);
    void  InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos);
    bool  FindRunningEvent(const uint32_t mask);
  public :
    Events(AnalyzerSoft *parent);
    ~Events();
    void GetEvents(const json &params, Response &response);
    void GetEvent(const json &params, Response &response);
    void SetEvent(const json &params, Response &response);
    void DelEvent(const json &params, Response &response);
    void Start();
    void Wait();
};