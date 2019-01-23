/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <string.h>
#include "AnalyzerInterface.h"
#include "Tools.h"
//------------- ---------------------------------------------------------------------------------------------------------
vector<float> AnalyzerInterface::FrequencyRanges;
vector<float> AnalyzerInterface::VoltageRanges;
vector<float> AnalyzerInterface::THDRanges;
vector<float> AnalyzerInterface::HarmonicsAllowed;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerInterface::AnalyzerInterface(char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerMode mode) : Database(id) {
  if(FrequencyRanges.empty()) {
    FrequencyRanges.push_back(-INFINITY);
    FrequencyRanges.push_back(-5);
    FrequencyRanges.push_back(-4);
    FrequencyRanges.push_back(-3);
    FrequencyRanges.push_back(-2);
    FrequencyRanges.push_back(-1);
    FrequencyRanges.push_back(-0.9);
    FrequencyRanges.push_back(-0.8);
    FrequencyRanges.push_back(-0.7);
    FrequencyRanges.push_back(-0.6);
    FrequencyRanges.push_back(-0.5);
    FrequencyRanges.push_back(-0.45);
    FrequencyRanges.push_back(-0.40);
    FrequencyRanges.push_back(-0.35);
    FrequencyRanges.push_back(-0.30);
    FrequencyRanges.push_back(-0.25);
    FrequencyRanges.push_back(-0.20);
    FrequencyRanges.push_back(-0.15);
    FrequencyRanges.push_back(-0.10);
    FrequencyRanges.push_back(-0.05);
    FrequencyRanges.push_back(0.00);
    FrequencyRanges.push_back(0.05);
    FrequencyRanges.push_back(0.10);
    FrequencyRanges.push_back(0.15);
    FrequencyRanges.push_back(0.20);
    FrequencyRanges.push_back(0.25);
    FrequencyRanges.push_back(0.30);
    FrequencyRanges.push_back(0.35);
    FrequencyRanges.push_back(0.40);
    FrequencyRanges.push_back(0.45);
    FrequencyRanges.push_back(0.5);
    FrequencyRanges.push_back(0.6);
    FrequencyRanges.push_back(0.7);
    FrequencyRanges.push_back(0.8);
    FrequencyRanges.push_back(0.9);
    FrequencyRanges.push_back(1);
    FrequencyRanges.push_back(2);
    FrequencyRanges.push_back(3);
    FrequencyRanges.push_back(4);
    FrequencyRanges.push_back(5);
    FrequencyRanges.push_back(INFINITY);
  }
  if(VoltageRanges.empty()) {
    VoltageRanges.push_back(-INFINITY);
    VoltageRanges.push_back(-20);
    VoltageRanges.push_back(-15);
    VoltageRanges.push_back(-10);
    VoltageRanges.push_back(-9);
    VoltageRanges.push_back(-8);
    VoltageRanges.push_back(-7);
    VoltageRanges.push_back(-6);
    VoltageRanges.push_back(-5);
    VoltageRanges.push_back(-4);
    VoltageRanges.push_back(-3);
    VoltageRanges.push_back(-2);
    VoltageRanges.push_back(-1);
    VoltageRanges.push_back(0);
    VoltageRanges.push_back(1);
    VoltageRanges.push_back(2);
    VoltageRanges.push_back(3);
    VoltageRanges.push_back(4);
    VoltageRanges.push_back(5);
    VoltageRanges.push_back(6);
    VoltageRanges.push_back(7);
    VoltageRanges.push_back(8);
    VoltageRanges.push_back(9);
    VoltageRanges.push_back(10);
    VoltageRanges.push_back(15);
    VoltageRanges.push_back(20);
    VoltageRanges.push_back(INFINITY);
  }
  if(THDRanges.empty()) {
    THDRanges.push_back(0);
    THDRanges.push_back(0.5);
    THDRanges.push_back(1);
    THDRanges.push_back(1.5);
    THDRanges.push_back(2);
    THDRanges.push_back(2.5);
    THDRanges.push_back(3);
    THDRanges.push_back(3.5);
    THDRanges.push_back(4);
    THDRanges.push_back(4.5);
    THDRanges.push_back(5);
    THDRanges.push_back(5.5);
    THDRanges.push_back(6);
    THDRanges.push_back(6.5);
    THDRanges.push_back(7);
    THDRanges.push_back(7.5);
    THDRanges.push_back(8);
    THDRanges.push_back(8.5);
    THDRanges.push_back(9);
    THDRanges.push_back(9.5);
    THDRanges.push_back(10);
    THDRanges.push_back(11);
    THDRanges.push_back(12);
    THDRanges.push_back(13);
    THDRanges.push_back(14);
    THDRanges.push_back(15);
    THDRanges.push_back(INFINITY);
  }
  if(HarmonicsAllowed.empty()) {
    HarmonicsAllowed.push_back(2);
    HarmonicsAllowed.push_back(5);
    HarmonicsAllowed.push_back(1);
    HarmonicsAllowed.push_back(6);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(1.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(3.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(3);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(2);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(1.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(1.5);
    HarmonicsAllowed.push_back(0.5);
    HarmonicsAllowed.push_back(1.5);
  }
  pNominalVoltage = nominal_voltage;
  pNominalFrequency = nominal_frequency;
  pName = name;
  pMaxCurrent = max_current;
  pMinCurrent = min_current;
  pMode = mode;
  pLastSave = 0;
  Tools::LogMsg("DEBUG", "Creating analyzer database tables\n");
  ExecResource("SQL/Database_Analyzer.sql");
  PrepareSentence("INSERT INTO analyzers(username, serial, contract, zipcode, frequency, voltage, name, geo) VALUES('admin', ?, '','',50,230,'','(0,0)') ON CONFLICT (serial) DO NOTHING;");
  BindSingleString(id);
  ExecSentence();
}
AnalyzerInterface::~AnalyzerInterface() {}
float AnalyzerInterface::GetMaxCurrent() {
  return pMaxCurrent;
}
const char *AnalyzerInterface::GetID() {
  return GetSchema();
}
float AnalyzerInterface::GetNominalVoltage() {
  return pNominalVoltage;
}
float AnalyzerInterface::GetNominalFrequency() {
  return pNominalFrequency;
}
void AnalyzerInterface::InitAggreg(Params_t &params, AggregType type) {
//  if(params.Type != UNINITIALIZED) return;
  params.Type = type;
  params.Timestamp = 0;
  params.RMS_I_Inputs.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 0);
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    params.Frequency = NAN;
    params.Flag = false;
    params.RMS_V_Inputs = 0;
    params.Freq_Inputs = 0;
    params.RMS_V.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.RMS_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
  }
  if(params.Type == AGGREG_1H) {
    params.StatsFrequency.resize(FrequencyRanges.size(), 0);
    params.StatsVoltage.resize(VoltageRanges.size(), 0);
    params.StatsTHD.resize(THDRanges.size(), 0);
    params.StatsHarmonics.resize(2, 0);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1H)) {
    params.Active_Power.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Power.resize((pMode == PHASE1) ? 1 : 3, NAN);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M)) {
    params.THD_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
    params.Phi.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Power_Factor.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.CurrentHarmonics.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M)) {
    params.THD_V.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.VoltageHarmonics.resize((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
  }
  if(params.Type == AGGREG_10C) {
    params.Active_Energy.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Phase_V.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Energy.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Apparent_Power.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params.Voltage.resize((pMode == PHASE1) ? 1 : 3);
    params.FFT_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params.FFT_V.resize((pMode == PHASE1) ? 1 : 3);
  }
}
void AnalyzerInterface::ResetAggreg(Params_t &params) {
  params.Frequency = NAN;
  params.Flag = false;
  params.Timestamp = 0;
  params.RMS_V_Inputs = 0;
  params.Freq_Inputs = 0;
  for(uint i = 0; i < params.StatsFrequency.size(); i++) params.StatsFrequency[i] = 0;
  for(uint i = 0; i < params.StatsVoltage.size(); i++) params.StatsVoltage[i] = 0;
  for(uint i = 0; i < params.StatsTHD.size(); i++) params.StatsTHD[i] = 0;
  for(uint i = 0; i < params.StatsHarmonics.size(); i++) params.StatsHarmonics[i] = 0;
  for(uint i = 0; i < params.RMS_I_Inputs.size(); i++) params.RMS_I_Inputs[i] = 0;
  for(uint i = 0; i < params.RMS_V.size(); i++) params.RMS_V[i] = NAN;
  for(uint i = 0; i < params.RMS_I.size(); i++) params.RMS_I[i] = NAN;
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = NAN;
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = NAN;
  for(uint i = 0; i < params.Active_Energy.size(); i++) params.Active_Energy[i] = NAN;
  for(uint i = 0; i < params.Reactive_Energy.size(); i++) params.Reactive_Energy[i] = NAN;
  for(uint i = 0; i < params.Apparent_Power.size(); i++) params.Apparent_Power[i] = NAN;
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = NAN;
  for(uint i = 0; i < params.THD_V.size(); i++) params.THD_V[i] = NAN;
  for(uint i = 0; i < params.THD_I.size(); i++) params.THD_I[i] = NAN;
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = NAN;
  for(uint i = 0; i < params.VoltageHarmonics.size(); i++) {
    for(uint n = 0; n < params.VoltageHarmonics[i].size(); n++) params.VoltageHarmonics[i][n] = NAN;
  }
  for(uint i = 0; i < params.CurrentHarmonics.size(); i++) {
    for(uint n = 0; n < params.CurrentHarmonics[i].size(); n++) params.CurrentHarmonics[i][n] = NAN;
  }
  for(uint i = 0; i < params.Voltage.size(); i++) params.Voltage[i].resize(0);
  for(uint i = 0; i < params.Current.size(); i++) params.Current[i].resize(0);
  for(uint i = 0; i < params.FFT_V.size(); i++) params.FFT_V[i].resize(0);
  for(uint i = 0; i < params.FFT_I.size(); i++) params.FFT_I[i].resize(0);
}
void AnalyzerInterface::SaveAggreg(Params_t &params) {
  if(params.Type == AGGREG_3S)
    PrepareSentence("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(?, ?, ?, ?, ?, ?, ?)");
  else if(params.Type == AGGREG_1M)  
    PrepareSentence("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(?, ?, ?, ?, ?) RETURNING sampletime) "
                    "INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) SELECT sampletime, ?, ?, ?, ?, ?, ?, ?, ? FROM Sample");
  else if(params.Type == AGGREG_10M)
    PrepareSentence("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(?, ?, ?, ?, ?)");
  else if(params.Type == AGGREG_15M)
    PrepareSentence("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(?, ?, ?)");
  else if(params.Type == AGGREG_1H)
    PrepareSentence("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt64(params.Timestamp);                                                 //sampletime
  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    BindArrayFloat(params.RMS_V);                                                       //rms_v 
    BindArrayFloat(params.RMS_I);                                                       //rms_i
    BindSingleFloat(params.Frequency);                                                  //frequency
    BindSingleBool(params.Flag);                                                        //flaged
  }
  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_1H)) {
    BindArrayFloat(params.Active_Power);                                                //active_power
    BindArrayFloat(params.Reactive_Power);                                              //reactive_power
  }
  if(params.Type == AGGREG_1M) {
    BindArrayFloat(params.Power_Factor);                                              //power_factor
    BindArrayFloat(params.THD_V);                                                     //thd_v
    BindArrayFloat(params.THD_I);                                                     //thd_i
    BindArrayFloat(params.Phi);                                                       //phi
    BindArrayArrayFloat(params.VoltageHarmonics);                                     //harmonics_v
    BindArrayArrayFloat(params.CurrentHarmonics);                                     //harmonics_i
  }
  if(params.Type == AGGREG_1H) {
    BindArrayUInt16(params.StatsVoltage);        //estadisticas de tension
    BindArrayUInt16(params.StatsFrequency);      //estadisticas de frequencia
    BindArrayUInt16(params.StatsTHD);            //estadisticas de distorsion harmonica
    BindArrayUInt16(params.StatsHarmonics);      //estadisticas de harmonicos en rango
  }
  ExecSentence();
  if(params.Type == AGGREG_1M) {
    PrepareSentence("DELETE FROM aggreg_ext_1m WHERE sampletime < ?");
    BindSingleUInt64(params.Timestamp - 86400000);   //Delete 1 day old
    ExecSentence();
  }
  pLastSave = Tools::GetTime();
}
json_object *AnalyzerInterface::GetJSONDescriptor() {
  json_object *Device = json_object_new_object();
  json_object_object_add(Device, "ID", json_object_new_string(GetID()));
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
json_object *AnalyzerInterface::GetJSONParams(json_object *series) {
  if((series == NULL) || (json_object_get_type(series) != json_type_array)) return NULL;
  Params_t Params = { UNINITIALIZED };
  ParamsType Copy = (ParamsType)0;
  for(uint i = 0; i < json_object_array_length(series); i++) {
    if(json_object_get_type(json_object_array_get_idx(series, i)) != json_type_string) return NULL;
    char *Serie = (char*)json_object_get_string(json_object_array_get_idx(series, i));
    if(strcmp(Serie, "FREQUENCY") == 0)            Copy = (ParamsType)(Copy | FREQUENCY);
    else if(strcmp(Serie, "FLAG") == 0)            Copy = (ParamsType)(Copy | FLAG);
    else if(strcmp(Serie, "RMS_V") == 0)           Copy = (ParamsType)(Copy | RMS_V);
    else if(strcmp(Serie, "RMS_I") == 0)           Copy = (ParamsType)(Copy | RMS_I);
    else if(strcmp(Serie, "ACTIVE_POWER") == 0)    Copy = (ParamsType)(Copy | ACTIVE_POWER);
    else if(strcmp(Serie, "REACTIVE_POWER") == 0)  Copy = (ParamsType)(Copy | REACTIVE_POWER);
    else if(strcmp(Serie, "ACTIVE_ENERGY") == 0)   Copy = (ParamsType)(Copy | ACTIVE_ENERGY);
    else if(strcmp(Serie, "REACTIVE_ENERGY") == 0) Copy = (ParamsType)(Copy | REACTIVE_ENERGY);
    else if(strcmp(Serie, "APPARENT_POWER") == 0)  Copy = (ParamsType)(Copy | APPARENT_POWER);
    else if(strcmp(Serie, "POWER_FACTOR") == 0)    Copy = (ParamsType)(Copy | POWER_FACTOR);
    else if(strcmp(Serie, "THD_V") == 0)           Copy = (ParamsType)(Copy | THD_V);
    else if(strcmp(Serie, "THD_I") == 0)           Copy = (ParamsType)(Copy | THD_I);
    else if(strcmp(Serie, "PHI") == 0)             Copy = (ParamsType)(Copy | PHI);
    else if(strcmp(Serie, "PHASE_V") == 0)         Copy = (ParamsType)(Copy | PHASE_V);
    else if(strcmp(Serie, "HARMONICS_V") == 0)     Copy = (ParamsType)(Copy | HARMONICS_V);
    else if(strcmp(Serie, "HARMONICS_I") == 0)     Copy = (ParamsType)(Copy | HARMONICS_I);
    else if(strcmp(Serie, "FFT_V") == 0)           Copy = (ParamsType)(Copy | FFT_V);
    else if(strcmp(Serie, "FFT_I") == 0)           Copy = (ParamsType)(Copy | FFT_I);
    else if(strcmp(Serie, "VOLTAGE") == 0)         Copy = (ParamsType)(Copy | VOLTAGE);
    else if(strcmp(Serie, "CURRENT") == 0)         Copy = (ParamsType)(Copy | CURRENT);
  }
  GetParams(Params, Copy);
  json_object *Result = json_object_new_object();
  if(Copy & FREQUENCY) json_object_object_add(Result, "freq", (isnan(Params.Frequency)) ? NULL : json_object_new_double(Params.Frequency));
  if(Copy & FLAG) json_object_object_add(Result, "flag", json_object_new_boolean(Params.Flag));
  if(Copy & RMS_V) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.RMS_V.size(); i++) json_object_array_add(Obj, (isnan(Params.RMS_V[i])) ? NULL : json_object_new_double(Params.RMS_V[i]));
    json_object_object_add(Result, "rms_v", Obj);
  }
  if(Copy & RMS_I) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.RMS_I.size(); i++) json_object_array_add(Obj, (isnan(Params.RMS_I[i])) ? NULL : json_object_new_double(Params.RMS_I[i]));
    json_object_object_add(Result, "rms_i", Obj);
  }
  if(Copy & ACTIVE_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Active_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Active_Power[i])) ? NULL : json_object_new_double(Params.Active_Power[i]));
    json_object_object_add(Result, "Active_Power", Obj);
  }
  if(Copy & REACTIVE_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Reactive_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Reactive_Power[i])) ? NULL : json_object_new_double(Params.Reactive_Power[i]));
    json_object_object_add(Result, "reactive_power", Obj);
  }
  if(Copy & ACTIVE_ENERGY) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Active_Energy.size(); i++) json_object_array_add(Obj, (isnan(Params.Active_Energy[i])) ? NULL : json_object_new_double(Params.Active_Energy[i]));
    json_object_object_add(Result, "active_energy", Obj);
  }
  if(Copy & REACTIVE_ENERGY) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Reactive_Energy.size(); i++) json_object_array_add(Obj, (isnan(Params.Reactive_Energy[i])) ? NULL : json_object_new_double(Params.Reactive_Energy[i]));
    json_object_object_add(Result, "reactive_energy", Obj);
  }
  if(Copy & APPARENT_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Apparent_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Apparent_Power[i])) ? NULL : json_object_new_double(Params.Apparent_Power[i]));
    json_object_object_add(Result, "apparent_power", Obj);
  }
  if(Copy & POWER_FACTOR) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Power_Factor.size(); i++) json_object_array_add(Obj, (isnan(Params.Power_Factor[i])) ? NULL : json_object_new_double(Params.Power_Factor[i]));
    json_object_object_add(Result, "power_factor", Obj);
  }
  if(Copy & THD_V) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.THD_V.size(); i++) json_object_array_add(Obj, (isnan(Params.THD_V[i])) ? NULL : json_object_new_double(Params.THD_V[i]));
    json_object_object_add(Result, "thd_v", Obj);
  }
  if(Copy & THD_I) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.THD_I.size(); i++) json_object_array_add(Obj, (isnan(Params.THD_I[i])) ? NULL : json_object_new_double(Params.THD_I[i]));
    json_object_object_add(Result, "thd_i", Obj);
  }
  if(Copy & PHI) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Phi.size(); i++) json_object_array_add(Obj, (isnan(Params.Phi[i])) ? NULL : json_object_new_double(Params.Phi[i]));
    json_object_object_add(Result, "phi", Obj);
  }
  if(Copy & PHASE_V) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Phase_V.size(); i++) json_object_array_add(Obj, (isnan(Params.Phi[i])) ? NULL : json_object_new_double(Params.Phi[i]));
    json_object_object_add(Result, "phi", Obj);
  }

  if(Copy & HARMONICS_V) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.VoltageHarmonics.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.VoltageHarmonics[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.VoltageHarmonics[i][j])) ? NULL : json_object_new_double(Params.VoltageHarmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "harmonics_v", Obj);
  }
  if(Copy & HARMONICS_I) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.CurrentHarmonics.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.CurrentHarmonics[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.CurrentHarmonics[i][j])) ? NULL : json_object_new_double(Params.CurrentHarmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "harmonics_i", Obj);
  }
  if(Copy & VOLTAGE) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Voltage[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Voltage[i][j])) ? NULL : json_object_new_double(Params.Voltage[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "wave_v", Obj);
  }
  if(Copy & CURRENT) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Current[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Current[i][j])) ? NULL : json_object_new_double(Params.Current[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "wave_i", Obj);
  }
  if(Copy & FFT_V) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.FFT_V.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.FFT_V[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.FFT_V[i][j])) ? NULL : json_object_new_double(Params.FFT_V[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "fft_v", Obj);
  }
  if(Copy & FFT_I) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.FFT_I.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.FFT_I[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.FFT_I[i][j])) ? NULL : json_object_new_double(Params.FFT_I[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "fft_i", Obj);
  }
  json_object_object_add(Result, "Sampletime", json_object_new_int64(Params.Timestamp));
  return Result;
}
AnalyzerInterface::AnalyzerMode AnalyzerInterface::GetMode() {
  return pMode;
}
void AnalyzerInterface::AccumulateAggreg(Params_t &dst, Params_t &src) {
  dst.Timestamp = src.Timestamp;
  for(uint i = 0; i < dst.RMS_V.size(); i++) dst.RMS_V[i] = AccumulateRMS(dst.RMS_V[i], src.RMS_V[i]);
  dst.RMS_V_Inputs += src.RMS_V_Inputs;
  dst.Frequency = AccumulateSUM(dst.Frequency, src.Frequency);
  dst.Freq_Inputs += src.Freq_Inputs;
  for(uint i = 0; i < dst.RMS_I.size(); i++) dst.RMS_I[i] = AccumulateRMS(dst.RMS_I[i], src.RMS_I[i]);
  for(uint i = 0; i < dst.RMS_I_Inputs.size(); i++) dst.RMS_I_Inputs[i] += src.RMS_I_Inputs[i];
  for(uint i = 0; i < dst.Active_Power.size(); i++) dst.Active_Power[i] = AccumulateSUM(dst.Active_Power[i], src.Active_Power[i]);
  for(uint i = 0; i < dst.Active_Energy.size(); i++) dst.Active_Energy[i] = AccumulateSUM(dst.Active_Energy[i], src.Active_Energy[i]);
  for(uint i = 0; i < dst.Reactive_Power.size(); i++) dst.Reactive_Power[i] = AccumulateSUM(dst.Reactive_Power[i], src.Reactive_Power[i]);
  for(uint i = 0; i < dst.Reactive_Energy.size(); i++) dst.Reactive_Energy[i] = AccumulateSUM(dst.Reactive_Energy[i], src.Reactive_Energy[i]);
  for(uint i = 0; i < dst.Power_Factor.size(); i++) dst.Power_Factor[i] = AccumulateSUM(dst.Power_Factor[i], src.Power_Factor[i]);
  for(uint i = 0; i < dst.THD_V.size(); i++) dst.THD_V[i] = AccumulateRMS(dst.THD_V[i], src.THD_V[i]);
  for(uint i = 0; i < dst.THD_I.size(); i++) dst.THD_I[i] = AccumulateRMS(dst.THD_I[i], src.THD_I[i]);
  for(uint i = 0; i < dst.Phi.size(); i++) dst.Phi[i] = AccumulateSUM(dst.Phi[i], src.Phi[i]);
  for(uint i = 0; i < dst.VoltageHarmonics.size(); i++) {
    for(uint j = 0; j < dst.VoltageHarmonics[i].size(); j++) dst.VoltageHarmonics[i][j] = AccumulateRMS(dst.VoltageHarmonics[i][j], src.VoltageHarmonics[i][j]);
  }
  for(uint i = 0; i < dst.CurrentHarmonics.size(); i++) {
    for(uint j = 0; j < dst.CurrentHarmonics[i].size(); j++) dst.CurrentHarmonics[i][j] = AccumulateRMS(dst.CurrentHarmonics[i][j], src.CurrentHarmonics[i][j]);
  }
  dst.Flag |= src.Flag;
}
void AnalyzerInterface::NormalizeAggreg(Params_t &params) {
  for(uint i = 0; i < params.RMS_V.size(); i++) params.RMS_V[i] = sqrt(params.RMS_V[i] / params.RMS_V_Inputs);
  for(uint i = 0; i < params.RMS_I.size(); i++) params.RMS_I[i] = sqrt(params.RMS_I[i] / params.RMS_I_Inputs[i]);
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = params.Active_Power[i] / params.RMS_I_Inputs[i];
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = params.Reactive_Power[i] / params.RMS_I_Inputs[i];
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = params.Power_Factor[i] / params.RMS_I_Inputs[i];
  for(uint i = 0; i < params.THD_I.size(); i++) params.THD_I[i] = sqrt(params.THD_I[i] / params.RMS_I_Inputs[i]);
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = params.Phi[i] / params.RMS_I_Inputs[i];
  for(uint i = 0; i < params.THD_V.size(); i++) params.THD_V[i] = sqrt(params.THD_V[i] / params.RMS_V_Inputs);
  for(uint i = 0; i < params.CurrentHarmonics.size(); i++) {
    for(uint j = 0; j < params.CurrentHarmonics[i].size(); j++) params.CurrentHarmonics[i][j] = sqrt(params.CurrentHarmonics[i][j] / params.RMS_I_Inputs[i]);
  }
  for(uint i = 0; i < params.VoltageHarmonics.size(); i++) {
    for(uint j = 0; j < params.VoltageHarmonics[i].size(); j++) params.VoltageHarmonics[i][j] = sqrt(params.VoltageHarmonics[i][j] / params.RMS_V_Inputs);
  }
  params.Frequency = params.Frequency / params.Freq_Inputs;
  if(params.Freq_Inputs > 1) params.Freq_Inputs = 1;
  if(params.RMS_V_Inputs > 1) params.RMS_V_Inputs = 1;
  for(uint i = 0; i < params.RMS_I_Inputs.size(); i++) {
    if(params.RMS_I_Inputs[i] > 1) params.RMS_I_Inputs[i] = 1;
  }
}
float AnalyzerInterface::AccumulateSUM(float val1, float val2) {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return val2;
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + val2;
}
float AnalyzerInterface::AccumulateRMS(float val1, float val2) {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return pow(val2, 2.0);
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + pow(val2, 2.0);
}
