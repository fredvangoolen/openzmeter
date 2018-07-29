/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
class AnalyzerInterface : protected Database {
  public :
    enum AnalyzerMode {PHASE1, PHASE3, PHASE3N};
    enum AggregType { AGGREG_10C, AGGREG_3S, AGGREG_1M, AGGREG_10M, AGGREG_1H };
  protected :
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
    } Params_t;
    typedef struct {
      float     *Voltage[3];
      float     *Current[4];
      float     *FFT_V[3];
      float     *FFT_i[4];
    } ExtParams_t;
  protected :
    char         *pID;
    float         pNominalVoltage;
    float         pNominalFrequency;
    char         *pName;
    float         pMaxCurrent;
    float         pMinCurrent;
    AnalyzerMode  pMode;
  protected :
    void SaveAggreg(Params_t *params);
    void AccumulateAggreg(Params_t *dst, Params_t *src);
    void NormalizeAggreg(Params_t *params);
    void ResetAggreg(Params_t *params);
  protected :
    virtual void GetParams(Params_t **params, ExtParams_t **extParams) = 0;
  public :
    AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode);
    float        GetNominalVoltage();
    float        GetNominalFrequency();
    char        *GetID();
    json_object *GetJSONDescriptor();
    json_object *GetJSONParams(char *series);
};
