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
#include "Analyzer.h"
#include "Tools.h"
Analyzer::Analyzer(char *name, uint8_t voltageChannel, uint8_t currentChannel, uint32_t id, float samplerate, float minCurrent) : Database() {
  pType = PHASE_1;
//  pVoltageChannel = voltageChannel;
//  pCurrentChannel = currentChannel;
  Init(name, id, samplerate, minCurrent);
}
Analyzer::Analyzer(char *name, bool useNeutral, uint32_t id, float samplerate, float minCurrent) : Database() {
  pType = (useNeutral) ? PHASE_3N : PHASE_3;
  Init(name, id, samplerate, minCurrent);
}
void Analyzer::Init(char *name, uint32_t id, float samplerate, float minCurrent) {
  pZC             = new ZeroCrossing((pType == PHASE_1) ? false : true, samplerate);
  pName           = name;
  pID             = id;
  pSamples        = 0; 
  pSequence       = 0;
  pCyclesLen[0]   = 0;
  pCycles         = 0;
  pWorking        = false;
  pTerminate      = false;
  pSamplerate     = samplerate;
  pMinCurrent     = minCurrent;
  pMaxSamples     = ceil(12.0 * samplerate / Devices::GetNominalFrequency());
  pWaitingThreads = 0;
  pDataReady      = false;
  pPrevTimestamp  = 0;
  for(uint8_t i = 0; i < 3; i++) pVoltageSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  for(uint8_t i = 0; i < 4; i++) pCurrentSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  for(uint8_t i = 0; i < 3; i++) {
    char Name[16];
    sprintf(Name, "FFT_V%u", i);
    pVoltageFFT[i] = new FFT(pMaxSamples, Name);
  }
  for(uint8_t i = 0; i < 4; i++) {
    char Name[16];
    sprintf(Name, "FFT_I%u", i);
    pCurrentFFT[i] = new FFT(pMaxSamples, Name);
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
  ExecSentence("PREPARE SAVE_3S       AS INSERT INTO aggreg_3s (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");    
  ExecSentence("PREPARE SAVE_1M       AS INSERT INTO aggreg_1m (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");  
  ExecSentence("PREPARE SAVE_1M_EXT   AS INSERT INTO aggreg_ext_1m (node_ext, sampletime_ext, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) VALUES($1, $2, $3, $4, $5, $6, $7, $8)");  
  ExecSentence("PREPARE SAVE_10M      AS INSERT INTO aggreg_10m(node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");  
  ExecSentence("PREPARE SAVE_1H       AS INSERT INTO aggreg_1h (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");
  ExecSentence("PREPARE DELETE_1M_EXT AS DELETE FROM aggreg_ext_1m WHERE node_ext = $1 AND sampletime_ext < $2");
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
  pthread_setname_np(pThread, (pType == PHASE_1) ? "Analyzer_1P" : (pType == PHASE_3) ? "Analyzer_3P" : "Analyzer_3P+N");
}
Analyzer::~Analyzer() {
  //Libero todas las estructuras
}
bool Analyzer::SampleAppend(float *voltageSamples, float *currentSamples, uint64_t sampletime) {

  for(uint8_t i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) pVoltageSamples[i][pSamples] = voltageSamples[i];
  for(uint8_t i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) pCurrentSamples[i][pSamples] = currentSamples[i];
  if(pAggreg10C.Timestamp == 0) pAggreg10C.Timestamp = sampletime;
  pSamples++;
  if(pSamples == pMaxSamples) {
    pthread_mutex_lock(&pThreadMutex);
    pWorking = true;
    pthread_mutex_unlock(&pThreadMutex);
    pthread_cond_signal(&pThreadCond);
    return true;
  }
  return false;
}
void Analyzer::SaveAggreg(Params_t *params) {
  if(params->Type == AGGREG_3S)
    PrepareSentence("EXECUTE SAVE_3S(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1M)  
    PrepareSentence("EXECUTE SAVE_1M(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_10M)  
    PrepareSentence("EXECUTE SAVE_10M(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1H)  
    PrepareSentence("EXECUTE SAVE_1H(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt32(pID);                                                                                     //node
  BindSingleUInt64(params->Timestamp);                                                                       //sampletime
  BindArrayFloat(params->RMS_V, (pType == PHASE_1) ? 1 : 3);                                                 //rms_v 
  BindArrayFloat(params->RMS_I, (pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4);                        //rms_i
  BindArrayFloat(params->Active_Power, (pType == PHASE_1) ? 1 : 3);                                          //active_power
  BindArrayFloat(params->Active_Energy, (pType == PHASE_1) ? 1 : 3);                                         //active_energy
  BindArrayFloat(params->Fundamental_Reactive_Power, (pType == PHASE_1) ? 1 : 3);                            //fundamental_reactive_power
  BindArrayFloat(params->Fundamental_Reactive_Energy, (pType == PHASE_1) ? 1 : 3);                           //fundamental_reactive_energy
  BindSingleFloat(params->Frequency);                                                                        //frequency
  BindSingleBool(params->Flag);                                                                              //flaged
  ExecSentence();
  if(params->Type == AGGREG_1M) {
    PrepareSentence("EXECUTE SAVE_1M_EXT(?, ?, ?, ?, ?, ?, ?, ?)");
    BindSingleUInt32(pID);                                                                                   //node
    BindSingleUInt64(params->Timestamp);                                                                     //sampletime
    BindArrayFloat(params->Power_Factor, (pType == PHASE_1) ? 1 : 3);                                        //power_factor
    BindArrayFloat(params->THD_V, (pType == PHASE_1) ? 1 : 3);                                               //thd_v
    BindArrayFloat(params->THD_I, (pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4);                      //thd_i
    BindArrayFloat(params->Phi, (pType == PHASE_1) ? 1 : 3);                                                 //phi
    BindArrayArrayFloat((float*)params->VoltageHarmonics, (pType == PHASE_1) ? 1 : 3, 50);                           //harmonics_v
    BindArrayArrayFloat((float*)params->CurrentHarmonics, (pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4, 50);  //harmonics_i
    ExecSentence();
    PrepareSentence("EXECUTE DELETE_1M_EXT(?, ?)");
    BindSingleUInt32(pID);                                                                                   //node
    BindSingleUInt64(params->Timestamp - 86400000);                                                          //sampletime
    ExecSentence();
  }
  ResetAggreg(params);
}
void Analyzer::AccumulateAggreg(Params_t *dst, Params_t *src) {
  dst->Timestamp = src->Timestamp;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->RMS_V[i] += pow(src->RMS_V[i], 2.0);
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) dst->RMS_I[i] += pow(src->RMS_I[i], 2.0);
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Active_Power[i] += src->Active_Power[i];
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Active_Energy[i] += src->Active_Energy[i];
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Fundamental_Reactive_Power[i] += src->Fundamental_Reactive_Power[i];
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Fundamental_Reactive_Energy[i] += src->Fundamental_Reactive_Energy[i];
  dst->Flag |= src->Flag;
  dst->Frequency += src->Frequency;
  if(dst->Type == AGGREG_1M) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Power_Factor[i] += src->Power_Factor[i];
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) dst->THD_I[i] += pow(src->THD_I[i], 2.0);
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) {
//      for(int j = 0; j < 50; j++) dst->CurrentHarmonics[j * 50 + i] += pow(src->CurrentHarmonics[j * 50 + i], 2.0);
    }
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->Phi[i] += src->Phi[i];
  }
  if((dst->Type == AGGREG_1M) || (dst->Type == AGGREG_10M)) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) dst->THD_V[i] += pow(src->THD_V[i], 2.0);
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) {
//      for(int j = 0; j < 50; j++) dst->VoltageHarmonics[i][j] += pow(src->VoltageHarmonics[i][j], 2.0);
    }
  }
  dst->Samples++;
}
void Analyzer::NormalizeAggreg(Params_t *params) {
  if(params->Samples > 1) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->RMS_V[i] = sqrt(params->RMS_V[i] / params->Samples);
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) params->RMS_I[i] = sqrt(params->RMS_I[i] / params->Samples);
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Active_Power[i] = params->Active_Power[i] / params->Samples;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Reactive_Power[i] = params->Fundamental_Reactive_Power[i] / params->Samples;
    params->Frequency = params->Frequency / params->Samples;
    if(params->Type == AGGREG_1M) {
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Power_Factor[i] = params->Power_Factor[i] / params->Samples;
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->THD_I[i] = sqrt(params->THD_I[i] / params->Samples);
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Phi[i] = params->Phi[i] / params->Samples;
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) {
//        for(int j = 0; j < 50; j++) params->CurrentHarmonics[i][j] = sqrt(params->CurrentHarmonics[i][j] / params->Samples);
      }
    }
    if((params->Type == AGGREG_1M) || (params->Type == AGGREG_1M)) {
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->THD_V[i] = sqrt(params->THD_V[i] / params->Samples);
      for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) {
//        for(int j = 0; j < 50; j++) params->VoltageHarmonics[i][j] = sqrt(params->VoltageHarmonics[i][j] / params->Samples);
      }    
    }
  }
}
void Analyzer::ResetAggreg(Params_t *params) {
  params->Timestamp = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->RMS_V[i] = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) params->RMS_I[i] = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Active_Power[i] = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Reactive_Power[i] = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Reactive_Energy[i] = 0;
  for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Active_Energy[i] = 0;
  params->Frequency = 0;
  params->Flag = false;
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M)) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->THD_I[i] = 0;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Phi[i] = 0;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Power_Factor[i] = 0; 
    memset(params->CurrentHarmonics, 0, sizeof(params->CurrentHarmonics));
  }
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M) || (params->Type == AGGREG_10M)) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->THD_V[i] = 0;
    memset(params->VoltageHarmonics, 0, sizeof(params->VoltageHarmonics));
  }
  if(params->Type == AGGREG_10C) {
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Active_Power[i] = 0;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Apparent_Power[i] = 0;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Apparent_Power[i] = 0;
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) params->Fundamental_Power_Factor[i] = 0;
  }
  params->Samples = 0;
}
void *Analyzer::Analyzer_Thread(void *args) {
  ((Analyzer*)args)->Analize();
  return NULL;
}
void Analyzer::Analize() {
  while(true) {
    pthread_mutex_lock(&pThreadMutex);
    while(pWorking == false) {
      if(pTerminate == true) return;
      pthread_cond_wait(&pThreadCond, &pThreadMutex);
    }
    uint64_t Start = Tools::GetTime();
    pthread_mutex_unlock(&pThreadMutex);
    
    
    

    
//// ceil(12.0 * pSamplerate / Devices::GetNominalFrequency());
//  pSamples = 0;
//  pVoltageSamples = voltageSamples;
//  pVoltageFFT     = voltageFFT;
//  pCurrentSamples = currentSamples;
//  pCurrentFFT     = currentFFT;
//  if(cycles == 20) {
//    for(int i = 0; i < cycles; i++) {
//      pPrevCyclesLenght = pPrevCyclesLenght - pPrevCycles[pPrevCyclesPos];
//      pPrevCycles[pPrevCyclesPos] = cyclesLen[i];
//      pPrevCyclesLenght = pPrevCyclesLenght + pPrevCycles[pPrevCyclesPos];
//      pPrevCyclesPos = (pPrevCyclesPos + 1) % ANALYZER_FREQCYCLES;
//    }
//    pAggreg10C.Frequency = ANALYZER_FREQCYCLES / (pPrevCyclesLenght / pSamplerate) / 2.0;
//  } else {
//    for(uint8_t i = 0; i < cycles; i++) pSamples = pSamples + cyclesLen[i];
//    pAggreg10C.Frequency = NAN;
//  }

////  pAggreg10C.Flag      = EventsDetect();
//  
    
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) pVoltageFFT[i]->Start(pVoltageSamples[i], pSamples);
    for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) pCurrentFFT[i]->Start(pCurrentSamples[i], pSamples);
    //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
    for(uint32_t i = 0; i < pSamples; i++) {
      for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] += pow(pVoltageSamples[j][i], 2.0);
      for(int j = 0; j < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] += pow(pCurrentSamples[j][i], 2.0);
      for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] += pVoltageSamples[j][i] * pCurrentSamples[j][i];
    }
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) pAggreg10C.RMS_V[j] = sqrt(pAggreg10C.RMS_V[j] / pSamples);
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) pAggreg10C.Active_Power[j] = pAggreg10C.Active_Power[j] / pSamples;
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); j++) pAggreg10C.RMS_I[j] = sqrt(pAggreg10C.RMS_I[j] / pSamples);
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) pAggreg10C.Apparent_Power[j] = pAggreg10C.RMS_V[j] * pAggreg10C.RMS_I[j];
    //Calculo harmonicos de tension
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) {
      pVoltageFFT[j]->Wait();
      for(int i = 0; i < 50; i++) {
        pAggreg10C.VoltageHarmonics[j][i] = pVoltageFFT[j]->BinPower(10 + i * 10);
        if(i > 0) pAggreg10C.THD_V[j] += pow(pAggreg10C.VoltageHarmonics[j][i], 2.0);
      }
      pAggreg10C.THD_V[j] = sqrt(pAggreg10C.THD_V[j]) / pAggreg10C.VoltageHarmonics[j][0];
    }
    for(int j = 0; j < ((pType == PHASE_1) ? 1 : 3); j++) {
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
    if(pType == PHASE_3N) {
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
//    if(pAggreg3S.Flag == false) {
//////      float Variation = PercentChange(pAggreg3S.Frequency, Tools::Electrical.NominalFrequency);
////      for(uint Range = 1; Range < (sizeof(FrequencyRanges) / sizeof(float)); Range++) {
//////        if((Variation >= FrequencyRanges[Range - 1]) && (Variation < FrequencyRanges[Range])) pStats.Frequency[Range - 1]++;
////      }
//    }
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
////    if(pAggreg10M.Flag == false) {
////      for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
//////        float Variation = PercentChange(pAggreg10M.RMS_V[i], Tools::Electrical.NominalVoltage);
////        for(uint Range = 1; Range < (sizeof(VoltageRanges) / sizeof(float)); Range++) {
//////          if((Variation >= VoltageRanges[Range - 1]) && (Variation < VoltageRanges[Range])) pStats.Voltage[Range - 1]++;
////        }
//////        Variation = pAggreg10M.THD_V[i];
////        for(uint Range = 1; Range < (sizeof(THDRanges) / sizeof(float)); Range++) {
//////          if((Variation >= THDRanges[Range - 1]) && (Variation < THDRanges[Range])) pStats.THD[Range - 1]++;
////        }
////        bool HarmonicsInRange = true;
////        for(int n = 0; n < 25; n++) {
//////          Variation = PercentChange(pAggreg10M.VoltageHarmonics[i][n + 1], Tools::Electrical.NominalVoltage);
//////          if(Variation > HarmonicsAllowed[n]) {
//////            HarmonicsInRange = false;
//////            break;
//////          }
////        }
////        if(HarmonicsInRange == true) {
////          pStats.Harmonics[0]++;
////        } else {
////          pStats.Harmonics[1]++;
////        }
////      }
////    }
      SaveAggreg(&pAggreg10M);
    }
    AccumulateAggreg(&pAggreg10M, &pAggreg10C);
    if(((pAggreg10C.Timestamp / 3600000) > (pPrevTimestamp / 3600000)) && (pPrevTimestamp != 0)) {
      Tools::LogMsg("DEBUG", "Storing 1H aggregation\n");
      pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
      NormalizeAggreg(&pAggreg1H);
      SaveAggreg(&pAggreg1H);
////    ResetStats();
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
    pSequence++;
    pthread_mutex_unlock(&pThreadMutex);
  }
}
json_object *Analyzer::GetJSONCopy(char *series) {
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
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.RMS_V[i]));
      } else if(strcmp(Serie, "rms_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.RMS_I[i]));
      } else if(strcmp(Serie, "active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Active_Power[i]));
      } else if(strcmp(Serie, "apparent_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Apparent_Power[i]));
      } else if(strcmp(Serie, "power_factor") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Power_Factor[i]));
      } else if(strcmp(Serie, "active_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Active_Energy[i]));
      } else if(strcmp(Serie, "fundamental_active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Active_Power[i]));
      } else if(strcmp(Serie, "fundamental_reactive_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Reactive_Power[i]));
      } else if(strcmp(Serie, "fundamental_apparent_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Apparent_Power[i]));
      } else if(strcmp(Serie, "fundamental_power_factor") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Power_Factor[i]));
      } else if(strcmp(Serie, "fundamental_reactive_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Fundamental_Reactive_Energy[i]));
      } else if(strcmp(Serie, "thd_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.THD_V[i]));
      } else if(strcmp(Serie, "thd_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.THD_I[i]));
      } else if(strcmp(Serie, "freq") == 0) {
        if(isnan(pAggreg10C.Frequency) == false) Obj = json_object_new_double(pAggreg10C.Frequency);
      } else if(strcmp(Serie, "phi") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) json_object_array_add(Obj, json_object_new_double(pAggreg10C.Phi[i]));
      } else if(strcmp(Serie, "flag") == 0) {
        Obj = json_object_new_boolean(pAggreg10C.Flag);
      } else if(strcmp(Serie, "harmonics_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pAggreg10C.VoltageHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "harmonics_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pAggreg10C.CurrentHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_v") == 0) {
       Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pSamples; j++) json_object_array_add(Obj2, json_object_new_double(pCurrentSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : 3); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < pVoltageFFT[i]->OutputLen; j++) json_object_array_add(Obj2, json_object_new_double(pVoltageFFT[i]->BinPower(j)));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < ((pType == PHASE_1) ? 1 : (pType == PHASE_3) ? 3 : 4); i++) {
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
bool Analyzer::IsFinished() {
  bool Result;
  pthread_mutex_lock(&pThreadMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pThreadMutex);
  return Result;
}
char *Analyzer::GetTypeString() {
  if(pType == PHASE_3N) return (char*)"3Phase";
  if(pType == PHASE_3)  return (char*)"3Phase";
  if(pType == PHASE_1)  return (char*)"1Phase";
}
uint32_t Analyzer::GetID() {
  return pID;
}
char *Analyzer::GetName() {
  return pName;
}