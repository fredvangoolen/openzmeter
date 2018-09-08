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
  pVoltageChannels = (mode == PHASE1) ? 1 : 3;
  pCurrentChannels = (mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4;
  pMaxSamples      = ceil(15.0 * samplerate / pNominalFrequency);
  pZC              = new ZeroCrossing((pMode == PHASE1) ? 1 : 3, samplerate, pNominalFrequency);
  pSyncName        = NULL;
  pSyncPassword    = NULL;
  pSyncServer      = NULL;
  pSyncThread      = 0;
  pSequence        = 0;
  pCycles          = 0;
  pWorking         = false;
  pTerminate       = false;
  pSamplerate      = samplerate;
  pWaitingThreads  = 0;
  pDataReady       = false;
  pPrevTimestamp   = 0;
  for(uint8_t i = 0; i < 20; i++) pCyclesLen[i] = 0;
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
    char Name[16];
    sprintf(Name, "FFT_V%u", i);
    pVoltageFFT[i] = new FFT(pMaxSamples, Name);
    pVoltageSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  }
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
    char Name[16];
    sprintf(Name, "FFT_I%u", i);
    pCurrentFFT[i] = new FFT(pMaxSamples, Name);
    pCurrentSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  }
  pAggreg10C.Type = AGGREG_10C;
  pAggreg3S.Type  = AGGREG_3S;
  pAggreg1M.Type  = AGGREG_1M;
  pAggreg10M.Type = AGGREG_10M;
  pAggreg1H.Type  = AGGREG_1H;
  ResetAggreg(&pAggreg10C);
  ResetAggreg(&pAggreg3S);
  ResetAggreg(&pAggreg1M);
  ResetAggreg(&pAggreg10M);
  ResetAggreg(&pAggreg1H);
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
  delete pZC;  
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) delete pVoltageFFT[i];
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) delete pCurrentFFT[i];
}
float AnalyzerSoft::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples, uint64_t sampletime) {
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) pVoltageSamples[i][pAggreg10C.Samples] = voltageSamples[i];
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) pCurrentSamples[i][pAggreg10C.Samples] = currentSamples[i];
  pAggreg10C.Samples++;
  pCyclesLen[pCycles]++;
  if(pZC->ProcessSample(voltageSamples) == true) pCycles++;
  if((pCycles == 20) || (pAggreg10C.Samples == pMaxSamples)) {
    pthread_mutex_lock(&pThreadMutex);
    pWorking = true;
    pthread_mutex_unlock(&pThreadMutex);
    pthread_cond_signal(&pThreadCond);
    return true;
  }
//////  pAggreg10C.Flag      = EventsDetect();
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
    pAggreg10C.Frequency = pZC->Frequency();
    //Inicia FFTs
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) pVoltageFFT[i]->Start(pVoltageSamples[i], pAggreg10C.Samples);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) pCurrentFFT[i]->Start(pCurrentSamples[i], pAggreg10C.Samples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    for(uint32_t i = 0; i < pAggreg10C.Samples; i++) {
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] += pow(pVoltageSamples[j][i], 2.0);
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] += pow(pCurrentSamples[j][i], 2.0);
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] += pVoltageSamples[j][i] * pCurrentSamples[j][i];
    }
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] = sqrt(pAggreg10C.RMS_V[j] / pAggreg10C.Samples);
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Samples;
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] = sqrt(pAggreg10C.RMS_I[j] / pAggreg10C.Samples);
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.Apparent_Power[j] = pAggreg10C.RMS_V[j] * pAggreg10C.RMS_I[j];
    //Calculo harmonicos de tension
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
      pVoltageFFT[j]->Wait();
      for(int i = 0; i < 50; i++) {
        pAggreg10C.VoltageHarmonics[j][i] = pVoltageFFT[j]->BinPower(10 + i * 10);
        if(i > 0) pAggreg10C.THD_V[j] += pow(pAggreg10C.VoltageHarmonics[j][i], 2.0);
      }
      pAggreg10C.THD_V[j] = sqrt(pAggreg10C.THD_V[j]) / pAggreg10C.VoltageHarmonics[j][0];
    }
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
      pCurrentFFT[j]->Wait();
      if(pAggreg10C.RMS_I[j] < pMinCurrent) continue;
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
        for(int i = 0; i < pVoltageChannels; i++) {
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
    pCycles = 0;
    for(uint8_t i = 0; i < 20; i++) pCyclesLen[i] = 0;
    pSequence++;
    pthread_mutex_unlock(&pThreadMutex);
  }
}
void AnalyzerSoft::GetParams(Params_t *params, ExtParams_t *extParams) {
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads++;
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  json_object *Result = NULL;
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
    memcpy(params, &pAggreg10C, sizeof(Params_t));
    if(extParams) {
      memset(extParams, 0, sizeof(ExtParams_t));
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) {
        extParams->Voltage[j] = (float*)malloc(sizeof(float) * params->Samples);
        for(uint32_t i = 0; i < params->Samples; i++) extParams->Voltage[j][i] = pVoltageSamples[j][i];
        extParams->FFT_V[j] = (float*)malloc(sizeof(float) * pVoltageFFT[j]->OutputLen);
        for(uint32_t i = 0; i < pVoltageFFT[j]->OutputLen; i++) extParams->FFT_V[j][i] = pVoltageFFT[j]->BinPower(i);
      }
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) {
        extParams->Current[j] = (float*)malloc(sizeof(float) * params->Samples);
        memcpy(extParams->Current[j], pCurrentSamples[j], sizeof(float) * params->Samples);
        
        extParams->FFT_I[j] = (float*)malloc(sizeof(float) * pCurrentFFT[j]->OutputLen);
        for(uint32_t i = 0; i < pCurrentFFT[j]->OutputLen; i++) extParams->FFT_I[j][i] = pCurrentFFT[j]->BinPower(i);
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
void AnalyzerSoft::AccumulateAggreg(Params_t *dst, Params_t *src) {
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
      bool      Live    = false;
      char     *Table   = (char*)"aggreg_3s";
      uint64_t *Time    = &Last3S;
      sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Table\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", Last3S);
      char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);
//      if(Result == NULL) {
//        char     *Table = (char*)"aggreg_1m";
//        uint64_t *Time  = &Last1M;
//        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Aggregation_Source\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", Last3S);
//        char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result == NULL) {
//        char     *Table = (char*)"aggreg_1m";
//        uint64_t *Time  = &Last1M;
//        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Aggregation_Source\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", Last3S);
//        char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
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
            *Time = atol(DB.ResultValue(0, "sampletime"));
            if((json_object_object_get_ex(JSONReader, "Live", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Live = json_object_get_int64(Find);
            Tools::LogMsg("DEBUG", "AnalyzerRemote: sampletime %" PRIu64 " saved in remote '%s' (Live: %s)\n", *Time, Table, (Live ? "true" : "false"));
          }
        }
      }
      usleep(1000000);
      
    }
  }
  curl_easy_cleanup(Curl);
  return NULL;
}
//result = sendaggreg(tipoAggreg)