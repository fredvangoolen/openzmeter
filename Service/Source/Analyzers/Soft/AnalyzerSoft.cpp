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
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const json &option, const float samplerate) : Analyzer(id, mode), Database(id) {
   //: AnalyzerInterface(id, mode)
  //Calc peak filter coefficients for nominal frequency
  for(uint n = 0; n < ZEROCROSSING_FILTERSTAGES; n++) {
    float w0 = 2 * M_PI * (pNominalFrequency / samplerate);
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
  pEvents = new Events(this, samplerate);
  pLastSamples.resize((pMode == PHASE1) ? 1 : 3);
  pBlockCycles.reserve(20);
  pCycleLen        = 0;
  pMaxSamples      = ceil((15.0 * samplerate) / pNominalFrequency);
  pSamplerate      = samplerate;
  pSequence        = 0;
  pWorking         = false;
  pTerminate       = false;
  pWaitingThreads  = 0;
  pDataReady       = false;
  pPrevTimestamp   = 0;
  InitAggreg(pAggreg10C, AGGREG_10C);
  InitAggreg(pAggreg3S, AGGREG_3S);
  InitAggreg(pAggreg1M, AGGREG_1M);
  InitAggreg(pAggreg10M, AGGREG_10M);
  InitAggreg(pAggreg15M, AGGREG_15M);
  InitAggreg(pAggreg1H, AGGREG_1H);

  uint32_t PhaseShift = ((pNominalFrequency / samplerate) / 4.0) * ZEROCROSSING_FILTERSTAGES;
  pDelayVoltage.resize(PhaseShift, vector<float>(pAggreg10C.Voltage_Samples.size(), 0.0));
//    for(uint32_t n = 0; n < PhaseShift; n++) pDelayVoltage[i].push_back(0.0);
  pDelayCurrent.resize(PhaseShift, vector<float>(pAggreg10C.Current_Samples.size(), 0.0));
//    for(int n = 0; n < 5; n++) pDelayCurrent[i].push_back(0.0);
  
  for(uint8_t i = 0; i < pAggreg10C.Voltage_Samples.size(); i++) {
    char Name[16];
    sprintf(Name, "FFT_V%u", i);
    pVoltageFFT.push_back(new FFT(Name));
  }
  for(uint8_t i = 0; i < pAggreg10C.Current_Samples.size(); i++) {
    char Name[16];
    sprintf(Name, "FFT_I%u", i);
    pCurrentFFT.push_back(new FFT(Name));
  }
  pthread_mutex_init(&pThreadMutex, NULL);
  pthread_cond_init (&pThreadCond, NULL);
  pthread_mutex_init(&pWaitingThreadsMutex, NULL);
  pthread_mutex_init(&pDataReadyMutex, NULL);
  pthread_cond_init (&pWaitingThreadsCond, NULL);
  pthread_cond_init (&pDataReadyCond, NULL);
  if(pthread_create(&pThread, NULL, Analyzer_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, (pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N");
}
AnalyzerSoft::~AnalyzerSoft() {
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  pthread_join(pThread, NULL);
  for(uint8_t i = 0; i < pVoltageFFT.size(); i++) delete pVoltageFFT[i];
  for(uint8_t i = 0; i < pCurrentFFT.size(); i++) delete pCurrentFFT[i];
  delete pEvents;
}
float AnalyzerSoft::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
bool AnalyzerSoft::BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime) {
  pDelayVoltage.push_back(voltageSamples);
  pDelayCurrent.push_back(currentSamples);
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  vector<float> Voltage = pDelayVoltage.front();
  vector<float> Current = pDelayCurrent.front();
  for(uint8_t i = 0; i < pAggreg10C.Voltage_Samples.size(); i++) pAggreg10C.Voltage_Samples[i].push_back(Voltage[i]);
  for(uint8_t i = 0; i < pAggreg10C.Current_Samples.size(); i++) pAggreg10C.Current_Samples[i].push_back(Current[i]);
  pDelayVoltage.pop_front();
  pDelayCurrent.pop_front();
  vector<float> Outs = voltageSamples;
  for(uint8_t i = 0; i < voltageSamples.size(); i++) {
    for(uint j = 0; j < pFilters.size(); j++) {
      float Out = Outs[i] * pFilters[j].A0 + pFilters[j].ChannelsZ1[i];
      pFilters[j].ChannelsZ1[i] = Outs[i] * pFilters[j].A1 + pFilters[j].ChannelsZ2[i] - pFilters[j].B1 * Out;
      pFilters[j].ChannelsZ2[i] = Outs[i] * pFilters[j].A2 - pFilters[j].B2 * Out;
      Outs[i] = Out;
    }
  }
  if(pBlockCycles.size() == pBlockCycles.capacity()) pBlockCycles.clear();
  pCycleLen++;
  if(((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) {
    pBlockCycles.push_back(pCycleLen);
    pCycles.push_back(pCycleLen);
    if(pCycles.size() > (ZEROCROSSING_SECONDS * pNominalFrequency)) pCycles.pop_front();
    pCycleLen = 0;
  }
  pLastSamples = Outs;  
  if((pBlockCycles.size() == pBlockCycles.capacity()) || (pAggreg10C.Voltage_Samples[0].size() == pMaxSamples)) {
    pthread_mutex_lock(&pThreadMutex);
    pWorking = true;
    pthread_mutex_unlock(&pThreadMutex);
    pthread_cond_signal(&pThreadCond);
    return true;
  }
  return false;
}
void *AnalyzerSoft::Analyzer_Thread(void *args) {
  AnalyzerSoft *This = (AnalyzerSoft*)args;
  while(true) {
    pthread_mutex_lock(&This->pThreadMutex);
    while(This->pWorking == false) {
      if(This->pTerminate == true) {
        pthread_mutex_unlock(&This->pThreadMutex);
        return NULL;
      }
      pthread_cond_wait(&This->pThreadCond, &This->pThreadMutex);
    }
    uint64_t Start = Tools::GetTime();
    pthread_mutex_unlock(&This->pThreadMutex);
    //Init FFTs and event detection threads
    This->pEvents->Start(&This->pAggreg10C, &This->pBlockCycles);
    for(uint i = 0; i < This->pVoltageFFT.size(); i++) This->pVoltageFFT[i]->Start(This->pAggreg10C.Voltage_Samples[i]);
    for(uint i = 0; i < This->pCurrentFFT.size(); i++) This->pCurrentFFT[i]->Start(This->pAggreg10C.Current_Samples[i]);
    This->pAggreg10C.Frequency = This->CalcFrequency();
    if(!isnan(This->pAggreg10C.Frequency)) This->pAggreg10C.Frequency_Inputs = 1;
    bool Valid = false;
    vector<float> RMS = vector<float>(This->pAggreg10C.Voltage.size(), 0);
    for(uint j = 0; j < This->pAggreg10C.Voltage.size(); j++) {
      for(uint32_t i = 0; i < This->pAggreg10C.Voltage_Samples[j].size(); i++) RMS[j] += pow(This->pAggreg10C.Voltage_Samples[j][i], 2.0);
      RMS[j] = sqrt(RMS[j] / This->pAggreg10C.Voltage_Samples[j].size());
      if(RMS[j] > 1) Valid = true;
    }
    if(Valid) This->pAggreg10C.Voltage.assign(RMS.begin(), RMS.end());
    if(Valid) This->pAggreg10C.Voltage_Inputs = 1;
    for(uint j = 0; j < This->pAggreg10C.Current.size(); j++) {
      float RMS = 0;
      for(uint32_t i = 0; i < This->pAggreg10C.Current_Samples[j].size(); i++) RMS += pow(This->pAggreg10C.Current_Samples[j][i], 2.0);
      RMS = sqrt(RMS / This->pAggreg10C.Current_Samples[j].size());
      if(RMS > This->pMinCurrent) {
        This->pAggreg10C.Current_Inputs[j] = 1;
        This->pAggreg10C.Current[j] = RMS;
        if(j < This->pAggreg10C.Voltage_Samples.size()) {
          for(uint32_t i = 0; i < This->pAggreg10C.Voltage_Samples[j].size(); i++) This->pAggreg10C.Active_Power[j] = This->AccumulateSUM(This->pAggreg10C.Active_Power[j], This->pAggreg10C.Voltage_Samples[j][i] * This->pAggreg10C.Current_Samples[j][i]);
          This->pAggreg10C.Active_Power[j] = This->pAggreg10C.Active_Power[j] / This->pAggreg10C.Current_Samples[j].size();
        }
      }
    }
    for(uint j = 0; j < This->pAggreg10C.Voltage.size(); j++) {
      This->pVoltageFFT[j]->Wait();
      if(isnan(This->pAggreg10C.Voltage[j])) continue;
      for(uint i = 0; i < This->pAggreg10C.Voltage_Harmonics[j].size(); i++) {
        This->pAggreg10C.Voltage_Harmonics[j][i] = This->pVoltageFFT[j]->Output[10 + i * 10];
        if(i > 0) This->pAggreg10C.Voltage_THD[j] = This->AccumulateSUM(This->pAggreg10C.Voltage_THD[j], norm(This->pAggreg10C.Voltage_Harmonics[j][i]));
      }
      This->pAggreg10C.Voltage_THD[j] = sqrt(This->pAggreg10C.Voltage_THD[j]) / sqrt(norm(This->pAggreg10C.Voltage_Harmonics[j][0]));
    }
    for(int j = 0; j < ((This->pMode == PHASE1) ? 1 : 3); j++) {
      This->pCurrentFFT[j]->Wait();
      if(isnan(This->pAggreg10C.Current[j]) || isnan(This->pAggreg10C.Voltage[j])) continue;
      This->pAggreg10C.Apparent_Power[j] = This->pAggreg10C.Voltage[j] * This->pAggreg10C.Current[j];
      for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[j].size(); i++) {
        This->pAggreg10C.Current_Harmonics[j][i] = This->pCurrentFFT[j]->Output[10 + i * 10];
        if(i > 0) This->pAggreg10C.Current_THD[j] = This->AccumulateSUM(This->pAggreg10C.Current_THD[j], norm(This->pAggreg10C.Current_Harmonics[j][i]));
        if(i == 0) {
          This->pAggreg10C.Reactive_Power[j] = sqrt(norm(This->pAggreg10C.Voltage_Harmonics[j][i] * This->pAggreg10C.Current_Harmonics[j][i]));
          float PHI = FFT::CalcAngle(This->pAggreg10C.Voltage_Harmonics[j][i], This->pAggreg10C.Current_Harmonics[j][i]);
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          This->pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
      This->pAggreg10C.Current_THD[j] = sqrt(This->pAggreg10C.Current_THD[j]) / sqrt(norm(This->pAggreg10C.Current_Harmonics[j][0]));
      This->pAggreg10C.Active_Energy[j] = (This->pAggreg10C.Active_Power[j] * This->pAggreg10C.Current_Samples[j].size()) / (This->pSamplerate * 3600.0);
      This->pAggreg10C.Power_Factor[j] = This->pAggreg10C.Active_Power[j] / This->pAggreg10C.Apparent_Power[j];
      This->pAggreg10C.Reactive_Energy[j] = (This->pAggreg10C.Reactive_Power[j] * This->pAggreg10C.Current_Inputs[j]) / (This->pSamplerate * 3600.0);
    }
    if(This->pMode == PHASE3N) {
      This->pCurrentFFT[3]->Wait();
      if(!isnan(This->pAggreg10C.Current[3])) {
        for(uint i = 0; i < This->pAggreg10C.Current_Harmonics[3].size(); i++) {
          This->pAggreg10C.Current_Harmonics[3][i] = This->pCurrentFFT[3]->Output[10 + i * 10];
          if(i > 0) This->pAggreg10C.Current_THD[3] = This->AccumulateSUM(This->pAggreg10C.Current_THD[3], norm(This->pAggreg10C.Current_Harmonics[3][i]));
        }
        This->pAggreg10C.Current_THD[3] = sqrt(This->pAggreg10C.Current_THD[3]) / sqrt(norm(This->pAggreg10C.Current_Harmonics[3][0]));
      }
    }
    This->pEvents->Wait();
    //Save results
    pthread_mutex_lock(&This->pDataReadyMutex);
    This->pDataReady = true;
    pthread_mutex_unlock(&This->pDataReadyMutex);
    pthread_cond_broadcast(&This->pDataReadyCond);
    if(((This->pSequence % 15) == 0) && (This->pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 3S aggregation\n");
      This->NormalizeAggreg(This->pAggreg3S);
      if(This->pAggreg3S.Flag == false) {
        float Variation = This->PercentChange(This->pAggreg3S.Frequency, This->pNominalFrequency);
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
          float Variation = This->PercentChange(This->pAggreg10M.Voltage[i], This->pNominalVoltage);
          for(uint Range = 0; Range < (VoltageRanges.size() - 1); Range++) {
            if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) This->pAggreg1H.StatsVoltage[Range]++;
          }
          for(uint Range = 0; Range < (THDRanges.size() - 1); Range++) {
            if((This->pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (This->pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) This->pAggreg1H.StatsTHD[Range]++;
          }
          bool HarmonicsInRange = true;
          for(uint n = 0; n < HarmonicsAllowed.size(); n++) {
            float Variation = This->PercentChange(sqrt(norm(This->pAggreg10M.Voltage_Harmonics[i][n + 1])), This->pNominalVoltage);
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
    pthread_mutex_lock(&This->pWaitingThreadsMutex);
    while(This->pWaitingThreads != 0) pthread_cond_wait(&This->pWaitingThreadsCond, &This->pWaitingThreadsMutex);
    pthread_mutex_unlock(&This->pWaitingThreadsMutex);
    pthread_mutex_lock(&This->pDataReadyMutex);
    This->pDataReady = false;
    pthread_mutex_unlock(&This->pDataReadyMutex);
    This->InitAggreg(This->pAggreg10C, AGGREG_10C);
    pthread_mutex_lock(&This->pThreadMutex);
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", This->GetSchema().c_str(), Tools::GetTime() - Start, This->pSequence);
    This->pWorking = false;
    This->pSequence++;
    pthread_mutex_unlock(&This->pThreadMutex);
  }
  return NULL;
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
void AnalyzerSoft::GetParams(Params_t &params, ParamsType &copy) {
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads++;
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  pthread_mutex_lock(&pDataReadyMutex);
  int Error = 0;
  while((pDataReady == false) && (Error == 0)) {
    struct timeval  Current;
    struct timespec Timeout;
    gettimeofday(&Current, NULL);
    Timeout.tv_sec  = Current.tv_sec + 5;
    Timeout.tv_nsec = 0;
    Error = pthread_cond_timedwait(&pDataReadyCond, &pDataReadyMutex, &Timeout);
  }
  pthread_mutex_unlock(&pDataReadyMutex);
  if(Error == 0) {
    if(params.Type == UNINITIALIZED) InitAggreg(params, AGGREG_10C);
    params.Timestamp = pAggreg10C.Timestamp;
    if(copy & FREQUENCY) params.Frequency = pAggreg10C.Frequency;
    if(copy & FLAG) params.Flag = pAggreg10C.Flag;
    if(copy & VOLTAGE) params.Voltage = pAggreg10C.Voltage;
    if(copy & CURRENT) params.Current = pAggreg10C.Current;
    if(copy & ACTIVE_POWER) params.Active_Power = pAggreg10C.Active_Power;
    if(copy & REACTIVE_POWER) params.Reactive_Power = pAggreg10C.Reactive_Power;
    if(copy & ACTIVE_ENERGY) params.Active_Energy = pAggreg10C.Active_Energy;
    if(copy & REACTIVE_ENERGY) params.Reactive_Energy = pAggreg10C.Reactive_Energy;
    if(copy & APPARENT_POWER) params.Apparent_Power = pAggreg10C.Apparent_Power;
    if(copy & POWER_FACTOR) params.Power_Factor = pAggreg10C.Power_Factor;
    if(copy & VOLTAGE_THD) params.Voltage_THD = pAggreg10C.Voltage_THD;
    if(copy & CURRENT_THD) params.Current_THD.assign(pAggreg10C.Current_THD.begin(), pAggreg10C.Current_THD.end());
    if(copy & VOLTAGE_PHASE) params.Voltage_Phase.assign(pAggreg10C.Voltage_Phase.begin(), pAggreg10C.Voltage_Phase.end());
    if(copy & PHI) params.Phi.assign(pAggreg10C.Phi.begin(), pAggreg10C.Phi.end());
    if(copy & VOLTAGE_HARMONICS) params.Voltage_Harmonics = pAggreg10C.Voltage_Harmonics;
    if(copy & CURRENT_HARMONICS) params.Current_Harmonics = pAggreg10C.Current_Harmonics;
    if(copy & VOLTAGE_SAMPLES) params.Voltage_Samples = pAggreg10C.Voltage_Samples;
    if(copy & CURRENT_SAMPLES) params.Current_Samples = pAggreg10C.Current_Samples;
    if(copy & VOLTAGE_FFT) {
      for(uint i = 0; i < params.Voltage_FFT.size(); i++) {
        for(uint n = 0; n < pVoltageFFT[i]->Output.size(); n++) params.Voltage_FFT[i].push_back(sqrt(norm(pVoltageFFT[i]->Output[n])));
      }
    }
    if(copy & CURRENT_FFT) {
      for(uint i = 0; i < params.Current_FFT.size(); i++) {
        for(uint n = 0; n < pCurrentFFT[i]->Output.size(); n++) params.Current_FFT[i].push_back(sqrt(norm(pCurrentFFT[i]->Output[n])));
      }
    }
  }
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsCond);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  if(copy & SYS_STATS) params.Sys_Stats = System::GetSystemDetails();
}
bool AnalyzerSoft::IsFinished() {
  bool Result;
  pthread_mutex_lock(&pThreadMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pThreadMutex);
  return Result;
}
void AnalyzerSoft::GetAnalyzer(HTTPRequest& req) {
void AnalyzerSoft::SetAnalyzer(HTTPRequest &req) {
  if(!Params["ID"].is_string() || !Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'ID', 'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
  Analyzer *Analyzer = NULL;
  Device *Dev = NULL;
  string Serial = Params["ID"];
  for(uint n = 0; n < pDevices.size(); n++) {
    Dev = pDevices[n];
    Analyzer = Dev->FindAnalyzer(Serial);
    if(Analyzer != NULL) break;
  }
  if(Analyzer == NULL) return req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  float MaxCurrent = Params["MaxCurrent"];
  float MinCurrent = Params["MinCurrent"];
  string Name = Params["Name"];
  float Freq = Params["NominalFreq"];
  float Voltage = Params["NominalVoltage"];
  json Result = json::object();
  if((MaxCurrent < 0) || (MinCurrent < 0) || (MinCurrent >= MaxCurrent) || (Freq < 0) || (Freq > 100) || (Voltage < 50) || (Voltage > 450)) return req.FromString("'MaxCurrent', 'MinCurrent', 'NominalFreq' or 'NominalVoltage' is out of valid ranges", MHD_HTTP_BAD_REQUEST);
  if(req.ExecSentenceNoResult("UPDATE analyzers SET frequency = " + req.Bind(Freq) + ", voltage = " + req.Bind(Voltage) + ", name = " + req.Bind(Name) + ", max_current = " + req.Bind(MaxCurrent) + ", min_current = " + req.Bind(MinCurrent) + " WHERE serial = " + req.Bind(Serial)) == true) {
    if(Dev->State() == Device::RUNNING) {
      Dev->Stop();
      Result["Result"] = Dev->Start();
    } else {
      Result["Result"] = true;
    }
  } else {
    Result["Result"] = false;
  }
  req.FromJSON(Result);


}
void AnalyzerSoft::GetSeries(HTTPRequest &req) {
  
  json Params = req.GetParams();
  //Insert defaults
  if(!Params["To"].is_number())         Params["To"] = 0;
  if(!Params["Period"].is_number())     Params["Period"] = 3600000;
  if(!Params["LastSample"].is_number()) Params["LastSample"] = 0;
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
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
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  string Query = "SELECT row_to_json(Result) AS result FROM ( SELECT " + req.Bind(From) + " AS " + req.BindID("From") + ", " + req.Bind(To) +" AS " + req.BindID("To") + ", ";
  Query += req.Bind((To - From) / PeriodSeconds) + " AS " + req.BindID("MaxSamples") + ", " + req.Bind(Aggreg) + " AS " + req.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
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
      Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
    } else if((Serie == "POWER_FACTOR") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(power_factor ORDER BY sampletime ASC), '{}') AS " + req.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(thd_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(thd_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(phi ORDER BY sampletime ASC), '{}') AS " + req.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(harmonics_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      Query += "COALESCE(ARRAY_AGG(harmonics_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_Harmonics") + ", ";
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
//  printf("%s\n", Query.c_str());
//  fflush(stdout);
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void AnalyzerSoft::GetSeriesNowCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json                          Params = req.GetParams();
  Analyzer::Params_t   Aggreg = { Analyzer::UNINITIALIZED };
  Analyzer::ParamsType Copy   = (Analyzer::ParamsType)0;
  if(!Params["Analyzer"].is_string()) {
    req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
    return;
  }
  if(!Params["Series"].is_array()) {
    req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    return;
  }
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
      return;
    }
    string Serie = Params["Series"][i];
    if(Serie == "FREQUENCY")              Copy = (Analyzer::ParamsType)(Copy | Analyzer::FREQUENCY);
    else if(Serie == "FLAG")              Copy = (Analyzer::ParamsType)(Copy | Analyzer::FLAG);
    else if(Serie == "VOLTAGE")           Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE);
    else if(Serie == "CURRENT")           Copy = (Analyzer::ParamsType)(Copy | Analyzer::CURRENT);
    else if(Serie == "ACTIVE_POWER")      Copy = (Analyzer::ParamsType)(Copy | Analyzer::ACTIVE_POWER);
    else if(Serie == "REACTIVE_POWER")    Copy = (Analyzer::ParamsType)(Copy | Analyzer::REACTIVE_POWER);
    else if(Serie == "ACTIVE_ENERGY")     Copy = (Analyzer::ParamsType)(Copy | Analyzer::ACTIVE_ENERGY);
    else if(Serie == "REACTIVE_ENERGY")   Copy = (Analyzer::ParamsType)(Copy | Analyzer::REACTIVE_ENERGY);
    else if(Serie == "APPARENT_POWER")    Copy = (Analyzer::ParamsType)(Copy | Analyzer::APPARENT_POWER);
    else if(Serie == "POWER_FACTOR")      Copy = (Analyzer::ParamsType)(Copy | Analyzer::POWER_FACTOR);
    else if(Serie == "VOLTAGE_THD")       Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE_THD);
    else if(Serie == "CURRENT_THD")       Copy = (Analyzer::ParamsType)(Copy | Analyzer::CURRENT_THD);
    else if(Serie == "PHI")               Copy = (Analyzer::ParamsType)(Copy | Analyzer::PHI);
    else if(Serie == "VOLTAGE_PHASE")     Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE_PHASE);
    else if(Serie == "VOLTAGE_HARMONICS") Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE_HARMONICS);
    else if(Serie == "CURRENT_HARMONICS") Copy = (Analyzer::ParamsType)(Copy | Analyzer::CURRENT_HARMONICS);
    else if(Serie == "VOLTAGE_FFT")       Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE_FFT);
    else if(Serie == "CURRENT_FFT")       Copy = (Analyzer::ParamsType)(Copy | Analyzer::CURRENT_FFT);
    else if(Serie == "VOLTAGE_SAMPLES")   Copy = (Analyzer::ParamsType)(Copy | Analyzer::VOLTAGE_SAMPLES);
    else if(Serie == "CURRENT_SAMPLES")   Copy = (Analyzer::ParamsType)(Copy | Analyzer::CURRENT_SAMPLES);
    else if(Serie == "SYS_STATS")         Copy = (Analyzer::ParamsType)(Copy | Analyzer::SYS_STATS);
    else return req.FromString("Unknown serie name", MHD_HTTP_BAD_REQUEST);
  }
//  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    AnalyzerInterface *Analyzer = (*it)->FindAnalyzer(Params["Analyzer"]);
//    if(Analyzer == NULL) continue;
//    Analyzer->GetParams(Aggreg, Copy);
    json Result;
    if(Copy & Analyzer::FREQUENCY) Result["Frequency"] = Aggreg.Frequency;
    if(Copy & Analyzer::FLAG) Result["Flag"] = Aggreg.Flag;
    if(Copy & Analyzer::VOLTAGE) Result["Voltage"] = Aggreg.Voltage;
    if(Copy & Analyzer::CURRENT) Result["Current"] = Aggreg.Current;
    if(Copy & Analyzer::ACTIVE_POWER) Result["Active_Power"] = Aggreg.Active_Power;
    if(Copy & Analyzer::REACTIVE_POWER) Result["Reactive_Power"] = Aggreg.Reactive_Power;
    if(Copy & Analyzer::ACTIVE_ENERGY)  Result["Active_Energy"] = Aggreg.Active_Energy;
    if(Copy & Analyzer::REACTIVE_ENERGY) Result["Reactive_Energy"] = Aggreg.Reactive_Energy;
    if(Copy & Analyzer::APPARENT_POWER) Result["Apparent_Power"] = Aggreg.Apparent_Power;
    if(Copy & Analyzer::POWER_FACTOR) Result["Power_Factor"] = Aggreg.Power_Factor;
    if(Copy & Analyzer::VOLTAGE_THD) Result["Voltage_THD"] = Aggreg.Voltage_THD;
    if(Copy & Analyzer::CURRENT_THD) Result["Current_THD"] = Aggreg.Current_THD;
    if(Copy & Analyzer::PHI) Result["Phi"] = Aggreg.Phi;
    if(Copy & Analyzer::VOLTAGE_PHASE) Result["Voltage_Phase"] = Aggreg.Voltage_Phase;
    if(Copy & Analyzer::VOLTAGE_HARMONICS) {
      Result["Voltage_Harmonics"] = json::array();
      for(uint n = 0; n < Aggreg.Voltage_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < Aggreg.Voltage_Harmonics[n].size(); i++) {
          json Row = json::array();
          Row.push_back(Aggreg.Voltage_Harmonics[n][i].real());
          Row.push_back(Aggreg.Voltage_Harmonics[n][i].imag());
          Ch.push_back(Row);
        }
        Result["Voltage_Harmonics"].push_back(Ch);
      }
    }
    if(Copy & Analyzer::CURRENT_HARMONICS) {
      Result["Current_Harmonics"] = json::array();
      for(uint n = 0; n < Aggreg.Current_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < Aggreg.Current_Harmonics[n].size(); i++) {
          json Row = json::array();
          Row.push_back(Aggreg.Current_Harmonics[n][i].real());
          Row.push_back(Aggreg.Current_Harmonics[n][i].imag());
          Ch.push_back(Row);
        }
        Result["Current_Harmonics"].push_back(Ch);
      }
    }
    if(Copy & Analyzer::VOLTAGE_SAMPLES) Result["Voltage_Samples"] = Aggreg.Voltage_Samples;
    if(Copy & Analyzer::CURRENT_SAMPLES) Result["Current_Samples"] = Aggreg.Current_Samples;
    if(Copy & Analyzer::VOLTAGE_FFT) Result["Voltage_FFT"] = Aggreg.Voltage_FFT;
    if(Copy & Analyzer::CURRENT_FFT) Result["Current_FFT"] = Aggreg.Current_FFT;
    if(Copy & Analyzer::SYS_STATS) Result["Sys_Stats"] = Aggreg.Sys_Stats;
    Result["Sampletime"] = Aggreg.Timestamp;
    req.FromJSON(Result);
    return;
//  }
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  return;
}
void AnalyzerSoft::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  string Aggreg = Params["Aggreg"];
  string Analyzer = Params["Analyzer"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer) + ", 'public'");
  req.ExecSentenceAsyncResult("SELECT ranges FROM analyzers, (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 "
                              "WHERE username = " + req.Bind(req.UserName()) + " OR serial IN (SELECT serial from permisions WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer) + ") LIMIT 1"  
  );
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("ranges"), MHD_HTTP_OK);
}




