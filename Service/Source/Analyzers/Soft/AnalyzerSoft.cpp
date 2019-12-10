/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#include "cctz/time_zone.h"
#include "muParser/muParser.h"
#include "AnalyzerSoft.h"
#include "Tools.h"
#include "SysSupport.h"
#include "Events.h"
#include "FFT.h"
#include "Sync.h"
#include "Rates.h"
#include "Alarms.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
const vector<float> AnalyzerSoft::FrequencyRanges  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
const vector<float> AnalyzerSoft::VoltageRanges    = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
const vector<float> AnalyzerSoft::THDRanges        = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
const vector<float> AnalyzerSoft::HarmonicsAllowed = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id) {
  LOGFile::Debug("Creating AnalyzerSoft database tables\n");
  Database DB(id);
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Tools::GetTime() - 86400000));   //Delete 1 day old
  DB.ExecSentenceNoResult("DELETE FROM aggreg_10c WHERE sampletime < " + DB.Bind(Tools::GetTime() - 3600000));       //Delete 1 hour old
  pMode            = mode;
  pVoltageChannels = (pMode == PHASE1) ? 1 : 3;
  pCurrentChannels = (pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4;
  pConfigPath      = optionPath;
  pSamplerate      = samplerate;
  pCycleLen        = 0;
  pTerminate       = false;
  pVoltageSamples  = vector<vector<float> >(pVoltageChannels, vector<float>());
  pCurrentSamples  = vector<vector<float> >(pCurrentChannels, vector<float>());
  pEventsDetector  = NULL;
  pSync            = NULL;
  pRates           = NULL;
  pAlarms          = NULL;
  pDelayCurrent    = NULL;
  pDelayVoltage    = NULL;
  pBlockCycles.reserve(20);
  InitAggreg(pAggreg10C, AGGREG_10C);
//  pVoltageFFT.reserve(pVoltageSamples.size());
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFT[i] = new FFT("FFT_V" + to_string(i));
//  pCurrentFFT.reserve(pCurrentSamples.size());
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFT[i] = new FFT("FFT_I" + to_string(i));
  Configure(CheckConfig(ConfigManager::ReadConfig(optionPath)));
  if(pthread_create(&pAnalyzerThread, NULL, Analyzer_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pAnalyzerThread = 0;
  } else {
    pthread_setname_np(pAnalyzerThread, (pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N");
  }
}
AnalyzerSoft::~AnalyzerSoft() {
  pMembersLocks.WriteLock();
  if(pSync != NULL) delete pSync;
  if(pRates != NULL) delete pRates;
  if(pAlarms != NULL) delete pAlarms;
  if(pEventsDetector != NULL) delete pEventsDetector;
  pMembersLocks.WriteUnlock();
  pTerminate = true;  
  pAnalyzerWorkerLocks.Abort(&pAnalyzerThread);
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  for(unsigned n = 0; n < pVoltageChannels; n++) delete pVoltageFFT[n];
  for(unsigned n = 0; n < pCurrentChannels; n++) delete pCurrentFFT[n];
}
void AnalyzerSoft::Configure(const json &options) {
  pName             = options["Name"];
  pNominalVoltage   = options["NominalVoltage"];
  pNominalFrequency = options["NominalFreq"];
  pMaxCurrent       = options["MaxCurrent"];
  pMinCurrent       = options["MinCurrent"];
  pTimeZone         = options["TimeZone"];
  pContract         = options["Contract"];
  pZipCode          = options["ZipCode"];
  pGeo              = options["GeoPosition"].get<vector<float> >();
  pSave10c          = options["Save10C"];
  pMaxSamples       = (pNominalFrequency == 0.0) ? pSamplerate * 0.2 : ceil((15.0 * pSamplerate) / pNominalFrequency);
  for(unsigned i = 0; i < pVoltageChannels; i++) pVoltageSamples[i].reserve(pMaxSamples);
  for(unsigned i = 0; i < pCurrentChannels; i++) pCurrentSamples[i].reserve(pMaxSamples);
  bool EventsEnabled = options["EventsEnabled"];
  if((EventsEnabled == false) && (pEventsDetector != NULL)) {
    delete pEventsDetector;
    pEventsDetector = NULL;
  } else if((EventsEnabled == true) && (pEventsDetector == NULL)) {
    pEventsDetector = new AnalyzerSoft::Events(this);
  }
  bool RatesEnabled = options["RatesEnabled"];
  if((RatesEnabled == true) && (pRates != NULL)) {
    delete pRates;  
  } else if((RatesEnabled == true) && (pRates == NULL)) {
    pRates = new AnalyzerSoft::Rates(this);
  }
  bool AlarmsEnabled = options["AlarmsEnabled"];
  if((AlarmsEnabled == true) && (pAlarms != NULL)) {
    delete pAlarms;  
  } else if((AlarmsEnabled == true) && (pAlarms == NULL)) {
    pAlarms = new AnalyzerSoft::Alarms(this, options["Alarms"]);
  }
  bool SyncEnabled = options["RemoteEnabled"];
  if((SyncEnabled == false) && (pSync != NULL)) {
    delete pSync;
    pSync = NULL;
  } else if(SyncEnabled == true) {
    string Server = options["RemoteServer"];
    string User = options["RemoteUser"];
    string Pass = options["RemotePass"];
    if(pSync == NULL) {
      pSync = new Sync(this, Server, User, Pass);
    } else {
      pSync->Configure(Server, User, Pass);
    }
  }
  //Calc peak filter coefficients for nominal frequency
  if(pNominalFrequency != 0.0) {
    for(unsigned n = 0; n < ZEROCROSSING_FILTERSTAGES; n++) {
      float w0 = 2 * M_PI * (pNominalFrequency / pSamplerate);
      float alpha = sin(w0) / 2.0;
      pFilters[n].A0 = alpha / (1.0 + alpha);
      pFilters[n].A1 = 0.0;
      pFilters[n].A2 = -alpha / (1.0 + alpha);
      pFilters[n].B1 = (-2.0 * cos(w0)) / (1.0 + alpha);
      pFilters[n].B2 = (1.0 - alpha) / (1.0 + alpha);
      memset(pFilters[n].ChannelsZ1, 0, sizeof(Filter_t::ChannelsZ1));
      memset(pFilters[n].ChannelsZ2, 0, sizeof(Filter_t::ChannelsZ2));
    }
    pDelayCurrentSize = (pSamplerate * ZEROCROSSING_FILTERSTAGES) / (pNominalFrequency * 2.0);
    pDelayVoltageSize = pDelayCurrentSize;
  } else {
    pDelayCurrentSize = 1;
    pDelayVoltageSize = 1;
  }
  pDelayCurrentPos = 0;
  pDelayCurrent = (float*)realloc(pDelayCurrent, sizeof(float) * pDelayCurrentSize * pCurrentChannels);
  pDelayVoltagePos = 0;
  pDelayVoltage = (float*)realloc(pDelayVoltage, sizeof(float) * pDelayVoltageSize * pVoltageChannels);
}
bool AnalyzerSoft::BufferAppend(const float *voltageSamples, const float *currentSamples) {
  pMembersLocks.ReadLock();
  float Outs[3];
  float *Ptr = &pDelayVoltage[pDelayVoltagePos * pVoltageChannels];
  for(unsigned i = 0; i < pVoltageChannels; i++) {
    pVoltageSamples[i].push_back(Ptr[i]);
    Ptr[i] = voltageSamples[i];
    Outs[i] = voltageSamples[i];
  }
  pDelayVoltagePos = (pDelayVoltagePos + 1) % pDelayVoltageSize;
  Ptr = &pDelayCurrent[pDelayCurrentPos * pCurrentChannels];
  for(unsigned i = 0; i < pCurrentChannels; i++) {
    pCurrentSamples[i].push_back(Ptr[i]);
    Ptr[i] = currentSamples[i];
  }
  pDelayCurrentPos = (pDelayCurrentPos + 1) % pDelayCurrentSize;
  for(unsigned i = 0; i < pVoltageChannels; i++) {
    for(unsigned n = 0; n < ZEROCROSSING_FILTERSTAGES; n++) {
      float Out = Outs[i] * pFilters[n].A0 + pFilters[n].ChannelsZ1[i];
      pFilters[n].ChannelsZ1[i] = Outs[i] * pFilters[n].A1 + pFilters[n].ChannelsZ2[i] - pFilters[n].B1 * Out;
      pFilters[n].ChannelsZ2[i] = Outs[i] * pFilters[n].A2 - pFilters[n].B2 * Out;
      Outs[i] = Out;
    }
  }
  if((pBlockCycles.size() == pBlockCycles.capacity()) || (pNominalFrequency == 0)) pBlockCycles.clear();
  pCycleLen++;
  if(pNominalFrequency != 0) {
    if((((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) && (pCycleLen > ((pSamplerate * 0.85) / (2.0 * pNominalFrequency)))) {
      pBlockCycles.push_back(pCycleLen);
      pCycles.push_back(pCycleLen);
      if(pCycles.size() > (ZEROCROSSING_SECONDS * pNominalFrequency)) pCycles.pop_front();
      pCycleLen = 0;
    }
    for(unsigned i = 0; i < pVoltageChannels; i++) pLastSamples[i] = Outs[i];
  }
  pMembersLocks.ReadUnlock();
  if((pBlockCycles.size() == pBlockCycles.capacity()) || (pVoltageSamples[0].size() == pMaxSamples)) {
    pAnalyzerWorkerLocks.WakeWorker();
    return true;
  }
  return false;
}
bool AnalyzerSoft::IsFinished() {
  return pAnalyzerWorkerLocks.IsWorking();
}
void AnalyzerSoft::InitAggreg(Params_t &params, AggregType type) {
  params.Type = type;
  params.Timestamp = 0;
  params.Frequency = NAN;
  params.Voltage_Inputs = 0;
  params.Frequency_Inputs = 0;
  params.Flag = false;
  params.Current_Inputs = vector<uint16_t>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 0);
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    params.Voltage = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
  }
  if(params.Type == AGGREG_1H) {
    params.StatsFrequency = vector<uint16_t>(FrequencyRanges.size() - 1, 0);
    params.StatsVoltage = vector<uint16_t>(VoltageRanges.size() - 1, 0);
    params.StatsTHD = vector<uint16_t>(THDRanges.size() - 1, 0);
    params.StatsHarmonics = vector<uint16_t>(2, 0);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1H)) {
    params.Active_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M)) {
    params.Current_THD = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
    params.Phi = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Power_Factor = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current_Harmonics = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
    params.Power_Harmonics = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M)) {
    params.Voltage_THD = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Harmonics = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
  }
  if(params.Type == AGGREG_10C) {
    params.Active_Energy = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Phase  = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Energy = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Apparent_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
  }
}
void AnalyzerSoft::AccumulateAggreg(Params_t &dst, const Params_t &src) {
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
  for(uint i = 0; i < dst.Power_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Power_Harmonics[i].size(); j++) dst.Power_Harmonics[i][j] = AccumulateSUM(dst.Power_Harmonics[i][j], src.Power_Harmonics[i][j]);
  }
  for(uint i = 0; i < dst.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Voltage_Harmonics[i].size(); j++) dst.Voltage_Harmonics[i][j] = AccumulateRMS(dst.Voltage_Harmonics[i][j], src.Voltage_Harmonics[i][j]);
  }
  for(uint i = 0; i < dst.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Current_Harmonics[i].size(); j++) dst.Current_Harmonics[i][j] = AccumulateRMS(dst.Current_Harmonics[i][j], src.Current_Harmonics[i][j]);
  }
  dst.Flag |= src.Flag;
}
void AnalyzerSoft::NormalizeAggreg(Params_t &params) {
  for(uint i = 0; i < params.Voltage.size(); i++) params.Voltage[i] = sqrt(params.Voltage[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current.size(); i++) params.Current[i] = sqrt(params.Current[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = params.Active_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = params.Reactive_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = params.Power_Factor[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Current_THD.size(); i++) params.Current_THD[i] = sqrt(params.Current_THD[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = params.Phi[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Voltage_THD.size(); i++) params.Voltage_THD[i] = sqrt(params.Voltage_THD[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Current_Harmonics[i].size(); j++) params.Current_Harmonics[i][j] = sqrt(params.Current_Harmonics[i][j] / (float)params.Current_Inputs[i]);
  }
  for(uint i = 0; i < params.Power_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Power_Harmonics[i].size(); j++) params.Power_Harmonics[i][j] = params.Power_Harmonics[i][j] / (float)params.Current_Inputs[i];
  }
  for(uint i = 0; i < params.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Voltage_Harmonics[i].size(); j++) params.Voltage_Harmonics[i][j] = sqrt(params.Voltage_Harmonics[i][j] / (float)params.Voltage_Inputs);
  }
  params.Frequency = params.Frequency / params.Frequency_Inputs;
  if(params.Frequency_Inputs > 1) params.Frequency_Inputs = 1;
  if(params.Voltage_Inputs > 1) params.Voltage_Inputs = 1;
  for(uint i = 0; i < params.Current_Inputs.size(); i++) {
    if(params.Current_Inputs[i] > 1) params.Current_Inputs[i] = 1;
  }
}
float AnalyzerSoft::AccumulateSUM(const float val1, const float val2) {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return val2;
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + val2;
}
float AnalyzerSoft::AccumulateRMS(const float val1, const float val2) {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return pow(val2, 2.0);
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + pow(val2, 2.0);
}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  Database DB(pSerial);
  if(params.Type == AGGREG_10C)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10c(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Voltage) + ", " + DB.Bind(params.Current) + ", " + DB.Bind(params.Frequency) + ", " + DB.Bind(params.Flag) + ", " + DB.Bind(params.Active_Power) + ", " + DB.Bind(params.Reactive_Power) + ", " + DB.Bind(params.Power_Factor) + ", " + DB.Bind(params.Voltage_THD) + ", " + DB.Bind(params.Current_THD) + ", " + DB.Bind(params.Phi) + ")");
  else if(params.Type == AGGREG_3S)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Voltage) + ", " + DB.Bind(params.Current) + ", " + DB.Bind(params.Frequency) + ", " + DB.Bind(params.Flag) + ", " + DB.Bind(params.Active_Power) + ", " + DB.Bind(params.Reactive_Power) + ")");
  else if(params.Type == AGGREG_1M)
    DB.ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Voltage) + ", " + DB.Bind(params.Current) + ", " + DB.Bind(params.Frequency) + ", " + DB.Bind(params.Flag) + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i, harmonics_p) SELECT sampletime, " + DB.Bind(params.Active_Power) + ", " + DB.Bind(params.Reactive_Power) + ", " + DB.Bind(params.Power_Factor) + ", " + DB.Bind(params.Voltage_THD) + ", " + DB.Bind(params.Current_THD) + ", " + DB.Bind(params.Phi) + ", " + DB.Bind(params.Voltage_Harmonics) + ", " + DB.Bind(params.Current_Harmonics) + ", " + DB.Bind(params.Power_Harmonics) + " FROM Sample");
  else if(params.Type == AGGREG_10M)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Voltage) + ", " + DB.Bind(params.Current) + ", " + DB.Bind(params.Frequency) + ", " + DB.Bind(params.Flag) + ")");
  else if(params.Type == AGGREG_15M)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Active_Power) + ", " + DB.Bind(params.Reactive_Power) + ")");
  else if(params.Type == AGGREG_1H)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES(" + DB.Bind(params.Timestamp) + ", " + DB.Bind(params.Voltage) + ", " + DB.Bind(params.Current) + ", " + DB.Bind(params.Frequency) + ", " + DB.Bind(params.Flag) + ", " + DB.Bind(params.Active_Power) + ", " + DB.Bind(params.Reactive_Power) + ", " + DB.Bind(params.StatsVoltage) + ", " + DB.Bind(params.StatsFrequency) + ", " + DB.Bind(params.StatsTHD) + ", " + DB.Bind(params.StatsHarmonics) + ")");
  if(params.Type == AGGREG_1M) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(params.Timestamp - 86400000));   //Delete 1 day old
  if((params.Type == AGGREG_1M) && (pSave10c == true)) DB.ExecSentenceNoResult("DELETE FROM aggreg_10c WHERE sampletime < " + DB.Bind(params.Timestamp - 3600000));     //Delete 1 hour old
  //TEST
