/* ================================== zMeter ===========================================================================
 * File:     Events.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <deque>
#include <list>
#include "Database.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
#define EVENT_RVC                      5.0        //% variation for RVC detection
#define EVENT_DIP_SWELL               10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION            90.0        //Bajada en % para generar interrupcion
#define EVENT_HYSTERESIS              02.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_RAWENVELOPETIME          0.5        //Time in seconds before and after event to store RAW samples
#define EVENT_RMSENVELOPETIME         60.0        //Time in seconds before and after event to store RMS samples
//----------------------------------------------------------------------------------------------------------------------
class Events : public Database {
  private :
    enum EventType  { EVT_NONE         = 0x00,
                      EVT_SWELL        = 0x01, 
                      EVT_DIP          = 0x02,
                      EVT_INTERRUPTION = 0x04, 
                      EVT_RVC          = 0x08
                    };
    typedef struct {
      vector<float> RMS;
      uint64_t      Timestamp;
//      uint32_t      Len;
    } EventCycle_t;
    typedef struct {
      enum EventType                 Type;                 //Running event type
      bool                           Clossing;             //Events ends waiting to fill post samples/cycles to save it
      uint64_t                       Start;                //Event start timestamp
      uint64_t                       Duration;             //Event duration in samples
      vector<float>                  Reference;            //Voltage reference for event (Stationary for RVC, nominal for others)
      vector<float>                  SteadyValue;          //event voltage change (Start to End relation on RVC, null on others)
      vector<float>                  PeakValue;            //max/min voltage variation along event
      vector<vector<float> >         BeginSamples;         //
      vector<vector<float> >         BeginRMS;             //
      vector<vector<float> >         InnerSamples;         //
      vector<vector<float> >         InnerRMS;             //
      vector<vector<float> >         EndSamples;           //
      vector<vector<float> >         EndRMS;               //
    } Event_t;
  private :
    pthread_t                        pThread;              //Hilo de ejecucion
    pthread_mutex_t                  pThreadMutex;         //Mutex para la variable de condicion
    pthread_cond_t                   pThreadCond;          //Variable de condicion para esperar
    bool                             pTerminate;           //Termination flag
    AnalyzerInterface::Params_t     *pParams;              //Input block to analyze
    vector<uint32_t>                *pCycles;              //Input cycles lengths
    AnalyzerInterface               *pAnalyzer;            //Analyzer parent
    float                            pSampleRate;          //Analyzer samplerate
    deque<vector<float> >            pPrevSamples;         //Previous samples
    deque<EventCycle_t>              pPrevCycles;          //Previous cycles
    bool                             pPrevStationary;      //Steady state flag
    list<Event_t>                    pEvents;              //Running events
  private :
    static void *Events_Thread(void *args);
    static float PercentChange(float in, float reference);
  private :
    bool FindRunningEvent(uint32_t mask);
  public :
    void Start(AnalyzerInterface::Params_t *params, vector<uint32_t> *cycles);
    void Wait();
    Events(AnalyzerInterface *interface, float samplerate);
    virtual ~Events();      
};