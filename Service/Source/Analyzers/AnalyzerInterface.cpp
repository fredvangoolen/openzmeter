/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <string.h>
#include "AnalyzerInterface.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
const float AnalyzerInterface::FrequencyRanges[]  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
const float AnalyzerInterface::VoltageRanges[]    = {-INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY};
const float AnalyzerInterface::THDRanges[]        = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY};
const float AnalyzerInterface::HarmonicsAllowed[] = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
//----------------------------------------------------------------------------------------------------------------------
AnalyzerInterface::AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode) : Database(id) {
  pID = id;
  pNominalVoltage = nominal_voltage;
  pNominalFrequency = nominal_frequency;
  pName = name;
  pMaxCurrent = max_current;
  pMinCurrent = min_current;
  pMode = mode;
  pLastSave = 0;
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_3s  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1m  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_10m (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1h  (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[] NOT NULL, rms_i real[] NOT NULL, active_power real[] NOT NULL, active_energy real[] NOT NULL, reactive_power real[] NOT NULL, reactive_energy real[] NOT NULL, freq real, flag boolean NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_ext_1m  (sampletime bigint NOT NULL PRIMARY KEY REFERENCES aggreg_1m(sampletime) ON UPDATE CASCADE ON DELETE CASCADE, power_factor real[] NOT NULL, thd_v real[] NOT NULL, thd_i real[] NOT NULL, phi real[] NOT NULL, harmonics_v real[] NOT NULL, harmonics_i real[] NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_stat_1h (sampletime bigint NOT NULL PRIMARY KEY REFERENCES aggreg_1h(sampletime) ON UPDATE CASCADE ON DELETE CASCADE, stat_voltage int[] NOT NULL, stat_frequency int[] NOT NULL, stat_thd int[] NOT NULL, stat_harmonics int[] NOT NULL) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS events (eventtime bigint NOT NULL PRIMARY KEY, duration  bigint NOT NULL, samplerate real, type character varying(16), voltage real, percent real, samples real[]) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS prices (sampletime bigint NOT NULL PRIMARY KEY, price real NOT NULL) WITH (OIDS=FALSE)");
}
AnalyzerInterface::~AnalyzerInterface() {}
float AnalyzerInterface::GetMaxCurrent() {
  return pMaxCurrent;
}
const char *AnalyzerInterface::GetID() {
  return pID;
}
float AnalyzerInterface::GetNominalVoltage() {
  return pNominalVoltage;
}
float AnalyzerInterface::GetNominalFrequency() {
  return pNominalFrequency;
}
void AnalyzerInterface::InitAggreg(Params_t* params, AggregType type) {
  params->Type = type;
  params->RMS_V.resize((pMode == PHASE1) ? 1 : 3);
  params->RMS_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
  params->Active_Power.resize((pMode == PHASE1) ? 1 : 3);
  params->Reactive_Power.resize((pMode == PHASE1) ? 1 : 3);
  params->Active_Energy.resize((pMode == PHASE1) ? 1 : 3);
  params->Reactive_Energy.resize((pMode == PHASE1) ? 1 : 3);
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M)) {
    params->THD_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params->Phi.resize((pMode == PHASE1) ? 1 : 3);
    params->Power_Factor.resize((pMode == PHASE1) ? 1 : 3);
    params->CurrentHarmonics.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    for(uint i = 0; i < params->CurrentHarmonics.size(); i++) params->CurrentHarmonics[i].resize(50);
  }
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M) || (params->Type == AGGREG_10M)) {
    params->THD_V.resize((pMode == PHASE1) ? 1 : 3);
    params->VoltageHarmonics.resize((pMode == PHASE1) ? 1 : 3);
    for(uint i = 0; i < params->VoltageHarmonics.size(); i++) params->VoltageHarmonics[i].resize(50);    
  }
  if(params->Type == AGGREG_10C) {
    params->Apparent_Power.resize((pMode == PHASE1) ? 1 : 3);
    params->Current.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params->Voltage.resize((pMode == PHASE1) ? 1 : 3);
  }
  ResetAggreg(params);
}
void AnalyzerInterface::ResetAggreg(Params_t *params) {
  params->Timestamp = 0;
  for(uint i = 0; i < params->RMS_V.size(); i++) params->RMS_V[i] = 0;
  for(uint i = 0; i < params->RMS_I.size(); i++) params->RMS_I[i] = 0;
  for(uint i = 0; i < params->Active_Power.size(); i++) params->Active_Power[i] = 0;
  for(uint i = 0; i < params->Reactive_Power.size(); i++) params->Reactive_Power[i] = 0;
  for(uint i = 0; i < params->Reactive_Energy.size(); i++) params->Reactive_Energy[i] = 0;
  for(uint i = 0; i < params->Active_Energy.size(); i++) params->Active_Energy[i] = 0;
  params->Frequency = 0;
  params->Flag = false;
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M)) {
    for(uint i = 0; i < params->THD_I.size(); i++) params->THD_I[i] = 0;
    for(uint i = 0; i < params->Phi.size(); i++) params->Phi[i] = 0;
    for(uint i = 0; i < params->Power_Factor.size(); i++) params->Power_Factor[i] = 0;
    for(uint i = 0; i < params->CurrentHarmonics.size(); i++) {
      for(uint n = 0; n < params->CurrentHarmonics[i].size(); n++) params->CurrentHarmonics[i][n] = 0.0;
    }
  }
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M) || (params->Type == AGGREG_10M)) {
    for(uint i = 0; i < params->THD_V.size(); i++) params->THD_V[i] = 0;
    for(uint i = 0; i < params->VoltageHarmonics.size(); i++) {
      for(uint n = 0; n < params->VoltageHarmonics[i].size(); n++) params->VoltageHarmonics[i][n] = 0.0;
    }
  }
  if(params->Type == AGGREG_10C) {
    for(uint i = 0; i < params->Apparent_Power.size(); i++) params->Apparent_Power[i] = 0;
    for(uint i = 0; i < params->Voltage.size(); i++) params->Voltage[i].resize(0);
    for(uint i = 0; i < params->Current.size(); i++) params->Current[i].resize(0);
  }
  if(params->Type == AGGREG_1H) {
    for(uint i = 0; i < 40; i++) params->StatsFrequency[i] = 0;
    for(uint i = 0; i < 26; i++) params->StatsVoltage[i] = 0;
    for(uint i = 0; i < 26; i++) params->StatsTHD[i] = 0;
    for(uint i = 0; i < 2; i++)  params->StatsHarmonics[i] = 0;
  }
  params->Samples = 0;
}
void AnalyzerInterface::SaveAggreg(Params_t *params) {
  if(params->Type == AGGREG_3S)
    PrepareSentence("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1M)  
    PrepareSentence("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) RETURNING sampletime) "
                    "INSERT INTO aggreg_ext_1m(sampletime, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) SELECT sampletime, ?, ?, ?, ?, ?, ? FROM Sample");
  else if(params->Type == AGGREG_10M)
    PrepareSentence("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1H)
    PrepareSentence("WITH Sample AS (INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, active_power, active_energy, reactive_power, reactive_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?) RETURNING sampletime) "
                    "INSERT INTO aggreg_stat_1h(sampletime, stat_voltage, stat_frequency, stat_thd, stat_harmonics) SELECT sampletime, ?, ?, ?, ? FROM Sample");
  BindSingleUInt64(params->Timestamp);                                                 //sampletime
  BindArrayFloat(params->RMS_V);                                                       //rms_v 
  BindArrayFloat(params->RMS_I);                                                       //rms_i
  BindArrayFloat(params->Active_Power);                                                //active_power
  BindArrayFloat(params->Active_Energy);                                               //active_energy
  BindArrayFloat(params->Reactive_Power);                                              //reactive_power
  BindArrayFloat(params->Reactive_Energy);                                             //reactive_energy
  BindSingleFloat(params->Frequency);                                                  //frequency
  BindSingleBool(params->Flag);                                                        //flaged
  if(params->Type == AGGREG_1M) {
    BindArrayFloat(params->Power_Factor);                                              //power_factor
    BindArrayFloat(params->THD_V);                                                     //thd_v
    BindArrayFloat(params->THD_I);                                                     //thd_i
    BindArrayFloat(params->Phi);                                                       //phi
    BindArrayArrayFloat(params->VoltageHarmonics, (pMode == PHASE1) ? 1 : 3, 50);                         //harmonics_v
    BindArrayArrayFloat(params->CurrentHarmonics, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 50); //harmonics_i
  } else if(params->Type == AGGREG_1H) {
    BindArrayUInt16(params->StatsVoltage, 26);        //estadisticas de tension
    BindArrayUInt16(params->StatsFrequency, 40);      //estadisticas de frequencia
    BindArrayUInt16(params->StatsTHD, 26);            //estadisticas de distorsion harmonica
    BindArrayUInt16(params->StatsHarmonics, 2);       //estadisticas de harmonicos en rango
  }
  ExecSentence();
  if(params->Type == AGGREG_1M) {
    PrepareSentence("DELETE FROM aggreg_ext_1m WHERE sampletime < ?");
    BindSingleUInt64(params->Timestamp - 86400000);   //Delete 1 day old
    ExecSentence();
  }
  pLastSave = Tools::GetTime();
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
  json_object_object_add(Device, "SampleFreq", json_object_new_double(pSamplerate));
  json_object_object_add(Device, "Online", json_object_new_boolean((Tools::GetTime() - pLastSave) < 20000));
  return Device;
}
json_object *AnalyzerInterface::GetJSONParams(char *series) {
  Params_t    Params;
  InitAggreg(&Params, AGGREG_10C);
//  ExtParams_t ExtParams;
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
  GetParams(QueryParams ? &Params : NULL, QueryParamsExt);
  json_object *Result = json_object_new_object();
  uint8_t VoltageChannels = ((pMode == PHASE1) ? 1 : 3);
  uint8_t CurrentChannels = ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
  if(RMS_V) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.RMS_V[i]));
    json_object_object_add(Result, "rms_v", Obj);
  }
  if(RMS_I) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < CurrentChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.RMS_I[i]));
    json_object_object_add(Result, "rms_i", Obj);
  }
  if(Active_Power) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.Active_Power[i]));
    json_object_object_add(Result, "active_power", Obj);
  }
  if(Apparent_Power) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.Apparent_Power[i]));
    json_object_object_add(Result, "apparent_power", Obj);
  }
  if(Power_Factor) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.Power_Factor[i]));
    json_object_object_add(Result, "power_factor", Obj);
  }
  if(Active_Energy) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(Params.Active_Energy[i]));
    json_object_object_add(Result, "active_energy", Obj);
  }
  if(THD_V) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.THD_V[i]));
    json_object_object_add(Result, "thd_v", Obj);
  }
  if(THD_I) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < CurrentChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.THD_I[i]));
    json_object_object_add(Result, "thd_i", Obj);
  }
  if(Frequency) {
    if(isnan(Params.Frequency) == false) json_object_object_add(Result, "freq", json_object_new_double(Params.Frequency));
  }
  if(Phi) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) json_object_array_add(Obj, json_object_new_double(Params.Phi[i]));
    json_object_object_add(Result, "phi", Obj);
  }
  if(Flag) {
    json_object_object_add(Result, "flag", json_object_new_boolean(Params.Flag));
  }
  if(VoltageHarmonics) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(Params.VoltageHarmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "harmonics_v", Obj);
  }
  if(CurrentHarmonics) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < CurrentChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(Params.CurrentHarmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "harmonics_i", Obj);
  }
  if(Voltage) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint32_t j = 0; j < Params.Samples; j++) json_object_array_add(Obj2, json_object_new_double(Params.Voltage[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "wave_v", Obj);
  }
  if(Current) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < CurrentChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint32_t j = 0; j < Params.Samples; j++) json_object_array_add(Obj2, json_object_new_double(Params.Current[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "wave_i", Obj);
  }
  if(FFT_V && (Params.Samples > 0)) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < VoltageChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < ((Params.Samples / 2) + 1); j++) json_object_array_add(Obj2, json_object_new_double(Params.FFT_V[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "fft_v", Obj);
  }
  if(FFT_I && (Params.Samples > 0)) {
    json_object *Obj = json_object_new_array();
    for(int i = 0; i < CurrentChannels; i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < ((Params.Samples / 2) + 1); j++) json_object_array_add(Obj2, json_object_new_double(Params.FFT_I[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "fft_i", Obj);
  }
  if(QueryParams) json_object_object_add(Result, "Sampletime", json_object_new_int64(Params.Timestamp));
  if(QueryParamsExt) {
//    for(int i = 0; i < VoltageChannels; i++) free(Params.Voltage[i]);
//    for(int i = 0; i < CurrentChannels; i++) free(Params.Current[i]);
//    for(int i = 0; i < VoltageChannels; i++) free(ExtParams.FFT_V[i]);
//    for(int i = 0; i < CurrentChannels; i++) free(ExtParams.FFT_I[i]);    
  }  
  return Result;
}