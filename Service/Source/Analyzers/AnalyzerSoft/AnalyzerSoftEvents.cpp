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
#include "AnalyzerSoftEvents.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
constexpr float AnalyzerSoftEvents::Event_RVC;
constexpr float AnalyzerSoftEvents::Event_DIP_SWELL;
constexpr float AnalyzerSoftEvents::Event_INT;
constexpr float AnalyzerSoftEvents::Event_Hysteresis;
constexpr float AnalyzerSoftEvents::Event_RAW_Time;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftEvents::AnalyzerSoftEvents(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate) {
  pMutex            = PTHREAD_MUTEX_INITIALIZER;
  pVoltageChannels  = (mode == Analyzer::PHASE1) ? 1 : 3;
  pSerial           = serial;
  pSamplerate       = samplerate;
  pNominalVoltage   = 230.0;
  pEvent            = NULL;
  pPrevStationary   = false;
  pPrevSamplesLen   = 0;
  pPrevSamplesPos   = 0;
  pPrevSamples      = NULL;
  pPrevCyclesLen    = 0;
  pPrevCyclesPos    = 0;
  pPrevCycles       = NULL;
  pWorker           = NULL;
  pHaveEvent        = false;
}
AnalyzerSoftEvents::~AnalyzerSoftEvents() {
  if(pEvent != NULL) delete pEvent;
  if(pPrevSamples != NULL) free(pPrevSamples);
  if(pPrevCycles != NULL) free(pPrevCycles);
  for(auto &Prev : pEvents) delete Prev;
  pEvents.clear();
}
void AnalyzerSoftEvents::Config(const bool enabled, const float nominalVoltage, const float nominalFreq) {
  pthread_mutex_lock(&pMutex);
  if(pPrevCycles != NULL) free(pPrevCycles);
  if(pPrevSamples != NULL) free(pPrevSamples);
  if(pEvent != NULL) delete pEvent;
  for(auto &Prev : pEvents) delete Prev;
  pEvents.clear();
  pNominalVoltage   = nominalVoltage;
  pNominalFrequency = nominalFreq;
  pPrevCycles       = NULL;
  pPrevSamples      = NULL;
  pEvent            = NULL;
  pPrevStationary   = false;
  pPrevSamplesLen   = Event_RAW_Time * pSamplerate;
  pPrevSamplesPos   = 0;
  pPrevCyclesPos    = 0;
  pPrevCyclesLen    = (nominalFreq * 2) - 1;
  pPrevSamples      = (float*)calloc(pPrevSamplesLen * pVoltageChannels, sizeof(float));
  pPrevCycles       = (float*)calloc(pPrevCyclesLen * pVoltageChannels, sizeof(float));
  if((enabled == false) && (pWorker != NULL)) {
    delete pWorker;
    pWorker = NULL;
  } else if((enabled == true) && (pWorker == NULL)) {
    pWorker = new Worker("EventsDet", [&](void *a) { this->Events_Func((const AnalyzerSoftBlock *)a); });
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftEvents::Run(const AnalyzerSoftBlock *block, const uint64_t timestamp) {
  pthread_mutex_lock(&pMutex);
  pHaveEvent = false;
  pTimestamp = timestamp;
  if(pWorker != NULL) pWorker->Run((void*)block);
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerSoftEvents::Wait() {
  pthread_mutex_lock(&pMutex);
  if(pWorker != NULL) pWorker->Wait();
  bool Result = ((pWorker != NULL) && pHaveEvent);
  pthread_mutex_unlock(&pMutex);
  return Result;
}
float AnalyzerSoftEvents::CalcChange(const float in, const float reference) const {
  return ((in - reference) / reference) * 100;
}
string AnalyzerSoftEvents::EventTypeToString(const AnalyzerSoftEvents::EventType type) const {
  return (type == EVT_RVC) ? "RVC" : (type == EVT_INTERRUPTION) ? "INTERRUPTION" : (type == EVT_DIP) ? "DIP" : "SWELL";
}
AnalyzerSoftEvents::Event_t *AnalyzerSoftEvents::InitEvent() {
  Event_t *Event = new Event_t;
  Event->UID = Tools::GetUUID();
  Event->SamplesDuration = 0;
  Event->CyclesDuration = 0;
  return Event;
}
//  float    *Buff = NULL;
//  auto CopyCycles = [&](float *buff, const uint16_t count) -> float* {
//    buff = (float*)realloc(buff, sizeof(float) * pVoltageChannels * count);
//    for(uint16_t n = 0; n < count; n++) {
//      for(uint8_t i = 0; i < pVoltageChannels; i++) buff[i * count + n] = pPrevCycles[((pPrevCyclesPos + pPrevCyclesLen - count + n) % pPrevCyclesLen) * pVoltageChannels + i];
//    }
//    return buff;
//  };
//  auto CopySamples = [&](float *buff, const uint16_t count) -> float* {
//    buff = (float*)realloc(buff, sizeof(float) * pVoltageChannels * count);
//    for(uint16_t n = 0; n < count; n++) {
//      for(uint8_t i = 0; i < pVoltageChannels; i++) buff[i * count + n] = pPrevSamples[((pPrevSamplesPos + pPrevSamplesLen - count + n) % pPrevSamplesLen) * pVoltageChannels + i];
//    }
//    return buff;
//  };
//  free(Buff);
void AnalyzerSoftEvents::Events_Func(const AnalyzerSoftBlock *block) {
  uint16_t Pos = 0;
  if(pNominalFrequency == 0.0) return;
  for(uint8_t i = 0; i < block->Cycles(); i++) {
    uint16_t CycleLen   = block->CycleLen(i);
    bool     Stationary = true;
    float    Tolerance  = Event_RVC - ((pPrevStationary == true) ? 0.0 : Event_Hysteresis);
    float    Cycle[3]   = {0.0, 0.0, 0.0};
    float    RMS_Avg[3] = {0.0, 0.0, 0.0};
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      for(uint16_t j = 0; j < CycleLen; j++) Cycle[n] += pow(block->VoltageSample(n, Pos + j), 2.0);
      Cycle[n] = RMS_Avg[n] = sqrt(Cycle[n] / CycleLen);
      for(uint16_t j = 0; j < pPrevCyclesLen; j++) RMS_Avg[n] += pPrevCycles[j * pVoltageChannels + n];
      RMS_Avg[n] = RMS_Avg[n] / (pPrevCyclesLen + 1);
    }
    for(uint8_t n = 0; (n < pVoltageChannels) && (Stationary == true); n++) {
      Stationary = (fabs(CalcChange(Cycle[n], RMS_Avg[n])) <= Tolerance);
      for(uint16_t j = 0; (j < pPrevCyclesLen) && (Stationary == true); j++) Stationary = (fabs(CalcChange(pPrevCycles[j * pVoltageChannels + n], RMS_Avg[n])) <= Tolerance);
    }
    if(pEvent != NULL) {
      bool IsRunning = true;
      for(uint8_t n = 0; (n < pVoltageChannels) && (IsRunning == true); n++) {
        if((pEvent->Type == EVT_INTERRUPTION) && (-CalcChange(Cycle[n], pNominalVoltage) > (Event_INT - Event_Hysteresis))) {
          IsRunning = false;
        } else if((pEvent->Type == EVT_DIP) && (-CalcChange(Cycle[n], pNominalVoltage) > (Event_DIP_SWELL - Event_Hysteresis))) {
          IsRunning = false;
        } else if((pEvent->Type == EVT_SWELL) && (CalcChange(Cycle[n], pNominalVoltage) > (Event_DIP_SWELL - Event_Hysteresis))) {
          IsRunning = false;
        } else if((pEvent->Type == EVT_RVC) && (Stationary == true)) {
          memcpy(pEvent->ChangeValue, RMS_Avg, sizeof(float) * pVoltageChannels);
          IsRunning = false;
        }
      }
      if(IsRunning == false) {
        string Type = EventTypeToString(pEvent->Type);
        LogFile::Debug("Event '%s' closed on analyzer '%s'\n", Type.c_str(), pSerial.c_str());
        Database DB(pSerial);
        DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", reference = " + DB.Bind(pEvent->Reference, pVoltageChannels) + ", peak = " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent->SamplesDuration) + " WHERE starttime = " + DB.Bind(pEvent->Start));
/*
        if(pEvent->CyclesDuration < pPrevCyclesLen) {
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RMS'");
        } else if(pEvent->CyclesDuration > pPrevCyclesLen) {
          pEvent->CyclesDuration = (pEvent.CyclesDuration < (pPrevCyclesLen * 2)) ? pEvent.CyclesDuration - pPrevCyclesLen : pPrevCyclesLen;
          DB.ExecSentenceNoResult("WITH tbl as (SELECT starttime as srctime, sampletype as srctype, samples2 as src, " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) +"::real[] as dst, generate_subscripts(samples2, 1) AS idx FROM events_ext) UPDATE events_ext SET samples2 = (SELECT array_agg(array(select unnest(src[idx:idx])) || array(select unnest(dst[idx:idx]))) FROM tbl WHERE srctype = sampletype and srctime = starttime) WHERE sampletype = 'RMS' and starttime = " + DB.Bind(pEvent.Start));
        }
        if(pEvent->SamplesDuration < pPrevSamplesLen) {
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RAW'");
        } else if(pEvent->SamplesDuration > pPrevSamplesLen) {
          pEvent->SamplesDuration = (pEvent->SamplesDuration < (pPrevSamplesLen * 2)) ? pEvent.SamplesDuration - pPrevSamplesLen : pPrevSamplesLen;
          DB.ExecSentenceNoResult("WITH tbl as (SELECT starttime as srctime, sampletype as srctype, samples2 as src, " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) +"::real[] as dst, generate_subscripts(samples2, 1) AS idx FROM events_ext) UPDATE events_ext SET samples2 = (SELECT array_agg(array(select unnest(src[idx:idx])) || array(select unnest(dst[idx:idx]))) FROM tbl WHERE srctype = sampletype and srctime = starttime) WHERE sampletype = 'RAW' and starttime = " + DB.Bind(pEvent.Start));
        }
        pEvent.CyclesDuration  = 0;
        pEvent.SamplesDuration = 0;*/
        pEvents.push_back(pEvent);
        pEvent = NULL;
      }
    }
    if(block->Sequence() > 100) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        if(((pEvent == NULL) || (pEvent->Type != EVT_INTERRUPTION)) && (-CalcChange(Cycle[n], pNominalVoltage) > Event_INT)) {
          if(pEvent == NULL) pEvent = InitEvent();
          pEvent->Type = EVT_INTERRUPTION;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent->Reference[i] = pNominalVoltage;
          memcpy(pEvent->PeakValue, Cycle, sizeof(float) * pVoltageChannels);
        } else if(((pEvent == NULL) || ((pEvent->Type != EVT_DIP) && (pEvent->Type != EVT_INTERRUPTION))) && (-CalcChange(Cycle[n], pNominalVoltage) > Event_DIP_SWELL)) {
          if(pEvent == NULL) pEvent = InitEvent();
          pEvent->Type = EVT_DIP;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent->Reference[i] = pNominalVoltage;
          memcpy(pEvent->PeakValue, Cycle, sizeof(float) * pVoltageChannels);
        } else if(((pEvent == NULL) || (pEvent->Type != EVT_SWELL)) && (CalcChange(Cycle[n], pNominalVoltage) > Event_DIP_SWELL)) {
          if(pEvent == NULL) pEvent = InitEvent();
          pEvent->Type = EVT_SWELL;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent->Reference[i] = pNominalVoltage;
          memcpy(pEvent->PeakValue, Cycle, sizeof(float) * pVoltageChannels);
        } else if((pEvent == NULL) && (Stationary == false) && (pPrevStationary == true)) {
          if(pEvent == NULL) pEvent = InitEvent();
          pEvent->Type = EVT_RVC;
          memcpy(pEvent->Reference, RMS_Avg, sizeof(float) * pVoltageChannels);
          memcpy(pEvent->PeakValue, Cycle, sizeof(float) * pVoltageChannels);
        }
      }
    }
    if(pEvent != NULL) {
      pHaveEvent = true;
      if(pEvent->SamplesDuration == 0) {
        string Type = EventTypeToString(pEvent->Type);
        LogFile::Debug("New event '%s' started on analyzer '%s'", Type.c_str(), pSerial.c_str());
        pEvent->Start = pTimestamp + (uint32_t)((Pos * 1000) / pSamplerate);
        Database DB(pSerial);
//        DB.ExecSentenceNoResult("INSERT INTO events(starttime, evttype, samplerate, reference, peak, change, notes, duration) VALUES (" + DB.Bind(pEvent.Start) + ", " + DB.Bind(Type) + ", " + DB.Bind(pSamplerate) + ", " + DB.Bind(pEvent.Reference, pVoltageChannels) + ", " + DB.Bind(pEvent.PeakValue, pVoltageChannels) + ", " + DB.Bind(pEvent.ChangeValue, pVoltageChannels) + ", '', 0)");
//        DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RMS', " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + ")");
//        DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RAW', " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + ")");
      } else if(pEvent->Type == EVT_INTERRUPTION) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmin(pEvent->PeakValue[n], Cycle[n]);
      } else if(pEvent->Type == EVT_DIP) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmin(pEvent->PeakValue[n], Cycle[n]);
      } else if(pEvent->Type == EVT_SWELL) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmax(pEvent->PeakValue[n], Cycle[n]);
      } else if(pEvent->Type == EVT_RVC) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = (fabs(pEvent->Reference[n] - Cycle[n]) > fabs(pEvent->Reference[n] - pEvent->PeakValue[n])) ? Cycle[n] : pEvent->PeakValue[n];
      }
    }
    for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevCycles[pPrevCyclesPos * pVoltageChannels + n] =  Cycle[n];
    pPrevCyclesPos  = (pPrevCyclesPos + 1) % pPrevCyclesLen;
