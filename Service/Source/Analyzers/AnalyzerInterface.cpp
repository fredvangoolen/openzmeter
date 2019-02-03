/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <string.h>
#include "AnalyzerInterface.h"
#include "Tools.h"
//------------- ---------------------------------------------------------------------------------------------------------
vector<float> AnalyzerInterface::FrequencyRanges = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
vector<float> AnalyzerInterface::VoltageRanges;
vector<float> AnalyzerInterface::THDRanges;
vector<float> AnalyzerInterface::HarmonicsAllowed;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerInterface::AnalyzerInterface(string id, AnalyzerMode mode) : Database(id.c_str()) {
  if(FrequencyRanges.empty()) {

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
  Tools::LogMsg("DEBUG", "Creating analyzer database tables\n");
  ExecResource("SQL/Database_Analyzer.sql");
  pMode = mode;
  pLastSave = 0;
  PrepareSentence("SELECT frequency, voltage, name, max_current, min_current FROM analyzers WHERE serial = ?");
  BindSingleString(id.c_str());
  if((ExecSentence() == false) || (ResultRows() != 1)) {
    pNominalVoltage = 230.0;
    pNominalFrequency = 50.0;
    pName = strdup("Unnamed input analyzer");
    pMaxCurrent = 50.0;
    pMinCurrent =  0.1;
    PrepareSentence("INSERT INTO analyzers(username, serial, contract, zipcode, frequency, voltage, name, geo, max_current, min_current) "
                    "VALUES('admin', ?, '', '', 50.0, 230.0, 'Unnamed input analyzer', POINT(0, 0), 50.0, 0.1)");
    BindSingleString(id.c_str());
    ExecSentence();    
  } else {
    pNominalVoltage = atof(ResultValue(0, "voltage"));
    pNominalFrequency = atof(ResultValue(0, "frequency"));
    pName = ResultValue(0, "name");
    pMaxCurrent = atof(ResultValue(0, "max_current"));
    pMinCurrent =  atof(ResultValue(0, "min_current"));
  }
}
AnalyzerInterface::~AnalyzerInterface() {
//  if(pName) free(pName);
}
float AnalyzerInterface::GetMaxCurrent() {
  return pMaxCurrent;
}
string AnalyzerInterface::GetID() {
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
  params.Current_Inputs.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 0);
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    params.Frequency = NAN;
    params.Flag = false;
    params.Voltage_Inputs = 0;
    params.Frequency_Inputs = 0;
    params.Voltage.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
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
    params.Current_THD.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
    params.Phi.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Power_Factor.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current_Harmonics.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M)) {
    params.Voltage_THD.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Harmonics.resize((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
  }
  if(params.Type == AGGREG_10C) {
    params.Active_Energy.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Phase.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Energy.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Apparent_Power.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current_Samples.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params.Voltage_Samples.resize((pMode == PHASE1) ? 1 : 3);
    params.Current_FFT.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
    params.Voltage_FFT.resize((pMode == PHASE1) ? 1 : 3);
  }
}
void AnalyzerInterface::ResetAggreg(Params_t &params) {
  params.Frequency = NAN;
  params.Flag = false;
  params.Timestamp = 0;
  params.Voltage_Inputs = 0;
  params.Frequency_Inputs = 0;
  for(uint i = 0; i < params.StatsFrequency.size(); i++) params.StatsFrequency[i] = 0;
  for(uint i = 0; i < params.StatsVoltage.size(); i++) params.StatsVoltage[i] = 0;
  for(uint i = 0; i < params.StatsTHD.size(); i++) params.StatsTHD[i] = 0;
  for(uint i = 0; i < params.StatsHarmonics.size(); i++) params.StatsHarmonics[i] = 0;
  for(uint i = 0; i < params.Current_Inputs.size(); i++) params.Current_Inputs[i] = 0;
  for(uint i = 0; i < params.Voltage.size(); i++) params.Voltage[i] = NAN;
  for(uint i = 0; i < params.Current.size(); i++) params.Current[i] = NAN;
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = NAN;
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = NAN;
  for(uint i = 0; i < params.Active_Energy.size(); i++) params.Active_Energy[i] = NAN;
  for(uint i = 0; i < params.Reactive_Energy.size(); i++) params.Reactive_Energy[i] = NAN;
  for(uint i = 0; i < params.Apparent_Power.size(); i++) params.Apparent_Power[i] = NAN;
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = NAN;
  for(uint i = 0; i < params.Voltage_THD.size(); i++) params.Voltage_THD[i] = NAN;
  for(uint i = 0; i < params.Current_THD.size(); i++) params.Current_THD[i] = NAN;
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = NAN;
  for(uint i = 0; i < params.Voltage_Harmonics.size(); i++) {
    for(uint n = 0; n < params.Voltage_Harmonics[i].size(); n++) params.Voltage_Harmonics[i][n] = NAN;
  }
  for(uint i = 0; i < params.Current_Harmonics.size(); i++) {
    for(uint n = 0; n < params.Current_Harmonics[i].size(); n++) params.Current_Harmonics[i][n] = NAN;
  }
  for(uint i = 0; i < params.Voltage_Samples.size(); i++) params.Voltage_Samples[i].resize(0);
  for(uint i = 0; i < params.Current_Samples.size(); i++) params.Current_Samples[i].resize(0);
  for(uint i = 0; i < params.Voltage_FFT.size(); i++) params.Voltage_FFT[i].resize(0);
  for(uint i = 0; i < params.Current_FFT.size(); i++) params.Current_FFT[i].resize(0);
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
    BindArrayFloat(params.Voltage);                                                       //rms_v 
    BindArrayFloat(params.Current);                                                       //rms_i
    BindSingleFloat(params.Frequency);                                                  //frequency
    BindSingleBool(params.Flag);                                                        //flaged
  }
  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_1H)) {
    BindArrayFloat(params.Active_Power);                                                //active_power
    BindArrayFloat(params.Reactive_Power);                                              //reactive_power
  }
  if(params.Type == AGGREG_1M) {
    BindArrayFloat(params.Power_Factor);                                              //power_factor
    BindArrayFloat(params.Voltage_THD);                                                     //thd_v
    BindArrayFloat(params.Current_THD);                                                     //thd_i
    BindArrayFloat(params.Phi);                                                       //phi
    BindArrayArrayFloat(params.Voltage_Harmonics);                                     //harmonics_v
    BindArrayArrayFloat(params.Current_Harmonics);                                     //harmonics_i
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
json AnalyzerInterface::GetJSONDescriptor() {
  json Response;
  Response["ID"] = GetID();
  Response["NominalVoltage"] = pNominalVoltage;
  Response["NominalFreq"] = pNominalFrequency;
  Response["Name"] = pName;
  Response["MaxCurrent"] = pMaxCurrent;
  Response["MinCurrent"] = pMinCurrent;
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleFreq"] = pSamplerate;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  return Response;
}
json_object *AnalyzerInterface::GetJSONParams(json_object *series) {
  if((series == NULL) || (json_object_get_type(series) != json_type_array)) return NULL;
  Params_t Params = { UNINITIALIZED };
  ParamsType Copy = (ParamsType)0;
  for(uint i = 0; i < json_object_array_length(series); i++) {
    if(json_object_get_type(json_object_array_get_idx(series, i)) != json_type_string) return NULL;
    char *Serie = (char*)json_object_get_string(json_object_array_get_idx(series, i));
    if(strcmp(Serie, "FREQUENCY") == 0)              Copy = (ParamsType)(Copy | FREQUENCY);
    else if(strcmp(Serie, "FLAG") == 0)              Copy = (ParamsType)(Copy | FLAG);
    else if(strcmp(Serie, "VOLTAGE") == 0)           Copy = (ParamsType)(Copy | VOLTAGE);
    else if(strcmp(Serie, "CURRENT") == 0)           Copy = (ParamsType)(Copy | CURRENT);
    else if(strcmp(Serie, "ACTIVE_POWER") == 0)      Copy = (ParamsType)(Copy | ACTIVE_POWER);
    else if(strcmp(Serie, "REACTIVE_POWER") == 0)    Copy = (ParamsType)(Copy | REACTIVE_POWER);
    else if(strcmp(Serie, "ACTIVE_ENERGY") == 0)     Copy = (ParamsType)(Copy | ACTIVE_ENERGY);
    else if(strcmp(Serie, "REACTIVE_ENERGY") == 0)   Copy = (ParamsType)(Copy | REACTIVE_ENERGY);
    else if(strcmp(Serie, "APPARENT_POWER") == 0)    Copy = (ParamsType)(Copy | APPARENT_POWER);
    else if(strcmp(Serie, "POWER_FACTOR") == 0)      Copy = (ParamsType)(Copy | POWER_FACTOR);
    else if(strcmp(Serie, "VOLTAGE_THD") == 0)       Copy = (ParamsType)(Copy | VOLTAGE_THD);
    else if(strcmp(Serie, "CURRENT_THD") == 0)       Copy = (ParamsType)(Copy | CURRENT_THD);
    else if(strcmp(Serie, "PHI") == 0)               Copy = (ParamsType)(Copy | PHI);
    else if(strcmp(Serie, "VOLTAGE_PHASE") == 0)     Copy = (ParamsType)(Copy | VOLTAGE_PHASE);
    else if(strcmp(Serie, "VOLTAGE_HARMONICS") == 0) Copy = (ParamsType)(Copy | VOLTAGE_HARMONICS);
    else if(strcmp(Serie, "CURRENT_HARMONICS") == 0) Copy = (ParamsType)(Copy | CURRENT_HARMONICS);
    else if(strcmp(Serie, "VOLTAGE_FFT") == 0)       Copy = (ParamsType)(Copy | VOLTAGE_FFT);
    else if(strcmp(Serie, "CURRENT_FFT") == 0)       Copy = (ParamsType)(Copy | CURRENT_FFT);
    else if(strcmp(Serie, "VOLTAGE_SAMPLES") == 0)   Copy = (ParamsType)(Copy | VOLTAGE_SAMPLES);
    else if(strcmp(Serie, "CURRENT_SAMPLES") == 0)   Copy = (ParamsType)(Copy | CURRENT_SAMPLES);
  }
  GetParams(Params, Copy);
  json_object *Result = json_object_new_object();
  if(Copy & FREQUENCY) json_object_object_add(Result, "Frequency", (isnan(Params.Frequency)) ? NULL : json_object_new_double(Params.Frequency));
  if(Copy & FLAG) json_object_object_add(Result, "Flag", json_object_new_boolean(Params.Flag));
  if(Copy & VOLTAGE) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage.size(); i++) json_object_array_add(Obj, (isnan(Params.Voltage[i])) ? NULL : json_object_new_double(Params.Voltage[i]));
    json_object_object_add(Result, "Voltage", Obj);
  }
  if(Copy & CURRENT) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current.size(); i++) json_object_array_add(Obj, (isnan(Params.Current[i])) ? NULL : json_object_new_double(Params.Current[i]));
    json_object_object_add(Result, "Current", Obj);
  }
  if(Copy & ACTIVE_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Active_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Active_Power[i])) ? NULL : json_object_new_double(Params.Active_Power[i]));
    json_object_object_add(Result, "Active_Power", Obj);
  }
  if(Copy & REACTIVE_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Reactive_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Reactive_Power[i])) ? NULL : json_object_new_double(Params.Reactive_Power[i]));
    json_object_object_add(Result, "Reactive_Power", Obj);
  }
  if(Copy & ACTIVE_ENERGY) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Active_Energy.size(); i++) json_object_array_add(Obj, (isnan(Params.Active_Energy[i])) ? NULL : json_object_new_double(Params.Active_Energy[i]));
    json_object_object_add(Result, "Active_Energy", Obj);
  }
  if(Copy & REACTIVE_ENERGY) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Reactive_Energy.size(); i++) json_object_array_add(Obj, (isnan(Params.Reactive_Energy[i])) ? NULL : json_object_new_double(Params.Reactive_Energy[i]));
    json_object_object_add(Result, "Reactive_Energy", Obj);
  }
  if(Copy & APPARENT_POWER) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Apparent_Power.size(); i++) json_object_array_add(Obj, (isnan(Params.Apparent_Power[i])) ? NULL : json_object_new_double(Params.Apparent_Power[i]));
    json_object_object_add(Result, "Apparent_Power", Obj);
  }
  if(Copy & POWER_FACTOR) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Power_Factor.size(); i++) json_object_array_add(Obj, (isnan(Params.Power_Factor[i])) ? NULL : json_object_new_double(Params.Power_Factor[i]));
    json_object_object_add(Result, "Power_Factor", Obj);
  }
  if(Copy & VOLTAGE_THD) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage_THD.size(); i++) json_object_array_add(Obj, (isnan(Params.Voltage_THD[i])) ? NULL : json_object_new_double(Params.Voltage_THD[i]));
    json_object_object_add(Result, "Voltage_THD", Obj);
  }
  if(Copy & CURRENT_THD) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current_THD.size(); i++) json_object_array_add(Obj, (isnan(Params.Current_THD[i])) ? NULL : json_object_new_double(Params.Current_THD[i]));
    json_object_object_add(Result, "Current_THD", Obj);
  }
  if(Copy & PHI) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Phi.size(); i++) json_object_array_add(Obj, (isnan(Params.Phi[i])) ? NULL : json_object_new_double(Params.Phi[i]));
    json_object_object_add(Result, "Phi", Obj);
  }
  if(Copy & VOLTAGE_PHASE) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage_Phase.size(); i++) json_object_array_add(Obj, (isnan(Params.Phi[i])) ? NULL : json_object_new_double(Params.Phi[i]));
    json_object_object_add(Result, "Voltage_Phase", Obj);
  }
  if(Copy & VOLTAGE_HARMONICS) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage_Harmonics.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Voltage_Harmonics[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Voltage_Harmonics[i][j])) ? NULL : json_object_new_double(Params.Voltage_Harmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Voltage_Harmonics", Obj);
  }
  if(Copy & CURRENT_HARMONICS) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current_Harmonics.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Current_Harmonics[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Current_Harmonics[i][j])) ? NULL : json_object_new_double(Params.Current_Harmonics[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Current_Harmonics", Obj);
  }
  if(Copy & VOLTAGE_SAMPLES) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage_Samples.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Voltage_Samples[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Voltage_Samples[i][j])) ? NULL : json_object_new_double(Params.Voltage_Samples[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Voltage_Samples", Obj);
  }
  if(Copy & CURRENT_SAMPLES) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current_Samples.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Current_Samples[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Current_Samples[i][j])) ? NULL : json_object_new_double(Params.Current_Samples[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Current_Samples", Obj);
  }
  if(Copy & VOLTAGE_FFT) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Voltage_FFT.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Voltage_FFT[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Voltage_FFT[i][j])) ? NULL : json_object_new_double(Params.Voltage_FFT[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Voltage_FFT", Obj);
  }
  if(Copy & CURRENT_FFT) {
    json_object *Obj = json_object_new_array();
    for(uint i = 0; i < Params.Current_FFT.size(); i++) {
      json_object *Obj2 = json_object_new_array();
      for(uint j = 0; j < Params.Current_FFT[i].size(); j++) json_object_array_add(Obj2, (isnan(Params.Current_FFT[i][j])) ? NULL : json_object_new_double(Params.Current_FFT[i][j]));
      json_object_array_add(Obj, Obj2);
    }
    json_object_object_add(Result, "Current_FFT", Obj);
  }
  json_object_object_add(Result, "Sampletime", json_object_new_int64(Params.Timestamp));
  return Result;
}
AnalyzerInterface::AnalyzerMode AnalyzerInterface::GetMode() {
  return pMode;
}
void AnalyzerInterface::AccumulateAggreg(Params_t &dst, Params_t &src) {
  dst.Timestamp = src.Timestamp;
  for(uint i = 0; i < dst.Voltage.size(); i++) dst.Voltage[i] = AccumulateRMS(dst.Voltage[i], src.Voltage[i]);
  dst.Voltage_Inputs += src.Voltage_Inputs;
  dst.Frequency = AccumulateSUM(dst.Frequency, src.Frequency);
  dst.Frequency_Inputs += src.Frequency_Inputs;
  for(uint i = 0; i < dst.Current.size(); i++) dst.Current[i] = AccumulateRMS(dst.Current[i], src.Current[i]);
  for(uint i = 0; i < dst.Current_Inputs.size(); i++) dst.Current_Inputs[i] += src.Current_Inputs[i];
  for(uint i = 0; i < dst.Active_Power.size(); i++) dst.Active_Power[i] = AccumulateSUM(dst.Active_Power[i], src.Active_Power[i]);
  for(uint i = 0; i < dst.Active_Energy.size(); i++) dst.Active_Energy[i] = AccumulateSUM(dst.Active_Energy[i], src.Active_Energy[i]);
  for(uint i = 0; i < dst.Reactive_Power.size(); i++) dst.Reactive_Power[i] = AccumulateSUM(dst.Reactive_Power[i], src.Reactive_Power[i]);
  for(uint i = 0; i < dst.Reactive_Energy.size(); i++) dst.Reactive_Energy[i] = AccumulateSUM(dst.Reactive_Energy[i], src.Reactive_Energy[i]);
  for(uint i = 0; i < dst.Power_Factor.size(); i++) dst.Power_Factor[i] = AccumulateSUM(dst.Power_Factor[i], src.Power_Factor[i]);
  for(uint i = 0; i < dst.Voltage_THD.size(); i++) dst.Voltage_THD[i] = AccumulateRMS(dst.Voltage_THD[i], src.Voltage_THD[i]);
  for(uint i = 0; i < dst.Current_THD.size(); i++) dst.Current_THD[i] = AccumulateRMS(dst.Current_THD[i], src.Current_THD[i]);
  for(uint i = 0; i < dst.Phi.size(); i++) dst.Phi[i] = AccumulateSUM(dst.Phi[i], src.Phi[i]);
  for(uint i = 0; i < dst.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Voltage_Harmonics[i].size(); j++) dst.Voltage_Harmonics[i][j] = AccumulateRMS(dst.Voltage_Harmonics[i][j], src.Voltage_Harmonics[i][j]);
  }
  for(uint i = 0; i < dst.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Current_Harmonics[i].size(); j++) dst.Current_Harmonics[i][j] = AccumulateRMS(dst.Current_Harmonics[i][j], src.Current_Harmonics[i][j]);
  }
  dst.Flag |= src.Flag;
}
void AnalyzerInterface::NormalizeAggreg(Params_t &params) {
  for(uint i = 0; i < params.Voltage.size(); i++) params.Voltage[i] = sqrt(params.Voltage[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current.size(); i++) params.Current[i] = sqrt(params.Current[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = params.Active_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = params.Reactive_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = params.Power_Factor[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Current_THD.size(); i++) params.Current_THD[i] = sqrt(params.Current_THD[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = params.Phi[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Voltage_THD.size(); i++) params.Voltage_THD[i] = sqrt(params.Voltage_THD[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Current_Harmonics[i].size(); j++) params.Current_Harmonics[i][j] = sqrt(params.Current_Harmonics[i][j] / params.Current_Inputs[i]);
  }
  for(uint i = 0; i < params.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Voltage_Harmonics[i].size(); j++) params.Voltage_Harmonics[i][j] = sqrt(params.Voltage_Harmonics[i][j] / params.Voltage_Inputs);
  }
  params.Frequency = params.Frequency / params.Frequency_Inputs;
  if(params.Frequency_Inputs > 1) params.Frequency_Inputs = 1;
  if(params.Voltage_Inputs > 1) params.Voltage_Inputs = 1;
  for(uint i = 0; i < params.Current_Inputs.size(); i++) {
    if(params.Current_Inputs[i] > 1) params.Current_Inputs[i] = 1;
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
