/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class AnalyzerInterface : protected Database {
  public :
    enum AnalyzerMode {PHASE1, PHASE3, PHASE3N};
    enum AggregType { UNINITIALIZED, AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_15M, AGGREG_1H };
    enum ParamsType { FREQUENCY = 0x00000001, FLAG = 0x00000002, RMS_V = 0x00000004, RMS_I = 0x00000008, ACTIVE_POWER = 0x00000010, REACTIVE_POWER = 0x00000020, ACTIVE_ENERGY = 0x00000040, REACTIVE_ENERGY = 0x00000080, APPARENT_POWER = 0x00000100, POWER_FACTOR = 0x00000200, THD_V = 0x00000400, THD_I = 0x00000800, PHASE_V = 0x00001000, PHI = 0x00002000, HARMONICS_V = 0x00004000, HARMONICS_I = 0x00008000, FFT_V = 0x00010000, FFT_I = 0x00020000, VOLTAGE = 0x00040000, CURRENT = 0x00080000 };
  protected :
    static vector<float>  FrequencyRanges;   //Rangos de frecuencias para estadisticas
    static vector<float>  VoltageRanges;     //Rangos de tensiones para estadisticas
    static vector<float>  THDRanges;         //Rangos de distorsion harmonica para estadisticas
    static vector<float>  HarmonicsAllowed;  //Rangos permitidos para cada harmonico
  public :
    typedef struct {
      AggregType              Type;                       //Aggregation type
      float                   Frequency;                  //Frecuencia
      bool                    Flag;                       //Periodo marcado
      uint64_t                Timestamp;                  //Aggregation timestampt
      uint16_t                RMS_V_Inputs;               //Valid RMS_V samples in aggregation
      uint16_t                Freq_Inputs;                //Valid Freq samples in aggregation
      vector<uint16_t>        StatsFrequency;             //Distribucion de frecuencias
      vector<uint16_t>        StatsVoltage;               //Distribucion de tensiones
      vector<uint16_t>        StatsTHD;                   //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      vector<uint16_t>        StatsHarmonics;             //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector<uint16_t>        RMS_I_Inputs;               //Valid RMS_I samples in aggregation
      vector<float>           RMS_V;                      //Aggregation average RMS voltage
      vector<float>           RMS_I;                      //Aggregation average RMS current
      vector<float>           Active_Power;               //Aggregation average Active Power (P)
      vector<float>           Reactive_Power;             //Reactive power
      vector<float>           Active_Energy;              //Energia consumida
      vector<float>           Reactive_Energy;            //Reactive energy
      vector<float>           Apparent_Power;             //Potencia aparente (S)
      vector<float>           Power_Factor;               //Factor de potencia (cos(PHI))
      vector<float>           THD_V;                      //Distorsion harmonica de la tension
      vector<float>           THD_I;                      //Distorsion harmonica de la corriente
      vector<float>           Phase_V;                    //Angulo entre canales de tension
      vector<float>           Phi;                        //Angulo de Phi
      vector< vector<float> > VoltageHarmonics;           //Armonicos del canal de tension
      vector< vector<float> > CurrentHarmonics;           //Armonicos del canal de corriente
      vector< vector<float> > FFT_V;                      //Resultados de la FFT en tension
      vector< vector<float> > FFT_I;                      //Resultados de la FFT en corriente
      vector< vector<float> > Voltage;                    //Voltage samples
      vector< vector<float> > Current;                    //Current samples
    } Params_t;
  protected :
    uint64_t      pLastSave;
    float         pNominalVoltage;
    float         pNominalFrequency;
    char         *pName;
    float         pMaxCurrent;
    float         pMinCurrent;
    float         pSamplerate;              //Velocidad de muestreo (NaN if unsuported)
    AnalyzerMode  pMode;
  protected :
    void  InitAggreg(Params_t &params, AggregType type);
    void  ResetAggreg(Params_t &params);
    void  SaveAggreg(Params_t &params);
    void  AccumulateAggreg(Params_t &src, Params_t &dst);
    void  NormalizeAggreg(Params_t &src);
    float AccumulateSUM(float val1, float val2);
    float AccumulateRMS(float val1, float val2);
  public :
    virtual void GetParams(Params_t &params, ParamsType &copy) = 0;
  public :
    AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode);
    virtual ~AnalyzerInterface();
    float        GetMaxCurrent();
    float        GetNominalVoltage();
    float        GetNominalFrequency();
    AnalyzerMode GetMode();
    const char  *GetID();
    json_object *GetJSONDescriptor();
    json_object *GetJSONParams(json_object *series);
};
