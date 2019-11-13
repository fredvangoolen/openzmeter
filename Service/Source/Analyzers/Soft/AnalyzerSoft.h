/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <pthread.h>
#include <inttypes.h>
#include <deque>
#include <list>
#include "Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
#define ZEROCROSSING_FILTERSTAGES          5
#define ZEROCROSSING_SECONDS              10        //Used seconds to calc frequency
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft final : public Analyzer {
  private :
    class Events;
    class FFT;
    class Sync;
    enum AggregType { UNINITIALIZED, AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
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
      vector< vector<float> >        Voltage_Harmonics;    //Harmonicos del canal de tension
      vector< vector<float> >        Current_Harmonics;    //Harmonicos del canal de corriente
      vector< vector<float> >        Power_Harmonics;      //Power harmonics
    } Params_t;
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
    static vector<float>  FrequencyRanges;   //Rangos de frecuencias para estadisticas
    static vector<float>  VoltageRanges;     //Rangos de tensiones para estadisticas
    static vector<float>  THDRanges;         //Rangos de distorsion harmonica para estadisticas
    static vector<float>  HarmonicsAllowed;  //Rangos permitidos para cada harmonico
  private :
    string                     pConfigPath;              //Path to configuration
    vector< vector<float> >    pVoltageSamples;          //Voltage samples
    vector< vector<float> >    pCurrentSamples;          //Current samples
    float                      pNominalVoltage;          //Analyzer nominal voltage
    float                      pNominalFrequency;        //Analyzer nominal frequency
    string                     pTimeZone;                //Country code
    string                     pContract;                //Contract identifier
    string                     pZipCode;                 //Installation zip code
    vector<float>              pGeo;                     //GPS position
    float                      pMaxCurrent;              //Max current, over this value warnings are shown
    float                      pMinCurrent;              //Min current, under this value some analysis are disabled
    float                      pSamplerate;              //Velocidad de muestreo
    AnalyzerMode               pMode;                    //Operation mode
    bool                       pSave10c;                 //Save 10 cycles aggregation to database
    Params_t                   pAggreg10C;               //Agregacion de 10 ciclos
    Params_t                   pAggreg3S;                //Agregacion de 3 segundos
    Params_t                   pAggreg1M;                //Agregacion de 1 minuto
    Params_t                   pAggreg10M;               //Agregacion de 10 minutos
    Params_t                   pAggreg15M;               //Agregacion de 15 minutos
    Params_t                   pAggreg1H;                //Agregacion de 1 hora
    vector<FFT*>               pVoltageFFT;              //Muestras de FFT tension del bloque
    vector<FFT*>               pCurrentFFT;              //Muestras de FFT corriente del bloque
    pthread_t                  pAnalyzerThread;          //Hilo de ejecucion
    WorkerLocks                pAnalyzerWorkerLocks;     //Handle analyzer thread
    bool                       pTerminate;               //Marca para finalizar proceso
    ProviderLocks              PNowDataLock;             //GetSeriesNow access control
    deque<vector<float> >      pDelayVoltage;            //Delay buffer for samples
    deque<vector<float> >      pDelayCurrent;            //Delay buffer for samples
    vector<Filter_t>           pFilters;                 //Filter bank
    deque<uint32_t>            pCycles;                  //Prev cycles lenghts
    vector<uint32_t>           pBlockCycles;             //Current block cycles lengths
    vector<float>              pLastSamples;             //Filter samples backup
    uint32_t                   pCycleLen;                //Current cycle len
    uint32_t                   pMaxSamples;              //Max block samples
    Events                    *pEventsDetector;          //Events detector
    Sync                      *pSync;                    //Synchronization object
  private :
    static void  *Analyzer_Thread(void *args);
//    static void  *Sync_Thread(void *args);
//    static void  *Request_Thread(void *args);
    static float CalcChange(const float in, const float reference);
    static void  AccumulateAggreg(Params_t &src, const Params_t &dst);
    static float AccumulateSUM(const float val1, const float val2);
    static float AccumulateRMS(const float val1, const float val2);
    static void  NormalizeAggreg(Params_t &src);
  private :
    void  Configure(const json &options);
    void  InitAggreg(Params_t &params, AggregType type);
    void  SaveAggreg(Params_t &params);
    float CalcFrequency();
  private :    
    void  GetAnalyzer(HTTPRequest &req) override;
    void  SetAnalyzer(HTTPRequest &req) override;
    void  GetSeries(HTTPRequest &req) override;
    void  GetSeriesNow(HTTPRequest &req) override;
    void  GetSeriesRange(HTTPRequest &req) override;
    void  GetEvents(HTTPRequest &req) override;
    void  GetEvent(HTTPRequest &req) override;
    void  SetEvent(HTTPRequest &req) override;
    void  DelEvent(HTTPRequest &req) override;
    void  GetRates(HTTPRequest &req) override;
    void  GetRate(HTTPRequest &req) override;
    void  SetRate(HTTPRequest &req) override;
    void  DelRate(HTTPRequest &req) override;
    void  AddRate(HTTPRequest &req) override;
    void  GetPrices(HTTPRequest &req) override;
    void  TestAlarm(HTTPRequest &req) override;
    void  GetAlarms(HTTPRequest &req) override;
    void  GetAlarm(HTTPRequest &req) override;
    void  SetAlarm(HTTPRequest &req) override;
    void  AddAlarm(HTTPRequest &req) override;
    void  DelAlarm(HTTPRequest &req) override;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples);
    bool IsFinished();
};