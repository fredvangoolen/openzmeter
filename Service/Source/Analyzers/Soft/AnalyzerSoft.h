/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <deque>
#include <list>
#include "Analyzers.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
#define ZEROCROSSING_FILTERSTAGES          5
#define ZEROCROSSING_SECONDS              10        //Used seconds to calc frequency
#define EVENT_RVC                        5.0        //% variation for RVC detection
#define EVENT_DIP_SWELL                 10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION              90.0        //Bajada en % para generar interrupcion
#define EVENT_HYSTERESIS                02.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_RAWENVELOPETIME            0.5        //Time in seconds before and after event to store RAW samples
#define EVENT_RMSENVELOPETIME           10.0        //Time in seconds before and after event to store RMS samples
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : public Analyzer, private Database {
  private :
    enum EventType { EVT_NONE = 0x00, EVT_SWELL = 0x01, EVT_DIP = 0x02, EVT_INTERRUPTION = 0x04, EVT_RVC = 0x08 };
    enum EventState { EVT_RUNNING, EVT_CLOSING, EVT_CLOSED };
    enum AggregType { UNINITIALIZED, AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
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
    typedef struct {
      AggregType                     Type;                 //Aggregation type
      float                          Frequency;            //Frecuencia
      bool                           Flag;                 //Periodo marcado
      uint64_t                       Timestamp;            //Aggregation timestampt
      uint16_t                       Voltage_Inputs;       //Valid RMS_V samples in aggregation
      uint16_t                       Frequency_Inputs;     //Valid Freq samples in aggregation
      vector<uint16_t>               StatsFrequency;       //Distribucion de frecuencias
      vector<uint16_t>               StatsVoltage;         //Distribucion de tensiones
      vector<uint16_t>               StatsTHD;             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      vector<uint16_t>               StatsHarmonics;       //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector<uint16_t>               Current_Inputs;       //Valid RMS_I samples in aggregation
      vector<float>                  Voltage;              //Aggregation average RMS voltage
      vector<float>                  Current;              //Aggregation average RMS current
      vector<float>                  Active_Power;         //Aggregation average Active Power (P)
      vector<float>                  Reactive_Power;       //Reactive power
      vector<float>                  Active_Energy;        //Energia consumida
      vector<float>                  Reactive_Energy;      //Reactive energy
      vector<float>                  Apparent_Power;       //Potencia aparente (S)
      vector<float>                  Power_Factor;         //Factor de potencia (cos(PHI))
      vector<float>                  Voltage_THD;          //Distorsion harmonica de la tension
      vector<float>                  Current_THD;          //Distorsion harmonica de la corriente
      vector<float>                  Voltage_Phase;        //Angulo entre canales de tension
      vector<float>                  Phi;                  //Angulo de Phi
      vector< vector<float> >        Voltage_Harmonics;    //Armonicos del canal de tension
      vector< vector<float> >        Current_Harmonics;    //Armonicos del canal de corriente
    } Params_t;
//    typedef struct {
//      char  *Buffer;
//      size_t Size;
//      size_t MaxSize;
//    } CurlBuff_t;
    typedef struct {
      float                 A0;
      float                 A1;
      float                 A2;
      float                 B1;
      float                 B2;
      vector<float>         ChannelsZ1;
      vector<float>         ChannelsZ2;
    } Filter_t;
  private :
    vector< vector<float> >    pVoltageSamples;          //Voltage samples
    vector< vector<float> >    pCurrentSamples;          //Current samples
    float                      pNominalVoltage;          //Analyzer nominal voltage
    float                      pNominalFrequency;        //Analyzer nominal frequency
    string                     pCountryCode;             //Country code
    string                     pContract;                //Contract identifier
    string                     pZipCode;                 //Installation zip code
    vector<float>              pGeo;                  //GPS position
    float                      pMaxCurrent;              //Max current, over this value warnings are shown
    float                      pMinCurrent;              //Min current, under this value some analysis are disabled
    float                      pSamplerate;              //Velocidad de muestreo (NaN if unsuported)
    AnalyzerMode               pMode;                    //Operation mode
    uint64_t                   pSequence;                //Numero de bloques analizados
    Params_t                   pAggreg10C;               //Agregacion de 10 ciclos
    Params_t                   pAggreg3S;                //Agregacion de 3 segundos
    Params_t                   pAggreg1M;                //Agregacion de 1 minuto
    Params_t                   pAggreg10M;               //Agregacion de 10 minutos
    Params_t                   pAggreg15M;               //Agregacion de 15 minutos
    Params_t                   pAggreg1H;                //Agregacion de 1 hora
    vector<FFT*>               pVoltageFFT;              //Muestras de FFT tension del bloque
    vector<FFT*>               pCurrentFFT;              //Muestras de FFT corriente del bloque
    bool                       pWorking;                 //Flag para indicar que el hilo esta trabajando
    pthread_t                  pThread;                  //Hilo de ejecucion
    pthread_mutex_t            pThreadMutex;             //Mutex para la variable de condicion
    pthread_cond_t             pThreadCond;              //Variable de condicion para esperar
    bool                       pTerminate;               //Marca para finalizar proceso
    uint64_t                   pPrevTimestamp;           //Timestamp del ultimo bloque procesado
    uint32_t                   pWaitingThreads;          //Hilos esperando datos
    pthread_mutex_t            pWaitingThreadsMutex;     //Mutex para registrar cambios en WaitingThreads
    pthread_cond_t             pWaitingThreadsCond;      //Variable de condicion para bloquear el analizer
    bool                       pDataReady;               //Datos disponibles
    pthread_mutex_t            pDataReadyMutex;          //Mutex para registrar cambios en DataReady
    pthread_cond_t             pDataReadyCond;           //Variable de condicion para bloquear los clientes
    deque<vector<float> >      pDelayVoltage;            //Delay buffer for samples
    deque<vector<float> >      pDelayCurrent;            //Delay buffer for samples
    vector<Filter_t>           pFilters;                 //Filter bank
    deque<uint32_t>            pCycles;                  //Prev cycles lenghts
    vector<uint32_t>           pBlockCycles;             //Current block cycles lengths
    vector<float>              pLastSamples;             //Filter samples backup
    uint32_t                   pCycleLen;                //Current cycle len
    uint32_t                   pMaxSamples;              //Max block samples
    pthread_t                  pEventThread;             //Hilo de ejecucion
    pthread_mutex_t            pEventMutex;              //Mutex para la variable de condicion
    pthread_cond_t             pEventCond;               //Variable de condicion para esperar
    deque<vector<float> >      pPrevSamples;             //Previous samples
    deque<EventCycle_t>        pPrevCycles;              //Previous cycles
    bool                       pPrevStationary;          //Steady state flag
    bool                       pEventsWorking;           //Events mutex is working
    list<Event_t>              pEvents;                  //Running events
  private :
    static void *Analyzer_Thread(void *args);
    static void *Events_Thread(void *args);
  private :
    void  AccumulateAggreg(Params_t &src, const Params_t &dst) const;
    float AccumulateSUM(const float val1, const float val2) const;
    float AccumulateRMS(const float val1, const float val2) const;
    void  NormalizeAggreg(Params_t &src) const;
    void  InitAggreg(Params_t &params, AggregType type) const;
    void  SaveAggreg(Params_t &params);
    void  SaveEvent(Event_t &evt);
    float CalcFrequency();
    float CalcChange(const float in, const float reference) const;
    void  InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos);
    bool  FindRunningEvent(const uint32_t mask);
    void  FindEvent();
    void  WaitEvent();
    void  GetAnalyzer(HTTPRequest &req);
    void  SetAnalyzer(HTTPRequest &req);
    void  GetSeries(HTTPRequest &req);
    void  GetSeriesNow(HTTPRequest &req);
    void  GetSeriesRange(HTTPRequest &req);
    void  GetEvents(HTTPRequest &req);
    void  GetEvent(HTTPRequest &req);
    void  SetEvent(HTTPRequest &req);
    void  DelEvent(HTTPRequest &req);
    void  GetRates(HTTPRequest &req);
    void  GetRate(HTTPRequest &req);
    void  SetRate(HTTPRequest &req);
    void  DelRate(HTTPRequest &req);
    void  AddRate(HTTPRequest &req);
    void  GetPrices(HTTPRequest &req);
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const json &option, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime);
    bool IsFinished();
};
