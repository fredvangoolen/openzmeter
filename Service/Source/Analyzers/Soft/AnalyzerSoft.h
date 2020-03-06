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
#include <inttypes.h>
#include <deque>
#include <list>
#include "muParser/muParser.h"
#include "cpp-httplib/httplib.h"
#include "Analyzer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
using namespace httplib;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft final : public Analyzer {
  private :
    static const uint8_t FILTERSTAGES         =  5;        //Filter stages for zerocrossing
    static const uint8_t ZEROCROSSING_SECONDS = 10;        //Used seconds to calc frequency
  public :
    enum AggregType  { UNINITIALIZED, AGGREG_200MS, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
    enum EventType   { EVT_NONE = 0x00, EVT_SWELL = 0x01, EVT_DIP = 0x02, EVT_INTERRUPTION = 0x04, EVT_RVC = 0x08 };
    enum EventState  { EVT_RUNNING, EVT_CLOSING, EVT_CLOSED };
    enum SyncState   { NOT_LOGGED, LOGGED, CONNECTED, TERMINATE };
   private :
    typedef struct {
      AggregType                     Type;                   //Aggregation type
      bool                           Flag;                   //Flagged period
      float                          Frequency;              //Frecuency
      float                          Unbalance;              //Unbalance (only in 3phase)
      float                          Voltage[3];             //Aggregation average RMS voltage
      float                          Current[4];             //Aggregation average RMS current
      uint64_t                       Timestamp;              //Aggregation timestampt
      uint16_t                       AggregSamples;          //Number of samples accumulated in aggreg
      vector<uint16_t>               StatsFrequency;         //Distribucion de frecuencias
      vector<uint16_t>               StatsVoltage;           //Distribucion de tensiones
      vector<uint16_t>               StatsTHD;               //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      vector<uint16_t>               StatsHarmonics;         //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector<uint16_t>               StatsUnbalance;         //Distribucion de unbalance
      float                          Active_Power[3];        //Aggregation average Active Power (P)
      float                          Reactive_Power[3];      //Reactive power
      float                          Active_Energy[3];       //Energia consumida
      float                          Reactive_Energy[3];     //Reactive energy
      float                          Apparent_Power[3];      //Potencia aparente (S)
      float                          Power_Factor[3];        //Factor de potencia (cos(PHI))
      float                          Voltage_THD[3];         //Distorsion harmonica de la tension
      float                          Current_THD[4];         //Distorsion harmonica de la corriente
      float                          Voltage_Phase[3];       //Angulo entre canales de tension
      float                          Phi[3];                 //Angulo de Phi
      float                          Voltage_Harmonics[150]; //Harmonicos del canal de tension
      float                          Current_Harmonics[200]; //Harmonicos del canal de corriente
      float                          Power_Harmonics[150];   //Power harmonics
    } Params_t;
    typedef struct {
     float *InPtr;
     float *OutPtr;
    } FFTChannel_t;
    typedef struct {
      float   A0;
      float   A1;
      float   A2;
      float   B1;
      float   B2;
      float   ChannelsZ1[3];
      float   ChannelsZ2[3];
    } Filter_t;
    typedef struct {
      uint32_t           AlarmID;
      string             Name;
      uint8_t            Priority;
      uint32_t           Samples;
      uint32_t           Window;
      AggregType         Aggreg;
      Parser             Evaluator;
      queue<bool>        Results;
    } Alarm_t;
    typedef struct {
      vector<float> RMS;
      uint64_t      Timestamp;
    } EventCycle_t;
    typedef struct {
      enum EventType            Type;                    //Running event type
      EventState                State;                   //Events state
      uint64_t                  Start;                   //Event start timestamp
      uint64_t                  Duration;                //Event duration in samples
      vector<float>             Reference;               //Voltage reference for event (Stationary for RVC, nominal for others)
      vector<float>             PeakValue;               //max/min voltage variation along event
      vector<float>             ChangeValue;             //diference voltage of event
      deque<vector<float> >     SamplesRAW_A;            //
      deque<vector<float> >     SamplesRMS_A;            //
      deque<vector<float> >     SamplesRAW_B;            //
      deque<vector<float> >     SamplesRMS_B;            //
    } Event_t;
    typedef struct {
      pthread_mutex_t         *Mutex;                    //Export list locks
      pthread_cond_t          *Cond;                     //Wakeup parent after last thread ends
      uint32_t                *Count;                    //Count of running sync threads
      string                   Server;
      string                   Cookie;
      string                   Method;
      json                     Params;
      uint64_t                 Sequence;
      AnalyzerSoft            *Analyzer;
    } Sync_t;
  private :
    static pthread_mutex_t      pFFTMutex;               //Planer mutex
    static uint32_t             pAlarmsID;               //Last assigned AlarmID
    static uint32_t             pFFTRunning;             //Currently running FFTs
  private :
    static const vector<float>  FrequencyRanges;         //Rangos de frecuencias para estadisticas
    static const vector<float>  VoltageRanges;           //Rangos de tensiones para estadisticas
    static const vector<float>  THDRanges;               //Rangos de distorsion harmonica para estadisticas
    static const vector<float>  UnbalanceRanges;         //Unbalance ranges
    static const vector<float>  HarmonicsAllowed;        //Rangos permitidos para cada harmonico
  private : //Not modified after creation
    const float                pSamplerate;              //Velocidad de muestreo
    const string               pConfigPath;              //Path to configuration
    const uint8_t              pVoltageChannels;         //Voltage channels
    const uint8_t              pCurrentChannels;         //Current channels
    const AnalyzerMode         pMode;                    //Operation mode
  private : //Used in BufferAppend, Configure and Analyzer_Func and some callbacks
    RWLocks                    pAnalyzerLock;            //Locks access to analysis shared variables
    bool                       pSave200MS;               //Save 10 cycles aggregation to database
    float                      pNominalVoltage;          //Analyzer nominal voltage
    float                      pNominalFrequency;        //Analyzer nominal frequency
    float                      pMaxCurrent;              //Max current, over this value warnings are shown
    float                      pMinCurrent;              //Min current, under this value some analysis are disabled
    uint32_t                   pMaxSamples;              //Max block samples
    string                     pTimeZone;                //Country code
    uint64_t                   pSequence;                //Sequence number of analyzed blocks
    uint16_t                   pBlockSamples;            //Number of samples in block
    uint16_t                   pBlockCyclesCount;        //Block cycles count
    uint16_t                   pBlockCycles[24];         //Max block sizes
    float                     *pVoltageSamples;          //Voltage samples
    float                     *pCurrentSamples;          //Current samples
    uint16_t                   pFFTSamples;              //FFT result samples
    float                     *pVoltageFFT;              //FFT voltage output (complex)
    float                     *pCurrentFFT;              //FFT current output (complex)
    FFTChannel_t               pVoltageFFTChannels[3];   //FFT configuration channels
    FFTChannel_t               pCurrentFFTChannels[4];   //FFT configuration channels
    float                      pLastSamples[3];          //Filter samples backup
    Filter_t                   pFilters[FILTERSTAGES];   //Filter bank
    float                     *pDelayVoltage;            //Delay buffer for samples
    float                     *pDelayCurrent;            //Delay buffer for samples
    uint16_t                   pDelaySize;               //Delay samples
    uint16_t                   pDelayPos;                //Delay pos
    uint16_t                   pFreqCyclesPos;           //Frequency cycles write position
    uint16_t                   pFreqCyclesLen;           //Frequency cycles buffer len
    uint32_t                  *pFreqCycles;              //Frequency calc cycles lenghts
    json                       pRates;                   //Applied rates local copy
  private : //Used only in Analyzer_Func
    uint64_t                   pPrevTimestamp;           //Previous block time
    Params_t                   pAggreg200MS;             //200ms aggregation
    Params_t                   pAggreg3S;                //3 seconds aggregation
    Params_t                   pAggreg1M;                //1 minute aggregation
    Params_t                   pAggreg10M;               //10 minutes aggregation
    Params_t                   pAggreg15M;               //15 minutes aggregation
    Params_t                   pAggreg1H;                //1 hour aggregation
  private : //Workers
    Worker                    *pAnalyzerWorker;          //Analyzer worker
    Worker                    *pVoltageFFTWorker[3];     //Voltage FFT worker
    Worker                    *pCurrentFFTWorker[4];     //Current FFT worker
    Worker                    *pAlarmsWorker;            //Alarms detection worker
    Worker                    *pEventsWorker;            //Events detector
  private : //Alarms related (should be locked)
//    RWLocks                    pAlarmsLock;              //Control access
    vector<Alarm_t>            pAlarms;                  //Alarms control object
  private : //Events related (only acceded inside analyzer thread)
    list<Event_t>              pEvents;                  //Running events
    bool                       pPrevStationary;          //Steady state flag
    deque<vector<float> >      pPrevSamples;             //Previous samples
    deque<EventCycle_t>        pPrevCycles;              //Previous cycles
  private : //Readed in thread, changed in configure
    pthread_t                  pSyncThread;              //Export thread
    bool                       pSyncTerminate;           //Sync status
    string                     pSyncServer;              //Export server address
    string                     pSyncUser;                //Export server user
    string                     pSyncPass;                //Export server password





    ProviderLocks              pNowDataLock;             //GetSeriesNow access control


  private :
    static void  Analyzer_Func(void *creation, void *run);
    static void  Events_Func(void *creation, void *run);
    static void  FFT_Func(void *creation, void *run);
    static void  Alarms_Func(void *creation, void *run);
    static void *Sync_Thread(void *params);
    static void *Sync_RespThread(void *params);
    static float CalcChange(const float in, const float reference);
    static float CalcAngle(const float *a, const float *b);
    static float BinPower(const float *a);
    static void  InitAlarm(Alarm_t &alarm, const json &config);

  private :
    void  Configure(const json &options);
    void  InitAggreg(Params_t &params, AggregType type);
    void  AccumulateAggreg(Params_t &src, const Params_t &dst);
    void  NormalizeAggreg(Params_t &src);
    void  SaveAggreg(Params_t &params);
    void  InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos);
    bool  FindRunningEvent(const uint32_t mask);
    void  SaveEvent(Event_t &evt);
  private :
    void  GetAnalyzer(const json &params, Response &response) override;
    void  SetAnalyzer(const json &params, Response &response) override;
    void  GetSeries(const json &params, Response &response) override;
    void  GetSeriesNow(const json &params, Response &response) override;
    void  GetSeriesRange(const json &params, Response &response) override;
    void  GetEvents(const json &params, Response &response) override;
    void  GetEvent(const json &params, Response &response) override;
    void  SetEvent(const json &params, Response &response) override;
    void  DelEvent(const json &params, Response &response) override;
    void  GetPrices(const json &params, Response &response) override;
    void  TestAlarm(const json &params, Response &response) override;
    void  GetAlarms(const json &params, Response &response) override;
    void  GetRemote(const json &params, Response &response) override;
    void  SetRemote(const json &params, Response &response) override;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(float *voltageSamples, float *currentSamples);
    bool IsWorking();
};