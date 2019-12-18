#include "Events.h"
#include "Database.h"
#include "AnalyzerSoft.h"
AnalyzerSoft::Events::Events(AnalyzerSoft *parent) {
  pParent          = parent;
  pTerminate       = false;
  pEventThread     = 0;
  pPrevStationary  = false;
  pPrevSamplesSize = EVENT_RAWENVELOPETIME * parent->pSamplerate;
  pPrevSamples     = (float*)malloc(pPrevSamplesSize * parent->pVoltageChannels);
  pPrevSamplesPos  = 0;
  if(pthread_create(&pEventThread, NULL, Events_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pEventThread = 0;
    return;
  }
  pthread_setname_np(pEventThread, "EventsDet");
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
void AnalyzerSoft::Events::SaveEvent(Event_t &evt) {
  Database DB(pParent->Serial());
  string Type = (evt.Type == EVT_RVC) ? "RVC" : (evt.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (evt.Type == EVT_DIP) ? "DIP" : "SWELL";
  vector<vector<float> > RMS = vector<vector< float> >(pParent->pVoltageChannels, vector<float>());
  vector<vector<float> > RAW = vector<vector< float> >(pParent->pVoltageChannels, vector<float>());
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
    DB.ExecSentenceNoResult("INSERT INTO events(starttime, duration, samplerate, finished, evttype, reference, peak, change, notes, samples1, rms1) VALUES (" + DB.Bind(evt.Start) + ", 0, " + DB.Bind(pParent->pSamplerate) + ", false, " + DB.Bind(Type) + ", " + DB.Bind(evt.Reference, pParent->pVoltageChannels) + ", " + DB.Bind(evt.PeakValue, pParent->pVoltageChannels) + ", " + DB.Bind(evt.ChangeValue, pParent->pVoltageChannels) + ", '', " + DB.Bind(RAW) + ", " + DB.Bind(RMS) + ")");
  } else if(evt.State == EVT_CLOSING) {
    DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", duration = " + DB.Bind(evt.Duration) + ", peak = " + DB.Bind(evt.PeakValue) + ", change = " + DB.Bind(evt.ChangeValue, pParent->pVoltageChannels) + ", samples2 = " + DB.Bind(RAW) + ", rms2 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
  } else {
    DB.ExecSentenceNoResult("UPDATE events SET finished = TRUE, samples3 = " + DB.Bind(RAW) + ", rms3 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
  }
  evt.SamplesRAW_A = deque<vector<float> >();
  evt.SamplesRMS_A = deque<vector<float> >();
  evt.SamplesRAW_B = deque<vector<float> >();
  evt.SamplesRMS_B = deque<vector<float> >();
}
void AnalyzerSoft::Events::InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos) {
  Event_t Event;
  Event.Type = type;
  Event.Duration = 0;
  Event.State = EVT_RUNNING;
  for(unsigned i = 0; i < pParent->pVoltageChannels; i++) {
    Event.Reference[i] = reference[i];
    Event.PeakValue[i] = reference[i];
    Event.ChangeValue[i] = reference[i];
  }
  Event.SamplesRAW_A = deque<vector<float> >();
  Event.SamplesRMS_A = deque<vector<float> >();
  Event.SamplesRAW_B = deque<vector<float> >();
  Event.SamplesRMS_B = deque<vector<float> >();
  for(auto it = pPrevCycles.rbegin(); it != pPrevCycles.rend(); it++) {
    Event.SamplesRMS_A.push_front(*it);
    if(Event.SamplesRMS_A.size() == (pParent->pNominalFrequency * 2 * EVENT_RMSENVELOPETIME)) break;
  }
  uint32_t Pos = pPrevSamplesPos;
  for(unsigned i = 0; i < pPrevSamplesSize; i++) {
    Pos = (Pos - 1) % pPrevSamplesSize;
    float *Ptr = &pPrevSamples[Pos * pParent->pVoltageChannels];
    vector<float> Sample;
    for(unsigned n = 0; n < pParent->pVoltageChannels; n++) Sample.push_back(Ptr[n]);
    Event.SamplesRAW_A.push_front(Sample);
  }
//  for(auto it = pPrevSamples.rbegin(); it != pPrevSamples.rend(); it++) {
//    Event.SamplesRAW_A.push_front(*it);
//    if(Event.SamplesRAW_A.size() == (EVENT_RAWENVELOPETIME * pParent->pSamplerate)) break;
//  }
  //A new event destroy any running RVC
  if(type != EVT_RVC) {
    for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); ) {
      if((it->Type == EVT_RVC) && (it->State == EVT_RUNNING)) {
        Database DB(pParent->Serial());
        DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(it->Start) + " AND evttype = 'RVC'");
        pEvents.erase(it++);
      } else {
        it++;
      }
    }
  }
  Event.Start = pParent->pAggreg10C.Timestamp + (uint32_t)((pos * 1000) / pParent->pSamplerate);
  SaveEvent(Event);
  pEvents.push_back(Event);
}
bool AnalyzerSoft::Events::FindRunningEvent(const uint32_t mask) {
  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
  }
  return false;
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
      vector<float> Cycle;
      uint32_t CycleLen = This->pParent->pBlockCycles[i];
      for(uint n = 0; n < This->pParent->pVoltageSamples.size(); n++) {
        float RMS = 0.0;
        for(uint32_t j = 0; j < CycleLen; j++) RMS += pow(This->pParent->pVoltageSamples[n][Pos + j], 2);
        Cycle.push_back(sqrt(RMS / CycleLen));
      }
      //Calc last 100 cycles AVG (current and previous ones)
      vector<float> RMS_Avg = Cycle;
      for(uint8_t n = 0; n < Cycle.size(); n++) {
        uint32_t Len = This->pPrevCycles.size();
        if(Len > 99) Len = 99;
        auto it = This->pPrevCycles.rbegin();
        for(uint32_t j = 0; j < Len; j++, it++) RMS_Avg[n] += (*it)[n];
        RMS_Avg[n] = RMS_Avg[n] / (Len + 1);
      }
      //Check steady state
      float Tolerance = EVENT_RVC - ((This->pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
      bool  Stationary = (This->pPrevCycles.size() >= 99);
      for(uint8_t n = 0; (n < RMS_Avg.size()) && (Stationary == true); n++) {
        auto it = This->pPrevCycles.rbegin();
        if(fabs(This->pParent->CalcChange(Cycle[n], RMS_Avg[n])) > Tolerance) {
          Stationary = false;
          break;
        }
        for(int j = 0; j < 99; j++) {
          if(fabs(This->pParent->CalcChange((*it)[n], RMS_Avg[n])) > Tolerance) {
            Stationary = false;
            break;
          }
        }
      }
      //Check for new events
      if( (This->pPrevCycles.size() >= RequiredRMS)) {
        for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
          //Check for interruption new events
          if((-This->pParent->CalcChange(Cycle[n], This->pParent->pNominalVoltage) > EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) {
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
          if((-This->pParent->CalcChange(Cycle[n], This->pParent->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP | EVT_INTERRUPTION) == false)) This->InitEvent(EVT_DIP, vector<float>(RMS_Avg.size(), This->pParent->pNominalVoltage), Pos);
          //Check for SWELL new events
          if((This->pParent->CalcChange(Cycle[n], This->pParent->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) This->InitEvent(EVT_SWELL, vector<float>(RMS_Avg.size(), This->pParent->pNominalVoltage), Pos);
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
              for(unsigned i = 0; i < This->pParent->pVoltageChannels; i++) it->ChangeValue[i] = RMS_Avg[i];
            } else {
              for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
                if(fabs(it->Reference[ch] - Cycle[ch]) > fabs(it->Reference[ch] - it->PeakValue[ch])) it->PeakValue[ch] = Cycle[ch];
              }
            }
          }
          //Check if INTERRUPTION event can be closed
          if(it->Type == EVT_INTERRUPTION) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
              if(-This->pParent->CalcChange(Cycle[ch], This->pParent->pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
                if(Cycle[ch] < it->PeakValue[ch]) it->PeakValue[ch] = Cycle[ch];
              }
            }
          }
          //Check if DIP event can be closed
          if(it->Type == EVT_DIP) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
              if(-This->pParent->CalcChange(Cycle[ch], This->pParent->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
                if(Cycle[ch] < it->PeakValue[ch]) it->PeakValue[ch] = Cycle[ch];
              }
            }
          }
          //Check if SWELL event can be closed
          if(it->Type == EVT_SWELL) {
            it->State = EVT_CLOSING;
            for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
              if(This->pParent->CalcChange(Cycle[ch], This->pParent->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
            }
            if(it->State == EVT_RUNNING) {
              for(uint8_t ch = 0; ch < Cycle.size(); ch++) {
                if(Cycle[ch] > it->PeakValue[ch]) it->PeakValue[ch] = Cycle[ch];
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
        //This->pPrevSamples.push_back(Sample);
        for(unsigned i = 0; i < This->pParent->pVoltageChannels; i++) This->pPrevSamples[This->pPrevSamplesPos * This->pParent->pVoltageChannels + i] = Sample[i];
        This->pPrevSamplesPos = (This->pPrevSamplesPos + 1) % This->pPrevSamplesSize;
        
//        if(This->pPrevSamples.size() > RequiredRAW) This->pPrevSamples.pop_front();
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
          it->SamplesRMS_A.push_back(Cycle);
        } else if(it->State == EVT_RUNNING) {
          it->SamplesRMS_B.push_back(Cycle);
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
void AnalyzerSoft::Events::GetEvents(HTTPRequest &req) {
  json Params = req.GetParams();
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT json_build_object('From', " + req.Bind(From) + ", 'To', " + req.Bind(To) + ", 'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), 'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                              "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), 'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), 'Finished', COALESCE(ARRAY_AGG(finished ORDER BY starttime ASC), '{}'), "
                              "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), 'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), 'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), 'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                              "FROM events WHERE starttime > " + req.Bind(LastSample) + " AND starttime >= " + req.Bind(From) + " AND starttime <= " + req.Bind(To));
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
}
void AnalyzerSoft::Events::GetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  if((Params["Samples"] != "RMS") && (Params["Samples"] != "RAW")) return req.FromString("'Samples' params must be 'RMS' or 'RAW'", MHD_HTTP_BAD_REQUEST);
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Samples = Params["Samples"];
  if(Samples == "RAW") {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', samples1, 'Samples', samples2, 'Post_Samples', samples3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  } else {
    req.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', rms1, 'Samples', rms2, 'Post_Samples', rms3) AS evt FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  }
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
}
void AnalyzerSoft::Events::SetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  string Notes = Params["Notes"];
  bool Result = req.ExecSentenceNoResult("UPDATE events SET Notes = " + req.Bind(Notes) + " WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::Events::DelEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  uint64_t ID = Params["StartTime"];
  string Type = Params["Type"];
  bool Result = req.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}