//  json Result;
//  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
//    Result["Voltage"] = params.Voltage;
//    Result["Current"] = params.Current;
//    Result["Frequency"] = params.Frequency;
//    Result["Flag"] = params.Flag;
//  }
//  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_1H)) {
//    Result["Active_Power"] = params.Active_Power;
//    Result["Reactive_Power"] = params.Reactive_Power;
//  }
//  if(params.Type == AGGREG_1M) {
//    Result["Power_Factor"] = params.Power_Factor;
//    Result["Voltage_THD"] = params.Voltage_THD;
//    Result["Current_THD"] = params.Current_THD;
//    Result["Phi"] = params.Phi;
//    Result["Voltage_Harmonics"] = params.Voltage_Harmonics;
//    Result["Current_Harmonics"] = params.Current_Harmonics;
//  }
//  Result["Aggregation"] = (params.Type == AGGREG_3S) ? "3S" : (params.Type == AGGREG_1M) ? "1M" : (params.Type == AGGREG_10M) ? "10M" : (params.Type == AGGREG_15M) ? "15M": "1H";
//  Result["Sampletime"] = params.Timestamp;
//  Tools::SendUDP(Result);
  pLastSave = Tools::GetTime();
}
float AnalyzerSoft::CalcChange(const float in, const float reference) {
  return ((in - reference) / reference) * 100;
}
void *AnalyzerSoft::Analyzer_Thread(void *args) {
  AnalyzerSoft *This = (AnalyzerSoft*)args;
  uint64_t Sequence      = 0;
  uint64_t PrevTimestamp = 0;
  Params_t Aggreg3S;
  This->InitAggreg(Aggreg3S, AGGREG_3S);
  Params_t Aggreg1M;
  This->InitAggreg(Aggreg1M, AGGREG_1M);
  Params_t Aggreg10M;
  This->InitAggreg(Aggreg10M, AGGREG_10M);
  Params_t Aggreg15M;
  This->InitAggreg(Aggreg15M, AGGREG_15M);
  Params_t Aggreg1H;
  This->InitAggreg(Aggreg1H, AGGREG_1H);
  while(This->pTerminate == false) {
    if(This->pAnalyzerWorkerLocks.WorkerWaitTask() == false) return NULL;
    uint64_t Start = Tools::GetTime();
    This->pAggreg10C.Timestamp = Start;
    //Init FFTs and event detection threads
    This->pMembersLocks.ReadLock();
    if(This->pEventsDetector != NULL) This->pEventsDetector->Start();
    for(uint i = 0; i < This->pVoltageChannels; i++) This->pVoltageFFT[i]->Start(This->pVoltageSamples[i]);
    for(uint i = 0; i < This->pCurrentChannels; i++) This->pCurrentFFT[i]->Start(This->pCurrentSamples[i]);
    if(This->pNominalFrequency != 0) {
      uint32_t Samples = 0;
      uint32_t MinCycle = This->pSamplerate / (This->pNominalFrequency * 1.15 * 2.0);
      uint32_t MaxCycle = This->pSamplerate / (This->pNominalFrequency * 0.85 * 2.0);
      for(uint i = 0; i < This->pCycles.size(); i++) {
        if((This->pCycles[i] < MinCycle) || (This->pCycles[i] > MaxCycle)) {
          Samples = 0;
          break;
        } else {
          Samples += This->pCycles[i];
        }
      }
      This->pAggreg10C.Frequency = (Samples == 0) ? NAN : This->pCycles.size() / ((2.0 * Samples) / This->pSamplerate);
    }
    if(!isnan(This->pAggreg10C.Frequency)) This->pAggreg10C.Frequency_Inputs = 1;
    bool Valid = false;
    vector<float> RMS = vector<float>(This->pAggreg10C.Voltage.size(), 0);
    for(unsigned j = 0; j < This->pVoltageChannels; j++) {
      for(uint32_t i = 0; i < This->pVoltageSamples[j].size(); i++) RMS[j] += pow(This->pVoltageSamples[j][i], 2.0);
      RMS[j] = sqrt(RMS[j] / This->pVoltageSamples[j].size());
      if(RMS[j] > 1) Valid = true;
    }
    if(Valid) This->pAggreg10C.Voltage.assign(RMS.begin(), RMS.end());
    if(Valid) This->pAggreg10C.Voltage_Inputs = 1;
    for(unsigned j = 0; j < This->pCurrentChannels; j++) {
      float RMS = 0;
      for(uint32_t i = 0; i < This->pCurrentSamples[j].size(); i++) RMS += pow(This->pCurrentSamples[j][i], 2.0);
      RMS = sqrt(RMS / This->pCurrentSamples[j].size());
      if(RMS > This->pMinCurrent) {
        This->pAggreg10C.Current_Inputs[j] = 1;
        This->pAggreg10C.Current[j] = RMS;
        if(j < This->pVoltageSamples.size()) {
          for(uint32_t i = 0; i < This->pVoltageSamples[j].size(); i++) This->pAggreg10C.Active_Power[j] = This->AccumulateSUM(This->pAggreg10C.Active_Power[j], This->pVoltageSamples[j][i] * This->pCurrentSamples[j][i]);
          This->pAggreg10C.Active_Power[j] = This->pAggreg10C.Active_Power[j] / This->pCurrentSamples[j].size();
        }
      }
    }
    for(unsigned j = 0; j < This->pVoltageChannels; j++) {
      This->pVoltageFFT[j]->Wait();
      if(isnan(This->pAggreg10C.Voltage[j])) continue;
      if(j == 0) {
        This->pAggreg10C.Voltage_Phase[0] = 0;
      } else {
        This->pAggreg10C.Voltage_Phase[j] = FFT::CalcAngle(This->pVoltageFFT[j]->Output[10], This->pVoltageFFT[0]->Output[10]) * 180.0 / M_PI;
      }
      for(uint i = 0; i < This->pAggreg10C.Voltage_Harmonics[j].size(); i++) {
        This->pAggreg10C.Voltage_Harmonics[j][i] = This->pVoltageFFT[j]->BinPower(10 + i * 10);
        if(i > 0) This->pAggreg10C.Voltage_THD[j] = This->AccumulateSUM(This->pAggreg10C.Voltage_THD[j], This->pAggreg10C.Voltage_Harmonics[j][i]);
      }
      This->pAggreg10C.Voltage_THD[j] = (This->pAggreg10C.Voltage_THD[j] / This->pAggreg10C.Voltage_Harmonics[j][0]) * 100.0;
    }
    for(int j = 0; j < This->pVoltageChannels; j++) {
      This->pCurrentFFT[j]->Wait();
      if(isnan(This->pAggreg10C.Current[j]) || isnan(This->pAggreg10C.Voltage[j])) continue;
      This->pAggreg10C.Apparent_Power[j] = This->pAggreg10C.Voltage[j] * This->pAggreg10C.Current[j];
      for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[j].size(); i++) {
        This->pAggreg10C.Current_Harmonics[j][i] = This->pCurrentFFT[j]->BinPower(10 + i * 10);
        This->pAggreg10C.Power_Harmonics[j][i] = This->pCurrentFFT[j]->BinPower(10 + i * 10) * This->pVoltageFFT[j]->BinPower(10 + i * 10) * cos(FFT::CalcAngle(This->pCurrentFFT[j]->Output[10 + i * 10] * (float)M_SQRT2, This->pVoltageFFT[j]->Output[10 + i * 10] * (float)M_SQRT2));
        if(i > 0) This->pAggreg10C.Current_THD[j] = This->AccumulateSUM(This->pAggreg10C.Current_THD[j], This->pAggreg10C.Current_Harmonics[j][i]);
      }
      float PHI = FFT::CalcAngle(This->pCurrentFFT[j]->Output[10], This->pVoltageFFT[j]->Output[10]);
      This->pAggreg10C.Reactive_Power[j] = This->pAggreg10C.Voltage_Harmonics[j][0] * This->pAggreg10C.Current_Harmonics[j][0] * sin(-PHI);
      This->pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
      This->pAggreg10C.Current_THD[j] = (This->pAggreg10C.Current_THD[j] / This->pAggreg10C.Current_Harmonics[j][0]) * 100.0;
      This->pAggreg10C.Active_Energy[j] = (This->pAggreg10C.Active_Power[j] * This->pCurrentSamples[j].size()) / (This->pSamplerate * 3600.0);
      This->pAggreg10C.Power_Factor[j] = This->pAggreg10C.Active_Power[j] / This->pAggreg10C.Apparent_Power[j];
      This->pAggreg10C.Reactive_Energy[j] = (This->pAggreg10C.Reactive_Power[j] * This->pAggreg10C.Current_Inputs[j]) / (This->pSamplerate * 3600.0);
    }
    if(This->pMode == PHASE3N) {
      This->pCurrentFFT[3]->Wait();
      if(!isnan(This->pAggreg10C.Current[3])) {
        for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[3].size(); i++) {
          This->pAggreg10C.Current_Harmonics[3][i] = This->pCurrentFFT[3]->BinPower(10 + i * 10);
          if(i > 0) This->pAggreg10C.Current_THD[3] = This->AccumulateSUM(This->pAggreg10C.Current_THD[3], This->pAggreg10C.Current_Harmonics[3][i]);
        }
        This->pAggreg10C.Current_THD[3] = (This->pAggreg10C.Current_THD[3] / This->pAggreg10C.Current_Harmonics[3][0]) * 100.0;
      }
    }
    if(This->pEventsDetector != NULL) This->pEventsDetector->Wait();
    //Save results
    if(((Sequence % 15) == 0) && (PrevTimestamp != 0)) {
      LOGFile::Debug("Storing 3S aggregation\n");
      This->NormalizeAggreg(Aggreg3S);
      if(This->pAlarms != NULL) This->pAlarms->Start(Aggreg3S);
      if(Aggreg3S.Flag == false) {
        float Variation = This->CalcChange(Aggreg3S.Frequency, This->pNominalFrequency);
        for(uint Range = 0; Range < 40; Range++) {
          if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) Aggreg1H.StatsFrequency[Range]++;
        }
      }
      This->SaveAggreg(Aggreg3S);
      if(This->pAlarms != NULL) This->pAlarms->Wait();
      This->InitAggreg(Aggreg3S, AGGREG_3S);
    }
    This->AccumulateAggreg(Aggreg3S, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 60000) > (PrevTimestamp / 60000)) && (PrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1M aggregation\n");
      Aggreg1M.Timestamp = Aggreg1M.Timestamp / 60000 * 60000 + 59999;
      This->NormalizeAggreg(Aggreg1M);
      if(This->pAlarms != NULL) This->pAlarms->Start(Aggreg1M);
      This->SaveAggreg(Aggreg1M);
      if(This->pAlarms != NULL) This->pAlarms->Wait();
      This->InitAggreg(Aggreg1M, AGGREG_1M);
    }
    This->AccumulateAggreg(Aggreg1M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 600000) > (PrevTimestamp / 600000)) && (PrevTimestamp != 0)) {
      LOGFile::Debug("Storing 10M aggregation\n");
      Aggreg10M.Timestamp = Aggreg10M.Timestamp / 600000 * 600000 + 599999;
      This->NormalizeAggreg(Aggreg10M);
      if(This->pAlarms != NULL) This->pAlarms->Start(Aggreg10M);
      if(Aggreg10M.Flag == false) {
        for(uint i = 0; i < Aggreg10M.Voltage.size(); i++) {
          float Variation = This->CalcChange(Aggreg10M.Voltage[i], This->pNominalVoltage);
          for(uint Range = 0; Range < (VoltageRanges.size() - 1); Range++) {
            if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) Aggreg1H.StatsVoltage[Range]++;
          }
          for(uint Range = 0; Range < (THDRanges.size() - 1); Range++) {
            if((Aggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (Aggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) Aggreg1H.StatsTHD[Range]++;
          }
          bool HarmonicsInRange = true;
          for(uint n = 0; n < HarmonicsAllowed.size(); n++) {
            float Variation = This->CalcChange(sqrt(norm(Aggreg10M.Voltage_Harmonics[i][n + 1])), This->pNominalVoltage);
            if(Variation > HarmonicsAllowed[n]) {
              HarmonicsInRange = false;
              break;
             }
          }
          if(HarmonicsInRange == true) {
            Aggreg1H.StatsHarmonics[0]++;
          } else {
            Aggreg1H.StatsHarmonics[1]++;
          }
        }
      }
      This->SaveAggreg(Aggreg10M);
      if(This->pAlarms != NULL) This->pAlarms->Wait();
      This->InitAggreg(Aggreg10M, AGGREG_10M);
    }
    This->pMembersLocks.ReadUnlock();
    This->pNowDataLock.ProviderWrite();
    This->AccumulateAggreg(Aggreg10M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 900000) > (PrevTimestamp / 900000)) && (PrevTimestamp != 0)) {
      LOGFile::Debug("Storing 15M aggregation\n");
      Aggreg15M.Timestamp = Aggreg15M.Timestamp / 900000 * 900000 + 899999;
      This->NormalizeAggreg(Aggreg15M);
      if(This->pAlarms != NULL) This->pAlarms->Start(Aggreg15M);
      This->SaveAggreg(Aggreg15M);
      if(This->pAlarms != NULL) This->pAlarms->Wait();
      This->InitAggreg(Aggreg15M, AGGREG_15M);
    }
    This->AccumulateAggreg(Aggreg15M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 3600000) > (PrevTimestamp / 3600000)) && (PrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1H aggregation\n");
      Aggreg1H.Timestamp = Aggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      This->NormalizeAggreg(Aggreg1H);
      if(This->pAlarms != NULL) This->pAlarms->Start(Aggreg1H);
      This->SaveAggreg(Aggreg1H);
      if(This->pAlarms != NULL) This->pAlarms->Wait();
      This->InitAggreg(Aggreg1H, AGGREG_1H);
    }
    This->AccumulateAggreg(Aggreg1H, This->pAggreg10C);
    PrevTimestamp = This->pAggreg10C.Timestamp;
    if(This->pSave10c) This->SaveAggreg(This->pAggreg10C);
    json Status = json::object();
    Status["Event"] = This->pAggreg10C.Flag;
    SysSupport::SetSystemDetails(Status);
    This->pNowDataLock.ProviderWait();
    This->InitAggreg(This->pAggreg10C, AGGREG_10C);
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", This->pSerial.c_str(), Tools::GetTime() - Start, Sequence);
    for(unsigned i = 0; i < This->pVoltageChannels; i++) This->pVoltageSamples[i].clear();
    for(unsigned i = 0; i < This->pCurrentChannels; i++) This->pCurrentSamples[i].clear();
