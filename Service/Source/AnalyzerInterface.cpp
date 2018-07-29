/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <string.h>
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
AnalyzerInterface::AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode) : Database(id) {
  pID = id;
  pNominalVoltage = nominal_voltage;
  pNominalFrequency = nominal_frequency;
  pName = name;
  pMaxCurrent = max_current;
  pMinCurrent = min_current;
  pMode = mode;
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_3s  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1m  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_10m (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1h  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_ext_1m  (sampletime_ext bigint NOT NULL PRIMARY KEY, power_factor real[] NOT NULL, thd_v real[] NOT NULL, thd_i real[] NOT NULL, phi real[] NOT NULL, harmonics_v real[] NOT NULL, harmonics_i real[] NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS events (starttime bigint NOT NULL PRIMARY KEY, duration  bigint NOT NULL, samplerate real, type character varying(16), voltage real, percent real, samples real[]) WITH (OIDS=FALSE)");
}
char *AnalyzerInterface::GetID() {
  return pID;
}
float AnalyzerInterface::GetNominalVoltage() {
  return pNominalVoltage;
}
float AnalyzerInterface::GetNominalFrequency() {
  return pNominalFrequency;
}
json_object *AnalyzerInterface::GetJSONDescriptor() {
  json_object *Device = json_object_new_object();
  json_object_object_add(Device, "ID", json_object_new_string(pID));
  json_object_object_add(Device, "NominalVoltage", json_object_new_double(pNominalVoltage));
  json_object_object_add(Device, "NominalFreq", json_object_new_double(pNominalFrequency));
  json_object_object_add(Device, "Name", json_object_new_string(pName));
  json_object_object_add(Device, "MaxCurrent", json_object_new_double(pMaxCurrent));
  json_object_object_add(Device, "MinCurrent", json_object_new_double(pMinCurrent));
  json_object_object_add(Device, "Mode", json_object_new_string((pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N"));
  return Device;
}
json_object *AnalyzerInterface::GetJSONParams(char *series) {
  Params_t    *Params    = NULL;
  ExtParams_t *ExtParams = NULL;
  bool RMS_V = false, RMS_I = false, Active_Power = false, Apparent_Power = false, Power_Factor = false, Active_Energy = false,
       Frequency = false, Flag = false, THD_V = false, THD_I = false, VoltageHarmonics = false, CurrentHarmonics = false, Phi = false,
       Voltage = false, Current = false, FFT_V = false, FFT_I = false, QueryParams = false, QueryParamsExt = false;
  char *Save;
  char *Serie = strtok_r(series, ", ", &Save);
  while(Serie != NULL) {
    if(strcmp(Serie, "rms_v") == 0)               RMS_V = QueryParams = true;
    else if(strcmp(Serie, "rms_i") == 0)          RMS_I = QueryParams = true;
    else if(strcmp(Serie, "active_power") == 0)   Active_Power = QueryParams = true;
    else if(strcmp(Serie, "apparent_power") == 0) Apparent_Power = QueryParams = true;
    else if(strcmp(Serie, "power_factor") == 0)   Power_Factor = QueryParams = true;
    else if(strcmp(Serie, "active_energy") == 0)  Active_Energy = QueryParams = true;
    else if(strcmp(Serie, "freq") == 0)           Frequency = QueryParams = true;
    else if(strcmp(Serie, "flag") == 0)           Flag = QueryParams = true;
    else if(strcmp(Serie, "thd_v") == 0)          THD_V = QueryParams = true;
    else if(strcmp(Serie, "thd_i") == 0)          THD_I = QueryParams = true;
    else if(strcmp(Serie, "harmonics_v") == 0)    VoltageHarmonics = QueryParams = true;
    else if(strcmp(Serie, "harmonics_i") == 0)    CurrentHarmonics = QueryParams = true;
    else if(strcmp(Serie, "phi") == 0)            Phi = QueryParams = true;
    else if(strcmp(Serie, "wave_v") == 0)         Voltage = QueryParams = QueryParamsExt = true;
    else if(strcmp(Serie, "wave_i") == 0)         Current = QueryParams = QueryParamsExt = true;
    else if(strcmp(Serie, "fft_v") == 0)          FFT_V = QueryParams = QueryParamsExt = true;
    else if(strcmp(Serie, "fft_i") == 0)          FFT_I = QueryParams = QueryParamsExt = true;
    Serie = strtok_r(NULL, ", ", &Save);
  }
  GetParams(QueryParams ? &Params : NULL, QueryParamsExt ? &ExtParams : NULL);
  json_object *Result = json_object_new_object();
  uint8_t VoltageChannels = ((pMode == PHASE1) ? 1 : 3);
  uint8_t CurrentChannels = ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
  if(RMS_V && Params) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params->RMS_V[i]));
    json_object_object_add(Result, Serie, Obj);
  }
    
//    if(strcmp(Serie, "rms_i") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(Params->RMS_I[i]));
//    } else if(strcmp(Serie, "active_power") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->Active_Power[i]));
//    } else if(strcmp(Serie, "apparent_power") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->Apparent_Power[i]));
//    } else if(strcmp(Serie, "power_factor") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->Power_Factor[i]));
//    } else if(strcmp(Serie, "active_energy") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->Active_Energy[i]));
//    } else if(strcmp(Serie, "thd_v") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->THD_V[i]));
//    } else if(strcmp(Serie, "thd_i") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(Params->THD_I[i]));
//    } else if(strcmp(Serie, "freq") == 0) {
//      if(isnan(Params->Frequency) == false) Obj = json_object_new_double(Params->Frequency);
//    } else if(strcmp(Serie, "phi") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params->Phi[i]));
//    } else if(strcmp(Serie, "flag") == 0) {
//      Obj = json_object_new_boolean(Params->Flag);
//    } else if(strcmp(Serie, "harmonics_v") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
//        json_object *Obj2 = json_object_new_array();
//        for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(Params->VoltageHarmonics[i][j]));
//        json_object_array_add(Obj, Obj2);
//      }
//    } else if(strcmp(Serie, "harmonics_i") == 0) {
//      Obj = json_object_new_array();
//      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
//        json_object *Obj2 = json_object_new_array();
//        for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(Params->CurrentHarmonics[i][j]));
//        json_object_array_add(Obj, Obj2);
//      }
//    } else if(strcmp(Serie, "wave_v") == 0) {
////      Obj = json_object_new_array();
////      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
////        json_object *Obj2 = json_object_new_array();
////        for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageSamples[i][j]));
////        json_object_array_add(Obj, Obj2);
////      }
//    } else if(strcmp(Serie, "wave_i") == 0) {
////      Obj = json_object_new_array();
////      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
////        json_object *Obj2 = json_object_new_array();
////        for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pCurrentSamples[i][j]));
////        json_object_array_add(Obj, Obj2);
////      }
//    } else if(strcmp(Serie, "fft_v") == 0) {
////      Obj = json_object_new_array();
////      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
////        json_object *Obj2 = json_object_new_array();
////        for(int j = 0; j < pVoltageFFT[i]->OutputLen; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageFFT[i]->BinPower(j)));
////        json_object_array_add(Obj, Obj2);
////      }
//    } else if(strcmp(Serie, "fft_i") == 0) {
////      Obj = json_object_new_array();
////      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
////        json_object *Obj2 = json_object_new_array();
////        for(int j = 0; j < pCurrentFFT[i]->OutputLen; j++) json_object_array_add(Obj2, json_object_new_double(pCurrentFFT[i]->BinPower(j)));
////        json_object_array_add(Obj, Obj2);
////      }
//    }
//    if(Obj != NULL) json_object_object_add(Result, Serie, Obj);
//    Serie = strtok_r(NULL, ", ", &Save);
//  }
  if(Params) json_object_object_add(Result, "Sampletime", json_object_new_int64(Params->Timestamp));
  //Libero lo que haya pedido
  return Result;
}

void AnalyzerInterface::SaveAggreg(Params_t *params) {
  if(params->Type == AGGREG_3S)
    PrepareSentence("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1M)  
    PrepareSentence("INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_10M)
    PrepareSentence("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1H)
    PrepareSentence("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt64(params->Timestamp);                                                                       //sampletime
  BindArrayFloat(params->RMS_V, (pMode == PHASE1) ? 1 : 3);                                                  //rms_v 
  BindArrayFloat(params->RMS_I, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);                          //rms_i
  BindArrayFloat(params->Active_Power, (pMode == PHASE1) ? 1 : 3);                                           //active_power
  BindArrayFloat(params->Active_Energy, (pMode == PHASE1) ? 1 : 3);                                          //active_energy
  BindArrayFloat(params->Reactive_Power, (pMode == PHASE1) ? 1 : 3);                                         //reactive_power
  BindArrayFloat(params->Reactive_Energy, (pMode == PHASE1) ? 1 : 3);                                        //reactive_energy
  BindSingleFloat(params->Frequency);                                                                        //frequency
  BindSingleBool(params->Flag);                                                                              //flaged
  ExecSentence();
  if(params->Type == AGGREG_1M) {
    PrepareSentence("INSERT INTO aggreg_ext_1m(sampletime_ext, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) VALUES(?, ?, ?, ?, ?, ?, ?)");  
    BindSingleUInt64(params->Timestamp);                                                                     //sampletime
    BindArrayFloat(params->Power_Factor, (pMode == PHASE1) ? 1 : 3);                                         //power_factor
    BindArrayFloat(params->THD_V, (pMode == PHASE1) ? 1 : 3);                                                //thd_v
    BindArrayFloat(params->THD_I, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);                        //thd_i
    BindArrayFloat(params->Phi, (pMode == PHASE1) ? 1 : 3);                                                  //phi
    BindArrayArrayFloat((float*)params->VoltageHarmonics, (pMode == PHASE1) ? 1 : 3, 50);                         //harmonics_v
    BindArrayArrayFloat((float*)params->CurrentHarmonics, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 50); //harmonics_i
    ExecSentence();
    PrepareSentence("DELETE FROM aggreg_ext_1m WHERE sampletime_ext < ?");
    BindSingleUInt64(params->Timestamp - 86400000);                                                          //sampletime
    ExecSentence();
  }
  ResetAggreg(params);
}
void AnalyzerInterface::AccumulateAggreg(Params_t *dst, Params_t *src) {
  dst->Timestamp = src->Timestamp;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->RMS_V[i] += pow(src->RMS_V[i], 2.0);
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) dst->RMS_I[i] += pow(src->RMS_I[i], 2.0);
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Active_Power[i] += src->Active_Power[i];
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Active_Energy[i] += src->Active_Energy[i];
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Reactive_Power[i] += src->Reactive_Power[i];
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Reactive_Energy[i] += src->Reactive_Energy[i];
  dst->Flag |= src->Flag;
  dst->Frequency += src->Frequency;
  if(dst->Type == AGGREG_1M) {
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Power_Factor[i] += src->Power_Factor[i];
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) dst->THD_I[i] += pow(src->THD_I[i], 2.0);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
      for(int j = 0; j < 50; j++) dst->CurrentHarmonics[i][j] += pow(src->CurrentHarmonics[i][j], 2.0);
    }
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Phi[i] += src->Phi[i];
  }
  if((dst->Type == AGGREG_1M) || (dst->Type == AGGREG_10M)) {
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->THD_V[i] += pow(src->THD_V[i], 2.0);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
      for(int j = 0; j < 50; j++) dst->VoltageHarmonics[i][j] += pow(src->VoltageHarmonics[i][j], 2.0);
    }
  }
  dst->Samples++;
}
void AnalyzerInterface::NormalizeAggreg(Params_t *params) {
  if(params->Samples > 1) {
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->RMS_V[i] = sqrt(params->RMS_V[i] / params->Samples);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) params->RMS_I[i] = sqrt(params->RMS_I[i] / params->Samples);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Active_Power[i] = params->Active_Power[i] / params->Samples;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Reactive_Power[i] = params->Reactive_Power[i] / params->Samples;
    params->Frequency = params->Frequency / params->Samples;
    if(params->Type == AGGREG_1M) {
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Power_Factor[i] = params->Power_Factor[i] / params->Samples;
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->THD_I[i] = sqrt(params->THD_I[i] / params->Samples);
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Phi[i] = params->Phi[i] / params->Samples;
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
        for(int j = 0; j < 50; j++) params->CurrentHarmonics[i][j] = sqrt(params->CurrentHarmonics[i][j] / params->Samples);
      }
    }
    if((params->Type == AGGREG_1M) || (params->Type == AGGREG_1M)) {
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->THD_V[i] = sqrt(params->THD_V[i] / params->Samples);
      for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
        for(int j = 0; j < 50; j++) params->VoltageHarmonics[i][j] = sqrt(params->VoltageHarmonics[i][j] / params->Samples);
      }    
    }
  }
}
void AnalyzerInterface::ResetAggreg(Params_t *params) {
  params->Timestamp = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->RMS_V[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) params->RMS_I[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Active_Power[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Reactive_Power[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Reactive_Energy[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Active_Energy[i] = 0;
  params->Frequency = 0;
  params->Flag = false;
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M)) {
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->THD_I[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Phi[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Power_Factor[i] = 0; 
    memset(params->CurrentHarmonics, 0, sizeof(params->CurrentHarmonics));
  }
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M) || (params->Type == AGGREG_10M)) {
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->THD_V[i] = 0;
    memset(params->VoltageHarmonics, 0, sizeof(params->VoltageHarmonics));
  }
  if(params->Type == AGGREG_10C) {
//    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Active_Power[i] = 0;
//    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Apparent_Power[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Apparent_Power[i] = 0;
//    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Power_Factor[i] = 0;
  }
  params->Samples = 0;
}






//  pDriverName      = driver;
//  pSamplerate      = 20000;
//  pMaxCurrent      = 50;
//  pMinCurrent      = 0.1;
//  pChannelMappings = NULL;
//  pActive          = false;

//  pBuffLen         = 0;
//  pLastUpdate      = 0;
//}
//bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate) {
//  pSamplerate      = samplerate;
//  return (pVoltageSamples != 0) && (pCurrentSamples != 0);
//}
//void DeviceInterface::BuffersReset() {
//  pUsed     = 0;
//  pWritePos = 0;
//  pReadPos  = 0;
//}



//  free(pChannelMappings);
//  if(pVoltageSamples) free(pVoltageSamples);
//  if(pCurrentSamples) free(pCurrentSamples);