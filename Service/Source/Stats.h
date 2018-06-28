/* ================================== zMeter ===========================================================================
 * File:     Stats.h
 * Author:   Eduardo Viciana (30 de mayo de 2018)
 * ------------------------------------------------------------------------------------------------------------------ */    
#pragma once
#include <stdint.h>
#include "Database.h"
class Stats {
  private :
    static const float FrequencyRanges[];   //Rangos de frecuencias para estadisticas
    static const float VoltageRanges[];     //Rangos de tensiones para estadisticas
    static const float THDRanges[];         //Rangos de distorsion harmonica para estadisticas
    static const float HarmonicsAllowed[];  //Rangos permitidos para cada harmonico
  private :
    float     pNominalVoltage;              //Nominal voltage
    float     pNominalFreq;                 //Nominal frequency
    uint16_t *pFrequency;                   //Distribucion de frecuencias
    uint16_t *pVoltage;                     //Distribucion de tensiones
    uint16_t *pTHD;                         //Cada posicion representa un rango desde 0% hasta 8% (y el ultimo otros valores)
    uint16_t  pHarmonics[2];                //Numero de muestras que cumplen los limites de harmonicos / no cumplen
    uint8_t   pChannels;                    //Numero de canales de la muestra
    Database *pDB;                          //Database donde se guardan estadisticas
  private :
    void  ResetStats();
    float PercentChange(float in, float reference);
  public :
    Stats(uint8_t channels, Database *db, float volt, float freq);
    void AppendFreq(float freq, bool flag);
    void AppendVoltage(float *voltage, bool flag);
    void AppendTHD(float *thd, bool flag);
    void AppendHarmonics(float *harmonics, bool flag);
    void SaveStats(uint64_t timestamp);
};