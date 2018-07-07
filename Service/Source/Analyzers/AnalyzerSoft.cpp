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
AnalyzerSoft::AnalyzerSoft(float samplerate, char *id, float nominal_voltage, float nominal_frequency, char *name, float max_current, float min_current, AnalyzerInterface::AnalyzerMode mode) : AnalyzerInterface(id, nominal_voltage, nominal_frequency, name, max_current, min_current, mode) {
  pVoltageChannels   = (mode == PHASE1) ? 1 : 3;
  pCurrentChannels   = (mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4;
  
  
  
  
  pMaxSamples        = ceil(15.0 * samplerate / pNominalFrequency);


  
  pZC             = new ZeroCrossing((pMode == PHASE1) ? 1 : 3, samplerate, pNominalFrequency);
  pStats          = new Stats((pMode != PHASE1) ? 1 : 3, this, pNominalVoltage, pNominalFrequency);
  pSamples        = 0; 
  pSequence       = 0;
  pCycles         = 0;
  pWorking        = false;
  pTerminate      = false;
  pSamplerate     = samplerate;
  
  pWaitingThreads = 0;
  pDataReady      = false;
  pPrevTimestamp  = 0;
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
  delete pZC;
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) delete pVoltageFFT[i];
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) delete pCurrentFFT[i];
}
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples, uint64_t sampletime) {
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) pVoltageSamples[i][pSamples] = voltageSamples[i];
  for(uint8_t i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) pCurrentSamples[i][pSamples] = currentSamples[i];
  pSamples++;
  pCyclesLen[pCycles]++;
  if(pZC->ProcessSample(voltageSamples) == true) pCycles++;
  if((pCycles == 20) || (pSamples == pMaxSamples)) {
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
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) pVoltageFFT[i]->Start(pVoltageSamples[i], pSamples);
    for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) pCurrentFFT[i]->Start(pCurrentSamples[i], pSamples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    for(uint32_t i = 0; i < pSamples; i++) {
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] += pow(pVoltageSamples[j][i], 2.0);
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] += pow(pCurrentSamples[j][i], 2.0);
      for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] += pVoltageSamples[j][i] * pCurrentSamples[j][i];
    }
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] = sqrt(pAggreg10C.RMS_V[j] / pSamples);
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] = pAggreg10C.Active_Power[j] / pSamples;
    for(int j = 0; j < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] = sqrt(pAggreg10C.RMS_I[j] / pSamples);
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
          pAggreg10C.Fundamental_Active_Power[j]   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          pAggreg10C.Fundamental_Reactive_Power[j] = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          pAggreg10C.Fundamental_Apparent_Power[j] = VoltageHarmonic * CurrentHarmonic;
          pAggreg10C.Fundamental_Power_Factor[j]   = pAggreg10C.Fundamental_Active_Power[j] / pAggreg10C.Fundamental_Apparent_Power[j];
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          pAggreg10C.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
      pAggreg10C.THD_I[j] = sqrt(pAggreg10C.THD_I[j]) / pAggreg10C.CurrentHarmonics[j][0];
      pAggreg10C.Active_Energy[j] = (pAggreg10C.Active_Power[j] * pSamples) / (pSamplerate * 3600.0);
      pAggreg10C.Power_Factor[j] = pAggreg10C.Active_Power[j] / pAggreg10C.Apparent_Power[j];
      pAggreg10C.Fundamental_Reactive_Energy[j] = (pAggreg10C.Fundamental_Reactive_Power[j] * pSamples) / (pSamplerate * 3600.0);
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
      pStats->AppendFreq(pAggreg3S.Frequency, pAggreg3S.Flag);
      SaveAggreg(&pAggreg3S);
    }
    AccumulateAggreg(&pAggreg3S, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 60000) > (pPrevTimestamp / 60000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 1M aggregation\n");
      pAggreg1M.Timestamp = pAggreg1M.Timestamp / 60000 * 60000 + 59999;
      NormalizeAggreg(&pAggreg1M);
      SaveAggreg(&pAggreg1M);
    }
    AccumulateAggreg(&pAggreg1M, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 600000) > (pPrevTimestamp / 600000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 10M aggregation\n");
      pAggreg10M.Timestamp = pAggreg10M.Timestamp / 600000 * 600000 + 599999;
      NormalizeAggreg(&pAggreg10M);
      pStats->AppendVoltage(pAggreg10M.RMS_V, pAggreg10M.Flag);
      pStats->AppendHarmonics((float*)pAggreg10M.VoltageHarmonics, pAggreg10M.Flag);
      pStats->AppendTHD(pAggreg10M.THD_V, pAggreg10M.Flag);
      SaveAggreg(&pAggreg10M);
    }
    AccumulateAggreg(&pAggreg10M, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 3600000) > (pPrevTimestamp / 3600000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 1H aggregation\n");
      pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      NormalizeAggreg(&pAggreg1H);
      SaveAggreg(&pAggreg1H);
      pStats->SaveStats(pAggreg1H.Timestamp);
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
    pSamples = 0;
    pCycles = 0;
    for(uint8_t i = 0; i < 20; i++) pCyclesLen[i] = 0;
    pSequence++;
    pthread_mutex_unlock(&pThreadMutex);
  }
}
json_object *AnalyzerSoft::GetJSONCopy(char *series) {
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
    Result = json_object_new_object();
    char *Save;
    char *Serie = strtok_r(series, ", ", &Save);
    while(Serie != NULL) {
      json_object *Obj = NULL;
      if(strcmp(Serie, "rms_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.RMS_V[i]));
      } else if(strcmp(Serie, "rms_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.RMS_I[i]));
      } else if(strcmp(Serie, "active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Active_Power[i]));
      } else if(strcmp(Serie, "apparent_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Apparent_Power[i]));
      } else if(strcmp(Serie, "power_factor") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Power_Factor[i]));
      } else if(strcmp(Serie, "active_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Active_Energy[i]));
      } else if(strcmp(Serie, "fundamental_active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Active_Power[i]));
      } else if(strcmp(Serie, "fundamental_reactive_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Reactive_Power[i]));
      } else if(strcmp(Serie, "fundamental_apparent_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Apparent_Power[i]));
      } else if(strcmp(Serie, "fundamental_power_factor") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Power_Factor[i]));
      } else if(strcmp(Serie, "fundamental_reactive_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Reactive_Energy[i]));
      } else if(strcmp(Serie, "thd_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.THD_V[i]));
      } else if(strcmp(Serie, "thd_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.THD_I[i]));
      } else if(strcmp(Serie, "freq") == 0) {
        if(isnan(pAggreg10C.Frequency) == false) Obj = json_object_new_double(pAggreg10C.Frequency);
      } else if(strcmp(Serie, "phi") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Phi[i]));
      } else if(strcmp(Serie, "flag") == 0) {
        Obj = json_object_new_boolean(pAggreg10C.Flag);
      } else if(strcmp(Serie, "harmonics_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pAggreg10C.VoltageHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "harmonics_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pAggreg10C.CurrentHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_v") == 0) {
       Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pCurrentSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < pVoltageFFT[i]->OutputLen; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageFFT[i]->BinPower(j)));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < pCurrentFFT[i]->OutputLen; j++) json_object_array_add(Obj2, json_object_new_double(pCurrentFFT[i]->BinPower(j)));
          json_object_array_add(Obj, Obj2);
        }
      }
      if(Obj != NULL) json_object_object_add(Result, Serie, Obj);
      Serie = strtok_r(NULL, ", ", &Save);
    }
    json_object_object_add(Result, "Sampletime", json_object_new_int64(pAggreg10C.Timestamp));
  }
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsCond);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  return Result;
}
bool AnalyzerSoft::IsFinished() {
  bool Result;
  pthread_mutex_lock(&pThreadMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pThreadMutex);
  return Result;
}
//char *AnalyzerSoft::GetTypeString() {
//  if(pMode == PHASE3N) return (char*)"3PhaseN";
//  if(pMode == PHASE3)  return (char*)"3Phase";
//  if(pMode == PHASE1)  return (char*)"1Phase";
//}
//char *AnalyzerSoft::GetID() {
//  return pID;
//}
//char *AnalyzerSoft::GetName() {
//  return pName;
//}