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
#include "Analyzer.h"
#include "FFT.h"
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft final : public Analyzer {
  private :
    static const uint8_t ZEROCROSSING_FILTERSTAGES =  5;
    static const uint8_t ZEROCROSSING_SECONDS      = 10;        //Used seconds to calc frequency
  private :
    class Events;
    class Sync;
    class Alarms;
  public :
      enum AggregType { UNINITIALIZED, AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
  private :
    typedef struct {
      AggregType                     Type;                 //Aggregation type
      float                          Frequency;            //Frecuencia
      float                          Unbalance;            //Unbalance only in 3phase
      bool                           Flag;                 //Periodo marcado
      uint64_t                       Timestamp;            //Aggregation timestampt
      uint16_t                       Voltage_Inputs;       //Valid RMS_V samples in aggregation
      uint16_t                       Frequency_Inputs;     //Valid Freq samples in aggregation
      vector<uint16_t>               StatsFrequency;       //Distribucion de frecuencias
      vector<uint16_t>               StatsVoltage;         //Distribucion de tensiones
      vector<uint16_t>               StatsTHD;             //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      vector<uint16_t>               StatsHarmonics;       //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector<uint16_t>               StatsUnbalance;       //Distribucion de unbalance
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
      float   A0;
      float   A1;
      float   A2;
      float   B1;
      float   B2;
      float   ChannelsZ1[3];
      float   ChannelsZ2[3];
    } Filter_t;
  private :
    static const vector<float>  FrequencyRanges;   //Rangos de frecuencias para estadisticas
    static const vector<float>  VoltageRanges;     //Rangos de tensiones para estadisticas
    static const vector<float>  THDRanges;         //Rangos de distorsion harmonica para estadisticas
    static const vector<float>  UnbalanceRanges;   //Unbalance ranges
    static const vector<float>  HarmonicsAllowed;  //Rangos permitidos para cada harmonico
  private : //Variables not changed after initialization or not need locking
    bool                       pTerminate;               //Marca para finalizar proceso
    float                      pSamplerate;              //Velocidad de muestreo
    string                     pConfigPath;              //Path to configuration
    uint32_t                   pCycleLen;                //Current cycle len
    pthread_t                  pAnalyzerThread;          //Hilo de ejecucion
    float                      pLastSamples[3];          //Filter samples backup
    AnalyzerMode               pMode;                    //Operation mode
    WorkerLocks                pAnalyzerWorkerLocks;     //Handle analyzer thread
    ProviderLocks              pNowDataLock;             //GetSeriesNow access control
    uint8_t                    pVoltageChannels;         //Voltage channels
    uint8_t                    pCurrentChannels;         //Current channels
  private :  //Config params (Writed in configure, readed in other places)
    Events                    *pEventsDetector;          //Events detector
    Sync                      *pSync;                    //Synchronization object
//    Rates                     *pRates;                   //Rate control object
    Alarms                    *pAlarms;                  //Alarms control object
    bool                       pSave10c;                 //Save 10 cycles aggregation to database
    float                      pNominalVoltage;          //Analyzer nominal voltage
    float                      pNominalFrequency;        //Analyzer nominal frequency
    float                      pMaxCurrent;              //Max current, over this value warnings are shown
    float                      pMinCurrent;              //Min current, under this value some analysis are disabled
    uint32_t                   pMaxSamples;              //Max block samples
    string                     pTimeZone;                //Country code
    string                     pContract;                //Contract identifier
    string                     pZipCode;                 //Installation zip code
    vector<float>              pGeo;                     //GPS position
    Filter_t                   pFilters[ZEROCROSSING_FILTERSTAGES]; //Filter bank
    float                     *pDelayVoltage;            //Delay buffer for samples
    uint16_t                   pDelayVoltageSize;        //Voltage delay samples
    uint16_t                   pDelayVoltagePos;         //Voltage delay pos
    float                     *pDelayCurrent;            //Delay buffer for samples
    uint16_t                   pDelayCurrentSize;        //Current delay samples
    uint16_t                   pDelayCurrentPos;         //Current delay pos
  private :  //Analyzer params (write in thread OR append, read in SeriesNow)
    vector< vector<float> >    pVoltageSamples;          //Voltage samples
    vector< vector<float> >    pCurrentSamples;          //Current samples
    FFT                       *pVoltageFFT[3];           //Muestras de FFT tension del bloque
    FFT                       *pCurrentFFT[4];           //Muestras de FFT corriente del bloque
    vector<uint32_t>           pBlockCycles;             //Current block cycles lengths
    deque<uint32_t>            pCycles;                  //Prev cycles lenghts
    Params_t                   pAggreg10C;               //Agregacion de 10 ciclos
  private :
    static void  *Analyzer_Thread(void *args);
    static float CalcChange(const float in, const float reference);
    static void  AccumulateAggreg(Params_t &src, const Params_t &dst);
    static float AccumulateSUM(const float val1, const float val2);
    static float AccumulateRMS(const float val1, const float val2);
    static void  NormalizeAggreg(Params_t &src);
  private :
    void  Configure(const json &options);
    void  InitAggreg(Params_t &params, AggregType type);
    void  SaveAggreg(Params_t &params);
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
    void  GetRates(const json &params, Response &response) override;
    void  GetRate(const json &params, Response &response) override;
    void  SetRate(const json &params, Response &response) override;
    void  DelRate(const json &params, Response &response) override;
    void  AddRate(const json &params, Response &response) override;
    void  GetPrices(const json &params, Response &response) override;
    void  TestAlarm(const json &params, Response &response) override;
    void  GetAlarms(const json &params, Response &response) override;
    void  GetAlarm(const json &params, Response &response) override;
    void  SetAlarm(const json &params, Response &response) override;
    void  AddAlarm(const json &params, Response &response) override;
    void  DelAlarm(const json &params, Response &response) override;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate);
    ~AnalyzerSoft();
    bool BufferAppend(const float *voltageSamples, const float *currentSamples);
    bool IsFinished();
};