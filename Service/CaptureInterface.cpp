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
#include "CaptureInterface.h"
#include "Tools.h"
pthread_mutex_t CaptureInterface::pFFTPlannerMutex = PTHREAD_MUTEX_INITIALIZER;
const float CaptureInterface::FrequencyRanges[]  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
const float CaptureInterface::VoltageRanges[]    = {-INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY};
const float CaptureInterface::THDRanges[]        = {0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY};
const float CaptureInterface::HarmonicsAllowed[] = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
void CaptureInterface::SaveWorkParams() {
  pthread_mutex_lock(&pDataReadyMutex);
  pDataReady = true;
  pthread_cond_broadcast(&pDataReadyWait);
  pthread_mutex_unlock(&pDataReadyMutex);

  if(((pWork.SequenceID % 15) == 0) && (pPrevTimestamp != 0)) {
    Tools::LogMsg("DEBUG", "Storing 3S aggregation\n");
    if(pAggreg3S.Flag == false) {
      float Variation = PercentChange(pAggreg3S.Frequency, Tools::Electrical.NominalFrequency);
      for(uint Range = 1; Range < (sizeof(FrequencyRanges) / sizeof(float)); Range++) {
        if((Variation >= FrequencyRanges[Range - 1]) && (Variation < FrequencyRanges[Range])) pStats.Frequency[Range - 1]++;
      }
    }
    SaveParams(&pAggreg3S);
  }
  AccumulateAggregation(&pAggreg3S, &pWork.Params);

  if(((pWork.Params.Timestamp / 60000) > (pPrevTimestamp / 60000)) && (pPrevTimestamp != 0)) {
    Tools::LogMsg("DEBUG", "Storing 1M aggregation\n");
    pAggreg1M.Timestamp = pAggreg1M.Timestamp / 60000 * 60000 + 59999;
    SaveParams(&pAggreg1M);
  }
  AccumulateAggregation(&pAggreg1M, &pWork.Params);    

  if(((pWork.Params.Timestamp / 600000) > (pPrevTimestamp / 600000)) && (pPrevTimestamp != 0)) {
    Tools::LogMsg("DEBUG", "Storing 10M aggregation\n");
    pAggreg10M.Timestamp = pAggreg10M.Timestamp / 600000 * 600000 + 599999;
    if(pAggreg10M.Flag == false) {
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
        float Variation = PercentChange(pAggreg10M.RMS_V[i], Tools::Electrical.NominalVoltage);
        for(uint Range = 1; Range < (sizeof(VoltageRanges) / sizeof(float)); Range++) {
          if((Variation >= VoltageRanges[Range - 1]) && (Variation < VoltageRanges[Range])) pStats.Voltage[Range - 1]++;
        }
        Variation = PercentChange(pAggreg10M.THD_V[i], 0);
        for(uint Range = 1; Range < (sizeof(THDRanges) / sizeof(float)); Range++) {
          if((Variation >= THDRanges[Range - 1]) && (Variation < THDRanges[Range])) pStats.THD[Range - 1]++;
        }
        bool HarmonicsInRange = true;
        for(int n = 0; n < 25; n++) {
          Variation = PercentChange(pAggreg10M.VoltageHarmonics[i][n + 1], 0);
          if(Variation > HarmonicsAllowed[n]) {
            HarmonicsInRange = false;
            break;
          }
        }
        if(HarmonicsInRange == true) {
          pStats.Harmonics[0]++;
        } else {
          pStats.Harmonics[1]++;
        }
      }
    }
    SaveParams(&pAggreg10M);
  }
  AccumulateAggregation(&pAggreg10M, &pWork.Params);
  
  if(((pWork.Params.Timestamp / 3600000) > (pPrevTimestamp / 3600000)) && (pPrevTimestamp != 0)) {
    Tools::LogMsg("DEBUG", "Storing 1H aggregation\n");
    pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
    SaveParams(&pAggreg1H);
    ResetStats();
  }
  AccumulateAggregation(&pAggreg1H, &pWork.Params);
  
  pPrevTimestamp = pWork.Params.Timestamp;
  pthread_mutex_lock(&pWaitingThreadsMutex);
  while(pWaitingThreads != 0) {
    pthread_cond_wait(&pWaitingThreadsWait, &pWaitingThreadsMutex);
  }
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  pthread_mutex_lock(&pDataReadyMutex);
  pDataReady = false;
  pthread_mutex_unlock(&pDataReadyMutex);
  ResetWork();
  ResetParams(&pWork.Params);
}
void CaptureInterface::AccumulateAggregation(Params_t *dst, Params_t *src) {
  dst->Timestamp = src->Timestamp;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->RMS_V[i] += pow(src->RMS_V[i], 2.0);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) dst->RMS_I[i] += pow(src->RMS_I[i], 2.0);
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Active_Power[i] += src->Active_Power[i];
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Active_Energy[i] += src->Active_Energy[i];
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Fundamental_Reactive_Power[i] += src->Fundamental_Reactive_Power[i];
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Fundamental_Reactive_Energy[i] += src->Fundamental_Reactive_Energy[i];
  dst->Flag |= src->Flag;
  dst->Frequency += src->Frequency;
  if(dst->Type == AGGREG_1M) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Power_Factor[i] += src->Power_Factor[i];
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->THD_I[i] += pow(src->THD_I[i], 2.0);
    for(int i = 0; i < (pTriphase ? 4 : 1); i++) {
      for(int j = 0; j < 50; j++) dst->CurrentHarmonics[i][j] += pow(src->CurrentHarmonics[i][j], 2.0);
    }
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->Phi[i] += src->Phi[i];
  }
  if((dst->Type == AGGREG_1M) || (dst->Type == AGGREG_10M)) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) dst->THD_V[i] += pow(src->THD_V[i], 2.0);
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
      for(int j = 0; j < 50; j++) dst->VoltageHarmonics[i][j] += pow(src->VoltageHarmonics[i][j], 2.0);
    }
  }
  dst->Samples++;
}
void CaptureInterface::SaveParams(Params_t *params) {
  if(params->Samples > 1) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->RMS_V[i] = sqrt(params->RMS_V[i] / params->Samples);
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->RMS_I[i] = sqrt(params->RMS_I[i] / params->Samples);
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Active_Power[i] = params->Active_Power[i] / params->Samples;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Reactive_Power[i] = params->Fundamental_Reactive_Power[i] / params->Samples;
    params->Frequency = params->Frequency / params->Samples;
    if(params->Type == AGGREG_1M) {
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Power_Factor[i] = params->Power_Factor[i] / params->Samples;
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_V[i] = sqrt(params->THD_V[i] / params->Samples);
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_I[i] = sqrt(params->THD_I[i] / params->Samples);
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Phi[i] = params->Phi[i] / params->Samples;
      for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
        for(int j = 0; j < 50; j++) params->VoltageHarmonics[i][j] = sqrt(params->VoltageHarmonics[i][j] / params->Samples);
      }
      for(int i = 0; i < (pTriphase ? 4 : 1); i++) {
        for(int j = 0; j < 50; j++) params->CurrentHarmonics[i][j] = sqrt(params->CurrentHarmonics[i][j] / params->Samples);
      }
    }
  }
  if(params->Type == AGGREG_3S)  
    PrepareSentence("EXECUTE SAVE_3S(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1M)  
    PrepareSentence("EXECUTE SAVE_1M(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_10M)  
    PrepareSentence("EXECUTE SAVE_10M(?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  else if(params->Type == AGGREG_1H)  
    PrepareSentence("EXECUTE SAVE_1H(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
  BindSingleUInt32(pNodeIndex);                                                    //node
  BindSingleUInt64(params->Timestamp);                                             //sampletime
  BindArrayFloat(params->RMS_V, (pTriphase == true) ? 3 : 1);                      //rms_v 
  BindArrayFloat(params->RMS_I, (pTriphase == true) ? 4 : 1);                      //rms_i
  BindArrayFloat(params->Active_Power, (pTriphase == true) ? 3 : 1);               //active_power
  BindArrayFloat(params->Active_Energy, (pTriphase == true) ? 3 : 1);              //active_energy
  BindArrayFloat(params->Fundamental_Reactive_Power, (pTriphase == true) ? 3 : 1); //fundamental_reactive_power
  BindArrayFloat(params->Fundamental_Reactive_Energy, (pTriphase == true) ? 3 : 1);//fundamental_reactive_energy
  BindSingleFloat(params->Frequency);                                              //frequency
  BindSingleBool(params->Flag);                                                    //flaged
  if(params->Type == AGGREG_1H) {
    BindArrayUInt16(pStats.Voltage, sizeof(VoltageRanges) / sizeof(float) - 1);     //estadisticas de tension
    BindArrayUInt16(pStats.Frequency, sizeof(FrequencyRanges) / sizeof(float) - 1); //estadisticas de frequencia
    BindArrayUInt16(pStats.THD, sizeof(THDRanges) / sizeof(float) - 1);             //estadisticas de distorsion harmonica
    BindArrayUInt16(pStats.Harmonics, 2);                                           //estadisticas de harmonicos en rango
  }
  ExecSentence();
  if(params->Type == AGGREG_1M) {
    PrepareSentence("EXECUTE SAVE_1M_EXT(?, ?, ?, ?, ?, ?, ?, ?)");
    BindSingleUInt32(pNodeIndex);                                                    //node
    BindSingleUInt64(params->Timestamp);                                             //sampletime
    BindArrayFloat(params->Power_Factor, (pTriphase == true) ? 3 : 1);               //power_factor
    BindArrayFloat(params->THD_V, (pTriphase == true) ? 3 : 1);                      //thd_v
    BindArrayFloat(params->THD_I, (pTriphase == true) ? 4 : 1);                      //thd_i
    BindArrayFloat(params->Phi, (pTriphase == true) ? 3 : 1);                        //phi
    BindArrayArrayFloat(params->VoltageHarmonics, (pTriphase == true) ? 3 : 1, 50);  //harmonics_v
    BindArrayArrayFloat(params->CurrentHarmonics, (pTriphase == true) ? 3 : 1, 50);  //harmonics_i
    ExecSentence();
    PrepareSentence("EXECUTE DELETE_1M_EXT(?, ?)");
    BindSingleUInt32(pNodeIndex);                                                    //node
    BindSingleUInt64(params->Timestamp - 86400000);                                  //sampletime
    ExecSentence();
  }
  ResetParams(params);
}
float CaptureInterface::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
void CaptureInterface::EventsDetect() {
  if(pWork.Cycles_Len != 20) {
    pWork.Params.Flag = true;
    return;
  }
  for(int i = 0; i < pWork.Cycles_Len; i++) {
    //Introduzco ciclo en la lista y calculo sus parametros de longitud y RMS
    EventCycle_t *Cycle = &pPrevCycles[pPrevCyclesWrite];
    Cycle->Len = pWork.Cycles[i].End - pWork.Cycles[i].Start;
    for(uint n = 0; n < (pTriphase ? 3 : 1); n++) {
      Cycle->RMS[n] = 0.0;
      for(uint j = 0; j < Cycle->Len; j++) 
        Cycle->RMS[n] += pow(pWork.VoltageSamples[n][j + pWork.Cycles[i].Start], 2);
      Cycle->RMS[n] = sqrt(Cycle->RMS[n] / Cycle->Len);
    }
    pPrevCyclesWrite = (pPrevCyclesWrite + 1) % 100;
    if(pPrevCyclesLen < 100) pPrevCyclesLen++;

    //Calculo tension media de los ultimos 100 ciclos y determino si estoy en regimen estacionario
    float *RMS_Avg = (float*)malloc(sizeof(float) * (pTriphase ? 3 : 1));
    bool Stationary = false;
    float Tolerance = (pRunningEvent.Type != EVT_NONE) ? EVENT_RVC_HYSTERESIS : EVENT_RVC;
    if(pPrevCyclesLen == 100) {
      Stationary = true;
      for(int n = 0; n < (pTriphase ? 3 : 1); n++) {
        RMS_Avg[n] = 0.0;
        for(int j = 0; j < pPrevCyclesLen; j++) RMS_Avg[n] += pPrevCycles[j].RMS[n];
        RMS_Avg[n] = RMS_Avg[n] / pPrevCyclesLen;
        if(Stationary == true) {
          for(int j = 0; j < pPrevCyclesLen; j++) {
            if(fabs(PercentChange(pPrevCycles[j].RMS[n], RMS_Avg[n])) > Tolerance) {
              Stationary = false;
              break;
            }
          }
        }
      }
    }
    //Compruebo si algun evento ha finalizado
    bool EventEnd = (pRunningEvent.Type != EVT_NONE);
    if(pRunningEvent.Type == EVT_RVC) {
      if(Stationary == false) EventEnd = false;
    } else if((pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL) || (pRunningEvent.Type == EVT_INTERRUPTION)) {
      for(int n = 0; n < (pTriphase ? 3 : 1); n++) {
        if(fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) < EVENT_HYSTERESIS) continue;
        EventEnd = false;
        break;
      }
    }
    if(EventEnd == true) SaveEvent(i, true);
    for(int n = 0; n < (pTriphase ? 3 : 1); n++) {
      //Inicio eventos tipo RVC si procede
      if((pRunningEvent.Type == EVT_NONE) && (pPrevStationary == true) && (Stationary == false)) {
        pRunningEvent.Type     = EVT_RVC;
        pRunningEvent.Start    = pWork.Params.Timestamp + ((pWork.Cycles[i].Start * 1000.0) / pSampleRate);
      }
      //Un evento DIP o SWELL se produce cuando un ciclo sale de los valores establecidos y si no habia eventos anteriores o era un RVC
      if(((pRunningEvent.Type == EVT_NONE) || (pRunningEvent.Type == EVT_RVC)) && (fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) > EVENT_DIP_SWELL)) {
        pRunningEvent.Type     = (Cycle->RMS[n] > Tools::Electrical.NominalVoltage) ? EVT_SWELL : EVT_DIP;
        pRunningEvent.Start    = pWork.Params.Timestamp + ((pWork.Cycles[i].Start * 1000.0) / pSampleRate);
      }
      if(((pRunningEvent.Type == EVT_NONE) || (pRunningEvent.Type == EVT_RVC) || (pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL)) && (fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) > EVENT_INTERRUPTION)) {
        pRunningEvent.Type     = EVT_INTERRUPTION;
        pRunningEvent.Start    = pWork.Params.Timestamp + ((pWork.Cycles[i].Start * 1000.0) / pSampleRate);
      }
      if((pRunningEvent.Type == EVT_RVC) && ((pRunningEvent.Duration == 0) || (fabs(pRunningEvent.Voltage) < fabs(Cycle->RMS[n] - pStationaryRMS[n])))) {
        pRunningEvent.Voltage = Cycle->RMS[n] - pStationaryRMS[n];
        pRunningEvent.Change  = PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage);          
      }
      if(((pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL) || (pRunningEvent.Type == EVT_INTERRUPTION))  && ((pRunningEvent.Duration == 0) || (fabs(pRunningEvent.Voltage) < fabs(Cycle->RMS[n] - Tools::Electrical.NominalVoltage)))) {
        pRunningEvent.Voltage  = Cycle->RMS[n] - Tools::Electrical.NominalVoltage;
        pRunningEvent.Change   = PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage);
        if((pRunningEvent.Type == EVT_DIP) && (Cycle->RMS[n] > Tools::Electrical.NominalVoltage)) pRunningEvent.Type = EVT_SWELL;
        if((pRunningEvent.Type == EVT_SWELL) && (Cycle->RMS[n] < Tools::Electrical.NominalVoltage)) pRunningEvent.Type = EVT_DIP;
      }
    }
    if(pRunningEvent.Type != EVT_NONE) SaveEvent(i, false);
    if((pPrevStationary == false) && (Stationary == true)) Tools::LogMsg("DEBUG", "Stationary voltage reached in node %d\n", pNodeIndex);
    if(Stationary == true) {
      free(pStationaryRMS);
      pStationaryRMS = RMS_Avg;
    }
    pPrevStationary = Stationary;
  }
}
void CaptureInterface::SaveEvent(uint8_t cycle, bool end) {
  float **Samples     = NULL;
  uint32_t SamplesLen = 0;
  if(end == false) {
    if(pRunningEvent.Type != EVT_RVC) {
      pWork.Params.Flag = true;
      pPrevCyclesLen = 0;
      pPrevStationary = false;
    }
    if(pRunningEvent.Duration == 0) {
      SamplesLen = EVENT_ENVELOPETIME * 2 * pSampleRate;
      Samples = (float**)malloc(sizeof(float*) * SamplesLen);
      for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
      SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start + pSampleRate * EVENT_ENVELOPETIME, SamplesLen);
    }
  } else {
    SamplesLen = MIN(EVENT_ENVELOPETIME * 2 * pSampleRate, pRunningEvent.Duration);
    Samples = (float**)malloc(sizeof(float*) * SamplesLen);
    for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
    SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start - EVENT_ENVELOPETIME * pSampleRate + SamplesLen, SamplesLen);
  }
  if((end == true) || (pRunningEvent.Duration != 0)) {
    char *Type = (char*)"UNKNOWN";
    if(pRunningEvent.Type == EVT_RVC) Type = (char*)"RVC";
    if(pRunningEvent.Type == EVT_DIP) Type = (char*)"DIP";  
    if(pRunningEvent.Type == EVT_SWELL) Type = (char*)"SWELL";
    if(pRunningEvent.Type == EVT_INTERRUPTION) Type = (char*)"INTERRUPTION";
    if(pRunningEvent.Type == EVT_GLITCH) Type = (char*)"GLITCH";
    PrepareSentence("EXECUTE SAVE_EVENT(?, ?, ?, ?, ?, ?, ?)");
    BindSingleUInt32(pNodeIndex);                                                                       //node
    BindSingleUInt64(pRunningEvent.Start);                                                              //starttime
    BindSingleFloat(pRunningEvent.Duration);                                                            //duration
    BindSingleString(Type);                                                                             //type
    BindSingleFloat(pRunningEvent.Voltage);                                                             //voltage
    BindSingleFloat(pRunningEvent.Change);                                                              //change
    BindArrayArrayFloat(Samples, SamplesLen, (pTriphase == true) ? 3 : 1);                              //samples
    ExecSentence();
  }
  if(end == true) {
    pRunningEvent.Type = EVT_NONE;
    pRunningEvent.Duration = 0;
  } else {
    pRunningEvent.Duration += pWork.Cycles[cycle].End - pWork.Cycles[cycle].Start;
  }
  if(Samples != NULL) {
    for(uint32_t i = 0; i < SamplesLen; i++) free(Samples[i]);  
    free(Samples);
  }
}
void CaptureInterface::Analyze(float minimunCurrent) {
  FFT_t FFT_Voltage[3];
  FFT_t FFT_Current[4];
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) FFT_Start(&FFT_Voltage[i], pWork.VoltageSamples[i], pWork.VoltageFFT[i]);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) FFT_Start(&FFT_Current[i], pWork.CurrentSamples[i], pWork.CurrentFFT[i]);
  //Calcula valores basicos (RMS, potencia activa y energia) que son validos aun habiendo eventos
  float V[3] = {0.0, 0.0, 0.0};
  float I[4] = {0.0, 0.0, 0.0, 0.0};
  float P[3] = {0.0, 0.0, 0.0};
  for(uint32_t i = 0; i < pWork.Samples; i++) {
    for(int j = 0; j < (pTriphase ? 3 : 1); j++) V[j] += pow(pWork.VoltageSamples[j][i], 2.0);
    for(int j = 0; j < (pTriphase ? 4 : 1); j++) I[j] += pow(pWork.CurrentSamples[j][i], 2.0);
    for(int j = 0; j < (pTriphase ? 3 : 1); j++) P[j] += pWork.VoltageSamples[j][i] * pWork.CurrentSamples[j][i];
  }
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.Active_Power[j] = P[j] / pWork.Samples;
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.RMS_V[j] = sqrt(V[j] / pWork.Samples);
  for(int j = 0; j < (pTriphase ? 4 : 1); j++) pWork.Params.RMS_I[j] = sqrt(I[j] / pWork.Samples);
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) pWork.Params.Apparent_Power[j] = pWork.Params.RMS_V[j] * pWork.Params.RMS_I[j];
  EventsDetect();
  for(int i = 0; i < pWork.Cycles_Len; i++) {
    pLastCyclesLenght = pLastCyclesLenght - pLastCycles[pLastCyclesPos];
    pLastCycles[pLastCyclesPos] = pWork.Cycles[i].End - pWork.Cycles[i].Start;
    pLastCyclesLenght = pLastCyclesLenght + pLastCycles[pLastCyclesPos];
    pLastCyclesPos = (pLastCyclesPos + 1) % PROCESSING_FREQCYCLES;
  }
  pWork.Params.Frequency = PROCESSING_FREQCYCLES / (pLastCyclesLenght / (float)pSampleRate) / 2.0;
  for(int j = 0; j < (pTriphase ? 3 : 1); j++) {
    FFT_Wait(&FFT_Voltage[j]);
    for(int i = 0; i < 50; i++) {
      pWork.Params.VoltageHarmonics[j][i] = sqrt(pow(pWork.VoltageFFT[j][10 + i * 10].Real, 2.0) + pow(pWork.VoltageFFT[j][10 + i * 10].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(i > 0) pWork.Params.THD_V[j] += pow(pWork.Params.VoltageHarmonics[j][i], 2.0);
    }
    pWork.Params.THD_V[j] = sqrt(pWork.Params.THD_V[j]) / pWork.Params.VoltageHarmonics[j][0];
  }
  for(int j = 0; j < (pTriphase ? 4 : 1); j++) {
    FFT_Wait(&FFT_Current[j]);
    if(pWork.Params.RMS_I[j] < minimunCurrent) continue;
    for(int i = 10; i <= 500 ; i = i + 10) {
      int   Harmonic = (i / 10) - 1;
      float CurrentHarmonic = sqrt(pow(pWork.CurrentFFT[j][i].Real, 2.0) + pow(pWork.CurrentFFT[j][i].Imag, 2.0)) / M_SQRT2 * 2.0;
      if(Harmonic > 0) pWork.Params.THD_I[j] += pow(CurrentHarmonic, 2.0);
      if(j < 3) {
        float VoltageHarmonic = pWork.Params.VoltageHarmonics[j][Harmonic];
        float PHI = CalcPhi(pWork.VoltageFFT[j][i], pWork.CurrentFFT[j][i]);
        pWork.Params.CurrentHarmonics[j][Harmonic] = CurrentHarmonic;
        if(Harmonic == 0) {
          pWork.Params.Fundamental_Active_Power[j]   = VoltageHarmonic * CurrentHarmonic * cos(PHI);
          pWork.Params.Fundamental_Reactive_Power[j] = VoltageHarmonic * CurrentHarmonic * sin(PHI);
          pWork.Params.Fundamental_Apparent_Power[j] = VoltageHarmonic * CurrentHarmonic;
          pWork.Params.Fundamental_Power_Factor[j]   = pWork.Params.Fundamental_Active_Power[j] / pWork.Params.Fundamental_Apparent_Power[j];
          PHI = fmod(PHI, 2.0 * M_PI);
          if(PHI > M_PI) PHI = PHI -  2.0 * M_PI;
          pWork.Params.Phi[j] = PHI * 180.0 / M_PI;
        }
      }
    }
    pWork.Params.THD_I[j] = sqrt(pWork.Params.THD_I[j]) / pWork.Params.CurrentHarmonics[j][0];
    if(j < 3) {
      pWork.Params.Active_Energy[j] = (pWork.Params.Active_Power[j] * pWork.Samples) / (pSampleRate * 3600.0);
      pWork.Params.Power_Factor[j] = pWork.Params.Active_Power[j] / pWork.Params.Apparent_Power[j];
      pWork.Params.Fundamental_Reactive_Energy[j] = (pWork.Params.Fundamental_Reactive_Power[j] * pWork.Samples) / (pSampleRate * 3600.0);
    }
  }
}
void CaptureInterface::AllocParams(Params_t *params, Aggreg_Type type) {
  params->RMS_V = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  params->RMS_I = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 4 : 1));
  params->Active_Power = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  params->Fundamental_Reactive_Power = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  params->Fundamental_Reactive_Energy = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  params->Active_Energy = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  if(type == AGGREG_10C) {
    params->Apparent_Power = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    params->Fundamental_Active_Power = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    params->Fundamental_Apparent_Power = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    params->Fundamental_Power_Factor = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1)); 
  }
  if((type == AGGREG_10C) || (type == AGGREG_1M) || (type == AGGREG_10M)) {
    params->THD_V = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    params->VoltageHarmonics = (float**)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    for(int i = 0; i < ((pTriphase == true) ? 3 : 1); i++) params->VoltageHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
  }
  if((type == AGGREG_10C) || (type == AGGREG_1M)) {
    params->Power_Factor = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
    params->THD_I = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 4 : 1));
    params->CurrentHarmonics = (float**)malloc(sizeof(float*) * ((pTriphase == true) ? 4 : 1));
    for(int i = 0; i < ((pTriphase == true) ? 4 : 1); i++) params->CurrentHarmonics[i] = (float*)malloc(sizeof(float*) * 50);
    params->Phi = (float*)malloc(sizeof(float*) * ((pTriphase == true) ? 3 : 1));
  }
  params->Type = type;
  ResetParams(params);
}
CaptureInterface::CaptureInterface(uint32_t samplerate, bool triphase, uint16_t id) : Database() {
  pNodeIndex           = id;
  pPrevTimestamp       = 0;
  pSampleRate          = samplerate;
  pTriphase            = triphase;
  pMaxSamples          = ceil(samplerate * 10.0 / ((Tools::Electrical.NominalFrequency == 50) ? 42.5 : 51.0));
  pWaitingThreads      = 0;
  pDataReady           = false;
  pTerminate           = false;
  pthread_mutex_init(&pWaitingThreadsMutex, NULL);
  pthread_mutex_init(&pDataReadyMutex, NULL);
  pthread_cond_init (&pWaitingThreadsWait, NULL);
  pthread_cond_init (&pDataReadyWait, NULL);
  pWork.VoltageSamples = (float**)malloc(sizeof(float*) * ((triphase == true) ? 3 : 1));
  pWork.VoltageFFT     = (Complex_t**)malloc(sizeof(Complex_t*) * ((triphase == true) ? 3 : 1));
  for(int i = 0; i < ((triphase == true) ? 3 : 1); i++) pWork.VoltageSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  for(int i = 0; i < ((triphase == true) ? 3 : 1); i++) pWork.VoltageFFT[i] = (Complex_t*)malloc(sizeof(Complex_t) * (ceil(pMaxSamples / 2) + 1));
  pWork.CurrentSamples = (float**)malloc(sizeof(float*) * ((triphase == true) ? 4 : 1));
  pWork.CurrentFFT     = (Complex_t**)malloc(sizeof(Complex_t*) * ((triphase == true) ? 4 : 1));
  for(int i = 0; i < ((triphase == true) ? 4 : 1); i++) pWork.CurrentSamples[i] = (float*)malloc(sizeof(float) * pMaxSamples);
  for(int i = 0; i < ((triphase == true) ? 4 : 1); i++) pWork.CurrentFFT[i] = (Complex_t*)malloc(sizeof(Complex_t) * (ceil(pMaxSamples / 2) + 1));
  AllocParams(&pWork.Params, AGGREG_10C);
  ResetWork();
  pRunningEvent.Type = EVT_NONE;
  pRunningEvent.Duration = 0;
  AllocParams(&pAggreg3S, AGGREG_3S);
  AllocParams(&pAggreg1M, AGGREG_1M);
  AllocParams(&pAggreg10M, AGGREG_10M);
  AllocParams(&pAggreg1H, AGGREG_1H);
  pStats.Voltage   = (uint16_t*)malloc((sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  pStats.Frequency = (uint16_t*)malloc((sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  pStats.THD       = (uint16_t*)malloc((sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  ResetStats();
  for(int i = 0; i < PROCESSING_FREQCYCLES; i++) {
    uint32_t Samples = (pSampleRate * i / 100);
    pLastCycles[i] = Samples - pLastCyclesLenght;
    pLastCyclesLenght += pLastCycles[i]; 
  }
  for(int i = 0; i < 100; i ++) pPrevCycles[i].RMS = (float*)malloc(sizeof(float) * (pTriphase ? 3 : 1));
  pStationaryRMS = NULL;
  pPrevCyclesLen = 0;
  pPrevCyclesWrite = 0;
  pPrevStationary = false;  
  pLastCyclesPos    = 0;
  pWork.SequenceID  = 0;
  ExecSentence("PREPARE SAVE_3S       AS INSERT INTO aggreg_3s (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");    
  ExecSentence("PREPARE SAVE_1M       AS INSERT INTO aggreg_1m (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");  
  ExecSentence("PREPARE SAVE_1M_EXT   AS INSERT INTO aggreg_ext_1m (node_ext, sampletime_ext, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i) VALUES($1, $2, $3, $4, $5, $6, $7, $8)");  
  ExecSentence("PREPARE SAVE_10M      AS INSERT INTO aggreg_10m(node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10)");  
  ExecSentence("PREPARE SAVE_1H       AS INSERT INTO aggreg_1h (node, sampletime, rms_v, rms_i, active_power, active_energy, fundamental_reactive_power, fundamental_reactive_energy, freq, flag, stat_voltage, stat_frequency, stat_thd, stat_harmonics) VALUES($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, $14)");
  ExecSentence("PREPARE DELETE_1M_EXT AS DELETE FROM aggreg_ext_1m WHERE node_ext = $1 AND sampletime_ext < $2");
  ExecSentence("PREPARE SAVE_EVENT    AS INSERT INTO events (node, starttime, duration, type, voltage, percent, samples) VALUES ($1, $2, $3, $4, $5, $6, $7) ON CONFLICT (starttime, node) DO UPDATE SET duration = $3, type = $4, voltage = $5, percent = $6, samples = events.samples || $7");
}
void CaptureInterface::ResetParams(Params_t *params) {
  params->Timestamp = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->RMS_V[i] = 0;
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) params->RMS_I[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Active_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Reactive_Power[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Reactive_Energy[i] = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Active_Energy[i] = 0;
  params->Frequency = 0;
  params->Flag = false;
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M)) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_I[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Phi[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Power_Factor[i] = 0; 
    for(int i = 0; i < (pTriphase ? 4 : 1); i++) memset(params->CurrentHarmonics[i], 0, sizeof(float) * 50);
  }
  if((params->Type == AGGREG_10C) || (params->Type == AGGREG_1M) || (params->Type == AGGREG_10M)) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->THD_V[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(params->VoltageHarmonics[i], 0, sizeof(float) * 50);
  }
  if(params->Type == AGGREG_10C) {
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Active_Power[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Apparent_Power[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Apparent_Power[i] = 0;
    for(int i = 0; i < (pTriphase ? 3 : 1); i++) params->Fundamental_Power_Factor[i] = 0;
  }
  params->Samples = 0;
}
void CaptureInterface::ResetWork() {
  pWork.SequenceID++;
  pWork.Samples = 0;
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.VoltageFFT[i], 0, sizeof(Complex_t) * (ceil(pMaxSamples / 2) + 1));
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) memset(pWork.CurrentFFT[i], 0, sizeof(Complex_t) * (ceil(pMaxSamples / 2) + 1));
  for(int i = 0; i < (pTriphase ? 3 : 1); i++) memset(pWork.VoltageSamples[i], 0, sizeof(float) * pMaxSamples);
  for(int i = 0; i < (pTriphase ? 4 : 1); i++) memset(pWork.CurrentSamples[i], 0, sizeof(float) * pMaxSamples);
  memset(pWork.Cycles, 0, sizeof(Cycle_t) * 20);
  pWork.Cycles_Len = 0;
}
void CaptureInterface::ResetStats() {
  memset(pStats.Voltage, 0, (sizeof(VoltageRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pStats.Frequency, 0, (sizeof(FrequencyRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pStats.THD, 0, (sizeof(THDRanges) / sizeof(float) - 1) * sizeof(uint16_t));
  memset(pStats.Harmonics, 0, 2 * sizeof(uint16_t));
}
CaptureInterface::~CaptureInterface() {
  //Libero todas las estructuras
}
uint32_t CaptureInterface::GetMaxSamples() {
  return pMaxSamples;
}
uint32_t CaptureInterface::SampleRate() {
  return pSampleRate;
}
bool CaptureInterface::Triphase() {
  return pTriphase;
}
float CaptureInterface::CalcPhi(Complex_t voltage, Complex_t current) {
  float Tmp;
  if((voltage.Real == 0) && (voltage.Imag == 0)) 
    Tmp = 0;
  else if(voltage.Real < 0)
    Tmp = atan(voltage.Imag / voltage.Real);
  else
    Tmp = atan(voltage.Imag / voltage.Real) + M_PI;
  
  if((current.Real == 0) && (current.Imag == 0)) 
    return Tmp;
  else if(current.Real < 0)
    return Tmp - atan(current.Imag / current.Real);
  else
    return Tmp - atan(current.Imag / current.Real) + M_PI;
}
float CaptureInterface::ApplyFilter(float in, Filter_t* filter) {
  float out = in * filter->A0 + filter->Z1;
  filter->Z1 = in * filter->A1 + filter->Z2 - filter->B1 * out;
  filter->Z2 = in * filter->A2 - filter->B2 * out;
  return out;
}
void *CaptureInterface::FFT_Thread(void *args) {
  FFT_t *FFT = (FFT_t *)args;
  fftwf_execute_dft_r2c(FFT->Plan, FFT->Input, (fftwf_complex*)FFT->Output);
  for(int i = 0; i < ceil(FFT->Len / 2 + 1); i++) {
    FFT->Output[i].Real = FFT->Output[i].Real / FFT->Len;
    FFT->Output[i].Imag = FFT->Output[i].Imag / FFT->Len;
  }
  return NULL;  
}
void CaptureInterface::FFT_Wait(FFT_t *fft) {
  if(pthread_join(fft->Thread, NULL) != 0) Tools::LogMsg("ERROR", "%s:%d -> (Unknown)\n", __func__, __LINE__);
  pthread_mutex_lock(&pFFTPlannerMutex);
  fftwf_destroy_plan(fft->Plan);
  pthread_mutex_unlock(&pFFTPlannerMutex);  
  return;
}
bool CaptureInterface::FFT_Start(FFT_t *fft, float *in, Complex_t *out) {
  pthread_mutex_lock(&pFFTPlannerMutex);
  fft->Plan = fftwf_plan_dft_r2c_1d(pWork.Samples, in, (fftwf_complex*)out, FFTW_ESTIMATE);
  pthread_mutex_unlock(&pFFTPlannerMutex);  
  fft->Len = pWork.Samples;
  fft->Input = in;
  fft->Output = out;
  if(pthread_create(&fft->Thread, NULL, FFT_Thread, (void*)fft) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    fftwf_destroy_plan(fft->Plan);
    return false;
  }
  return true;
}
json_object *CaptureInterface::GetJSONCopy(char *series) {
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
    Error = pthread_cond_timedwait(&pDataReadyWait, &pDataReadyMutex, &Timeout);
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
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.RMS_V[i]));
      } else if(strcmp(Serie, "rms_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 4 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.RMS_I[i]));
      } else if(strcmp(Serie, "active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Active_Power[i]));
//      } else if(strcmp(Serie, "apparent_power") == 0) {
//        Obj = json_object_new_array();
//        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Apparent_Power[i]));
      } else if(strcmp(Serie, "power_factor") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Power_Factor[i]));
      } else if(strcmp(Serie, "active_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Active_Energy[i]));
      } else if(strcmp(Serie, "fundamental_active_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Fundamental_Active_Power[i]));
      } else if(strcmp(Serie, "fundamental_reactive_power") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Fundamental_Reactive_Power[i]));
//      } else if(strcmp(Serie, "fundamental_apparent_power") == 0) {
//        Obj = json_object_new_array();
//        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Fundamental_Apparent_Power[i]));
//      } else if(strcmp(Serie, "fundamental_power_factor") == 0) {
//        Obj = json_object_new_array();
//        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Fundamental_Power_Factor[i]));
      } else if(strcmp(Serie, "fundamental_reactive_energy") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Fundamental_Reactive_Energy[i]));
      } else if(strcmp(Serie, "thd_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.THD_V[i]));
      } else if(strcmp(Serie, "thd_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 4 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.THD_I[i]));
      } else if(strcmp(Serie, "freq") == 0) {
        Obj = json_object_new_double(pWork.Params.Frequency);
      } else if(strcmp(Serie, "phi") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) json_object_array_add(Obj, json_object_new_double(pWork.Params.Phi[i]));
      } else if(strcmp(Serie, "flag") == 0) {
        Obj = json_object_new_boolean(pWork.Params.Flag);
      } else if(strcmp(Serie, "harmonics_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pWork.Params.VoltageHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "harmonics_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 4 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < 50; j++) json_object_array_add(Obj2, json_object_new_double(pWork.Params.CurrentHarmonics[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pWork.Samples; j++) json_object_array_add(Obj2, json_object_new_double(pWork.VoltageSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "wave_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 4 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(uint32_t j = 0; j < pWork.Samples; j++) json_object_array_add(Obj2, json_object_new_double(pWork.CurrentSamples[i][j]));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_v") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 3 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < ceil(pWork.Samples / 2) + 1; j++) json_object_array_add(Obj2, json_object_new_double(sqrt(pow(pWork.VoltageFFT[i][j].Real, 2.0) + pow(pWork.VoltageFFT[i][j].Imag, 2.0)) / M_SQRT2 * 2.0));
          json_object_array_add(Obj, Obj2);
        }
      } else if(strcmp(Serie, "fft_i") == 0) {
        Obj = json_object_new_array();
        for(int i = 0; i < (pTriphase ? 4 : 1); i++) {
          json_object *Obj2 = json_object_new_array();
          for(int j = 0; j < ceil(pWork.Samples / 2) + 1; j++) json_object_array_add(Obj2, json_object_new_double(sqrt(pow(pWork.CurrentFFT[i][j].Real, 2.0) + pow(pWork.CurrentFFT[i][j].Imag, 2.0)) / M_SQRT2 * 2.0));
          json_object_array_add(Obj, Obj2);
        }
      }
      if(Obj != NULL) json_object_object_add(Result, Serie, Obj);
      Serie = strtok_r(NULL, ", ", &Save);
    }
    json_object_object_add(Result, "Sampletime", json_object_new_int64(pWork.Params.Timestamp));
  }
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsWait);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  return Result;
}