//    This->pCurrentSamples = vector<vector<float> >((This->pMode == PHASE1) ? 1 : (This->pMode == PHASE3) ? 3 : 4, vector<float>(0));
//    This->pVoltageSamples = vector<vector<float> >((This->pMode == PHASE1) ? 1 : 3, vector<float>(0));
    Sequence++;
    This->pAnalyzerWorkerLocks.WorkerEndsTask();
  }
  return NULL;
}
json AnalyzerSoft::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed analyzer";
  ConfigOUT["NominalVoltage"] = 230.0;
  ConfigOUT["NominalFreq"] = 50.0;
  ConfigOUT["MaxCurrent"] = 35.0;
  ConfigOUT["MinCurrent"] = 0.1;
  ConfigOUT["TimeZone"] = "UTC";
  ConfigOUT["Contract"] = "";
  ConfigOUT["ZipCode"] = "";
  ConfigOUT["GeoPosition"] = json::parse("[0, 0]");
  ConfigOUT["RemoteEnabled"] = false;
  ConfigOUT["RemoteServer"] = "";
  ConfigOUT["RemoteUser"] = "";
  ConfigOUT["RemotePass"] = "";
  ConfigOUT["Save10C"] = false;
  ConfigOUT["EventsEnabled"] = false;
  ConfigOUT["RatesEnabled"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
  ConfigOUT["Alarms"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFreq") && config["NominalFreq"].is_number() && (config["NominalFreq"] >= 0) && (config["NominalFreq"] <= 400)) ConfigOUT["NominalFreq"] = config["NominalFreq"];
  if(config.contains("MaxCurrent") && config["MaxCurrent"].is_number() && (config["MaxCurrent"] >= 1) && (config["MaxCurrent"] <= 100000)) ConfigOUT["MaxCurrent"] = config["MaxCurrent"];
  if(config.contains("MinCurrent") && config["MinCurrent"].is_number() && (config["MinCurrent"] >= 0.01) && (config["MinCurrent"] <= 100)) ConfigOUT["MinCurrent"] = config["MinCurrent"];
  if(config.contains("TimeZone") && config["TimeZone"].is_string()) {
    time_zone TimeZone;
    string TZ = config["TimeZone"];
    if(cctz::load_time_zone(TZ, &TimeZone) == true) ConfigOUT["TimeZone"] = TZ;
  }
  if(config.contains("Contract") && config["Contract"].is_string()) ConfigOUT["Contract"] = config["Contract"];
  if(config.contains("ZipCode") && config["ZipCode"].is_string()) ConfigOUT["ZipCode"] = config["ZipCode"];
  if(config.contains("GeoPosition") && config["GeoPosition"].is_array() && (config["GeoPosition"].size() == 2) && config["GeoPosition"][0].is_number() && config["GeoPosition"][1].is_number()) ConfigOUT["GeoPosition"] = config["GeoPosition"];
  if(config.contains("RemoteEnabled") && config["RemoteEnabled"].is_boolean()) ConfigOUT["RemoteEnabled"] = config["RemoteEnabled"];
  if(config.contains("RemoteServer") && config["RemoteServer"].is_string()) ConfigOUT["RemoteServer"] = config["RemoteServer"];
  if(config.contains("RemoteUser") && config["RemoteUser"].is_string()) ConfigOUT["RemoteUser"] = config["RemoteUser"];
  if(config.contains("RemotePass") && config["RemotePass"].is_string()) ConfigOUT["RemotePass"] = config["RemotePass"];
  if(config.contains("Save10C") && config["Save10C"].is_boolean()) ConfigOUT["Save10C"] = config["Save10C"];
  if(config.contains("EventsEnabled") && config["EventsEnabled"].is_boolean()) ConfigOUT["EventsEnabled"] = config["EventsEnabled"];
  if(config.contains("RatesEnabled") && config["RatesEnabled"].is_boolean()) ConfigOUT["RatesEnabled"] = config["RatesEnabled"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
  if(config.contains("Alarms") && config["Alarms"].is_array()) ConfigOUT["Alarms"] = Alarms::CheckConfig(config["Alarms"]);
  return ConfigOUT;
}
void AnalyzerSoft::GetAnalyzer(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Response = CheckConfig(ConfigManager::ReadConfig(pConfigPath));
  pMembersLocks.ReadLock();
  Response["Analyzer"] = pSerial;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  Response.erase("Alarms");
  if(pSync != NULL) Response["RemoteLogged"] = pSync->Logged();
  req.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename))) AS size from pg_tables WHERE schemaname = " + req.Bind(pSerial));
  if(req.FecthRow()) Response["Size"] = req.ResultUInt64("size");
  pMembersLocks.ReadUnlock();
  req.FromJSON(Response);
}
void AnalyzerSoft::SetAnalyzer(HTTPRequest &req) {
  if(CheckRights(req, CONFIG) == false) return;
  json Config = CheckConfig(req.GetParams());
  Config.erase("Alarms");
  ConfigManager::WriteConfig(pConfigPath, Config);
  pMembersLocks.WriteLock();
  Configure(ConfigManager::ReadConfig(pConfigPath));
  pMembersLocks.WriteUnlock();
  req.FromString("", MHD_HTTP_OK);
}
void AnalyzerSoft::GetSeries(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  //Insert defaults
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 200;
  string Aggreg = Params["Aggreg"];
  if(Aggreg == "3S") PeriodSeconds = 3000;
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  string Query = "SELECT ROW_TO_JSON(Result) AS result FROM ( SELECT " + req.Bind(From) + " AS " + req.BindID("From") + ", " + req.Bind(To) +" AS " + req.BindID("To") + ", ";
  Query += req.Bind((To - From) / PeriodSeconds) + " AS " + req.BindID("MaxSamples") + ", " + req.Bind(Aggreg) + " AS " + req.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    string Serie = Params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + req.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
      }
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
      }
    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (((Aggreg == "10C") || Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_p, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Power_Harmonics") + ", ";      
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {   
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics_Stats") + ", ";
    } else {
      return req.FromString("Invalid serie in selected aggregation", MHD_HTTP_BAD_REQUEST);
    }
  }
  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + req.BindID("Sampletime") + " FROM ";
  if(Params["Aggreg"] == "1M") {
    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
  } else {
    Query += "aggreg_" + Aggreg;
  }
  Query += " WHERE sampletime > " + req.Bind(LastSample) + " AND sampletime >= " + req.Bind(From) + " AND sampletime <= " + req.Bind(To) + ") AS result";
  req.ExecSentenceAsyncResult(Query);
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void AnalyzerSoft::GetSeriesNow(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(pNowDataLock.ConsumerWait(5000) == false) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  json Result = json::object();
  Result["Sampletime"] = pAggreg10C.Timestamp;
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      pNowDataLock.ConsumerEnds();
      return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    }
    string Serie = Params["Series"][i];
    if(Serie == "FREQUENCY") {
      Result["Frequency"] = pAggreg10C.Frequency;
    } else if(Serie == "FLAG") {
      Result["Flag"] = pAggreg10C.Flag;
    } else if(Serie == "VOLTAGE") {
      Result["Voltage"] = pAggreg10C.Voltage;
    } else if(Serie == "CURRENT") {
      Result["Current"] = pAggreg10C.Current;
    } else if(Serie == "ACTIVE_POWER") {
      Result["Active_Power"] = pAggreg10C.Active_Power;
    } else if(Serie == "REACTIVE_POWER") {
      Result["Reactive_Power"] = pAggreg10C.Reactive_Power;
    } else if(Serie == "ACTIVE_ENERGY") {
      Result["Active_Energy"] = pAggreg10C.Active_Energy;
    } else if(Serie == "REACTIVE_ENERGY") {
      Result["Reactive_Energy"] = pAggreg10C.Reactive_Energy;
    } else if(Serie == "APPARENT_POWER") {
      Result["Apparent_Power"] = pAggreg10C.Apparent_Power;
    } else if(Serie == "POWER_FACTOR") {
      Result["Power_Factor"] = pAggreg10C.Power_Factor;
    } else if(Serie == "VOLTAGE_THD") {
      Result["Voltage_THD"] = pAggreg10C.Voltage_THD;
    } else if(Serie == "CURRENT_THD") {
      Result["Current_THD"] = pAggreg10C.Current_THD;
    } else if(Serie == "PHI") {
      Result["Phi"] = pAggreg10C.Phi;
    } else if(Serie == "VOLTAGE_PHASE") {
       Result["Voltage_Phase"] = pAggreg10C.Voltage_Phase;
    } else if(Serie == "VOLTAGE_HARMONICS") {
      Result["Voltage_Harmonics"] = json::array();
      for(unsigned n = 0; n < pAggreg10C.Voltage_Harmonics.size(); n++) {
        json Ch = json::array();
        for(unsigned i = 0; i < pAggreg10C.Voltage_Harmonics[n].size(); i++) {
          json Row = json::array();
          complex<float> Val = pVoltageFFT[n]->Output[10 + i * 10] * (float)M_SQRT2;
          Row.push_back(Val.real());
          Row.push_back(Val.imag());
          Ch.push_back(Row);
        }
        Result["Voltage_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "CURRENT_HARMONICS") {
      Result["Current_Harmonics"] = json::array();
      for(unsigned n = 0; n < pAggreg10C.Current_Harmonics.size(); n++) {
        json Ch = json::array();
        for(unsigned i = 0; i < pAggreg10C.Current_Harmonics[n].size(); i++) {
          json Row = json::array();
          complex<float> Val = pCurrentFFT[n]->Output[10 + i * 10] * (float)M_SQRT2;
          Row.push_back(Val.real());
          Row.push_back(Val.imag());
          Ch.push_back(Row);
        }
        Result["Current_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "POWER_HARMONICS") {
      Result["Power_Harmonics"] = json::array();
      for(uint n = 0; n < pAggreg10C.Power_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < pAggreg10C.Power_Harmonics[n].size(); i++) Ch.push_back(pAggreg10C.Power_Harmonics[n][i]);
        Result["Power_Harmonics"].push_back(Ch);        
      }
    } else if(Serie == "VOLTAGE_FFT") {
      Result["Voltage_FFT"] = json::array();
      for(uint i = 0; i < pVoltageChannels; i++) {
        json Ch = json::array();
        for(uint n = 0; n < pVoltageFFT[i]->Output.size(); n++) Ch.push_back(pVoltageFFT[i]->BinPower(n));
        Result["Voltage_FFT"].push_back(Ch);
      }
    } else if(Serie == "CURRENT_FFT") {
      Result["Current_FFT"] = json::array();
      for(uint i = 0; i < pCurrentChannels; i++) {
        json Ch = json::array();
        for(uint n = 0; n < pCurrentFFT[i]->Output.size(); n++) Ch.push_back(pCurrentFFT[i]->BinPower(n));
        Result["Current_FFT"].push_back(Ch);
      }
    } else if(Serie == "VOLTAGE_SAMPLES") {
      Result["Voltage_Samples"] = pVoltageSamples;
    } else if(Serie == "CURRENT_SAMPLES") {
      Result["Current_Samples"] = pCurrentSamples;
    } else if(Serie == "SYS_STATS") {
      Result["Sys_Stats"] = SysSupport::GetSystemDetails();
    } else {
      pNowDataLock.ConsumerEnds();
      return req.FromString("Unknown serie name", MHD_HTTP_BAD_REQUEST);
    }
  }
  pNowDataLock.ConsumerEnds();
  req.FromJSON(Result);
}
void AnalyzerSoft::GetSeriesRange(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  string Aggreg = Params["Aggreg"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial) + ", 'public'");
  req.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("ranges"), MHD_HTTP_OK);
}
void AnalyzerSoft::GetEvents(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->GetEvents(req);
  } else {
    req.FromString("Events detection disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetEvent(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->GetEvent(req);
  } else {
    req.FromString("Events detection disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::SetEvent(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->SetEvent(req);
  } else {
    req.FromString("Events detection disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::DelEvent(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->DelEvent(req);
  } else {
    req.FromString("Events detection disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetRates(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->GetRates(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetRate(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->GetRate(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();  
}
void AnalyzerSoft::SetRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->SetRate(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::DelRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->DelRate(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();  
}
void AnalyzerSoft::AddRate(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->AddRate(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();    
}
void AnalyzerSoft::GetPrices(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pRates != NULL) {
    pRates->GetPrices(req);
  } else {
    req.FromString("Rates disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::TestAlarm(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->TestAlarm(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetAlarms(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->GetAlarms(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();  
}
void AnalyzerSoft::GetAlarm(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->GetAlarm(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::SetAlarm(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->SetAlarm(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::AddAlarm(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->AddAlarm(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::DelAlarm(HTTPRequest &req) {
  if(CheckRights(req, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->DelAlarm(req);
  } else {
    req.FromString("Alarms disabled", MHD_HTTP_SERVICE_UNAVAILABLE);
  }
  pMembersLocks.ReadUnlock();
}
