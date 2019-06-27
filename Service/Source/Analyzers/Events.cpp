/* ================================== zMeter ===========================================================================
 * File:     Events.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <math.h>
#include <sys/types.h>
#include "Tools.h"
#include "Events.h"
Events::Events(AnalyzerInterface *interface, float samplerate) : Database(interface->Serial()) {
  pSampleRate = samplerate;
  pTerminate = false;
  pParams = NULL;
  pCycles = NULL;
  pAnalyzer = interface;
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
    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
  }
  return false;
}
void Events::Save(Event_t &evt) {
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
    ExecSentenceNoResult("INSERT INTO events(starttime, duration, finished, samplerate, evttype, reference, peak, change, notes, samples1, rms1) VALUES (" + Bind(evt.Start) + ", 0, FALSE," + Bind(pSampleRate) + ", " + Bind(Type) + ", " + Bind(evt.Reference) + ", " + Bind(evt.PeakValue) + ", " + Bind(evt.ChangeValue) + ", '', " + Bind(RAW) + ", " + Bind(RMS) + ")");
  } else if(evt.State == EVT_CLOSING) {
    ExecSentenceNoResult("UPDATE events SET duration = " + Bind(evt.Duration) + ", peak = " + Bind(evt.PeakValue) + ", change = " + Bind(evt.ChangeValue) + ", samples2 = " + Bind(RAW) + ", rms2 = " + Bind(RMS) + " WHERE starttime = " + Bind(evt.Start) + " AND evttype = " + Bind(Type));
  } else {
    ExecSentenceNoResult("UPDATE events SET samples3 = " + Bind(RAW) + ", rms3 = " + Bind(RMS) +  + ", finished = TRUE WHERE starttime = " + Bind(evt.Start) + " AND evttype = " + Bind(Type));
  }
  evt.SamplesRAW_A = deque<vector<float> >();
  evt.SamplesRMS_A = deque<vector<float> >();
  evt.SamplesRAW_B = deque<vector<float> >();
  evt.SamplesRMS_B = deque<vector<float> >();
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
      if(Stationary != This->pPrevStationary)
        LOGFile::Debug("Analyzer '%s' %s stationary state\n", This->pAnalyzer->Serial().c_str(), Stationary ? "reach" : "leave");
      //Check for new events
      if((This->pPrevSamples.size() >= (EVENT_RAWENVELOPETIME * This->pSampleRate)) && (This->pPrevCycles.size() >= (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME))) {
        for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
          Event_t Event;
          Event.Type = EVT_NONE;
          //Check for interruption new events
          if((-PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) > EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) Event.Type = EVT_INTERRUPTION;
          //Check for DIP new events
          if((-PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP) == false)) Event.Type = EVT_DIP;
          //Check for SWELL new events
          if((PercentChange(Cycle.RMS[n], This->pAnalyzer->GetNominalVoltage()) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) Event.Type = EVT_SWELL;
          //Check for RVC new events
          if((Stationary == false) && (This->pPrevStationary == true) && (This->FindRunningEvent(EVT_INTERRUPTION | EVT_DIP | EVT_SWELL | EVT_RVC) == false)) Event.Type = EVT_RVC;
          if(Event.Type != EVT_NONE) {
            Event.Reference = (Event.Type == EVT_RVC) ? RMS_Avg : vector<float>(RMS_Avg.size(), This->pAnalyzer->GetNominalVoltage());
            Event.PeakValue = Cycle.RMS;
            Event.ChangeValue = vector<float>(Cycle.RMS.size(), 0.0);
            Event.Duration = 0;
            Event.State = EVT_RUNNING;
            Event.Start =  This->pParams->Timestamp + ((Pos * 1000) / This->pSampleRate);
            Event.SamplesRAW_A = deque<vector<float> >();
            Event.SamplesRMS_A = deque<vector<float> >();
            Event.SamplesRAW_B = deque<vector<float> >();
            Event.SamplesRMS_B = deque<vector<float> >();
            for(deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin(); it != This->pPrevCycles.rend(); it++) {
              Event.SamplesRMS_A.push_front(it->RMS);
              if(Event.SamplesRMS_A.size() == (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME)) break;
            }
            for(deque<vector<float> >::reverse_iterator it = This->pPrevSamples.rbegin(); it != This->pPrevSamples.rend(); it++) {
              Event.SamplesRAW_A.push_front(*it);
              if(Event.SamplesRAW_A.size() == (EVENT_RAWENVELOPETIME * This->pSampleRate)) break;
            }
            This->Save(Event);
            This->pEvents.push_back(Event);
          }
        }
      }
      //Insert current cycle samples in prev data buffers and in running events than need its
      for(uint n = 0; n < CycleLen; n++) {
        vector<float> Sample;
        for(uint j = 0; j < This->pParams->Voltage_Samples.size(); j++) Sample.push_back(This->pParams->Voltage_Samples[j][n + Pos]);
        This->pPrevSamples.push_back(Sample);
        if(This->pPrevSamples.size() > (This->pSampleRate * EVENT_RAWENVELOPETIME)) This->pPrevSamples.pop_front();
        for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
          if(it->SamplesRAW_A.size() < (This->pSampleRate * EVENT_RAWENVELOPETIME)) {
            it->SamplesRAW_A.push_back(Sample);
          } else {
            it->SamplesRAW_B.push_back(Sample);
            if(it->SamplesRAW_B.size() > (This->pSampleRate * EVENT_RAWENVELOPETIME)) it->SamplesRAW_B.pop_front();
          }
        }
      }
      //Insert current cycle in prev data buffers and in running events than need its
      This->pPrevCycles.push_back(Cycle);
      if(This->pPrevCycles.size() > (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME)) This->pPrevCycles.pop_front();
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
        if(it->SamplesRMS_A.size() < (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME)) {
          it->SamplesRMS_A.push_back(Cycle.RMS);
        } else {
          it->SamplesRMS_B.push_back(Cycle.RMS);
          if(it->SamplesRMS_B.size() > (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME)) it->SamplesRMS_B.pop_front();
        }
      }
      //Check events state
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); ) {
        //If closed evend finish post event period save it and remove from list
        if((it->State == EVT_CLOSING) && (it->SamplesRAW_A.size() == (This->pSampleRate * EVENT_RAWENVELOPETIME)) && (it->SamplesRMS_A.size() == (This->pAnalyzer->GetNominalFrequency() * 2 * EVENT_RMSENVELOPETIME))) {
          it->State = EVT_CLOSED;
          This->Save(*it);
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
              if(-PercentChange(Cycle.RMS[ch], This->pAnalyzer->GetNominalVoltage()) > (EVENT_INTERRUPTION - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
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
              if(-PercentChange(Cycle.RMS[ch], This->pAnalyzer->GetNominalVoltage()) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
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
              if(PercentChange(Cycle.RMS[ch], This->pAnalyzer->GetNominalVoltage()) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(Cycle.RMS[ch] > it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }          
          if(it->State == EVT_RUNNING) {
            it->Duration += CycleLen;
            if(it->Type != EVT_RVC) This->pParams->Flag = true;
          } else {
            This->Save(*it);
          }
          it++;
        }
      }      
      Pos = Pos + CycleLen;
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
  return ((in - reference) / reference) * 100.0;
}