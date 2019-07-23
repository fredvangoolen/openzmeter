/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <complex>
#include <nlohmann/json.hpp>
#include "Tools.h"
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Analyzer : protected RWLocks {
  protected :
    static vector<float>  FrequencyRanges;   //Rangos de frecuencias para estadisticas
    static vector<float>  VoltageRanges;     //Rangos de tensiones para estadisticas
    static vector<float>  THDRanges;         //Rangos de distorsion harmonica para estadisticas
    static vector<float>  HarmonicsAllowed;  //Rangos permitidos para cada harmonico
  public :
    enum AnalyzerMode { PHASE1, PHASE3, PHASE3N };
    enum AnalyzerRights { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
    enum AggregType { UNINITIALIZED, AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
  public :
    typedef struct {
      AggregType                        Type;                       //Aggregation type
      float                             Frequency;                  //Frecuencia
      bool                              Flag;                       //Periodo marcado
      uint64_t                          Timestamp;                  //Aggregation timestampt
      uint16_t                          Voltage_Inputs;             //Valid RMS_V samples in aggregation
      uint16_t                          Frequency_Inputs;           //Valid Freq samples in aggregation
      vector<uint16_t>                  StatsFrequency;             //Distribucion de frecuencias
      vector<uint16_t>                  StatsVoltage;               //Distribucion de tensiones
      vector<uint16_t>                  StatsTHD;                   //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      vector<uint16_t>                  StatsHarmonics;             //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector<uint16_t>                  Current_Inputs;             //Valid RMS_I samples in aggregation
      vector<float>                     Voltage;                    //Aggregation average RMS voltage
      vector<float>                     Current;                    //Aggregation average RMS current
      vector<float>                     Active_Power;               //Aggregation average Active Power (P)
      vector<float>                     Reactive_Power;             //Reactive power
      vector<float>                     Active_Energy;              //Energia consumida
      vector<float>                     Reactive_Energy;            //Reactive energy
      vector<float>                     Apparent_Power;             //Potencia aparente (S)
      vector<float>                     Power_Factor;               //Factor de potencia (cos(PHI))
      vector<float>                     Voltage_THD;                //Distorsion harmonica de la tension
      vector<float>                     Current_THD;                //Distorsion harmonica de la corriente
      vector<float>                     Voltage_Phase;              //Angulo entre canales de tension
      vector<float>                     Phi;                        //Angulo de Phi
      vector< vector<complex<float> > > Voltage_Harmonics;          //Armonicos del canal de tension
      vector< vector<complex<float> > > Current_Harmonics;          //Armonicos del canal de corriente
//      vector< vector<float> >           Voltage_FFT;                //Resultados de la FFT en tension
//      vector< vector<float> >           Current_FFT;                //Resultados de la FFT en corriente
      vector< vector<float> >           Voltage_Samples;            //Voltage samples
      vector< vector<float> >           Current_Samples;            //Current samples
    } Params_t;
  protected :
    uint64_t        pLastSave;                //Timestamp of last save or communication
    float           pNominalVoltage;          //Analyzer nominal voltage
    float           pNominalFrequency;        //Analyzer nominal frequency
    string          pSerial;                  //Serial number
    string          pCountryCode;             //Country code
    string          pContract;                //Contract identifier
    string          pZipCode;                 //Installation zip code
    float           pGeo[2];                  //GPS position
    string          pName;                    //Analyzer description
    float           pMaxCurrent;              //Max current, over this value warnings are shown
    float           pMinCurrent;              //Min current, under this value some analysis are disabled
    float           pSamplerate;              //Velocidad de muestreo (NaN if unsuported)
    AnalyzerMode    pMode;                    //Operation mode
  protected :
    void           InitAggreg(Params_t &params, AggregType type) const;
    void           AccumulateAggreg(Params_t &src, const Params_t &dst) const;
    void           NormalizeAggreg(Params_t &src) const;
    float          AccumulateSUM(const float val1, const float val2) const;
    float          AccumulateRMS(const float val1, const float val2) const;
    complex<float> AccumulateRMS(const complex<float> &val1, const complex<float> &val2) const;
    bool           CheckRights(HTTPRequest &req, const uint neededAccess) const;
  protected :
    virtual void GetAnalyzer(HTTPRequest &req) = 0;
    virtual void SetAnalyzer(HTTPRequest &req) = 0;
    virtual void GetSeries(HTTPRequest &req) = 0;
    virtual void GetSeriesNow(HTTPRequest &req) = 0;
    virtual void GetSeriesRange(HTTPRequest &req) = 0;
    virtual void GetEvents(HTTPRequest &req) = 0;
    virtual void GetEvent(HTTPRequest &req) = 0;
    virtual void SetEvent(HTTPRequest &req) = 0;
    virtual void DelEvent(HTTPRequest &req) = 0;
    virtual void GetTariffs(HTTPRequest &req) = 0;
    virtual void GetTariff(HTTPRequest &req) = 0;
    virtual void SetTariff(HTTPRequest &req) = 0;
    virtual void DelTariff(HTTPRequest &req) = 0;
    virtual void AddTariff(HTTPRequest &req) = 0;
    virtual void GetPrices(HTTPRequest &req) = 0;
  public :
    static json  CheckConfig(const json &config);
  public :
    Analyzer(const string &id, const AnalyzerMode mode);
    virtual ~Analyzer();
//    float        GetMaxCurrent() const;
//    float        GetNominalVoltage() const;
//    float        GetNominalFrequency() const;
//    AnalyzerMode Mode() const;
    string Serial() const;
    json   GetJSONDescriptor() const;
    void   GetAnalyzerCallback(HTTPRequest &req);
    void   SetAnalyzerCallback(HTTPRequest &req);
    void   GetSeriesCallback(HTTPRequest &req);
    void   GetSeriesNowCallback(HTTPRequest &req);
    void   GetSeriesRangeCallback(HTTPRequest &req);
    void   GetEventsCallback(HTTPRequest &req);
    void   GetEventCallback(HTTPRequest &req);
    void   SetEventCallback(HTTPRequest &req);
    void   DelEventCallback(HTTPRequest &req);
    void   GetTariffsCallback(HTTPRequest &req);
    void   GetTariffCallback(HTTPRequest &req);
    void   SetTariffCallback(HTTPRequest &req);
    void   DelTariffCallback(HTTPRequest &req);
    void   AddTariffCallback(HTTPRequest &req);
    void   GetPricesCallback(HTTPRequest &req);
};