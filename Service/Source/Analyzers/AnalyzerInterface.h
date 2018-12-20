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
    enum AggregType { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
  protected :
    static const float FrequencyRanges[];   //Rangos de frecuencias para estadisticas
    static const float VoltageRanges[];     //Rangos de tensiones para estadisticas
    static const float THDRanges[];         //Rangos de distorsion harmonica para estadisticas
    static const float HarmonicsAllowed[];  //Rangos permitidos para cada harmonico
  public :
    typedef struct {
      AggregType              Type;                       //Tipo de agregacion
      uint64_t                Timestamp;                  //Timestamp de la muestra inicial
      uint32_t                Samples;                    //Muestras que han formado esta agregacion
      vector<float>           RMS_V;                      //Valor RMS de la tension durante el bloque
      vector<float>           RMS_I;                      //Valor RMS de la intensidad durante el bloque
      vector<float>           Active_Power;               //Potencia activa (P)
      vector<float>           Apparent_Power;             //Potencia aparente (S)
      vector<float>           Power_Factor;               //Factor de potencia (cos(PHI))
      vector<float>           Active_Energy;              //Energia consumida
      vector<float>           Reactive_Power;             //Reactive power
      vector<float>           Reactive_Energy;            //Reactive energy
      float                   Frequency;                  //Frecuencia
      bool                    Flag;                       //Periodo marcado
      vector<float>           THD_V;                      //Distorsion harmonica de la tension
      vector<float>           THD_I;                      //Distorsion harmonica de la corriente
      vector< vector<float> > VoltageHarmonics;           //Armonicos del canal de tension
      vector< vector<float> > CurrentHarmonics;           //Armonicos del canal de corriente
      vector<float>           Phi;                        //Angulo de Phi
      uint16_t                StatsFrequency[40];         //Distribucion de frecuencias
      uint16_t                StatsVoltage[26];           //Distribucion de tensiones
      uint16_t                StatsTHD[26];               //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      uint16_t                StatsHarmonics[2];          //Numero de muestras que cumplen los limites de harmonicos / no cumplen
      vector< vector<float> > FFT_V;                      //Resultados de la FFT en tension
      vector< vector<float> > FFT_I;                      //Resultados de la FFT en corriente
      vector< vector<float> > Voltage;
      vector< vector<float> > Current;
    } Params_t;
  protected :
    uint64_t      pLastSave;
    const char   *pID;
    float         pNominalVoltage;
    float         pNominalFrequency;
    char         *pName;
    float         pMaxCurrent;
    float         pMinCurrent;
    float         pSamplerate;              //Velocidad de muestreo (NaN if unsuported)
    AnalyzerMode  pMode;
  protected :
    void InitAggreg(Params_t *params, AggregType type);
    void ResetAggreg(Params_t *params);
    void SaveAggreg(Params_t *params);
  public :
    virtual void GetParams(Params_t *params, bool extParams) = 0;
  public :
    AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode);
    virtual ~AnalyzerInterface();
    float        GetMaxCurrent();
    float        GetNominalVoltage();
    float        GetNominalFrequency();
    AnalyzerMode GetMode();
    const char  *GetID();
    json_object *GetJSONDescriptor();
    json_object *GetJSONParams(char *series);
};
