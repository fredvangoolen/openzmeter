/* ================================== zMeter ===========================================================================
 * File:     Events.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <math.h>
#include "Tools.h"
#include "Events.h"
Events::Events(AnalyzerInterface *interface, float samplerate) : Database(interface->Serial()) {
  pSampleRate = samplerate;
  pTerminate = false;
  pParams = NULL;
  pCycles = NULL;
  pAnalyzer = interface;
  pPrevSamples.resize((interface->Mode() == AnalyzerInterface::PHASE1) ? 1 : 3);
  pPrevStationary = false;
  pthread_cond_init(&pThreadCond, NULL);
  pthread_mutex_init(&pThreadMutex, NULL);
  pThread = 0;
  if(pthread_create(&pThread, NULL, Events_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, "EventsDet");  
}
Events::~Events() {
  if(pThread == 0) return;
  pTerminate = true;
  pthread_cond_signal(&pThreadCond);
  if(pthread_join(pThread, NULL) != 0) LOGFile::Error("%s:%d -> (Unknown)\n", __func__, __LINE__);
}
void Events::Start(AnalyzerInterface::Params_t *params, vector<uint32_t> *cycles) {
  pthread_mutex_lock(&pThreadMutex);
  if(pParams != NULL) {
    pthread_mutex_unlock(&pThreadMutex);
    return;
  }
  pParams = params;
  pCycles = cycles;
  pthread_mutex_unlock(&pThreadMutex);
  pthread_cond_signal(&pThreadCond);
}
void Events::Wait() {
  pthread_mutex_lock(&pThreadMutex);
  while(pParams != NULL) {
    if(pTerminate == true) return;
    pthread_cond_wait(&pThreadCond, &pThreadMutex);
  } 
  pthread_mutex_unlock(&pThreadMutex);
}
bool Events::FindRunningEvent(uint32_t mask) {
  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
    if((it->Type & mask) && (it->Clossing == false)) return true;
  }
  return false;
}
void *Events::Events_Thread(void* args) {
  Events *This = (Events*)args;
  while(true) {
    pthread_mutex_lock(&This->pThreadMutex);
    while(This->pParams == NULL) {
      if(This->pTerminate == true) {
        pthread_mutex_unlock(&This->pThreadMutex);
        return NULL;
      }
      pthread_cond_wait(&This->pThreadCond, &This->pThreadMutex);
    }
    pthread_mutex_unlock(&This->pThreadMutex);
    uint32_t Pos = 0;
    This->pParams->Flag = false;
    for(uint32_t i = 0; i < This->pCycles->size(); i++) {
      //Calc cycle stats
      EventCycle_t Cycle;
      uint32_t CycleLen = (*This->pCycles)[i];
      for(uint n = 0; n < This->pParams->Voltage_Samples.size(); n++) {
        float RMS = 0.0;
        for(uint32_t j = 0; j < CycleLen; j++) RMS += pow(This->pParams->Voltage_Samples[n][Pos + j], 2);
        Cycle.RMS.push_back(sqrt(RMS / CycleLen));
      }
      //Calc last 100 cycles AVG (current and previous ones)
      vector<float> RMS_Avg = Cycle.RMS;
      for(uint8_t n = 0; n < Cycle.RMS.size(); n++) {
        uint32_t Len = This->pPrevCycles.size();
        if(Len > 99) Len = 99;
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        for(uint32_t j = 0; j < Len; j++, it++) RMS_Avg[n] += it->RMS[n];
        RMS_Avg[n] = RMS_Avg[n] / Len;
      }
      //Check steady state
      float Tolerance = EVENT_RVC - ((This->pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
      bool  Stationary = (This->pPrevCycles.size() >= 99);
      for(uint8_t n = 0; (n < RMS_Avg.size()) && (Stationary == true); n++) {
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        if(fabs(PercentChange(Cycle.RMS[n], RMS_Avg[n])) > Tolerance) {
          Stationary = false;
          break;
        }
        for(int j = 0; j < 99; j++) {
          if(fabs(PercentChange(it->RMS[n], RMS_Avg[n])) > Tolerance) {
            Stationary = false;
            break;
          }
        }
      }
      if(Stationary != This->pPrevStationary) LOGFile::Debug("Analyzer '%s' %s stationary state\n", This->pAnalyzer->Serial().c_str(), Stationary ? "reach" : "leave");
      
      //Check for new events
      if(This->pPrevSamples.size() > (EVENT_RAWENVELOPETIME * This->pSampleRate)) {
        for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
          Event_t Event;
          Event.Type = EVT_NONE;
          //Check for interruption new events
          if((PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) < EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) Event.Type = EVT_INTERRUPTION;
          //Check for DIP new events
          if((PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) < EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP) == false)) Event.Type = EVT_DIP;
          //Check for SWELL new events
          if((PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) Event.Type = EVT_SWELL;
          //Check for RVC new events
          if((Stationary == false) && (This->pPrevStationary == true) && (This->FindRunningEvent(EVT_INTERRUPTION | EVT_DIP | EVT_SWELL | EVT_RVC) == false)) Event.Type = EVT_RVC;
          if(Event.Type != EVT_NONE) {
            Event.Reference = (Event.Type == EVT_RVC) ? RMS_Avg : vector<float>(RMS_Avg.size(), This->pAnalyzer->GetNominalVoltage());
            Event.SteadyValue = vector<float>(RMS_Avg.size(), 0);
            Event.PeakValue = vector<float>(RMS_Avg.size(), 0);
            Event.Duration = 0;
            Event.Clossing = false;
            Event.Start =  This->pParams->Timestamp + ((Pos * 1000.0) / This->pSampleRate);
            Event.BeginSamples.assign(This->pPrevSamples.end() - (EVENT_RAWENVELOPETIME * This->pSampleRate), This->pPrevSamples.end());
            for(deque<EventCycle_t>::iterator it = This->pPrevCycles.begin(); it != This->pPrevCycles.end(); it++) Event.BeginRMS.push_back(it->RMS);
            This->pEvents.push_back(Event);
          }
        }
      }
      //Insert current cycle/samples in prev data buffers and in running events than need its
      for(uint n = 0; n < CycleLen; n++) {
        vector<float> Sample;
        for(uint j = 0; j < This->pParams->Voltage_Samples.size(); j++) Sample.push_back(This->pParams->Voltage_Samples[j][n + Pos]);
        This->pPrevSamples.push_back(Sample);
        while(This->pPrevSamples.size() > (This->pSampleRate * EVENT_RAWENVELOPETIME)) This->pPrevSamples.pop_front();
        for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
          if(it->Clossing == false) {
            if(it->InnerSamples.size() < (This->pSampleRate * EVENT_RAWENVELOPETIME)) it->InnerSamples.push_back(Sample);
          } else {
            if(it->EndSamples.size() < (This->pSampleRate * EVENT_RAWENVELOPETIME)) it->EndSamples.push_back(Sample);
          }
        }
      }
      This->pPrevCycles.push_back(Cycle);
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
        if(it->Clossing == false) {
          if(it->InnerRMS.size() < (This->pAnalyzer->GetNominalFrequency() * EVENT_RMSENVELOPETIME * 2)) it->InnerRMS.push_back(Cycle.RMS);
        } else {
          if(it->EndRMS.size() < (This->pAnalyzer->GetNominalFrequency() * EVENT_RMSENVELOPETIME * 2)) it->EndRMS.push_back(Cycle.RMS);
        }
      }
      if(This->pPrevCycles.size() > (This->pAnalyzer->GetNominalFrequency() * EVENT_RMSENVELOPETIME * 2)) This->pPrevCycles.pop_front(); 
      Pos = Pos + CycleLen;      
      
      //Update/close/save running events
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
        if((it->Clossing == true) && (This->pPrevCycles.size() == (This->pAnalyzer->GetNominalFrequency() * EVENT_RMSENVELOPETIME * 2))) {
 //Save y elimino
        } else {
          if(it->Type != EVT_RVC) This->pParams->Flag = true;
       //    if(EventEnd == true) SaveEvent(i, true);//Compruebo si algun evento ha finalizado
//    bool EventEnd = (pRunningEvent.Type != EVT_NONE);
//    if(pRunningEvent.Type == EVT_RVC) {
//      if(Stationary == false) EventEnd = false;
//    } else if((pRunningEvent.Type == EVT_DIP) || (pRunningEvent.Type == EVT_SWELL) || (pRunningEvent.Type == EVT_INTERRUPTION)) {
//      for(int n = 0; n < (pTriphase ? 3 : 1); n++) {
//        if(fabs(PercentChange(Cycle->RMS[n], Tools::Electrical.NominalVoltage)) < EVENT_HYSTERESIS) continue;
//        EventEnd = false;
//        break;
//      }
//    }
//    if(EventEnd == true) SaveEvent(i, true);

        
//Si sigue sin estar cerrado actualizo

          for(uint n = 0; n < RMS_Avg.size(); n++) {
            if(((it->Type == EVT_INTERRUPTION) || (it->Type == EVT_DIP)) && (RMS_Avg[n] < it->PeakValue[n])) it->PeakValue[n] = RMS_Avg[n];
            if((it->Type == EVT_SWELL) && (RMS_Avg[n] > it->PeakValue[n])) it->PeakValue[n] = RMS_Avg[n];
            if((it->Type == EVT_RVC) && (fabs(it->Reference[n] - it->PeakValue[n]) < fabs(it->Reference[n] - RMS_Avg[n]))) it->PeakValue[n] = RMS_Avg[n];
          }
        }
      }

      
      



      
      
      This->pPrevStationary = Stationary;
    }
    pthread_mutex_lock(&This->pThreadMutex);
    This->pParams = NULL;
    pthread_mutex_unlock(&This->pThreadMutex);
    pthread_cond_signal(&This->pThreadCond);
  }
  return NULL;
}


float Events::PercentChange(float in, float reference) {
  return ((in - reference) / reference) * 100;
}
//void Events::SaveEvent(uint8_t cycle, bool end) {
//  float **Samples     = NULL;
//  uint32_t SamplesLen = 0;
//  if(end == false) {
//    if(pRunningEvent.Type != EVT_RVC) {
//      pWork.Params.Flag = true;
//      pPrevCyclesLen = 0;
//      pPrevStationary = false;
//    }
//    if(pRunningEvent.Duration == 0) {
//      SamplesLen = EVENT_ENVELOPETIME * 2 * pSampleRate;
//      Samples = (float**)malloc(sizeof(float*) * SamplesLen);
//      for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
//      SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start + pSampleRate * EVENT_ENVELOPETIME, SamplesLen);
//    }
//  } else {
//    SamplesLen = MIN(EVENT_ENVELOPETIME * 2 * pSampleRate, pRunningEvent.Duration);
//    Samples = (float**)malloc(sizeof(float*) * SamplesLen);
//    for(uint32_t i = 0; i < SamplesLen; i++) Samples[i] = (float*)malloc(sizeof(float) * ((pTriphase == true) ? 3 : 1));
//    SamplesLen = SamplesCopy(Samples, pWork.Cycles[19].End - pWork.Cycles[cycle].Start - EVENT_ENVELOPETIME * pSampleRate + SamplesLen, SamplesLen);
//  }
//  if((end == true) || (pRunningEvent.Duration == 0)) {
//    char *Type = (char*)"UNKNOWN";
//    if(pRunningEvent.Type == EVT_RVC) Type = (char*)"RVC";
//    if(pRunningEvent.Type == EVT_DIP) Type = (char*)"DIP";  
//    if(pRunningEvent.Type == EVT_SWELL) Type = (char*)"SWELL";
//    if(pRunningEvent.Type == EVT_INTERRUPTION) Type = (char*)"INTERRUPTION";
//    if(pRunningEvent.Type == EVT_GLITCH) Type = (char*)"GLITCH";
//    PrepareSentence("EXECUTE SAVE_EVENT(?, ?, ?, ?, ?, ?, ?)");
//    BindSingleUInt32(pNodeIndex);                                                                       //node
//    BindSingleUInt64(pRunningEvent.Start);                                                              //starttime
//    BindSingleFloat(pRunningEvent.Duration);                                                            //duration
//    BindSingleString(Type);                                                                             //type
//    BindSingleFloat(pRunningEvent.Voltage);                                                             //voltage
//    BindSingleFloat(pRunningEvent.Change);                                                              //change
//    BindArrayArrayFloat(Samples, SamplesLen, (pTriphase == true) ? 3 : 1);                              //samples
//    ExecSentence();
//  }
//  if(end == true) {
//    pRunningEvent.Type = EVT_NONE;
//    pRunningEvent.Duration = 0;
//  } else {
////    pRunningEvent.Duration += pWork.Cycles[cycle].End - pWork.Cycles[cycle].Start;
//  }
//  if(Samples != NULL) {
//    for(uint32_t i = 0; i < SamplesLen; i++) free(Samples[i]);  
//    free(Samples);
//  }
//}
//Events::Events() {
//  pRunningEvent.Type = EVT_NONE;
//  pRunningEvent.Duration = 0;
//  pStationaryRMS = NULL;
//  pPrevCyclesLen = 0;
//  pPrevCyclesWrite = 0;
//  pPrevStationary = false;  
//  for(int i = 0; i < 100; i ++) pPrevCycles[i].RMS = (float*)malloc(sizeof(float) * (pTriphase ? 3 : 1));
//  ExecSentence("PREPARE SAVE_EVENT AS INSERT INTO events (node, starttime, duration, type, voltage, percent, samples) VALUES ($1, $2, $3, $4, $5, $6, $7) ON CONFLICT (starttime, node) DO UPDATE SET duration = $3, type = $4, voltage = $5, percent = $6, samples = events.samples || $7");
//}