//    if(pEvent.Type != EVT_NONE) {
//      pEvent.CyclesDuration++;
//      if(pEvent.CyclesDuration == pPrevCyclesLen) {
//        Database DB(pSerial);
//        DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RMS'");
//      }
//    }
//    for(auto &Event : pEvents) {
//      Event.CyclesDuration++;
//      if(Event.CyclesDuration == pPrevCyclesLen) {
//        Database DB(pSerial);
//        DB.ExecSentenceNoResult("UPDATE events_ext SET samples3 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(Event.Start) + " AND sampletype = 'RMS'");
//      }
//    }
    for(uint16_t j = 0; j < CycleLen; j++) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevSamples[pPrevSamplesPos * pVoltageChannels + n] = block->VoltageSample(n, Pos + j);
      pPrevSamplesPos = (pPrevSamplesPos + 1) % pPrevSamplesLen;
//      if(pEvent.Type != EVT_NONE) {
//        pEvent.SamplesDuration++;
//        if(pEvent.SamplesDuration == pPrevSamplesLen) {
//          Database DB(pSerial);
//          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RAW'");
//        }
      }
//      for(auto &Event : pEvents) {
//        Event.SamplesDuration++;
//        if(Event.SamplesDuration == pPrevSamplesLen) {
//          Database DB(pSerial);
//          DB.ExecSentenceNoResult("UPDATE events_ext SET samples3 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(Event.Start) + " AND sampletype = 'RAW'");
//        }
//      }
//    }
//    while(pEvents.size() != 0) {
//      Event_t *Tmp = &pEvents.front();
//      if((Tmp->SamplesDuration < pPrevSamplesLen) || (Tmp->CyclesDuration < pPrevCyclesLen)) break;
//      string Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
//      LogFile::Debug("Event '%s' on analyzer '%s' finished", Type.c_str(), pSerial.c_str());
//      pEvents.pop_front();
//    }
    Pos             = Pos + CycleLen;
    pPrevStationary = Stationary && (pEvent == NULL);
  }
}
void AnalyzerSoftEvents::GetEvents(const json &params, Response &response) {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To = params["To"];
  uint64_t From = params["From"];
  Database DB(pSerial);
  if((params.contains("Update") == true) && (params["Update"].is_boolean() == true) && (params["Update"] == true)) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET eventsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  auto Func = [&, To, From](uint64_t offset, DataSink &sink) -> bool {
    json Headers = {"StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes"};
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(starttime, evttype, samplerate, reference, peak, change, duration, notes) FROM events WHERE starttime >= " + DB.Bind(From) + " AND starttime <= " + DB.Bind(To));
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " << DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return false;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftEvents::GetEventsRange(const json &params, Response &response) {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM events) AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftEvents::DelEventsRange(const json &params, Response &response) {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime >= " + DB.Bind(From) + " AND starttime <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftEvents::GetEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false)) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  if((params["Samples"] != "RMS") && (params["Samples"] != "RAW")) {
    response.set_content("'Samples' params must be 'RMS' or 'RAW'", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  uint64_t ID      = params["StartTime"];
  string   Samples = params["Samples"];
  DB.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', samples1, 'Inner_Samples', samples2, 'Post_Samples', samples3) AS evt FROM events_ext WHERE starttime = " + DB.Bind(ID) + " AND sampletype = " + DB.Bind(Samples));
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("evt"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoftEvents::SetEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Type") == false) || (params["Type"].is_string() == false)) return HTTPServer::FillResponse(response, "'Type' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Notes") == false) || (params["Notes"].is_string() == false)) return HTTPServer::FillResponse(response, "'Notes' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t ID    = params["StartTime"];
  string   Type  = params["Type"];
  string   Notes = params["Notes"];
  bool Result = DB.ExecSentenceNoResult("UPDATE events SET Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftEvents::DelEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t ID = params["StartTime"];
  bool Result = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(ID));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------