void Analyzer::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  req.ExecSentenceAsyncResult("SELECT json_build_object('From', " + req.Bind(From) + ", 'To', " + req.Bind(To) + ", 'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), 'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                              "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), 'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), 'Finished', COALESCE(ARRAY_AGG(finished ORDER BY starttime ASC), '{}'), "
                              "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), 'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), 'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), 'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                              "FROM events WHERE starttime > " + req.Bind(LastSample) + " AND starttime >= " + req.Bind(From) + " AND starttime <= " + req.Bind(To));
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
}
void Analyzer::GetEventCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
  GetEvent(req);
}
void Analyzer::DelEventCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string()) return req.FromString("'Analyzer', 'StartTime' and 'Type' params are requiered", MHD_HTTP_BAD_REQUEST);
  DelEvent(req);
}
void Analyzer::SetEventCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Notes"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Notes' params are requiered", MHD_HTTP_BAD_REQUEST);
  SetEvent(req);
}
void Analyzer::GetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) {
    if(!Params["RateID"].is_number_unsigned()) {
      req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') as result FROM rates WHERE username = 'admin' OR username = " + req.Bind(req.UserName()));
    } else {
      uint32_t RateID = Params["RateID"];
      req.ExecSentenceAsyncResult("SELECT json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Template', template) as result FROM rates WHERE (username = 'admin' OR username = " + req.Bind(req.UserName()) + ") AND rate = " + req.Bind(RateID));
    }  
  } else {
    string Analyzer = Params["Analyzer"];
    if(!Params["RateID"].is_number_unsigned()) {
      req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', analyzers_rates.name, 'StartDate', startdate, 'EndDate', enddate))), '[]') as result FROM analyzers_rates LEFT JOIN analyzers USING(serial) WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer));
    } else {
      uint32_t RateID = Params["RateID"];
      req.ExecSentenceAsyncResult("SELECT json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) as result FROM analyzers_rates where rate = " + req.Bind(RateID));
    }
  }
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void Analyzer::GetTariffCallback(HTTPRequest &req) {}
void Analyzer::SetTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  string   TimeZone = Params["CountryCode"];
  req.ExecSentenceAsyncResult("SELECT timezone FROM country WHERE country_code = " + req.Bind(TimeZone));
  if(req.FecthRow() == false) return req.FromString("Invalid 'CountryCode'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Template"].is_object()) return req.FromString("'Template' param required", MHD_HTTP_BAD_REQUEST);
  Tariff T(Start, End, req.ResultString("timezone"));
  if(T.SetTemplate(Params["Template"]) == false) return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  if(!Params["Analyzer"].is_string()) {
    //System tariff
    if(!Params["RateID"].is_number_unsigned()) {
      req.FromString("", (req.ExecSentenceNoResult("INSERT INTO rates(name, username, country_code, template, startdate, enddate) VALUES (" + req.Bind(Name) + ", " + req.Bind(req.UserName()) + ", 'UTC', " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Start) + ", " + req.Bind(End) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    } else {
      uint32_t RateID = Params["RateID"];
      req.FromString("", (req.ExecSentenceNoResult("UPDATE rates SET name = " + req.Bind(Name) + ", country_code = 'UTC', template = " + req.Bind(T.GetTemplate().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE username = " + req.Bind(req.UserName()) + " AND rate = " + req.Bind(RateID)) == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    }
  } else {
    //Analyzer tariff
    string Analyzer = Params["Analyzer"];
    if(!Params["Config"].is_object()) return req.FromString("'Config' param required", MHD_HTTP_BAD_REQUEST);    
    if(T.SetConfig(Params["Config"]) == false) return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
    if(!Params["RateID"].is_number_unsigned()) {
      req.FromString("", (req.ExecSentenceNoResult("INSERT INTO analyzers_rates(serial, startdate, enddate, config, template, name) VALUES (" + req.Bind(Analyzer) + ", " + req.Bind(Start) + ", " + req.Bind(End) + ", " + req.Bind(T.GetConfig().dump()) + ", " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Name) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    } else {
      uint32_t RateID = Params["RateID"];
      req.FromString("", (req.ExecSentenceNoResult("UPDATE analyzers_rates SET name = " + req.Bind(Name) + ", template = " + req.Bind(T.GetTemplate().dump()) + ", config = " + req.Bind(T.GetConfig().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE rate = " + req.Bind(RateID) + " AND serial IN (SELECT serial FROM analyzers WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    }
  }
}
void Analyzer::DelTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  uint32_t RateID = Params["RateID"];
  if(!Params["Analyzer"].is_string()) {
    req.FromString("", (req.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + req.Bind(RateID) + " AND username = " + req.Bind(req.UserName())) == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
  } else {
    string Analyzer = Params["Analyzer"];
    req.FromString("", (req.ExecSentenceNoResult("DELETE FROM analyzers_rates WHERE rate = " + req.Bind(RateID) + " AND serial IN (SELECT serial from analyzers WHERE serial = " + req.Bind(Analyzer) + " AND username = " + req.Bind(req.UserName()) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
  }
}
void Analyzer::AddTariffCallback(HTTPRequest &req) {}
void Analyzer::GetPricesCallback(HTTPRequest &req) {
  ///TODO Comprobar permisos
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  if(!Params["To"].is_number_unsigned() || !Params["Period"].is_number_unsigned() || !Params["LastSample"].is_number_unsigned()) return req.FromString("'To', 'Period' and 'LastSample' must be a numbers", MHD_HTTP_BAD_REQUEST);
  uint64_t To         = Params.value<uint64_t>("To", 0);
  uint64_t Period     = Params.value<uint64_t>("Period", 3600000);
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  string   Aggreg     = Params["Aggreg"];
  uint64_t Now        = Tools::GetTime();
  if(To == 0) To = Now;
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  string TimeZone = "Etc/UTC";
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
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
  req.ExecSentenceAsyncResult("SELECT currency, timezone, to_json(array_agg(json_build_object('Name', analyzers_rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as tariff " 
                   "FROM analyzers, analyzers_rates, country WHERE country.country_code = analyzers.country_code AND analyzers_rates.serial = analyzers.serial AND analyzers.serial = " + req.Bind(Analyzer) + " AND enddate >= " + req.Bind(From) + " AND startdate <= " + req.Bind(To) + 
                   " AND analyzers.serial IN (SELECT serial FROM analyzers WHERE username = " + req.Bind(req.UserName()) + " UNION SELECT serial FROM permisions WHERE username = " + req.Bind(req.UserName()) + ") GROUP BY currency, timezone");
  json DBTariffs = json::array();
  if(req.FecthRow()) {
    Result["Currency"] = req.ResultString("currency");
    TimeZone = req.ResultString("timezone");
    DBTariffs = req.ResultJSON("tariff");
  }
  TariffList Tariffs(DBTariffs, TimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
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
  return;  
}




void AnalyzerSoft::GetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  if((Params["Samples"] != "RMS") && (Params["Samples"] != "RAW")) return req.FromString("'Samples' params must be 'RMS' or 'RAW'", MHD_HTTP_BAD_REQUEST);
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Samples = Params["Samples"];
  if(Samples == "RAW") {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', samples1, 'Samples', samples2, 'Post_Samples', samples3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  } else {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', rms1, 'Samples', rms2, 'Post_Samples', rms3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  }
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
}
void AnalyzerSoft::SetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pSerial));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Notes = Params["Notes"];
  req.FromString("", req.ExecSentenceNoResult("UPDATE events SET Notes = " + req.Bind(Notes) + " WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type)) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::DelEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  req.FromString("", req.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type)) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
