/* ================================== zMeter ===========================================================================
 * File:     Events.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <math.h>
#include "Tools.h"
#include "Events.h"
float Events::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
void Events::EventsDetect(Analyzer::Work_t *work) {
  if(work->Cycles_Len != 20) {
    work->Params.Flag = true;
    return;
  }
  for(int i = 0; i < work->Cycles_Len; i++) {
    //Introduzco ciclo en la lista y calculo sus parametros de longitud y RMS
    EventCycle_t *Cycle = &pPrevCycles[pPrevCyclesWrite];
    Cycle->Len = work->Cycles[i].End - work->Cycles[i].Start;
    for(uint n = 0; n < (pTriphase ? 3 : 1); n++) {
      Cycle->RMS[n] = 0.0;
      for(uint j = 0; j < Cycle->Len; j++) Cycle->RMS[n] += pow(work->VoltageSamples[n][j + work->Cycles[i].Start], 2);
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
//        if(fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) < EVENT_HYSTERESIS) continue;
        EventEnd = false;
        break;
      }
    }
    if(EventEnd == true) SaveEvent(i, true);
    for(int n = 0; n < (pTriphase ? 3 : 1); n++) {
      //Inicio eventos tipo RVC si procede
      if((pRunningEvent.Type == EVT_NONE) && (pPrevStationary == true) && (Stationary == false)) {
        pRunningEvent.Type     = EVT_RVC;
        pRunningEvent.Start    = work->Params.Timestamp + ((work->Cycles[i].Start * 1000.0) / pSampleRate);
      }
      //Un evento DIP o SWELL se produce cuando un ciclo sale de los valores establecidos y si no habia eventos anteriores o era un RVC
//      if(fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) > EVENT_DIP_SWELL) {
//        if(pRunningEvent.Type == EVT_NONE) pRunningEvent.Start = pWork.Params.Timestamp + ((pWork.Cycles[i].Start * 1000.0) / pSampleRate);
//        if((pRunningEvent.Type == EVT_NONE) || (pRunningEvent.Type == EVT_RVC)) pRunningEvent.Type = (Cycle->RMS[n] > Tools::Electrical.NominalVoltage) ? EVT_SWELL : EVT_DIP;
//      }
//      if(fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) > EVENT_INTERRUPTION) {
//        if(pRunningEvent.Type == EVT_NONE) pRunningEvent.Start = pWork.Params.Timestamp + ((pWork.Cycles[i].Start * 1000.0) / pSampleRate);
//        if((pRunningEvent.Type == EVT_NONE) || (pRunningEvent.Type == EVT_RVC) || (pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL)) pRunningEvent.Type     = EVT_INTERRUPTION;
//      }
      if((pRunningEvent.Type == EVT_RVC) && ((pRunningEvent.Duration == 0) || (fabs(pRunningEvent.Voltage) < fabs(Cycle->RMS[n] - pStationaryRMS[n])))) {
        pRunningEvent.Voltage = Cycle->RMS[n] - pStationaryRMS[n];
//        pRunningEvent.Change  = PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage);          
      }
//      if(((pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL) || (pRunningEvent.Type == EVT_INTERRUPTION))  && ((pRunningEvent.Duration == 0) || (fabs(pRunningEvent.Voltage) < fabs(Cycle->RMS[n] - Tools::Electrical.NominalVoltage)))) {
//        pRunningEvent.Voltage  = Cycle->RMS[n] - Tools::Electrical.NominalVoltage;
//        pRunningEvent.Change   = PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage);
//        if((pRunningEvent.Type == EVT_DIP) && (Cycle->RMS[n] > Tools::Electrical.NominalVoltage)) pRunningEvent.Type = EVT_SWELL;
//        if((pRunningEvent.Type == EVT_SWELL) && (Cycle->RMS[n] < Tools::Electrical.NominalVoltage)) pRunningEvent.Type = EVT_DIP;
//      }
    }
    if(pRunningEvent.Type != EVT_NONE) SaveEvent(i, false);
    if(Stationary == true) {
      free(pStationaryRMS);
      pStationaryRMS = RMS_Avg;
    }
    pPrevStationary = Stationary;
  }
}
void Events::SaveEvent(uint8_t cycle, bool end) {
  float **Samples     = NULL;
  uint32_t SamplesLen = 0;
  if(end == false) {
    if(pRunningEvent.Type != EVT_RVC) {
//      pWork.Params.Flag = true;
      pPrevCyclesLen = 0;
      pPrevStationary = false;
    }
    if(pRunningEvent.Duration == 0) {
      SamplesLen = EVENT_ENVELOPETIME * 2 * pSampleRate;
      Samples = (float**)malloc(sizeof(float*) * SamplesLen);
      for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
//      SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start + pSampleRate * EVENT_ENVELOPETIME, SamplesLen);
    }
  } else {
//    SamplesLen = MIN(EVENT_ENVELOPETIME * 2 * pSampleRate, pRunningEvent.Duration);
    Samples = (float**)malloc(sizeof(float*) * SamplesLen);
    for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
//    SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start - EVENT_ENVELOPETIME * pSampleRate + SamplesLen, SamplesLen);
  }
  if((end == true) || (pRunningEvent.Duration == 0)) {
    char *Type = (char*)"UNKNOWN";
    if(pRunningEvent.Type == EVT_RVC) Type = (char*)"RVC";
    if(pRunningEvent.Type == EVT_DIP) Type = (char*)"DIP";  
    if(pRunningEvent.Type == EVT_SWELL) Type = (char*)"SWELL";
    if(pRunningEvent.Type == EVT_INTERRUPTION) Type = (char*)"INTERRUPTION";
    if(pRunningEvent.Type == EVT_GLITCH) Type = (char*)"GLITCH";
    PrepareSentence("EXECUTE SAVE_EVENT(?, ?, ?, ?, ?, ?, ?)");
//    BindSingleUInt32(pNodeIndex);                                                                       //node
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
//    pRunningEvent.Duration += pWork.Cycles[cycle].End - pWork.Cycles[cycle].Start;
  }
  if(Samples != NULL) {
    for(uint32_t i = 0; i < SamplesLen; i++) free(Samples[i]);  
    free(Samples);
  }
}
Events::Events() {
  pRunningEvent.Type = EVT_NONE;
  pRunningEvent.Duration = 0;
  pStationaryRMS = NULL;
  pPrevCyclesLen = 0;
  pPrevCyclesWrite = 0;
  pPrevStationary = false;  
  for(int i = 0; i < 100; i ++) pPrevCycles[i].RMS = (float*)malloc(sizeof(float) * (pTriphase ? 3 : 1));
  ExecSentence("PREPARE SAVE_EVENT AS INSERT INTO events (node, starttime, duration, type, voltage, percent, samples) VALUES ($1, $2, $3, $4, $5, $6, $7) ON CONFLICT (starttime, node) DO UPDATE SET duration = $3, type = $4, voltage = $5, percent = $6, samples = events.samples || $7");
}