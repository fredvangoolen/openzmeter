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
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_3s (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[], rms_i real[], active_power real[], active_energy real[], reactive_power real[], reactive_energy real[], freq real, flag boolean) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1m (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[], rms_i real[], active_power real[], active_energy real[], reactive_power real[], reactive_energy real[], freq real, flag boolean) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_10m (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[], rms_i real[], active_power real[], active_energy real[], reactive_power real[], reactive_energy real[], freq real, flag boolean) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_1h (sampletime bigint NOT NULL PRIMARY KEY, rms_v real[], rms_i real[], active_power real[], active_energy real[], reactive_power real[], reactive_energy real[], freq real, flag boolean) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_ext_1m (sampletime_ext bigint NOT NULL PRIMARY KEY, power_factor real[], thd_v real[], thd_i real[], phi real[], harmonics_v real[], harmonics_i real[]) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS aggreg_stat_1h (sampletime_ext bigint NOT NULL PRIMARY KEY, power_factor real[], thd_v real[], thd_i real[], phi real[], harmonics_v real[], harmonics_i real[]) WITH (OIDS=FALSE)");
  ExecSentence("CREATE TABLE IF NOT EXISTS events (starttime bigint NOT NULL PRIMARY KEY, duration  bigint, samplerate real, type character varying(16), voltage real, percent real, samples real[]) WITH (OIDS=FALSE)");
}
char *AnalyzerInterface::GetID() {
  return pID;
}
float AnalyzerInterface::GetNominalVoltage() {
  return pNominalVoltage;
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
void AnalyzerInterface::SaveAggreg(Params_t *params) {
  //  ExecSentence("PREPARE SAVE_3S       AS INSERT INTO aggreg_3s (sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)");    
//  ExecSentence("PREPARE SAVE_1M       AS INSERT INTO aggreg_1m (sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)");  
//  ExecSentence("PREPARE SAVE_1M_EXT   AS INSERT INTO aggreg_ext_1m (sampletime_ext, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) VALUES($1, $2, $3, $4, $5, $6, $7)");  
//  ExecSentence("PREPARE SAVE_10M      AS INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)");  
//  ExecSentence("PREPARE SAVE_1H       AS INSERT INTO aggreg_1h (sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9)");
//  ExecSentence("PREPARE DELETE_1M_EXT AS DELETE FROM aggreg_ext_1m WHERE sampletime_ext < $1");

  if(params->Type == AGGREG_3S)
    PrepareSentence("INSERT INTO aggreg_3s (sampletime, rms_v, rms_i, active_power, active_energy, freq, flag) VALUES(?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1M)  
    PrepareSentence("EXECUTE SAVE_1M(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_10M)  
    PrepareSentence("EXECUTE SAVE_10M(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1H)  
    PrepareSentence("EXECUTE SAVE_1H(?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt64(params->Timestamp);                                                                       //sampletime
  BindArrayFloat(params->RMS_V, (pMode == PHASE1) ? 1 : 3);                                                 //rms_v 
  BindArrayFloat(params->RMS_I, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);                        //rms_i
  BindArrayFloat(params->Active_Power, (pMode == PHASE1) ? 1 : 3);                                          //active_power
  BindArrayFloat(params->Active_Energy, (pMode == PHASE1) ? 1 : 3);                                         //active_energy
//  BindArrayFloat(params->Fundamental_Reactive_Power, (pMode == PHASE1) ? 1 : 3);                            //fundamental_reactive_power
//  BindArrayFloat(params->Fundamental_Reactive_Energy, (pMode == PHASE1) ? 1 : 3);                           //fundamental_reactive_energy
  BindSingleFloat(params->Frequency);                                                                        //frequency
  BindSingleBool(params->Flag);                                                                              //flaged
  ExecSentence();
  if(params->Type == AGGREG_1M) {
    PrepareSentence("EXECUTE SAVE_1M_EXT(?, ?, ?, ?, ?, ?, ?)");
    BindSingleUInt64(params->Timestamp);                                                                     //sampletime
    BindArrayFloat(params->Power_Factor, (pMode == PHASE1) ? 1 : 3);                                        //power_factor
    BindArrayFloat(params->THD_V, (pMode == PHASE1) ? 1 : 3);                                               //thd_v
    BindArrayFloat(params->THD_I, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);                      //thd_i
    BindArrayFloat(params->Phi, (pMode == PHASE1) ? 1 : 3);                                                 //phi
    BindArrayArrayFloat((float*)params->VoltageHarmonics, (pMode == PHASE1) ? 1 : 3, 50);                           //harmonics_v
    BindArrayArrayFloat((float*)params->CurrentHarmonics, (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 50);  //harmonics_i
    ExecSentence();
    PrepareSentence("EXECUTE DELETE_1M_EXT(?)");
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
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Fundamental_Reactive_Power[i] += src->Fundamental_Reactive_Power[i];
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Fundamental_Reactive_Energy[i] += src->Fundamental_Reactive_Energy[i];
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
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Reactive_Power[i] = params->Fundamental_Reactive_Power[i] / params->Samples;
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
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Reactive_Power[i] = 0;
  for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Reactive_Energy[i] = 0;
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
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Active_Power[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Apparent_Power[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Apparent_Power[i] = 0;
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) params->Fundamental_Power_Factor[i] = 0;
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