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
#include "AnalyzerSoft.h"
#include "Tools.h"
#include "Support/SysSupport.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
vector<float> AnalyzerSoft::FrequencyRanges  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
vector<float> AnalyzerSoft::VoltageRanges    = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
vector<float> AnalyzerSoft::THDRanges        = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
vector<float> AnalyzerSoft::HarmonicsAllowed = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), Database(id) {
  LOGFile::Debug("Creating AnalyzerSoft database tables\n");
  ExecResource("SQL/Database_AnalyzerSoft.sql");
  pConfigPath      = optionPath;
  pMode            = mode;
  pSamplerate      = samplerate;
  pThread          = 0;
  pEventThread     = 0;
  pCycleLen        = 0;
  pSequence        = 0;
  pTerminate       = false;
  pPrevTimestamp   = 0;
  pPrevStationary  = false;
  pCurrentSamples  = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(0.0));
  pVoltageSamples  = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(0.0));
  pBlockCycles.reserve(20);
  pLastSamples.resize((pMode == PHASE1) ? 1 : 3);  
  InitAggreg(pAggreg10C, AGGREG_10C);
  InitAggreg(pAggreg3S, AGGREG_3S);
  InitAggreg(pAggreg1M, AGGREG_1M);
  InitAggreg(pAggreg10M, AGGREG_10M);
  InitAggreg(pAggreg15M, AGGREG_15M);
  InitAggreg(pAggreg1H, AGGREG_1H);
  for(uint8_t i = 0; i < pVoltageSamples.size(); i++) pVoltageFFT.push_back(new FFT("FFT_V" + to_string(i)));
  for(uint8_t i = 0; i < pCurrentSamples.size(); i++) pCurrentFFT.push_back(new FFT("FFT_I" + to_string(i)));
  Configure(ConfigManager::ReadConfig(optionPath));
  if(pthread_create(&pEventThread, NULL, Events_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pEventThread = 0;
    return;
  }
  if(pthread_create(&pThread, NULL, Analyzer_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pThread = 0;
    return;
  }
  pthread_setname_np(pEventThread, "EventsDet");
  pthread_setname_np(pThread, (pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N");  
}
AnalyzerSoft::~AnalyzerSoft() {
  WaitInstances();
  pTerminate = true;
  Abort(&pThread);
  while(pVoltageFFT.size() > 0) { 
    FFT *Tmp = pVoltageFFT.back();
    pVoltageFFT.pop_back();
    delete Tmp;
  }
  while(pCurrentFFT.size() > 0) { 
    FFT *Tmp = pCurrentFFT.back();
    pCurrentFFT.pop_back();
    delete Tmp;
  }
  pEventLocks.Abort(&pEventThread);
}
void AnalyzerSoft::Configure(const json &options) {
  pConfigLocks.WriteLock();
  pName             = options["Name"];
  pNominalVoltage   = options["NominalVoltage"];
  pNominalFrequency = options["NominalFreq"];
  pMaxCurrent       = options["MaxCurrent"];
  pMinCurrent       = options["MinCurrent"];
  pTimeZone         = options["TimeZone"];
  pContract         = options["Contract"];
  pZipCode          = options["ZipCode"];
  pGeo              = options["GeoPosition"].get<vector<float> >();
  pMaxSamples       = ceil((15.0 * pSamplerate) / pNominalFrequency);
  pPrevSamples.resize(0);
  pPrevCycles.resize(0);
  //Calc peak filter coefficients for nominal frequency
  pFilters.clear();
  for(uint n = 0; n < ZEROCROSSING_FILTERSTAGES; n++) {
    float w0 = 2 * M_PI * (pNominalFrequency / pSamplerate);
    float alpha = sin(w0) / 2.0;
    Filter_t Stage;
    Stage.A0 = alpha / (1.0 + alpha);
    Stage.A1 = 0.0;
    Stage.A2 = -alpha / (1.0 + alpha);
    Stage.B1 = (-2.0 * cos(w0)) / (1.0 + alpha);
    Stage.B2 = (1.0 - alpha) / (1.0 + alpha);
    Stage.ChannelsZ1 = vector<float>((pMode == PHASE1) ? 1 : 3, 0.0);
    Stage.ChannelsZ2 = vector<float>((pMode == PHASE1) ? 1 : 3, 0.0);
    pFilters.push_back(Stage);
  }
  uint32_t PhaseShift = (pSamplerate * ZEROCROSSING_FILTERSTAGES) / (pNominalFrequency * 4.0);
  pDelayVoltage.resize(PhaseShift, vector<float>(pVoltageSamples.size(), 0.0));
  pDelayCurrent.resize(PhaseShift, vector<float>(pCurrentSamples.size(), 0.0));
  pConfigLocks.WriteUnlock();
  string Server = options["RemoteServer"];
  string User = options["RemoteUser"];
  string Pass = options["RemotePass"];
  bool   Enabled = options["RemoteEnabled"];
  SetSync(Enabled, Server, User, Pass);
}
bool AnalyzerSoft::BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime) {
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  pConfigLocks.ReadLock();
  vector<float> Voltage = pDelayVoltage.front();
  vector<float> Current = pDelayCurrent.front();
  for(uint8_t i = 0; i < pVoltageSamples.size(); i++) pVoltageSamples[i].push_back(Voltage[i]);
  for(uint8_t i = 0; i < pCurrentSamples.size(); i++) pCurrentSamples[i].push_back(Current[i]);
  pDelayVoltage.pop_front();
  pDelayCurrent.pop_front();
  pDelayVoltage.push_back(voltageSamples);
  pDelayCurrent.push_back(currentSamples);
  vector<float> Outs = voltageSamples;
  for(uint8_t i = 0; i < voltageSamples.size(); i++) {
    for(uint j = 0; j < pFilters.size(); j++) {
      float Out = Outs[i] * pFilters[j].A0 + pFilters[j].ChannelsZ1[i];
      pFilters[j].ChannelsZ1[i] = Outs[i] * pFilters[j].A1 + pFilters[j].ChannelsZ2[i] - pFilters[j].B1 * Out;
      pFilters[j].ChannelsZ2[i] = Outs[i] * pFilters[j].A2 - pFilters[j].B2 * Out;
      Outs[i] = Out;
    }
  }
  pConfigLocks.ReadUnlock();
  if(pBlockCycles.size() == pBlockCycles.capacity()) pBlockCycles.clear();
  pCycleLen++;
  if(((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) {
    pBlockCycles.push_back(pCycleLen);
    pCycles.push_back(pCycleLen);
    if(pCycles.size() > (ZEROCROSSING_SECONDS * pNominalFrequency)) pCycles.pop_front();
    pCycleLen = 0;
  }
  pLastSamples = Outs;  
  if((pBlockCycles.size() == pBlockCycles.capacity()) || (pVoltageSamples[0].size() == pMaxSamples)) {
    WakeWorker();
    return true;
  }
  return false;
}
bool AnalyzerSoft::IsFinished() {
  return IsWorking();
}
void AnalyzerSoft::InitAggreg(Params_t &params, AggregType type) const {
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
void AnalyzerSoft::AccumulateAggreg(Params_t &dst, const Params_t &src) const {
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
void AnalyzerSoft::NormalizeAggreg(Params_t &params) const {
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
float AnalyzerSoft::AccumulateSUM(const float val1, const float val2) const {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return val2;
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + val2;
}
float AnalyzerSoft::AccumulateRMS(const float val1, const float val2) const {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return pow(val2, 2.0);
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + pow(val2, 2.0);
}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  if(params.Type == AGGREG_3S)
    ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" + Bind(params.Timestamp) + ", " + Bind(params.Voltage) + ", " + Bind(params.Current) + ", " + Bind(params.Frequency) + ", " + Bind(params.Flag) + ", " + Bind(params.Active_Power) + ", " + Bind(params.Reactive_Power) + ")");
  else if(params.Type == AGGREG_1M)
    ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + Bind(params.Timestamp) + ", " + Bind(params.Voltage) + ", " + Bind(params.Current) + ", " + Bind(params.Frequency) + ", " + Bind(params.Flag) + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) SELECT sampletime, " + Bind(params.Active_Power) + ", " + Bind(params.Reactive_Power) + ", " + Bind(params.Power_Factor) + ", " + Bind(params.Voltage_THD) + ", " + Bind(params.Current_THD) + ", " + Bind(params.Phi) + ", " + Bind(params.Voltage_Harmonics) + ", " + Bind(params.Current_Harmonics) + " FROM Sample");
  else if(params.Type == AGGREG_10M)
    ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + Bind(params.Timestamp) + ", " + Bind(params.Voltage) + ", " + Bind(params.Current) + ", " + Bind(params.Frequency) + ", " + Bind(params.Flag) + ")");
  else if(params.Type == AGGREG_15M)
    ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" + Bind(params.Timestamp) + ", " + Bind(params.Active_Power) + ", " + Bind(params.Reactive_Power) + ")");
  else if(params.Type == AGGREG_1H)
    ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES(" + Bind(params.Timestamp) + ", " + Bind(params.Voltage) + ", " + Bind(params.Current) + ", " + Bind(params.Frequency) + ", " + Bind(params.Flag) + ", " + Bind(params.Active_Power) + ", " + Bind(params.Reactive_Power) + ", " + Bind(params.StatsVoltage) + ", " + Bind(params.StatsFrequency) + ", " + Bind(params.StatsTHD) + ", " + Bind(params.StatsHarmonics) + ")");
  if(params.Type == AGGREG_1M) ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + Bind(params.Timestamp - 86400000));   //Delete 1 day old
  //TEST
  json Result;
  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    Result["Voltage"] = params.Voltage;
    Result["Current"] = params.Current;
    Result["Frequency"] = params.Frequency;
    Result["Flag"] = params.Flag;
  }
  if((params.Type == AGGREG_3S) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_1H)) {
    Result["Active_Power"] = params.Active_Power;
    Result["Reactive_Power"] = params.Reactive_Power;
  }
  if(params.Type == AGGREG_1M) {
    Result["Power_Factor"] = params.Power_Factor;
    Result["Voltage_THD"] = params.Voltage_THD;
    Result["Current_THD"] = params.Current_THD;
    Result["Phi"] = params.Phi;
//    Result["Voltage_Harmonics"] = params.Voltage_Harmonics;
//    Result["Current_Harmonics"] = params.Current_Harmonics;
  }
  Result["Aggregation"] = (params.Type == AGGREG_3S) ? "3S" : (params.Type == AGGREG_1M) ? "1M" : (params.Type == AGGREG_10M) ? "10M" : (params.Type == AGGREG_15M) ? "15M": "1H";
  Result["Sampletime"] = params.Timestamp;
  Tools::SendUDP(Result);
  pLastSave = Tools::GetTime();
}
void AnalyzerSoft::SaveEvent(Event_t &evt) {
  string Type = (evt.Type == EVT_RVC) ? "RVC" : (evt.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (evt.Type == EVT_DIP) ? "DIP" : "SWELL";
  vector<vector<float> > RMS = vector<vector< float> >(evt.Reference.size(), vector<float>());
  vector<vector<float> > RAW = vector<vector< float> >(evt.Reference.size(), vector<float>());
  for(deque<vector<float> >::iterator it = evt.SamplesRMS_A.begin(); it != evt.SamplesRMS_A.end(); it++) {
    for(uint8_t ch = 0; ch < it->size(); ch++) RMS[ch].push_back((*it)[ch]);
  }
  for(deque<vector<float> >::iterator it = evt.SamplesRMS_B.begin(); it != evt.SamplesRMS_B.end(); it++) {
    for(uint8_t ch = 0; ch < it->size(); ch++) RMS[ch].push_back((*it)[ch]);
  }
  for(deque<vector<float> >::iterator it = evt.SamplesRAW_A.begin(); it != evt.SamplesRAW_A.end(); it++) {
    for(uint8_t ch = 0; ch < it->size(); ch++) RAW[ch].push_back((*it)[ch]);
  }
  for(deque<vector<float> >::iterator it = evt.SamplesRAW_B.begin(); it != evt.SamplesRAW_B.end(); it++) {
    for(uint8_t ch = 0; ch < it->size(); ch++) RAW[ch].push_back((*it)[ch]);
  }
  if(evt.State == EVT_RUNNING) {
    ExecSentenceNoResult("INSERT INTO events(starttime, duration, samplerate, finished, evttype, reference, peak, change, notes, samples1, rms1) VALUES (" + Bind(evt.Start) + ", 0, " + Bind(pSamplerate) + ", false, " + Bind(Type) + ", " + Bind(evt.Reference) + ", " + Bind(evt.PeakValue) + ", " + Bind(evt.ChangeValue) + ", '', " + Bind(RAW) + ", " + Bind(RMS) + ")");
  } else if(evt.State == EVT_CLOSING) {
    ExecSentenceNoResult("UPDATE events SET duration = " + Bind(evt.Duration) + ", peak = " + Bind(evt.PeakValue) + ", change = " + Bind(evt.ChangeValue) + ", samples2 = " + Bind(RAW) + ", rms2 = " + Bind(RMS) + " WHERE starttime = " + Bind(evt.Start) + " AND evttype = " + Bind(Type));
  } else {
    ExecSentenceNoResult("UPDATE events SET finished = TRUE, samples3 = " + Bind(RAW) + ", rms3 = " + Bind(RMS) + " WHERE starttime = " + Bind(evt.Start) + " AND evttype = " + Bind(Type));
  }
  evt.SamplesRAW_A = deque<vector<float> >();
  evt.SamplesRMS_A = deque<vector<float> >();
  evt.SamplesRAW_B = deque<vector<float> >();
  evt.SamplesRMS_B = deque<vector<float> >();
}
float AnalyzerSoft::CalcFrequency() {
  uint32_t Samples = 0;
  uint32_t MinCycle = pSamplerate / (pNominalFrequency * 1.15 * 2.0);
  uint32_t MaxCycle = pSamplerate / (pNominalFrequency * 0.85 * 2.0);
  for(uint i = 0; i < pCycles.size(); i++) {
    if((pCycles[i] < MinCycle) || (pCycles[i] > MaxCycle)) return NAN;
    Samples += pCycles[i];
  }
  return pCycles.size() / ((2.0 * Samples) / pSamplerate);
}
float AnalyzerSoft::CalcChange(const float in, const float reference) const {
  return ((in - reference) / reference) * 100;
}
void AnalyzerSoft::InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos) {
  Event_t Event;
  Event.Type = type;
  Event.Duration = 0;
  Event.State = EVT_RUNNING;
  Event.Reference = reference;
  Event.PeakValue = reference;
  Event.ChangeValue = reference;
  Event.SamplesRAW_A = deque<vector<float> >();
  Event.SamplesRMS_A = deque<vector<float> >();
  Event.SamplesRAW_B = deque<vector<float> >();
  Event.SamplesRMS_B = deque<vector<float> >();
  for(deque<EventCycle_t>::reverse_iterator it = pPrevCycles.rbegin(); it != pPrevCycles.rend(); it++) {
    Event.SamplesRMS_A.push_front(it->RMS);
    if(Event.SamplesRMS_A.size() == (pNominalFrequency * 2 * EVENT_RMSENVELOPETIME)) break;
  }
  for(deque<vector<float> >::reverse_iterator it = pPrevSamples.rbegin(); it != pPrevSamples.rend(); it++) {
    Event.SamplesRAW_A.push_front(*it);
    if(Event.SamplesRAW_A.size() == (EVENT_RAWENVELOPETIME * pSamplerate)) break;
  }
  //A new event destroy any running RVC
  if(type != EVT_RVC) {
    for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); ) {
      if((it->Type == EVT_RVC) && (it->State == EVT_RUNNING)) {
        ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + Bind(it->Start) + " AND evttype = 'RVC'");
        pEvents.erase(it++);
      } else {
        it++;
      }
    }
  }
  Event.Start = pAggreg10C.Timestamp + (uint32_t)((pos * 1000) / pSamplerate);
  SaveEvent(Event);
  pEvents.push_back(Event);
}
bool AnalyzerSoft::FindRunningEvent(const uint32_t mask) {
  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
  }
  return false;
}
void *AnalyzerSoft::Analyzer_Thread(void *args) {
  AnalyzerSoft *This = (AnalyzerSoft*)args;
  while(This->pTerminate == false) {
    if(This->WorkerWaitTask() == false) break;
    uint64_t Start = Tools::GetTime();
    //Init FFTs and event detection threads
    This->pEventLocks.WakeWorker();
    for(uint i = 0; i < This->pVoltageFFT.size(); i++) This->pVoltageFFT[i]->Start(This->pVoltageSamples[i]);
    for(uint i = 0; i < This->pCurrentFFT.size(); i++) This->pCurrentFFT[i]->Start(This->pCurrentSamples[i]);
    This->pAggreg10C.Frequency = This->CalcFrequency();
    if(!isnan(This->pAggreg10C.Frequency)) This->pAggreg10C.Frequency_Inputs = 1;
    bool Valid = false;
    vector<float> RMS = vector<float>(This->pAggreg10C.Voltage.size(), 0);
    for(uint j = 0; j < This->pAggreg10C.Voltage.size(); j++) {
      for(uint32_t i = 0; i < This->pVoltageSamples[j].size(); i++) RMS[j] += pow(This->pVoltageSamples[j][i], 2.0);
      RMS[j] = sqrt(RMS[j] / This->pVoltageSamples[j].size());
      if(RMS[j] > 1) Valid = true;
    }
    if(Valid) This->pAggreg10C.Voltage.assign(RMS.begin(), RMS.end());
    if(Valid) This->pAggreg10C.Voltage_Inputs = 1;
    for(uint j = 0; j < This->pAggreg10C.Current.size(); j++) {
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
    for(uint j = 0; j < This->pAggreg10C.Voltage.size(); j++) {
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
      This->pAggreg10C.Voltage_THD[j] = This->pAggreg10C.Voltage_THD[j] / This->pAggreg10C.Voltage_Harmonics[j][0];
    }
    for(int j = 0; j < ((This->pMode == PHASE1) ? 1 : 3); j++) {
      This->pCurrentFFT[j]->Wait();
      if(isnan(This->pAggreg10C.Current[j]) || isnan(This->pAggreg10C.Voltage[j])) continue;
      This->pAggreg10C.Apparent_Power[j] = This->pAggreg10C.Voltage[j] * This->pAggreg10C.Current[j];
      for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[j].size(); i++) {
        This->pAggreg10C.Current_Harmonics[j][i] = This->pCurrentFFT[j]->BinPower(10 + i * 10);
        if(i > 0) This->pAggreg10C.Current_THD[j] = This->AccumulateSUM(This->pAggreg10C.Current_THD[j], This->pAggreg10C.Current_Harmonics[j][i]);
      }
      float PHI = FFT::CalcAngle(This->pCurrentFFT[j]->Output[10], This->pVoltageFFT[j]->Output[10]);
      This->pAggreg10C.Reactive_Power[j] = This->pAggreg10C.Voltage_Harmonics[j][0] * This->pAggreg10C.Current_Harmonics[j][0] * sin(-PHI);
      This->pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
      This->pAggreg10C.Current_THD[j] = This->pAggreg10C.Current_THD[j] / This->pAggreg10C.Current_Harmonics[j][0];
      This->pAggreg10C.Active_Energy[j] = (This->pAggreg10C.Active_Power[j] * This->pCurrentSamples[j].size()) / (This->pSamplerate * 3600.0);
      This->pAggreg10C.Power_Factor[j] = This->pAggreg10C.Active_Power[j] / This->pAggreg10C.Apparent_Power[j];
      This->pAggreg10C.Reactive_Energy[j] = (This->pAggreg10C.Reactive_Power[j] * This->pAggreg10C.Current_Inputs[j]) / (This->pSamplerate * 3600.0);
    }
    if(This->pMode == PHASE3N) {
      This->pCurrentFFT[3]->Wait();
      if(!isnan(This->pAggreg10C.Current[3])) {
        for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[3].size(); i++) {
          This->pAggreg10C.Current_Harmonics[3][i] = This->pCurrentFFT[3]->BinPower(10 + i * 10);
          if(i > 0) This->pAggreg10C.Current_THD[3] = This->AccumulateSUM(This->pAggreg10C.Current_THD[3], norm(This->pAggreg10C.Current_Harmonics[3][i]));
        }
        This->pAggreg10C.Current_THD[3] = sqrt(This->pAggreg10C.Current_THD[3]) / sqrt(norm(This->pAggreg10C.Current_Harmonics[3][0]));
      }
    }
    This->pEventLocks.WaitWorker();
    //Save results
    This->PNowDataLock.ProviderWrite();
    if(((This->pSequence % 15) == 0) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 3S aggregation\n");
      This->NormalizeAggreg(This->pAggreg3S);
      if(This->pAggreg3S.Flag == false) {
        float Variation = This->CalcChange(This->pAggreg3S.Frequency, This->pNominalFrequency);
        for(uint Range = 0; Range < 40; Range++) {
          if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) This->pAggreg1H.StatsFrequency[Range]++;
        }
      }
      This->SaveAggreg(This->pAggreg3S);
      This->InitAggreg(This->pAggreg3S, AGGREG_3S);
    }
    This->AccumulateAggreg(This->pAggreg3S, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 60000) > (This->pPrevTimestamp / 60000)) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1M aggregation\n");
      This->pAggreg1M.Timestamp = This->pAggreg1M.Timestamp / 60000 * 60000 + 59999;
      This->NormalizeAggreg(This->pAggreg1M);
      This->SaveAggreg(This->pAggreg1M);
      This->InitAggreg(This->pAggreg1M, AGGREG_1M);
    }
    This->AccumulateAggreg(This->pAggreg1M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 600000) > (This->pPrevTimestamp / 600000)) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 10M aggregation\n");
      This->pAggreg10M.Timestamp = This->pAggreg10M.Timestamp / 600000 * 600000 + 599999;
      This->NormalizeAggreg(This->pAggreg10M);
      if(This->pAggreg10M.Flag == false) {
        for(uint i = 0; i < This->pAggreg10M.Voltage.size(); i++) {
          float Variation = This->CalcChange(This->pAggreg10M.Voltage[i], This->pNominalVoltage);
          for(uint Range = 0; Range < (VoltageRanges.size() - 1); Range++) {
            if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) This->pAggreg1H.StatsVoltage[Range]++;
          }
          for(uint Range = 0; Range < (THDRanges.size() - 1); Range++) {
            if((This->pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (This->pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) This->pAggreg1H.StatsTHD[Range]++;
          }
          bool HarmonicsInRange = true;
          for(uint n = 0; n < HarmonicsAllowed.size(); n++) {
            float Variation = This->CalcChange(sqrt(norm(This->pAggreg10M.Voltage_Harmonics[i][n + 1])), This->pNominalVoltage);
            if(Variation > HarmonicsAllowed[n]) {
              HarmonicsInRange = false;
              break;
             }
          }
          if(HarmonicsInRange == true) {
            This->pAggreg1H.StatsHarmonics[0]++;
          } else {
            This->pAggreg1H.StatsHarmonics[1]++;
          }
        }
      }
      This->SaveAggreg(This->pAggreg10M);
      This->InitAggreg(This->pAggreg10M, AGGREG_10M);
    }
    This->AccumulateAggreg(This->pAggreg10M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 900000) > (This->pPrevTimestamp / 900000)) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 15M aggregation\n");
      This->pAggreg15M.Timestamp = This->pAggreg15M.Timestamp / 900000 * 900000 + 899999;
      This->NormalizeAggreg(This->pAggreg15M);
      This->SaveAggreg(This->pAggreg15M);
      This->InitAggreg(This->pAggreg15M, AGGREG_15M);
    }
    This->AccumulateAggreg(This->pAggreg15M, This->pAggreg10C);
    if(((This->pAggreg10C.Timestamp / 3600000) > (This->pPrevTimestamp / 3600000)) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1H aggregation\n");
      This->pAggreg1H.Timestamp = This->pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      This->NormalizeAggreg(This->pAggreg1H);
      This->SaveAggreg(This->pAggreg1H);
      This->InitAggreg(This->pAggreg1H, AGGREG_1H);
    }
    This->AccumulateAggreg(This->pAggreg1H, This->pAggreg10C);
    This->pPrevTimestamp = This->pAggreg10C.Timestamp;
    This->PNowDataLock.ProviderWait();
    This->InitAggreg(This->pAggreg10C, AGGREG_10C);
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", This->GetSchema().c_str(), Tools::GetTime() - Start, This->pSequence);
    This->pCurrentSamples = vector<vector<float> >((This->pMode == PHASE1) ? 1 : (This->pMode == PHASE3) ? 3 : 4, vector<float>(0));
    This->pVoltageSamples = vector<vector<float> >((This->pMode == PHASE1) ? 1 : 3, vector<float>(0));
    This->pSequence++;
    This->WorkerEndsTask();
  }
  return NULL;
}
void *AnalyzerSoft::Events_Thread(void* args) {
  AnalyzerSoft *This = (AnalyzerSoft*)args;
  while(This->pTerminate == false) {
    if(This->pEventLocks.WorkerWaitTask() == false) break;
    This->pConfigLocks.ReadLock();
    uint32_t RequiredRAW = This->pSamplerate * EVENT_RAWENVELOPETIME;
    uint32_t RequiredRMS = This->pNominalFrequency * 2 * EVENT_RMSENVELOPETIME;
    uint32_t Pos = 0;
    This->pAggreg10C.Flag = false;
    for(uint32_t i = 0; i < This->pBlockCycles.size(); i++) {
      //Calc cycle stats
      EventCycle_t Cycle;
      uint32_t CycleLen = This->pBlockCycles[i];
      for(uint n = 0; n < This->pVoltageSamples.size(); n++) {
        float RMS = 0.0;
        for(uint32_t j = 0; j < CycleLen; j++) RMS += pow(This->pVoltageSamples[n][Pos + j], 2);
        Cycle.RMS.push_back(sqrt(RMS / CycleLen));
      }
      //Calc last 100 cycles AVG (current and previous ones)
      vector<float> RMS_Avg = Cycle.RMS;
      for(uint8_t n = 0; n < Cycle.RMS.size(); n++) {
        uint32_t Len = This->pPrevCycles.size();
        if(Len > 99) Len = 99;
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        for(uint32_t j = 0; j < Len; j++, it++) RMS_Avg[n] += it->RMS[n];
        RMS_Avg[n] = RMS_Avg[n] / (Len + 1);
      }
      //Check steady state
      float Tolerance = EVENT_RVC - ((This->pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
      bool  Stationary = (This->pPrevCycles.size() >= 99);
      for(uint8_t n = 0; (n < RMS_Avg.size()) && (Stationary == true); n++) {
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        if(fabs(This->CalcChange(Cycle.RMS[n], RMS_Avg[n])) > Tolerance) {
          Stationary = false;
          break;
        }
        for(int j = 0; j < 99; j++) {
          if(fabs(This->CalcChange(it->RMS[n], RMS_Avg[n])) > Tolerance) {
            Stationary = false;
            break;
          }
        }
      }
      if(Stationary != This->pPrevStationary) LOGFile::Debug("Analyzer '%s' %s stationary state\n", This->pSerial.c_str(), (Stationary ? "reach" : "leave"));
      //Check for new events
      if((This->pPrevSamples.size() >= RequiredRAW) && (This->pPrevCycles.size() >= RequiredRMS)) {
        for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
          //Check for interruption new events
          if((-This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) This->InitEvent(EVT_INTERRUPTION, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
          //Check for DIP new events
          if((-This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP) == false)) This->InitEvent(EVT_DIP, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
          //Check for SWELL new events
          if((This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) This->InitEvent(EVT_SWELL, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
          //Check for RVC new events
          if((Stationary == false) && (This->pPrevStationary == true) && (This->FindRunningEvent(EVT_RVC | EVT_DIP | EVT_SWELL | EVT_INTERRUPTION) == false)) This->InitEvent(EVT_RVC, RMS_Avg, Pos);
        }
      }
      //Check events state
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); ) {
        //If closed evend finish post event period save it and remove from list
        if((it->State == EVT_CLOSING) && (it->SamplesRAW_A.size() == RequiredRAW) && (it->SamplesRMS_A.size() == RequiredRMS)) {
          it->State = EVT_CLOSED;
          This->SaveEvent(*it);
          This->pEvents.erase(it++);
        } else if(it->State == EVT_CLOSING) {
          it++;
        } else {
          //Check if RVC event can be closed, or update values
          if(it->Type == EVT_RVC) {
            if(Stationary == true) {
              it->State = EVT_CLOSING;
              it->ChangeValue = RMS_Avg;
            } else {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(fabs(it->Reference[ch] - Cycle.RMS[ch]) > fabs(it->Reference[ch] - it->PeakValue[ch])) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }
          //Check if INTERRUPTION event can be closed
          if(it->Type == EVT_INTERRUPTION) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
              if(-This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(Cycle.RMS[ch] < it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }
          //Check if DIP event can be closed
          if(it->Type == EVT_DIP) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
              if(-This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(Cycle.RMS[ch] < it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }
          //Check if SWELL event can be closed
          if(it->Type == EVT_SWELL) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
              if(This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(Cycle.RMS[ch] > it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }          
          if(it->State == EVT_RUNNING) {
            it->Duration += CycleLen;
            if(it->Type != EVT_RVC) This->pAggreg10C.Flag = true;
          } else {
            This->SaveEvent(*it);
          }
          it++;
        }
      }      
      //Insert current cycle samples in prev data buffers and in running events than need its
      for(uint n = 0; n < CycleLen; n++) {
        vector<float> Sample;
        for(uint j = 0; j < This->pVoltageSamples.size(); j++) Sample.push_back(This->pVoltageSamples[j][n + Pos]);
        This->pPrevSamples.push_back(Sample);
        if(This->pPrevSamples.size() > RequiredRAW) This->pPrevSamples.pop_front();
        for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
          if(it->SamplesRAW_A.size() < RequiredRAW) {
            it->SamplesRAW_A.push_back(Sample);
          } else if(it->State == EVT_RUNNING) {
            it->SamplesRAW_B.push_back(Sample);
            if(it->SamplesRAW_B.size() > RequiredRAW) it->SamplesRAW_B.pop_front();
          }
        }
      }
      //Insert current cycle in prev data buffers and in running events than need its
      This->pPrevCycles.push_back(Cycle);
      if(This->pPrevCycles.size() > RequiredRMS) This->pPrevCycles.pop_front();
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
        if(it->SamplesRMS_A.size() < RequiredRMS) {
          it->SamplesRMS_A.push_back(Cycle.RMS);
        } else if(it->State == EVT_RUNNING) {
          it->SamplesRMS_B.push_back(Cycle.RMS);
          if(it->SamplesRMS_B.size() > RequiredRMS) it->SamplesRMS_B.pop_front();
        }
      }      
      Pos = Pos + CycleLen;
      This->pPrevStationary = Stationary;
    }
    This->pConfigLocks.ReadUnlock();
    This->pEventLocks.WorkerEndsTask();
  }
  return NULL;
}
json AnalyzerSoft::CheckConfig(const json &config) {
  json ConfigOUT = config;
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
  return ConfigOUT;
}
void AnalyzerSoft::GetAnalyzer(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Response = json::object();
  pConfigLocks.ReadLock();
  Response["Analyzer"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["NominalVoltage"] = pNominalVoltage;
  Response["NominalFreq"] = pNominalFrequency;
  Response["MaxCurrent"] = pMaxCurrent;
  Response["MinCurrent"] = pMinCurrent;
  Response["TimeZone"] = pTimeZone;
  Response["Contract"] = pContract;
  Response["ZipCode"] = pZipCode;
  Response["GeoPosition"] = pGeo;
  Response["SampleRate"] = pSamplerate;
  req.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename))) AS size from pg_tables WHERE schemaname = " + req.Bind(pSerial));
  if(req.FecthRow()) Response["Size"] = req.ResultUInt64("size");
  json Remote = GetJSONSyncConfig();  
  Response.insert(Remote.begin(), Remote.end());
  pConfigLocks.ReadUnlock();
  req.FromJSON(Response);
  Dereference();
}
void AnalyzerSoft::SetAnalyzer(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, CONFIG) == false) {
    Dereference();
    return;
  }
  json Config = CheckConfig(req.GetParams());
  ConfigManager::WriteConfig(pConfigPath, Config);
  Configure(Config);
  json Result = json::object();
  Result["Result"] = true;
  req.FromJSON(Result);
  Dereference();
}
void AnalyzerSoft::GetSeries(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  //Insert defaults
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 3000;
  string Aggreg = Params["Aggreg"];
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    Dereference();
    return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  }
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  string Query = "SELECT row_to_json(Result) AS result FROM ( SELECT " + req.Bind(From) + " AS " + req.BindID("From") + ", " + req.Bind(To) +" AS " + req.BindID("To") + ", ";
  Query += req.Bind((To - From) / PeriodSeconds) + " AS " + req.BindID("MaxSamples") + ", " + req.Bind(Aggreg) + " AS " + req.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      Dereference();
      return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    }
    string Serie = Params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + req.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
      }
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
      }
    } else if((Serie == "POWER_FACTOR") && (Aggreg == "1M")) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && (Aggreg == "1M")) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (Aggreg == "1M")) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && (Aggreg == "1M")) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + req.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {   
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics_Stats") + ", ";
    } else {
      Dereference();
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
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::GetSeriesNow(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  if(PNowDataLock.ConsumerWait(5000) == false) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  json Result = json::object();
  Result["Sampletime"] = pAggreg10C.Timestamp;
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      Dereference();
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
      for(uint n = 0; n < pAggreg10C.Voltage_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < pAggreg10C.Voltage_Harmonics[n].size(); i++) {
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
      for(uint n = 0; n < pAggreg10C.Current_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < pAggreg10C.Current_Harmonics[n].size(); i++) {
          json Row = json::array();
          complex<float> Val = pCurrentFFT[n]->Output[10 + i * 10] * (float)M_SQRT2;
          Row.push_back(Val.real());
          Row.push_back(Val.imag());
          Ch.push_back(Row);
        }
        Result["Current_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "VOLTAGE_FFT") {
      Result["Voltage_FFT"] = json::array();
      for(uint i = 0; i < pVoltageFFT.size(); i++) {
        json Ch = json::array();
        for(uint n = 0; n < pVoltageFFT[i]->Output.size(); n++) Ch.push_back(pVoltageFFT[i]->BinPower(n));
        Result["Voltage_FFT"].push_back(Ch);
      }
    } else if(Serie == "CURRENT_FFT") {
      Result["Current_FFT"] = json::array();
      for(uint i = 0; i < pCurrentFFT.size(); i++) {
        json Ch = json::array();
        for(uint n = 0; n < pCurrentFFT[i]->Output.size(); n++) Ch.push_back(pCurrentFFT[i]->BinPower(n));
        Result["Current_FFT"].push_back(Ch);
      }
    } else if(Serie == "VOLTAGE_SAMPLES") {
      Result["Voltage_Samples"] = pVoltageSamples;
    } else if(Serie == "CURRENT_SAMPLES") {
      Result["Current_Samples"] = pCurrentSamples;
    } else if(Serie == "SYS_STATS") {
      Result["Sys_Stats"] = System::GetSystemDetails();
    } else {
      Dereference();
      return req.FromString("Unknown serie name", MHD_HTTP_BAD_REQUEST);
    }
  }
  req.FromJSON(Result);
  PNowDataLock.ConsumerEnds();
  Dereference();
}
void AnalyzerSoft::GetSeriesRange(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  string Aggreg = Params["Aggreg"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial) + ", 'public'");
  req.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("ranges"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::GetEvents(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  req.ExecSentenceAsyncResult("SELECT json_build_object('From', " + req.Bind(From) + ", 'To', " + req.Bind(To) + ", 'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), 'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                              "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), 'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), 'Finished', COALESCE(ARRAY_AGG(finished ORDER BY starttime ASC), '{}'), "
                              "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), 'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), 'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), 'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                              "FROM events WHERE starttime > " + req.Bind(LastSample) + " AND starttime >= " + req.Bind(From) + " AND starttime <= " + req.Bind(To));
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::GetEvent(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  if((Params["Samples"] != "RMS") && (Params["Samples"] != "RAW")) {
    Dereference();
    return req.FromString("'Samples' params must be 'RMS' or 'RAW'", MHD_HTTP_BAD_REQUEST);
  }
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Samples = Params["Samples"];
  if(Samples == "RAW") {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', samples1, 'Samples', samples2, 'Post_Samples', samples3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  } else {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', rms1, 'Samples', rms2, 'Post_Samples', rms3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  }
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::SetEvent(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Notes = Params["Notes"];
  bool Result = req.ExecSentenceNoResult("UPDATE events SET Notes = " + req.Bind(Notes) + " WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
  Dereference();
}
void AnalyzerSoft::DelEvent(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  bool Result = req.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
  Dereference();
}
void AnalyzerSoft::GetRates(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') as result FROM rates");
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::GetRate(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) {
    Dereference();
    return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  }
  uint32_t RateID = Params["RateID"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  req.ExecSentenceAsyncResult("SELECT json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) as result FROM rates where rate = " + req.Bind(RateID));
  if(!req.FecthRow()) {
    Dereference();
    return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  }
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
  Dereference();
}
void AnalyzerSoft::SetRate(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  if(!Params["RateID"].is_number() || !Params["Config"].is_object() || !Params["Template"].is_object() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string()) {
    Dereference();
    return req.FromString("'RateID', 'StartDate', 'EndDate', Config', 'Template' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  }
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  uint32_t RateID   = Params["RateID"];
  Tariff T(Start, End, pTimeZone);
  if(T.SetTemplate(Params["Template"]) == false) {
    Dereference();
    return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  }
  if(T.SetConfig(Params["Config"]) == false) {
    Dereference();
    return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
  }
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  bool Result = req.ExecSentenceNoResult("UPDATE rates SET name = " + req.Bind(Name) + ", template = " + req.Bind(T.GetTemplate().dump()) + ", config = " + req.Bind(T.GetConfig().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE rate = " + req.Bind(RateID));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
  Dereference();
}
void AnalyzerSoft::DelRate(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) {
    Dereference();
    return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  }
  uint32_t RateID = Params["RateID"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  bool Result = req.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + req.Bind(RateID));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
  Dereference();
}
void AnalyzerSoft::AddRate(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  json Params = req.GetParams();
  if(!Params["Config"].is_object() || !Params["Template"].is_object() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string()) {
    Dereference();
    return req.FromString("'StartDate', 'EndDate', 'Config', 'Template' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  }
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  Tariff T(Start, End, pTimeZone);
  if(T.SetTemplate(Params["Template"]) == false) {
    Dereference();
    return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  }
  if(T.SetConfig(Params["Config"]) == false) {
    Dereference();
    return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
  }
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  bool Result = req.ExecSentenceNoResult("INSERT INTO rates(startdate, enddate, config, template, name) VALUES (" + req.Bind(Start) + ", " + req.Bind(End) + ", " + req.Bind(T.GetConfig().dump()) + ", " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Name) + ")");
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
  Dereference();
}
void AnalyzerSoft::GetPrices(HTTPRequest &req) {
  Reference();
  if(CheckRights(req, READ | WRITE | CONFIG) == false) {
    Dereference();
    return;
  }
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  uint64_t To         = Params["To"];
  uint64_t Period     = Params["Period"];
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  string   Aggreg     = Params["Aggreg"];
  uint64_t Now        = Tools::GetTime();
  if(To == 0) To = Now;
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  //TODO CHECK time zone
//  string TimeZone = "Etc/UTC";
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    Dereference();
    return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  }
  json Result;
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Power"]       = json::array();
  Result["Reactive_Power"]     = json::array();
  Result["Period_Name"]        = json::array();
  Result["Period_Color"]       = json::array();
  Result["Contracted_Power"]   = json::array();
  Result["Expense"]            = json::array();
  Result["Bills"]              = json::array();
  //Load involved tariffs, currency and timezone
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  req.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + req.Bind(From) + " AND startdate <= " + req.Bind(To));
  json DBTariffs = json::array();
  if(req.FecthRow()) {
    Result["Currency"] = "€";//req.ResultString("currency");
//    TimeZone = req.ResultString("timezone");
    DBTariffs = req.ResultJSON("rate");
  }
  TariffList Tariffs(DBTariffs, pTimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  req.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + req.Bind(BillingFrom) + " AND sampletime <= " + req.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = req.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (req.ResultUInt64("sampletime") == time)) {
      Active = req.ResultFloat("active_power");
      Reactive = req.ResultFloat("reactive_power");
      MoreData = req.FecthRow();
    }
    Tariff *Tariff = Tariffs.Find(time);
    if((Tariff != NULL) && (Tariff->PushSample(time, Active, Reactive) == true) && ((time + 900000) != BillingTo)) continue;  //If under a tariff push samples until bill cycle ends or no more data!
    //Recover/group values
    do {
      string   Color      = "";
      string   Period     = "";
      uint64_t Time       = time;
      float    Cost       = NAN;
      float    Contracted = NAN;
      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
        Result["Bills"].push_back(Tariff->GetResume(Now));
        break;
      }
      if(Time <= Now) AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
      if(Time <= Now) AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
      if(Time <= Now) AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
      if(Aggreg == "1H") {
        if((Time % 3600000) != 3599999) {
          if(Tariff != NULL) continue;
          break;
        }
        AccActive = AccActive / 4.0;
        AccReactive = AccReactive / 4.0;
      }
      if((Time > LastSample) && (Time >= From) && (Time <= To)) {
        Result["Sampletime"].push_back(Time);
        Result["Active_Power"].push_back(AccActive);
        Result["Reactive_Power"].push_back(AccReactive);
        Result["Contracted_Power"].push_back(Contracted);
        if(Color.empty() == true)
          Result["Period_Color"].push_back(nullptr);
        else 
          Result["Period_Color"].push_back(Color);
        if(Period.empty() == true)
          Result["Period_Name"].push_back(nullptr);
        else 
          Result["Period_Name"].push_back(Period);
        Result["Expense"].push_back(AccCost);
      }
      AccActive = NAN;
      AccReactive = NAN;
      AccCost = NAN;
    } while(Tariff != NULL);
  }
  req.FromJSON(Result);
  Dereference();
}