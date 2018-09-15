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
      AggregType Type;                          //Tipo de agregacion
      uint64_t   Timestamp;                     //Timestamp de la muestra inicial
      uint32_t   Samples;                       //Muestras que han formado esta agregacion
      float      RMS_V[3];                      //Valor RMS de la tension durante el bloque
      float      RMS_I[4];                      //Valor RMS de la intensidad durante el bloque
      float      Active_Power[3];               //Potencia activa (P)
      float      Apparent_Power[3];             //Potencia aparente (S)
      float      Power_Factor[3];               //Factor de potencia (cos(PHI))
      float      Active_Energy[3];              //Energia consumida
      float      Reactive_Power[3];             //Reactive power
      float      Reactive_Energy[3];            //Reactive energy
      float      Frequency;                     //Frecuencia
      bool       Flag;                          //Periodo marcado
      float      THD_V[3];                      //Distorsion harmonica de la tension
      float      THD_I[4];                      //Distorsion harmonica de la corriente
      float      VoltageHarmonics[3][50];       //Armonicos del canal de tension
      float      CurrentHarmonics[4][50];       //Armonicos del canal de corriente
      float      Phi[3];                        //Angulo de Phi
      uint16_t   StatsFrequency[40];            //Distribucion de frecuencias
      uint16_t   StatsVoltage[26];              //Distribucion de tensiones
      uint16_t   StatsTHD[26];                  //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
      uint16_t   StatsHarmonics[2];             //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    } Params_t;
    typedef struct {
      float     *Voltage[3];
      float     *Current[4];
      vector< vector<float> >     FFT_V;
      vector< vector<float> >     FFT_I;
    } ExtParams_t;
  protected :
    uint64_t      pLastSave;
    char         *pID;
    float         pNominalVoltage;
    float         pNominalFrequency;
    char         *pName;
    float         pMaxCurrent;
    float         pMinCurrent;
    float         pSamplerate;              //Velocidad de muestreo (NaN if unsuported)
    AnalyzerMode  pMode;
  protected :
    void ResetAggreg(Params_t *params);
    void SaveAggreg(Params_t *params);
  public :
    virtual void GetParams(Params_t *params, ExtParams_t *extParams) = 0;
  public :
    AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode);
    virtual ~AnalyzerInterface();
    float        GetMaxCurrent();
    float        GetNominalVoltage();
    float        GetNominalFrequency();
    char        *GetID();
    json_object *GetJSONDescriptor();
    json_object *GetJSONParams(char *series);
};
