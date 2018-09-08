/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.cpp
 * Author:   Eduardo Viciana (agosto de 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "AnalyzerRemote.h"
AnalyzerRemote::AnalyzerRemote(char* id, float nominalVoltage, float nominalFreq, char* name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online) : AnalyzerInterface(id, nominalVoltage, nominalFreq, name, maxCurrent, minCurrent, mode) {
 //llamo al update config
}
void AnalyzerRemote::UpdateConfig(float nominalVoltage, float nominalFreq, char* name, float maxCurrent, float minCurrent, AnalyzerMode mode, float sampleFreq, bool online) {
  //Si ha cambiado el modo tengo que asegurarme que las muestras que hay son coherentes, si no la web fallara!
}
void AnalyzerRemote::GetParams(Params_t *params, ExtParams_t *extParams) {

}
uint64_t AnalyzerRemote::SaveJSON(json_object *json, Database *db) {
  json_object *Find;
  char *Table = NULL;
 
  if((json_object_object_get_ex(json, "Table", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Aggreg = (char*)json_object_get_string(Find);
  
//  
//  if((json_object_object_get_ex(config, "aggreg", &Find) == true) && (json_object_get_type(Find) == json_type_string)) {
//    if(ReadJSONFloatArray(Find, pVoltageGain, 3) == false) {
//      Tools::LogMsg("ERROR", "CaptureV2: Voltage gain must have 3 double values\n");
//      return;
//    }
//  }
//  
//    if(json_object_array_length(object) != len) return false;
//  for(int i = 0; i < len; i++) {
//    if(json_object_get_type(json_object_array_get_idx(object, i)) != json_type_double) return false;
//    dst[i] = json_object_get_double(json_object_array_get_idx(object, i));
//  }
//  return true;
//  
//  
//  
//  
//  
//  
//  
//        AggregType Type;                          //Tipo de agregacion
//      uint64_t   Timestamp;                     //Timestamp de la muestra inicial
//      uint32_t   Samples;                       //Muestras que han formado esta agregacion
//      float      RMS_V[3];                      //Valor RMS de la tension durante el bloque
//      float      RMS_I[4];                      //Valor RMS de la intensidad durante el bloque
//      float      Active_Power[3];               //Potencia activa (P)
//      float      Apparent_Power[3];             //Potencia aparente (S)
//      float      Power_Factor[3];               //Factor de potencia (cos(PHI))
//      float      Active_Energy[3];              //Energia consumida
//      float      Reactive_Power[3];             //Reactive power
//      float      Reactive_Energy[3];            //Reactive energy
//      float      Frequency;                     //Frecuencia
//      bool       Flag;                          //Periodo marcado
//      float      THD_V[3];                      //Distorsion harmonica de la tension
//      float      THD_I[4];                      //Distorsion harmonica de la corriente
//      float      VoltageHarmonics[3][50];       //Armonicos del canal de tension
//      float      CurrentHarmonics[4][50];       //Armonicos del canal de corriente
//      float      Phi[3];                        //Angulo de Phi
//    } Params_t;
  
  return true;

}