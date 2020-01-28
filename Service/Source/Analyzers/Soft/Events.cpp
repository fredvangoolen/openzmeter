// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "Events.h"
#include "Database.h"
#include "AnalyzerSoft.h"
AnalyzerSoft::Events::Events(AnalyzerSoft *parent) {
//  pParent         = parent;
//  pTerminate      = false;
//  pEventThread    = 0;
//  pPrevStationary = false;
//  if(pthread_create(&pEventThread, NULL, Events_Thread, (void*)this) != 0) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pEventThread = 0;
//    return;
//  }
//  pthread_setname_np(pEventThread, "EventsDet");
}
void AnalyzerSoft::Events::Start() {
  pWorkerLocks.WakeWorker();
}
void AnalyzerSoft::Events::Wait() {
  pWorkerLocks.WaitWorker();
}
AnalyzerSoft::Events::~Events() {
  pTerminate = true;
  pWorkerLocks.Abort(&pEventThread);
}


void *AnalyzerSoft::Events::Events_Thread(void* args) {
  Events *This = (Events*)args;
  uint32_t RequiredRAW = This->pParent->pSamplerate * EVENT_RAWENVELOPETIME;
  uint32_t RequiredRMS = This->pParent->pNominalFrequency * 2 * EVENT_RMSENVELOPETIME;
  while(This->pTerminate == false) {
    if(This->pWorkerLocks.WorkerWaitTask() == false) break;
    uint32_t Pos = 0;
    This->pParent->pAggreg10C.Flag = false;
    for(uint32_t i = 0; i < This->pParent->pBlockCycles.size(); i++) {
      //Calc cycle stats
      EventCycle_t Cycle;
      uint32_t CycleLen = This->pParent->pBlockCycles[i];
      for(uint n = 0; n < This->pParent->pVoltageSamples.size(); n++) {
        float RMS = 0.0;
        for(uint32_t j = 0; j < CycleLen; j++) RMS += pow(This->pParent->pVoltageSamples[n][Pos + j], 2);
        Cycle.RMS.push_back(sqrt(RMS / CycleLen));
      }
      //Calc last 100 cycles AVG (current and previous ones)
      vector<float> RMS_Avg = Cycle.RMS;
      for(uint8_t n = 0; n < Cycle.RMS.size(); n++) {
        uint32_t Len = This->pPrevCycles.size();
        if(Len > 99) Len = 99;
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        for(uint32_t j = 0; j < Len; j++, it++) RMS_Avg[n] += it->RMS[n];
        RMS_Avg[n] = RMS_Avg[n] / (Len + 1);
      }
      //Check steady state
      float Tolerance = EVENT_RVC - ((This->pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
      bool  Stationary = (This->pPrevCycles.size() >= 99);
      for(uint8_t n = 0; (n < RMS_Avg.size()) && (Stationary == true); n++) {
        deque<EventCycle_t>::reverse_iterator it = This->pPrevCycles.rbegin();
        if(fabs(This->pParent->CalcChange(Cycle.RMS[n], RMS_Avg[n])) > Tolerance) {
          Stationary = false;
          break;
        }
        for(int j = 0; j < 99; j++) {
          if(fabs(This->pParent->CalcChange(it->RMS[n], RMS_Avg[n])) > Tolerance) {
            Stationary = false;
            break;
          }
        }
      }
      //Check for new events
      if((This->pPrevSamples.size() >= RequiredRAW) && (This->pPrevCycles.size() >= RequiredRMS)) {
        for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
          //Check for interruption new events
          if((-This->pParent->CalcChange(Cycle.RMS[n], This->pParent->pNominalVoltage) > EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) {
            bool Found = false;
            for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
              if((it->Type != EVT_DIP) || (it->State != EVT_RUNNING) || (it->Duration > (EVENT_RAWENVELOPETIME * This->pParent->pSamplerate))) continue;
              it->Type = EVT_INTERRUPTION;
              Found = true;
              break;
            }
            if(Found == false) This->InitEvent(EVT_INTERRUPTION, vector<float>(RMS_Avg.size(), This->pParent->pNominalVoltage), Pos);
          }
          //Check for DIP new events
          if((-This->pParent->CalcChange(Cycle.RMS[n], This->pParent->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP | EVT_INTERRUPTION) == false)) This->InitEvent(EVT_DIP, vector<float>(RMS_Avg.size(), This->pParent->pNominalVoltage), Pos);
          //Check for SWELL new events
          if((This->pParent->CalcChange(Cycle.RMS[n], This->pParent->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) This->InitEvent(EVT_SWELL, vector<float>(RMS_Avg.size(), This->pParent->pNominalVoltage), Pos);
          //Check for RVC new events
          if((Stationary == false) && (This->pPrevStationary == true) && (This->FindRunningEvent(EVT_RVC | EVT_DIP | EVT_SWELL | EVT_INTERRUPTION) == false)) This->InitEvent(EVT_RVC, RMS_Avg, Pos);
        }
      }
      //Check events state
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); ) {
        //If closed evend finish post event period save it and remove from list
        if((it->State == EVT_CLOSING) && (it->SamplesRAW_A.size() == RequiredRAW) && (it->SamplesRMS_A.size() == RequiredRMS)) {
          it->State = EVT_CLOSED;
          This->SaveEvent(*it);
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
              if(-This->pParent->CalcChange(Cycle.RMS[ch], This->pParent->pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS)) it->State = EVT_RUNNING;
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
              if(-This->pParent->CalcChange(Cycle.RMS[ch], This->pParent->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
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
              if(This->pParent->CalcChange(Cycle.RMS[ch], This->pParent->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
                if(Cycle.RMS[ch] > it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
              }
            }
          }
          if(it->State == EVT_RUNNING) {
            it->Duration += CycleLen;
            if(it->Type != EVT_RVC) This->pParent->pAggreg10C.Flag = true;
          } else {
            This->SaveEvent(*it);
          }
          it++;
        }
      }
      //Insert current cycle samples in prev data buffers and in running events than need its
      for(uint n = 0; n < CycleLen; n++) {
        vector<float> Sample;
        for(uint j = 0; j < This->pParent->pVoltageSamples.size(); j++) Sample.push_back(This->pParent->pVoltageSamples[j][n + Pos]);
        This->pPrevSamples.push_back(Sample);
        if(This->pPrevSamples.size() > RequiredRAW) This->pPrevSamples.pop_front();
        for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
          if(it->SamplesRAW_A.size() < RequiredRAW) {
            it->SamplesRAW_A.push_back(Sample);
          } else if(it->State == EVT_RUNNING) {
            it->SamplesRAW_B.push_back(Sample);
            if(it->SamplesRAW_B.size() > RequiredRAW) it->SamplesRAW_B.pop_front();
          }
        }
      }
      //Insert current cycle in prev data buffers and in running events than need its
      This->pPrevCycles.push_back(Cycle);
      if(This->pPrevCycles.size() > RequiredRMS) This->pPrevCycles.pop_front();
      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
        if(it->SamplesRMS_A.size() < RequiredRMS) {
          it->SamplesRMS_A.push_back(Cycle.RMS);
        } else if(it->State == EVT_RUNNING) {
          it->SamplesRMS_B.push_back(Cycle.RMS);
          if(it->SamplesRMS_B.size() > RequiredRMS) it->SamplesRMS_B.pop_front();
        }
      }
      Pos = Pos + CycleLen;
      This->pPrevStationary = Stationary;
    }
    This->pWorkerLocks.WorkerEndsTask();
  }
  return NULL;
}

