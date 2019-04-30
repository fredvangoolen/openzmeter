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
//#include <curl/curl.h>
#include "AnalyzerSoft.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(string id, AnalyzerInterface::AnalyzerMode mode, float samplerate) : AnalyzerInterface(id.c_str(), mode) {
  float Q = 1.0;
  float K = tan(M_PI * (pNominalFrequency / samplerate));
  float Norm = 1 / (1 + K / Q + K * K);
//  pFilters.A0 = K / Q * Norm;
//  pFilters.A1 = 0;
//  pFilters.A2 = -pFilters.A0;
//  pFilters.B1 = 2 * (K * K - 1) * Norm;
//  pFilters.B2 = (1 - K / Q + K * K) * Norm;
  pLastSamples.resize((pMode == PHASE1) ? 1 : 3);
//  pFilters.Channels.resize((pMode == PHASE1) ? 1 : 3);
  pCycles.reserve(ZEROCROSSING_SECONDS * pNominalFrequency);
  pBlockCycles.reserve(20);
  pCyclesPos       = 0;
  pCycleLen        = 0;
  pMaxSamples      = ceil(15.0 * samplerate / pNominalFrequency);
  pSamplerate      = samplerate;
//  pSyncName        = NULL;
//  pSyncPassword    = NULL;
//  pSyncServer      = NULL;
//  pSyncThread      = 0;
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
  pDelayVoltage.resize(pAggreg10C.Voltage_Samples.size());
  pDelayCurrent.resize(pAggreg10C.Current_Samples.size());
  for(uint8_t i = 0; i < pAggreg10C.Voltage_Samples.size(); i++) {
    char Name[16];
    sprintf(Name, "FFT_V%u", i);
    pVoltageFFT.push_back(new FFT(Name));
    uint32_t PhaseShift = (pSamplerate * ZEROCROSSING_FILTERSTAGES) / (pNominalFrequency * 2) + 3;
    pDelayVoltage[i].Reset(PhaseShift);
    for(uint32_t n = 0; n < PhaseShift; n++) pDelayVoltage[i].Push(0.0);
  }
  for(uint8_t i = 0; i < pAggreg10C.Current_Samples.size(); i++) {
    char Name[16];
    sprintf(Name, "FFT_I%u", i);
    pCurrentFFT.push_back(new FFT(Name));
    pDelayCurrent[i].Reset(1);
    for(int n = 0; n < 1; n++) pDelayCurrent[i].Push(0.0);
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
//  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
  for(uint8_t i = 0; i < pVoltageFFT.size(); i++) delete pVoltageFFT[i];
  for(uint8_t i = 0; i < pCurrentFFT.size(); i++) delete pCurrentFFT[i];
}
float AnalyzerSoft::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
bool AnalyzerSoft::BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime) {
  for(uint8_t i = 0; i < pAggreg10C.Voltage_Samples.size(); i++) pDelayVoltage[i].Push(voltageSamples[i]);
  for(uint8_t i = 0; i < pAggreg10C.Current_Samples.size(); i++) pDelayCurrent[i].Push(currentSamples[i]);
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  for(uint8_t i = 0; i < pAggreg10C.Voltage_Samples.size(); i++) pAggreg10C.Voltage_Samples[i].push_back(pDelayVoltage[i].Pop());
  for(uint8_t i = 0; i < pAggreg10C.Current_Samples.size(); i++) pAggreg10C.Current_Samples[i].push_back(pDelayCurrent[i].Pop());
  vector<float> Outs = voltageSamples;
//  for(uint8_t i = 0; i < voltageSamples.size(); i++) {
//    for(int j = 0; j < ZEROCROSSING_FILTERSTAGES; j++) {
//      float Out = Outs[i] * pFilters.A0 + pFilters.Channels[i].Z1[j];
//      pFilters.Channels[i].Z1[j] = Outs[i] * pFilters.A1 + pFilters.Channels[i].Z2[j] - pFilters.B1 * Out;
//      pFilters.Channels[i].Z2[j] = Outs[i] * pFilters.A2 - pFilters.B2 * Out;
//      Outs[i] = Out;
//    }
//  }
  if(pBlockCycles.size() == pBlockCycles.capacity()) pBlockCycles.clear();
  pCycleLen++;
  if((((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) && (pAggreg10C.Voltage_Samples[0].size() < pMaxSamples)) {
    pBlockCycles.push_back(pCycleLen);
    if(pCycles.size() < pCycles.capacity()) {
      pCycles.push_back(pCycleLen);
    } else {
      pCycles[pCyclesPos] = pCycleLen;
      pCyclesPos = (pCyclesPos + 1) % pCycles.capacity();
    }
    pCycleLen = 0;
  }
  pLastSamples = Outs;
  if(pBlockCycles.size() == pBlockCycles.capacity()) {
    pthread_mutex_lock(&pThreadMutex);
    pWorking = true;
    pthread_mutex_unlock(&pThreadMutex);
    pthread_cond_signal(&pThreadCond);
    return true;
  }
  return false;
}
void *AnalyzerSoft::Analyzer_Thread(void *args) {
  ((AnalyzerSoft*)args)->Analyze();
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
  return (Samples / pSamplerate) * 10.0;
}
void AnalyzerSoft::Analyze() {
  while(true) {
    pthread_mutex_lock(&pThreadMutex);
    while(pWorking == false) {
      if(pTerminate == true) return;
      pthread_cond_wait(&pThreadCond, &pThreadMutex);
    }
    uint64_t Start = Tools::GetTime();
    pthread_mutex_unlock(&pThreadMutex);
    //Inicia FFTs
    for(uint i = 0; i < pVoltageFFT.size(); i++) pVoltageFFT[i]->Start(pAggreg10C.Voltage_Samples[i]);
    for(uint i = 0; i < pCurrentFFT.size(); i++) pCurrentFFT[i]->Start(pAggreg10C.Current_Samples[i]);
    pAggreg10C.Frequency = CalcFrequency();
    if(!isnan(pAggreg10C.Frequency)) pAggreg10C.Frequency_Inputs = 1;
    bool Valid = false;
    vector<float> RMS = vector<float>(pAggreg10C.Voltage.size(), 0);
    for(uint j = 0; j < pAggreg10C.Voltage.size(); j++) {
      for(uint32_t i = 0; i < pAggreg10C.Voltage_Samples[j].size(); i++) RMS[j] += pow(pAggreg10C.Voltage_Samples[j][i], 2.0);
      RMS[j] = sqrt(RMS[j] / pAggreg10C.Voltage_Samples[j].size());
      if(RMS[j] > 1) Valid = true;
    }
    if(Valid) pAggreg10C.Voltage.assign(RMS.begin(), RMS.end());
    if(Valid) pAggreg10C.Voltage_Inputs = 1;
    for(uint j = 0; j < pAggreg10C.Current.size(); j++) {
      float RMS = 0;
      for(uint32_t i = 0; i < pAggreg10C.Current_Samples[j].size(); i++) RMS += pow(pAggreg10C.Current_Samples[j][i], 2.0);
      RMS = sqrt(RMS / pAggreg10C.Current_Samples[j].size());
      if(RMS > pMinCurrent) {
        pAggreg10C.Current_Inputs[j] = 1;
        pAggreg10C.Current[j] = RMS;
        for(uint32_t i = 0; i < pAggreg10C.Current_Samples[j].size(); i++) pAggreg10C.Active_Power[j] = AccumulateSUM(pAggreg10C.Active_Power[j], pAggreg10C.Voltage_Samples[j][i] * pAggreg10C.Current_Samples[j][i]);
        pAggreg10C.Active_Power[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Current_Samples[j].size();
      }
    }
    for(uint j = 0; j < pAggreg10C.Voltage.size(); j++) {
      pVoltageFFT[j]->Wait();
      if(isnan(pAggreg10C.Voltage[j])) continue;
      for(uint i = 0; i < pAggreg10C.Voltage_Harmonics[j].size(); i++) {
        pAggreg10C.Voltage_Harmonics[j][i] = pVoltageFFT[j]->BinPower(10 + i * 10);
        if(i > 0) pAggreg10C.Voltage_THD[j] = AccumulateSUM(pAggreg10C.Voltage_THD[j], pow(pAggreg10C.Voltage_Harmonics[j][i], 2.0));
      }
      pAggreg10C.Voltage_THD[j] = sqrt(pAggreg10C.Voltage_THD[j]) / pAggreg10C.Voltage_Harmonics[j][0];
    }
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
      pCurrentFFT[j]->Wait();
      if(isnan(pAggreg10C.Current[j]) || isnan(pAggreg10C.Voltage[j])) continue;
      pAggreg10C.Apparent_Power[j] = pAggreg10C.Voltage[j] * pAggreg10C.Current[j];
      for(int i = 10; i <= 500 ; i = i + 10) {
        int   Harmonic = (i / 10) - 1;
        float CurrentHarmonic = pCurrentFFT[j]->BinPower(i);
        if(Harmonic > 0) pAggreg10C.Current_THD[j] = AccumulateSUM(pAggreg10C.Current_THD[j], pow(CurrentHarmonic, 2.0));
        float VoltageHarmonic = pAggreg10C.Voltage_Harmonics[j][Harmonic];
        float PHI = FFT::CalcAngle(pVoltageFFT[j]->Output[i], pCurrentFFT[j]->Output[i]);
        pAggreg10C.Current_Harmonics[j][Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
          pAggreg10C.Reactive_Power[j] = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
      pAggreg10C.Current_THD[j] = sqrt(pAggreg10C.Current_THD[j]) / pAggreg10C.Current_Harmonics[j][0];
      pAggreg10C.Active_Energy[j] = (pAggreg10C.Active_Power[j] * pAggreg10C.Current_Samples[j].size()) / (pSamplerate * 3600.0);
      pAggreg10C.Power_Factor[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Apparent_Power[j];
      pAggreg10C.Reactive_Energy[j] = (pAggreg10C.Reactive_Power[j] * pAggreg10C.Current_Inputs[j]) / (pSamplerate * 3600.0);
    }
    if((pMode == PHASE3N) && (!isnan(pAggreg10C.Current[3]))) {
      pCurrentFFT[3]->Wait();
      for(int i = 10; i <= 500 ; i = i + 10) {
        int   Harmonic = (i / 10) - 1;
        float CurrentHarmonic = pCurrentFFT[3]->BinPower(i);
        if(Harmonic > 0) pAggreg10C.Current_THD[3] = AccumulateSUM(pAggreg10C.Current_THD[3], pow(CurrentHarmonic, 2.0));
      }
    }
    //Guardar resultados
    pthread_mutex_lock(&pDataReadyMutex);
    pDataReady = true;
    pthread_mutex_unlock(&pDataReadyMutex);
    pthread_cond_broadcast(&pDataReadyCond);
    if(((pSequence % 15) == 0) && (pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 3S aggregation\n");
      NormalizeAggreg(pAggreg3S);
      if(pAggreg3S.Flag == false) {
        float Variation = PercentChange(pAggreg3S.Frequency, pNominalFrequency);
        for(uint Range = 0; Range < 40; Range++) {
          if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) pAggreg1H.StatsFrequency[Range]++;
        }
      }
      SaveAggreg(pAggreg3S);
      InitAggreg(pAggreg3S, AGGREG_3S);
    }
    AccumulateAggreg(pAggreg3S, pAggreg10C);
    if(((pAggreg10C.Timestamp / 60000) > (pPrevTimestamp / 60000)) && (pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1M aggregation\n");
      pAggreg1M.Timestamp = pAggreg1M.Timestamp / 60000 * 60000 + 59999;
      NormalizeAggreg(pAggreg1M);
      SaveAggreg(pAggreg1M);
      InitAggreg(pAggreg1M, AGGREG_1M);
    }
    AccumulateAggreg(pAggreg1M, pAggreg10C);
    if(((pAggreg10C.Timestamp / 600000) > (pPrevTimestamp / 600000)) && (pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 10M aggregation\n");
      pAggreg10M.Timestamp = pAggreg10M.Timestamp / 600000 * 600000 + 599999;
      NormalizeAggreg(pAggreg10M);
      if(pAggreg10M.Flag == false) {
        for(uint i = 0; i < pAggreg10M.Voltage.size(); i++) {
          float Variation = PercentChange(pAggreg10M.Voltage[i], pNominalVoltage);
          for(uint Range = 0; Range < (VoltageRanges.size() - 1); Range++) {
            if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) pAggreg1H.StatsVoltage[Range]++;
          }
          for(uint Range = 0; Range < (THDRanges.size() - 1); Range++) {
            if((pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) pAggreg1H.StatsTHD[Range]++;
          }
          bool HarmonicsInRange = true;
          for(uint n = 0; n < HarmonicsAllowed.size(); n++) {
            float Variation = PercentChange(pAggreg10M.Voltage_Harmonics[i][n + 1], pNominalVoltage);
            if(Variation > HarmonicsAllowed[n]) {
              HarmonicsInRange = false;
              break;
             }
          }
          if(HarmonicsInRange == true) {
            pAggreg1H.StatsHarmonics[0]++;
          } else {
            pAggreg1H.StatsHarmonics[1]++;
          }
        }
      }
      SaveAggreg(pAggreg10M);
      InitAggreg(pAggreg10M, AGGREG_10M);
    }
    AccumulateAggreg(pAggreg10M, pAggreg10C);
    if(((pAggreg10C.Timestamp / 900000) > (pPrevTimestamp / 900000)) && (pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 15M aggregation\n");
      pAggreg15M.Timestamp = pAggreg15M.Timestamp / 900000 * 900000 + 899999;
      NormalizeAggreg(pAggreg15M);
      SaveAggreg(pAggreg15M);
      InitAggreg(pAggreg15M, AGGREG_15M);
    }
    AccumulateAggreg(pAggreg15M, pAggreg10C);
    if(((pAggreg10C.Timestamp / 3600000) > (pPrevTimestamp / 3600000)) && (pPrevTimestamp != 0)) {
      LOGFile::Debug("Storing 1H aggregation\n");
      pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      NormalizeAggreg(pAggreg1H);
      SaveAggreg(pAggreg1H);
      InitAggreg(pAggreg1H, AGGREG_1H);
    }
    AccumulateAggreg(pAggreg1H, pAggreg10C);
    pPrevTimestamp = pAggreg10C.Timestamp;
    pthread_mutex_lock(&pWaitingThreadsMutex);
    while(pWaitingThreads != 0) pthread_cond_wait(&pWaitingThreadsCond, &pWaitingThreadsMutex);
    pthread_mutex_unlock(&pWaitingThreadsMutex);
    pthread_mutex_lock(&pDataReadyMutex);
    pDataReady = false;
    pthread_mutex_unlock(&pDataReadyMutex);
    InitAggreg(pAggreg10C, AGGREG_10C);
    pthread_mutex_lock(&pThreadMutex);
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", pName.c_str(), Tools::GetTime() - Start, pSequence);
    pWorking = false;
    pSequence++;
    pthread_mutex_unlock(&pThreadMutex);
  }
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
        for(uint n = 0; n < pVoltageFFT[i]->Output.size(); n++) params.Voltage_FFT[i].push_back(pVoltageFFT[i]->BinPower(n));
      }
    }
    if(copy & CURRENT_FFT) {
      for(uint i = 0; i < params.Current_FFT.size(); i++) {
        for(uint n = 0; n < pCurrentFFT[i]->Output.size(); n++) params.Current_FFT[i].push_back(pCurrentFFT[i]->BinPower(n));
      }
    }
  }
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsCond);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  if(copy & SYS_STATS) params.Sys_Stats = Tools::GetSysInfo();
}
bool AnalyzerSoft::IsFinished() {
  bool Result;
  pthread_mutex_lock(&pThreadMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pThreadMutex);
  return Result;
}
//void AnalyzerSoft::ConfigRemote(json_object* config) {
//  json_object *Find;
//  if((json_object_object_get_ex(config, "Username", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncName = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(config, "Password", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncPassword = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(config, "RemoteHost", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncServer = (char*)json_object_get_string(Find);
//  if((pSyncName == NULL) || (pSyncPassword == NULL) || (pSyncServer == NULL)) {
//    LOGFile::Error("AnayzerSoft: Remote functionality mode need 'Username', 'Password' and 'RemoteServer' params\n");
//    return;
//  }
//  if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
//    LOGFile::Error("AnayzerSoft: Failed to load libcurl\n");
//    return;  
//  }
//  if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return;
//  }
//}
//void AnalyzerSoft::ConfigPrices(json_object* config) {
//  pPrices = PricesInterface::Config(GetSchema(), config);
//}
//size_t AnalyzerSoft::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
//  CurlBuff_t *Buff = (CurlBuff_t*)userp;
//  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
//    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
//    if(Buff->Buffer == NULL) {
//      return 0;
//    } else {
//      Buff->MaxSize = Buff->Size + size * nmemb + 1;
//    }
//  }
//  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
//  Buff->Size += size * nmemb;
//  Buff->Buffer[Buff->Size] = 0;
//  return size * nmemb;
//}
//void *AnalyzerSoft::Sync_Thread(void *args) {
//  int Prio = sched_get_priority_min(SCHED_RR);
//  if(Prio == -1) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//  } else {
//    struct sched_param Param;
//    Param.sched_priority = Prio;
//    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise thread priority -> %s\n", __func__);
//  }
//  AnalyzerSoft *This = (AnalyzerSoft*)args;
//  pthread_setname_np(This->pSyncThread, "Sync");
//  CurlBuff_t   CurlBuff;
//  json_object *Find;
//  size_t       JSONLen;
//  long         HTTPCode;
//  char         Tmp[1024];
//  uint64_t     Last3S      = 0;
//  uint64_t     Last1M      = 0;
//  uint64_t     Last10M     = 0;
//  uint64_t     Last1H      = 0;
//  uint64_t     LastEvent   = 0;
//  uint64_t     LastPrice   = 0;
//  bool         Registered  = false;
//  bool         Initialized = false;
//  CURL        *Curl        = curl_easy_init();
//  curl_slist  *Headers     = NULL;
//  Database     DB(This->GetID());
//  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
//  curl_easy_setopt(Curl, CURLOPT_POST, 1);
//  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
//  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
//  while(!This->pTerminate) {
//    if(Registered == false) {
//      if(Headers == NULL) {
//        curl_slist_free_all(Headers);
//        Headers = curl_slist_append(Headers, "Accept: application/json");
//        Headers = curl_slist_append(Headers, "Accept-Encoding: deflate");
//        Headers = curl_slist_append(Headers, "Connection: keep-alive");
//        Headers = curl_slist_append(Headers, "Content-Type: application/json");
//        curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);
//      }
//      json_object *JSONWriter = json_object_new_object();
//      json_object_object_add(JSONWriter, "Username", json_object_new_string(This->pSyncName));
//      json_object_object_add(JSONWriter, "Password", json_object_new_string(This->pSyncPassword));
//      json_object_object_add(JSONWriter, "UpdateLastLogin", json_object_new_boolean(false));
//      CurlBuff.Size = 0;
//      snprintf(Tmp, sizeof(Tmp), "%s/LogIn", This->pSyncServer);
//      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//      CURLcode Code = curl_easy_perform(Curl);
//      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//      if((Code != CURLE_OK) || (HTTPCode != 200)) {
//        LOGFile::Warning("AnalyzerRemote: failed to register on remote server\n");
//      } else {
//        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//          if((json_object_object_get_ex(JSONReader, "Result", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) {
//            Registered = json_object_get_boolean(Find);
//            if(Registered) Tools::LogMsg("INFO", "AnalyzerRemote: logged to remote server\n");
//          }
//          json_object_put(JSONReader);
//        }
//      }
//      if(Registered == false) usleep(1000000);
//      json_object_put(JSONWriter);
//    } else if(Initialized == false) {
//      snprintf(Tmp, sizeof(Tmp), "%s/setRemoteAnalyzer", This->pSyncServer);
//      json_object *JSONWriter = This->GetJSONDescriptor();
//      CurlBuff.Size = 0;
//      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//      CURLcode Code = curl_easy_perform(Curl);
//      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//      if((Code != CURLE_OK) || (HTTPCode != 200)) {
//        LOGFile::Warning("AnalyzerRemote: failed to config on remote server\n");
//      } else {
//        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//          if((json_object_object_get_ex(JSONReader, "aggreg_3s", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last3S = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_1m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1M = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_10m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last10M = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_1h", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1H = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "events", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastEvent = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "prices", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastPrice = json_object_get_int64(Find);
//          Tools::LogMsg("INFO", "AnalyzerRemote: configured on remote server\n");
//          Initialized = true;
//        }
//      }
//      json_object_put(JSONWriter);
//      if(Initialized == false) usleep(1000000);
//    } else {
//      bool      Live  = false;
//      char     *Table = (char*)"aggreg_3s";
//      uint64_t *Time  = &Last3S;
////      sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Table\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//      char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);
//      if(Result == NULL) {
//        Table = (char*)"aggreg_1m";
//        Time  = &Last1M;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1m' AS \"Aggregation_Source\", * FROM (SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS aggreg_1m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result == NULL) {
//        Table = (char*)"aggreg_10m";
//        Time  = &Last10M;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_10m' AS \"Aggregation_Source\", * FROM aggreg_10m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result == NULL) {
//        Table = (char*)"aggreg_10m";
//        Time  = &Last1H;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1h' AS \"Aggregation_Source\", * FROM aggreg_1h WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result != NULL) {
//        snprintf(Tmp, sizeof(Tmp), "%s/setRemoteData", This->pSyncServer);
//        CurlBuff.Size = 0;
//        curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//        curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, Result);
//        curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//        CURLcode Code = curl_easy_perform(Curl);
//        curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//        if((Code != CURLE_OK) || (HTTPCode != 200)) {
//          LOGFile::Warning("AnalyzerRemote: failed send data to server\n");
//        } else {
//          json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//          if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//            *Time = atol(DB.ResultValue(0, (char*)"sampletime"));
//            if((json_object_object_get_ex(JSONReader, "Live", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Live = json_object_get_int64(Find);
//            LOGFile::Debug("AnalyzerRemote: sampletime %" PRIu64 " saved in remote '%s' (Live: %s)\n", *Time, Table, (Live ? "true" : "false"));
//          }
//        }
//      } else {
//        usleep(1000000);
//      }
//    }
//  }
//  curl_easy_cleanup(Curl);
//  return NULL;
//}