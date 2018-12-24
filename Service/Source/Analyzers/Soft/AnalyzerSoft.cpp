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
#include <curl/curl.h>
#include "AnalyzerSoft.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(float samplerate, char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerInterface::AnalyzerMode mode) : AnalyzerInterface(id, nominal_voltage, nominal_frequency, name, max_current, min_current, mode) {
  float Q = 1.0;
  float K = tan(M_PI * (nominal_frequency / samplerate));
  float Norm = 1 / (1 + K / Q + K * K);
  pFilters.A0 = K / Q * Norm;
  pFilters.A1 = 0;
  pFilters.A2 = -pFilters.A0;
  pFilters.B1 = 2 * (K * K - 1) * Norm;
  pFilters.B2 = (1 - K / Q + K * K) * Norm;
  pLastSamples.resize((pMode == PHASE1) ? 1 : 3);
  pFilters.Channels.resize((pMode == PHASE1) ? 1 : 3);
  pCycles.reserve(ZEROCROSSING_SECONDS * nominal_frequency);
  pBlockCycles.reserve(20);
  pCyclesPos       = 0;
  pCycleLen        = 0;
  pMaxSamples      = ceil(15.0 * samplerate / nominal_frequency);
  pSamplerate      = samplerate;
  pSyncName        = NULL;
  pSyncPassword    = NULL;
  pSyncServer      = NULL;
  pSyncThread      = 0;
  pSequence        = 0;
  pWorking         = false;
  pTerminate       = false;
  pWaitingThreads  = 0;
  pDataReady       = false;
  pPrevTimestamp   = 0;
  InitAggreg(&pAggreg10C, AGGREG_10C);
  InitAggreg(&pAggreg3S, AGGREG_3S);
  InitAggreg(&pAggreg1M, AGGREG_1M);
  InitAggreg(&pAggreg10M, AGGREG_10M);
  InitAggreg(&pAggreg1H, AGGREG_1H);
  pDelayVoltage.resize(pAggreg10C.Voltage.size());
  pDelayCurrent.resize(pAggreg10C.Current.size());
  for(uint8_t i = 0; i < pAggreg10C.Voltage.size(); i++) {
    char Name[16];
    sprintf(Name, "FFT_V%u", i);
    pVoltageFFT.push_back(new FFT(Name));
    uint32_t PhaseShift = (pSamplerate * ZEROCROSSING_FILTERSTAGES) / (nominal_frequency * 2) + 3;
    pDelayVoltage[i].Reset(PhaseShift);
    for(int n = 0; n < PhaseShift; n++) pDelayVoltage[i].Push(0.0);
  }
  for(uint8_t i = 0; i < pAggreg10C.Current.size(); i++) {
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
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, (pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N");
}
AnalyzerSoft::~AnalyzerSoft() {
  pTerminate = true;
  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
//  delete pZC;  
  for(uint8_t i = 0; i < pAggreg10C.Voltage.size(); i++) delete pVoltageFFT[i];
  for(uint8_t i = 0; i < pAggreg10C.Current.size(); i++) delete pCurrentFFT[i];
}
float AnalyzerSoft::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
bool AnalyzerSoft::BufferAppend(const vector<float> &voltageSamples, const vector<float> &currentSamples, uint64_t sampletime) {
  for(uint8_t i = 0; i < pAggreg10C.Voltage.size(); i++) pDelayVoltage[i].Push(voltageSamples[i]);
  for(uint8_t i = 0; i < pAggreg10C.Current.size(); i++) pDelayCurrent[i].Push(currentSamples[i]);
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
//  vector<float> VoltageSamples = pDelayVoltage.Pop();
//  vector<float> CurrentSamples = pDelayCurrent.Pop();
  for(uint8_t i = 0; i < pAggreg10C.Voltage.size(); i++) pAggreg10C.Voltage[i].push_back(pDelayVoltage[i].Pop());
  for(uint8_t i = 0; i < pAggreg10C.Current.size(); i++) pAggreg10C.Current[i].push_back(pDelayCurrent[i].Pop());
  pAggreg10C.Samples++;

  vector<float> Outs = voltageSamples;
  for(uint8_t i = 0; i < voltageSamples.size(); i++) {
    for(int j = 0; j < ZEROCROSSING_FILTERSTAGES; j++) {
      float Out = Outs[i] * pFilters.A0 + pFilters.Channels[i].Z1[j];
      pFilters.Channels[i].Z1[j] = Outs[i] * pFilters.A1 + pFilters.Channels[i].Z2[j] - pFilters.B1 * Out;
      pFilters.Channels[i].Z2[j] = Outs[i] * pFilters.A2 - pFilters.B2 * Out;
      Outs[i] = Out;
    }
  }
  if(pBlockCycles.size() == pBlockCycles.capacity()) pBlockCycles.clear();
  pCycleLen++;
  if((((pLastSamples[0] >= 0.0) && (Outs[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (Outs[0] > 0.0))) && (pAggreg10C.Samples < pMaxSamples)) {
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
void AnalyzerSoft::Analyze() {
  while(true) {
    pthread_mutex_lock(&pThreadMutex);
    while(pWorking == false) {
      if(pTerminate == true) return;
      pthread_cond_wait(&pThreadCond, &pThreadMutex);
    }
    uint64_t Start = Tools::GetTime();
    pthread_mutex_unlock(&pThreadMutex);
    uint32_t Samples = 0;
    uint32_t MinCycle = pSamplerate / (pNominalFrequency * 1.15 * 2.0);
    uint32_t MaxCycle = pSamplerate / (pNominalFrequency * 0.85 * 2.0);
    for(uint i = 0; i < pCycles.size(); i++) {
      if((pCycles[i] < MinCycle) || (pCycles[i] > MaxCycle)) {
        Samples = 0;
        break;
      } else {
        Samples += pCycles[i];
      }
    } 
    if(Samples > 0) pAggreg10C.Frequency = (Samples / pSamplerate) * 10.0;
    //Inicia FFTs
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) pVoltageFFT[i]->Start(&pAggreg10C.Voltage[i]);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) pCurrentFFT[i]->Start(&pAggreg10C.Current[i]);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    for(uint32_t i = 0; i < pAggreg10C.Samples; i++) {
      for(uint j = 0; j < pAggreg10C.RMS_V.size(); j++) pAggreg10C.RMS_V[j] += pow(pAggreg10C.Voltage[j][i], 2.0);
      for(uint j = 0; j < pAggreg10C.RMS_I.size(); j++) pAggreg10C.RMS_I[j] += pow(pAggreg10C.Current[j][i], 2.0);
      for(uint j = 0; j < pAggreg10C.Active_Power.size(); j++) pAggreg10C.Active_Power[j] += pAggreg10C.Voltage[j][i] * pAggreg10C.Current[j][i];
    }
    for(uint j = 0; j < pAggreg10C.RMS_V.size(); j++) pAggreg10C.RMS_V[j] = sqrt(pAggreg10C.RMS_V[j] / pAggreg10C.Samples);
    for(uint j = 0; j < pAggreg10C.RMS_I.size(); j++) pAggreg10C.RMS_I[j] = sqrt(pAggreg10C.RMS_I[j] / pAggreg10C.Samples);
    for(uint j = 0; j < pAggreg10C.Active_Power.size(); j++) pAggreg10C.Active_Power[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Samples;
    //Calculo harmonicos de tension
    for(uint j = 0; j < pAggreg10C.VoltageHarmonics.size(); j++) {
      pVoltageFFT[j]->Wait();
      for(uint i = 0; i < pAggreg10C.VoltageHarmonics[j].size(); i++) {
        pAggreg10C.VoltageHarmonics[j][i] = pVoltageFFT[j]->BinPower(10 + i * 10);
        if(i > 0) pAggreg10C.THD_V[j] += pow(pAggreg10C.VoltageHarmonics[j][i], 2.0);
      }
      pAggreg10C.THD_V[j] = sqrt(pAggreg10C.THD_V[j]) / pAggreg10C.VoltageHarmonics[j][0];
    }
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
      pCurrentFFT[j]->Wait();
      if(pAggreg10C.RMS_I[j] < pMinCurrent) {
        pAggreg10C.RMS_I[j] = NAN;
        pAggreg10C.Active_Power[j] = NAN;
        continue;
      }
      pAggreg10C.Apparent_Power[j] = pAggreg10C.RMS_V[j] * pAggreg10C.RMS_I[j];
      for(int i = 10; i <= 500 ; i = i + 10) {
        int   Harmonic = (i / 10) - 1;
        float CurrentHarmonic = pCurrentFFT[j]->BinPower(i);
        if(Harmonic > 0) pAggreg10C.THD_I[j] += pow(CurrentHarmonic, 2.0);
        float VoltageHarmonic = pAggreg10C.VoltageHarmonics[j][Harmonic];
        float PHI = FFT::CalcAngle(pVoltageFFT[j]->Output[i], pCurrentFFT[j]->Output[i]);
        pAggreg10C.CurrentHarmonics[j][Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
//          pAggreg10C.Fundamental_Active_Power[j]   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          pAggreg10C.Reactive_Power[j] = VoltageHarmonic * CurrentHarmonic * sin(PHI);
//          pAggreg10C.Fundamental_Apparent_Power[j] = VoltageHarmonic * CurrentHarmonic;
//          pAggreg10C.Fundamental_Power_Factor[j]   = pAggreg10C.Fundamental_Active_Power[j] / pAggreg10C.Fundamental_Apparent_Power[j];
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
      pAggreg10C.THD_I[j] = sqrt(pAggreg10C.THD_I[j]) / pAggreg10C.CurrentHarmonics[j][0];
      pAggreg10C.Active_Energy[j] = (pAggreg10C.Active_Power[j] * pAggreg10C.Samples) / (pSamplerate * 3600.0);
      pAggreg10C.Power_Factor[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Apparent_Power[j];
      pAggreg10C.Reactive_Energy[j] = (pAggreg10C.Reactive_Power[j] * pAggreg10C.Samples) / (pSamplerate * 3600.0);
    }
    if(pMode == PHASE3N) {
      pCurrentFFT[3]->Wait();
      for(int i = 10; i <= 500 ; i = i + 10) {
        int   Harmonic = (i / 10) - 1;
        float CurrentHarmonic = pCurrentFFT[3]->BinPower(i);
        if(Harmonic > 0) pAggreg10C.THD_I[3] += pow(CurrentHarmonic, 2.0);
      }
    }
    //Guardar resultados
    pthread_mutex_lock(&pDataReadyMutex);
    pDataReady = true;
    pthread_mutex_unlock(&pDataReadyMutex);
    pthread_cond_broadcast(&pDataReadyCond);
    if(((pSequence % 15) == 0) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 3S aggregation\n");
      NormalizeAggreg(&pAggreg3S);
      if(pAggreg3S.Flag == false) {
        float Variation = PercentChange(pAggreg3S.Frequency, pNominalFrequency);
        for(uint Range = 0; Range < 40; Range++) {
          if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) pAggreg1H.StatsFrequency[Range]++;
        }
      }
      SaveAggreg(&pAggreg3S);
      ResetAggreg(&pAggreg3S);
    }
    AccumulateAggreg(&pAggreg3S, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 60000) > (pPrevTimestamp / 60000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 1M aggregation\n");
      pAggreg1M.Timestamp = pAggreg1M.Timestamp / 60000 * 60000 + 59999;
      NormalizeAggreg(&pAggreg1M);
      SaveAggreg(&pAggreg1M);
      ResetAggreg(&pAggreg1M);
    }
    AccumulateAggreg(&pAggreg1M, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 600000) > (pPrevTimestamp / 600000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 10M aggregation\n");
      pAggreg10M.Timestamp = pAggreg10M.Timestamp / 600000 * 600000 + 599999;
      NormalizeAggreg(&pAggreg10M);
      if(pAggreg10M.Flag == false) {
        for(uint i = 0; i < pAggreg10M.RMS_V.size(); i++) {
          float Variation = PercentChange(pAggreg10M.RMS_V[i], pNominalVoltage);
          for(uint Range = 0; Range < 26; Range++) {
            if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) pAggreg1H.StatsVoltage[Range]++;
          }
          for(uint Range = 0; Range < 26; Range++) {
            if((pAggreg10M.THD_V[i] >= THDRanges[Range]) && (pAggreg10M.THD_V[i] < THDRanges[Range + 1])) pAggreg1H.StatsTHD[Range]++;
          }
          bool HarmonicsInRange = true;
          for(int n = 0; n < 25; n++) {
            float Variation = PercentChange(pAggreg10M.VoltageHarmonics[i][n], pNominalVoltage);
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
      SaveAggreg(&pAggreg10M);
      ResetAggreg(&pAggreg10M);
    }
    AccumulateAggreg(&pAggreg10M, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 3600000) > (pPrevTimestamp / 3600000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 1H aggregation\n");
      pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      NormalizeAggreg(&pAggreg1H);
      SaveAggreg(&pAggreg1H);
      ResetAggreg(&pAggreg1H);
    }
    AccumulateAggreg(&pAggreg1H, &pAggreg10C);
    pPrevTimestamp = pAggreg10C.Timestamp;
    pthread_mutex_lock(&pWaitingThreadsMutex);
    while(pWaitingThreads != 0) pthread_cond_wait(&pWaitingThreadsCond, &pWaitingThreadsMutex);
    pthread_mutex_unlock(&pWaitingThreadsMutex);
    pthread_mutex_lock(&pDataReadyMutex);
    pDataReady = false;
    pthread_mutex_unlock(&pDataReadyMutex);
    ResetAggreg(&pAggreg10C);
    pthread_mutex_lock(&pThreadMutex);
    Tools::LogMsg("DEBUG", "Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", pName, Tools::GetTime() - Start, pSequence);
    pWorking = false;
    pSequence++;
    pthread_mutex_unlock(&pThreadMutex);
  }
}
void AnalyzerSoft::GetParams(Params_t *params, bool extended) {
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
    *params = pAggreg10C;
//    memcpy(params, &pAggreg10C, sizeof(Params_t));
    if(extended) {
//      memset(extParams, 0, sizeof(ExtParams_t));
      params->FFT_V.resize((pMode == PHASE1) ? 1 : 3);
      params->FFT_I.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
      params->Voltage.resize((pMode == PHASE1) ? 1 : 3);
      params->Current.resize((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4);
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
        params->Voltage[j].reserve(params->Samples); 
//        params->Voltage[j] = (float*)malloc(sizeof(float) * params->Samples);
        for(uint32_t i = 0; i < params->Samples; i++) params->Voltage[j][i] = pAggreg10C.Voltage[j][i];
          params->FFT_V[j].reserve(pVoltageFFT[j]->Output.size()); 
        // extParams->FFT_V[j] = (float*)malloc(sizeof(float) * pVoltageFFT[j]->OutputLen);
        for(uint32_t i = 0; i < pVoltageFFT[j]->Output.size(); i++) params->FFT_V[j][i] = pVoltageFFT[j]->BinPower(i);
      }
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) {
        params->Current[j].reserve(params->Samples); 

//        extParams->Current[j] = (float*)malloc(sizeof(float) * params->Samples);
//        memcpy(extParams->Current[j], pCurrentSamples[j], sizeof(float) * params->Samples);
        for(uint32_t i = 0; i < params->Samples; i++) params->Current[j][i] = pAggreg10C.Current[j][i];
        params->FFT_I[j].reserve(pCurrentFFT[j]->Output.size());
  //      extParams->FFT_I[j] = (float*)malloc(sizeof(float) * pCurrentFFT[j]->OutputLen);
        for(uint32_t i = 0; i < pCurrentFFT[j]->Output.size(); i++) params->FFT_I[j][i] = pCurrentFFT[j]->BinPower(i);
      }
    }
  }
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsCond);
  pthread_mutex_unlock(&pWaitingThreadsMutex);  
}
bool AnalyzerSoft::IsFinished() {
  bool Result;
  pthread_mutex_lock(&pThreadMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pThreadMutex);
  return Result;
}
void AnalyzerSoft::ConfigRemote(json_object* config) {
  json_object *Find;
  if((json_object_object_get_ex(config, "Username", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncName = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "Password", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncPassword = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "RemoteHost", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncServer = (char*)json_object_get_string(Find);
  if((pSyncName == NULL) || (pSyncPassword == NULL) || (pSyncServer == NULL)) {
    Tools::LogMsg("ERROR", "AnayzerSoft: Remote functionality mode need 'Username', 'Password' and 'RemoteServer' params\n");
    return;
  }
  if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
    Tools::LogMsg("ERROR", "AnayzerSoft: Failed to load libcurl\n");
    return;  
  }
  if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
}
void AnalyzerSoft::ConfigPrices(json_object* config) {
  pPrices = PricesInterface::Config(pID, config);
}
void AnalyzerSoft::AccumulateAggreg(Params_t *dst, Params_t *src) {
  dst->Timestamp = src->Timestamp;
  //Si habia nan y entra nan      -> nada
  //Si habia nan y entra no nan   hay = nuevo
  //si no habia nan y entra nan   -> nada
  //Si no habia nan y entr no nan -> hay += nuevo
  
  for(uint i = 0; i < dst->RMS_V.size(); i++) {
    if(isnan(src->RMS_V[i])) continue;
    if(isnan(dst->RMS_V[i])) dst->RMS_V[i] = 0.0;
    dst->RMS_V[i] += pow(src->RMS_V[i], 2.0);
  }
  for(uint i = 0; i < dst->RMS_I.size(); i++) dst->RMS_I[i] += pow(src->RMS_I[i], 2.0);
  for(uint i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Active_Power[i] += src->Active_Power[i];
  for(uint i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Active_Energy[i] += src->Active_Energy[i];
  for(uint i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Reactive_Power[i] += src->Reactive_Power[i];
  for(uint i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) dst->Reactive_Energy[i] += src->Reactive_Energy[i];
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
void AnalyzerSoft::NormalizeAggreg(Params_t *params) {
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
size_t AnalyzerSoft::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlBuff_t *Buff = (CurlBuff_t*)userp;
  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
    if(Buff->Buffer == NULL) {
      return 0;
    } else {
      Buff->MaxSize = Buff->Size + size * nmemb + 1;
    }
  }
  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
  Buff->Size += size * nmemb;
  Buff->Buffer[Buff->Size] = 0;
  return size * nmemb;
}
void *AnalyzerSoft::Sync_Thread(void *args) {
  int Prio = sched_get_priority_min(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("WARN", "Cant raise thread priority -> %s\n", __func__);
  }
  AnalyzerSoft *This = (AnalyzerSoft*)args;
  pthread_setname_np(This->pSyncThread, "Sync");
  CurlBuff_t   CurlBuff;
  json_object *Find;
  size_t       JSONLen;
  long         HTTPCode;
  char         Tmp[1024];
  uint64_t     Last3S      = 0;
  uint64_t     Last1M      = 0;
  uint64_t     Last10M     = 0;
  uint64_t     Last1H      = 0;
  uint64_t     LastEvent   = 0;
  uint64_t     LastPrice   = 0;
  bool         Registered  = false;
  bool         Initialized = false;
  CURL        *Curl        = curl_easy_init();
  curl_slist  *Headers     = NULL;
  Database     DB(This->pID);
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1);
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
  while(!This->pTerminate) {
    if(Registered == false) {
      if(Headers == NULL) {
        curl_slist_free_all(Headers);
        Headers = curl_slist_append(Headers, "Accept: application/json");
        Headers = curl_slist_append(Headers, "Accept-Encoding: deflate");
        Headers = curl_slist_append(Headers, "Connection: keep-alive");
        Headers = curl_slist_append(Headers, "Content-Type: application/json");
        curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);
      }
      json_object *JSONWriter = json_object_new_object();
      json_object_object_add(JSONWriter, "Username", json_object_new_string(This->pSyncName));
      json_object_object_add(JSONWriter, "Password", json_object_new_string(This->pSyncPassword));
      json_object_object_add(JSONWriter, "UpdateLastLogin", json_object_new_boolean(false));
      CurlBuff.Size = 0;
      snprintf(Tmp, sizeof(Tmp), "%s/LogIn", This->pSyncServer);
      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
      CURLcode Code = curl_easy_perform(Curl);
      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
      if((Code != CURLE_OK) || (HTTPCode != 200)) {
        Tools::LogMsg("WARN", "AnalyzerRemote: failed to register on remote server\n");
      } else {
        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
          if((json_object_object_get_ex(JSONReader, "Result", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) {
            Registered = json_object_get_boolean(Find);
            if(Registered) Tools::LogMsg("INFO", "AnalyzerRemote: logged to remote server\n");
          }
          json_object_put(JSONReader);
        }
      }
      if(Registered == false) usleep(1000000);
      json_object_put(JSONWriter);
    } else if(Initialized == false) {
      snprintf(Tmp, sizeof(Tmp), "%s/setRemoteAnalyzer", This->pSyncServer);
      json_object *JSONWriter = This->GetJSONDescriptor();
      CurlBuff.Size = 0;
      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
      CURLcode Code = curl_easy_perform(Curl);
      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
      if((Code != CURLE_OK) || (HTTPCode != 200)) {
        Tools::LogMsg("WARN", "AnalyzerRemote: failed to config on remote server\n");
      } else {
        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
          if((json_object_object_get_ex(JSONReader, "aggreg_3s", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last3S = json_object_get_int64(Find);
          if((json_object_object_get_ex(JSONReader, "aggreg_1m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1M = json_object_get_int64(Find);
          if((json_object_object_get_ex(JSONReader, "aggreg_10m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last10M = json_object_get_int64(Find);
          if((json_object_object_get_ex(JSONReader, "aggreg_1h", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1H = json_object_get_int64(Find);
          if((json_object_object_get_ex(JSONReader, "events", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastEvent = json_object_get_int64(Find);
          if((json_object_object_get_ex(JSONReader, "prices", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastPrice = json_object_get_int64(Find);
          Tools::LogMsg("INFO", "AnalyzerRemote: configured on remote server\n");
          Initialized = true;
        }
      }
      json_object_put(JSONWriter);
      if(Initialized == false) usleep(1000000);
    } else {
      bool      Live  = false;
      char     *Table = (char*)"aggreg_3s";
      uint64_t *Time  = &Last3S;
      sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Table\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
      char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);
      if(Result == NULL) {
        Table = (char*)"aggreg_1m";
        Time  = &Last1M;
        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1m' AS \"Aggregation_Source\", * FROM (SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS aggreg_1m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
      }
      if(Result == NULL) {
        Table = (char*)"aggreg_10m";
        Time  = &Last10M;
        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_10m' AS \"Aggregation_Source\", * FROM aggreg_10m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
      }
      if(Result == NULL) {
        Table = (char*)"aggreg_10m";
        Time  = &Last1H;
        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1h' AS \"Aggregation_Source\", * FROM aggreg_1h WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
      }
      if(Result != NULL) {
        snprintf(Tmp, sizeof(Tmp), "%s/setRemoteData", This->pSyncServer);
        CurlBuff.Size = 0;
        curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
        curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, Result);
        curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
        CURLcode Code = curl_easy_perform(Curl);
        curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
        if((Code != CURLE_OK) || (HTTPCode != 200)) {
          Tools::LogMsg("WARN", "AnalyzerRemote: failed send data to server\n");
        } else {
          json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
          if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
            *Time = atol(DB.ResultValue(0, (char*)"sampletime"));
            if((json_object_object_get_ex(JSONReader, "Live", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Live = json_object_get_int64(Find);
            Tools::LogMsg("DEBUG", "AnalyzerRemote: sampletime %" PRIu64 " saved in remote '%s' (Live: %s)\n", *Time, Table, (Live ? "true" : "false"));
          }
        }
      } else {
        usleep(1000000);
      }
    }
  }
  curl_easy_cleanup(Curl);
  return NULL;
}