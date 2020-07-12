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
extern "C" {
  #include "mqtt-c/mqtt.h"
  #include "mqtt-c/mqtt_pal.h"
}
#include "muParser/muParser.h"
#include "cpp-httplib/httplib.h"
#include "Analyzer.h"
#include "RatesEval.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
using namespace httplib;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft : public Analyzer {
  private :
    static const uint8_t FILTERSTAGES         =  5;        //Filter stages for zerocrossing
    static const uint8_t ZEROCROSSING_SECONDS = 10;        //Used seconds to calc frequency
  public :
    enum AggregType  { AGGREG_UNINITIALIZED = 0, AGGREG_200MS, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
    enum EventType   { EVT_NONE = 0x00, EVT_SWELL = 0x01, EVT_DIP = 0x02, EVT_INTERRUPTION = 0x04, EVT_RVC = 0x08 };
    enum SyncState   { NOT_LOGGED, LOGGED, CONNECTED, TERMINATE };
  private :
    static constexpr float   FrequencyRanges[]   = { -INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY };
    static constexpr uint8_t FrequencyRangesLen  = (sizeof(FrequencyRanges) / sizeof(float)) - 1;
    static constexpr float   VoltageRanges[]     = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
    static constexpr uint8_t VoltageRangesLen    = (sizeof(VoltageRanges) / sizeof(float)) - 1;
    static constexpr float   THDRanges[]         = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
    static constexpr uint8_t THDRangesLen        = (sizeof(THDRanges) / sizeof(float)) - 1;
    static constexpr float   UnbalanceRanges[]   = {0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.25, 2.50, 2.75, 3.0, 3.25, 3.50, 3.75, 4.0, 4.5, 5.0, INFINITY};
    static constexpr uint8_t UnbalanceRangesLen  = (sizeof(UnbalanceRanges) / sizeof(float)) - 1;
    static constexpr float   HarmonicsAllowed[]  = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
    static constexpr uint8_t HarmonicsAllowedLen = (sizeof(HarmonicsAllowed) / sizeof(float));
  public :
      typedef struct {
      AggregType               Type;                               //Aggregation type
      bool                     Flag;                               //Flagged period
      float                    Frequency;                          //Frecuency
      float                    Unbalance;                          //Unbalance (only in 3phase)
      float                    Voltage[3];                         //Aggregation average RMS voltage
      float                    Current[4];                         //Aggregation average RMS current
      uint64_t                 Timestamp;                          //Aggregation timestampt
      uint16_t                 AggregSamples;                      //Number of samples accumulated in aggreg
      uint16_t                 StatsFrequency[FrequencyRangesLen]; //Distribucion de frecuencias
      uint16_t                 StatsVoltage[VoltageRangesLen];     //Distribucion de tensiones
      uint16_t                 StatsTHD[THDRangesLen];             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      uint16_t                 StatsHarmonics[2];                  //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      uint16_t                 StatsUnbalance[UnbalanceRangesLen]; //Distribucion de unbalance
      float                    Active_Power[3];                    //Aggregation average Active Power (P)
      float                    Reactive_Power[3];                  //Reactive power
      float                    Active_Energy[3];                   //Energia consumida
      float                    Reactive_Energy[3];                 //Reactive energy
      float                    Apparent_Power[3];                  //Potencia aparente (S)
      float                    Power_Factor[3];                    //Factor de potencia (cos(PHI))
      float                    Voltage_THD[3];                     //Distorsion harmonica de la tension
      float                    Current_THD[4];                     //Distorsion harmonica de la corriente
      float                    Voltage_Phase[3];                   //Angulo entre canales de tension
      float                    Phi[3];                             //Angulo de Phi
      float                    Voltage_Harmonics[150];             //Harmonicos del canal de tension
      float                    Current_Harmonics[200];             //Harmonicos del canal de corriente
      float                    Power_Harmonics[150];               //Power harmonics
    } Params_t;
  private :
    typedef struct {
      float Re;
      float Im;
    } Complex_t;

    typedef struct {
     float                    *InPtr;
     Complex_t                *OutPtr;
    } FFTChannel_t;
    typedef struct {
      float                    A0;
      float                    A1;
      float                    A2;
      float                    B1;
      float                    B2;
      float                    ChannelsZ1[3];
      float                    ChannelsZ2[3];
    } Filter_t;
    typedef struct {
      string                   Name;
      uint8_t                  Priority;
      uint16_t                 Samples;
      uint16_t                 Window;
      uint16_t                 ResultPos;
      uint16_t                 ResultCount;
      AggregType               Aggreg;
      Parser                   Evaluator;
      uint8_t                 *Results;
      bool                     Triggered;
    } Alarm_t;
    typedef struct {
      enum EventType           Type;                     //Running event type
      uint64_t                 Start;                    //Event start timestamp
      uint64_t                 SamplesDuration;          //Event duration in samples
      float                    Reference[3];             //Voltage reference for event (Stationary for RVC, nominal for others)
      float                    PeakValue[3];             //max/min voltage variation along event
      float                    ChangeValue[3];           //diference voltage of event
      uint32_t                 CyclesDuration;           //Event duration in samples
    } Event_t;
    typedef struct {
      pthread_mutex_t         *Mutex;                    //Export list locks
      pthread_cond_t          *Cond;                     //Wakeup parent after last thread ends
      uint32_t                *Count;                    //Count of running sync threads
      string                   Server;                   //
      string                   Cookie;                   //
      string                   Method;                   //
      json                     Params;                   //
      uint64_t                 Sequence;                 //
      AnalyzerSoft            *Analyzer;                 //
    } Sync_t;
  private :
    static pthread_mutex_t     pFFTMutex;                //Planer mutex
    static uint32_t            pFFTRunning;              //Currently running FFTs
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
    time_zone                  pTimeZone;                //Country code
    uint64_t                   pSequence;                //Sequence number of analyzed blocks
    uint16_t                   pBlockSamples;            //Number of samples in block
    uint8_t                    pBlockCyclesCount;        //Block cycles count
    uint16_t                   pBlockCycles[24];         //Max block sizes
    float                     *pVoltageSamples;          //Voltage samples
    float                     *pCurrentSamples;          //Current samples
    uint32_t                   pMaxFFTSamples;           //Max FFT block samples
    uint16_t                   pFFTSamples;              //FFT result samples
    Complex_t                 *pVoltageFFT;              //FFT voltage output (complex)
    Complex_t                 *pCurrentFFT;              //FFT current output (complex)
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
    json                      *pRates;                   //Applied rates
    ProviderLocks              pNowDataLock;             //GetSeriesNow access control
  private : //Used only in Analyzer_Func
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
    vector<Alarm_t>            pAlarms;                  //Alarms control object
  private : //Events related (only acceded inside analyzer thread)
    Event_t                    pEvent;                   //Running event
    list<Event_t>              pEvents;                  //Close pending events
    bool                       pPrevStationary;          //Steady state flag
    uint16_t                   pPrevSamplesLen;          //Previous samples count
    uint16_t                   pPrevSamplesPos;          //Previous samples write pos
    float                     *pPrevSamples;             //Previous samples
    uint16_t                   pPrevCyclesLen;           //Previous cycles count
    uint16_t                   pPrevCyclesPos;           //Previous cycles write pos
    float                     *pPrevCycles;              //Previous cycles
  private : //Readed in thread, changed in configure
    pthread_t                  pSyncThread;              //Export thread
    bool                       pSyncTerminate;           //Sync terminate signal
    string                     pSyncServer;              //Export server address
    string                     pSyncUser;                //Export server user
    string                     pSyncPass;                //Export server password
  private :
    pthread_t                  pMQTTThread;              //MQTT thread
    mqtt_client                pMQTTClient;              //MQTT client state
    uint32_t                   pMQTTBuffSize;            //MQTT buffers size
    uint8_t                   *pMQTTSendBuff;            //MQTT send buffer
    uint8_t                   *pMQTTRecvBuff;            //MQTT recv buffer
    bool                       pMQTTTerminate;           //MQTT terminate signal
    string                     pMQTTServer;              //MQTT server address
    string                     pMQTTPrefix;              //MQTT prefix
    string                     pMQTTUser;                //MQTT server user
    string                     pMQTTPass;                //MQTT server password
    bool                       pMQTT3S;                  //Enable 3 seconds publish
    bool                       pMQTT1M;                  //Enable 1 minute publish
    bool                       pMQTT10M;                 //Enable 10 minutes publish
    bool                       pMQTT15M;                 //Enable 15 minutes publish
    bool                       pMQTT1H;                  //Enable 1 hour publish
    bool                       pMQTTAlarms;              //Enable alarms publish
    bool                       pMQTTConfig;              //Enable config publish
  private :
    static void *Sync_Thread(void *params);
    static void *MQTT_Thread(void *params);
    static void *Sync_RespThread(void *params);
    static float CalcChange(const float in, const float reference);
    static float CalcAngle(const Complex_t &a, const Complex_t &b);
    static float BinPower(const Complex_t &a);
  private :
    void Analyzer_Func();
    void Events_Func();
    void FFT_Func(FFTChannel_t *ch);
    void Alarms_Func(Params_t *aggreg);
    void Configure(const json &options);
    void InitAggreg(Params_t &params, AggregType type);
    void InitAlarmParser(Parser &parser, const string &expr, const AggregType &type);
    void AccumulateAggreg(Params_t &src, const Params_t &dst);
    void NormalizeAggreg(Params_t &src);
    void SaveAggreg(Params_t &params);
    void PublishAggreg(const Params_t &params, const string &topic);
    void PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered);
    void PublishConfig(const json &options);
    bool FindRunningEvent(const uint32_t mask);
    json CalcGA();
    void PeriodicTasks();
  private :
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetSeries(const json &params, Response &response) override;
    void GetSeriesNow(const json &params, Response &response) override;
    void GetWaveStream(const json &params, Response &response) override;
    void GetSeriesRange(const json &params, Response &response) override;
    void DelSeriesRange(const json &params, Response &response) override;
    void GetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;
    void GetPrices(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void GetRemote(const json &params, Response &response) override;
    void SetRemote(const json &params, Response &response) override;
    bool HandleModbus(const uint8_t *buff, const uint16_t start, const uint8_t len);
  public :
    static json CheckConfig(const json &config, const Analyzer::AnalyzerMode mode, string *error);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(float *voltageSamples, float *currentSamples, const uint32_t buffUsage);
    bool IsWorking();
};
//----------------------------------------------------------------------------------------------------------------------