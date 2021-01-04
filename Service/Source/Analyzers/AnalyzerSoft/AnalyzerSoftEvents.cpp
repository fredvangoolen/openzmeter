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
  if(pWorker != NULL) delete pWorker;
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
  Event->SamplesDuration = 0;
  Event->CyclesDuration = 0;
  return Event;
}
void AnalyzerSoftEvents::SaveSamples(const uint64_t &time, const uint8_t type, const uint16_t len) {
  uint16_t MaxLen;
  uint16_t ReadPos;
  Database DB(pSerial);
  float   *Ptr, *Row;
  string   Result = "'{";
  if((type == SAMPLES_RAW_PRESTART) || (type == SAMPLES_RAW_POSSTART) || (type == SAMPLES_RAW_PREEND) || (type == SAMPLES_RAW_POSEND)) {
    MaxLen  = pPrevSamplesLen;
    ReadPos = pPrevSamplesPos + (pPrevSamplesLen - len);
    Ptr     = pPrevSamples;
  } else {
    MaxLen  = pPrevCyclesLen;
    ReadPos = pPrevCyclesPos + (pPrevCyclesLen - len);
    Ptr     = pPrevCycles;
  }
  for(uint16_t i = 0; i < len; i++) {
    ReadPos = (ReadPos + 1) % MaxLen;
    Row     = &Ptr[ReadPos * pVoltageChannels];
    Result.append((i == 0) ? "{" : ", {");
    for(uint32_t n = 0; n < pVoltageChannels; n++) Result.append(((n == 0) ? "" : ", ") + (isnan(Row[n]) ? string("NULL") : to_string(Row[n])));
    Result.append("}");
  }
  Result.append("}'");
  DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples) VALUES (" + DB.Bind(time) + ", " + DB.Bind(type) + ", " + Result + ")");
}
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
      bool IsRunning = false;
      for(uint8_t n = 0; (n < pVoltageChannels) && (IsRunning == false); n++) {
        if((pEvent->Type == EVT_INTERRUPTION) && (-CalcChange(Cycle[n], pNominalVoltage) > (Event_INT - Event_Hysteresis))) {
          IsRunning = true;
        } else if((pEvent->Type == EVT_DIP) && (-CalcChange(Cycle[n], pNominalVoltage) > (Event_DIP_SWELL - Event_Hysteresis))) {
          IsRunning = true;
        } else if((pEvent->Type == EVT_SWELL) && (CalcChange(Cycle[n], pNominalVoltage) > (Event_DIP_SWELL - Event_Hysteresis))) {
          IsRunning = true;
        }
      }
      if((pEvent->Type == EVT_RVC) && (Stationary == false)) IsRunning = true;
      if(IsRunning == false) {
        string Type = EventTypeToString(pEvent->Type);
        LogFile::Debug("Event '%s' closed on analyzer '%s'", Type.c_str(), pSerial.c_str());
        Database DB(pSerial);
        if(pEvent->Type == EVT_RVC) memcpy(pEvent->ChangeValue, RMS_Avg, sizeof(float) * pVoltageChannels);
        DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", reference = " + DB.Bind(pEvent->Reference, pVoltageChannels) + ", peak = " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent->SamplesDuration) + " WHERE starttime = " + DB.Bind(pEvent->Start));
        if(pEvent->CyclesDuration  < pPrevCyclesLen)  SaveSamples(pEvent->Start, SAMPLES_RMS_POSSTART, 0);
        if(pEvent->SamplesDuration < pPrevSamplesLen) SaveSamples(pEvent->Start, SAMPLES_RAW_POSSTART, 0);
        SaveSamples(pEvent->Start, SAMPLES_RMS_PREEND, (pEvent->CyclesDuration < pPrevCyclesLen) ? pEvent->CyclesDuration : (pEvent->CyclesDuration < (pPrevCyclesLen * 2)) ? pEvent->CyclesDuration - pPrevCyclesLen : pPrevCyclesLen);
        SaveSamples(pEvent->Start, SAMPLES_RAW_PREEND, (pEvent->SamplesDuration < pPrevSamplesLen) ? pEvent->SamplesDuration : (pEvent->SamplesDuration < (pPrevSamplesLen * 2)) ? pEvent->SamplesDuration - pPrevSamplesLen : pPrevSamplesLen);
        pEvent->CyclesDuration  = 0;
        pEvent->SamplesDuration = 0;
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
        DB.ExecSentenceNoResult("INSERT INTO events(starttime, evttype, samplerate, reference, peak, change, notes, duration) VALUES (" + DB.Bind(pEvent->Start) + ", " + DB.Bind(Type) + ", " + DB.Bind(pSamplerate) + ", " + DB.Bind(pEvent->Reference, pVoltageChannels) + ", " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", '', 0)");
        SaveSamples(pEvent->Start, SAMPLES_RMS_PRESTART, pPrevCyclesLen);
        SaveSamples(pEvent->Start, SAMPLES_RAW_PRESTART, pPrevSamplesLen);
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
    if(pEvent != NULL) {
      pEvent->CyclesDuration++;
      if(pEvent->CyclesDuration == pPrevCyclesLen) SaveSamples(pEvent->Start, SAMPLES_RMS_POSSTART, pPrevCyclesLen);
    }
    for(auto &Event : pEvents) {
      Event->CyclesDuration++;
      if(Event->CyclesDuration == pPrevCyclesLen) SaveSamples(Event->Start, SAMPLES_RMS_POSEND, pPrevCyclesLen);
    }
    for(uint16_t j = 0; j < CycleLen; j++) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevSamples[pPrevSamplesPos * pVoltageChannels + n] = block->VoltageSample(n, Pos + j);
      pPrevSamplesPos = (pPrevSamplesPos + 1) % pPrevSamplesLen;
      if(pEvent != NULL) {
        pEvent->SamplesDuration++;
        if(pEvent->SamplesDuration == pPrevSamplesLen) SaveSamples(pEvent->Start, SAMPLES_RAW_POSSTART, pPrevSamplesLen);
      }
      for(auto &Event : pEvents) {
        Event->SamplesDuration++;
        if(Event->SamplesDuration == pPrevSamplesLen) SaveSamples(Event->Start, SAMPLES_RAW_POSEND, pPrevSamplesLen);
      }
    }
    while(pEvents.size() != 0) {
      Event_t *Tmp = pEvents.front();
      if((Tmp->SamplesDuration < pPrevSamplesLen) || (Tmp->CyclesDuration < pPrevCyclesLen)) break;
      LogFile::Debug("Event '%s' on analyzer '%s' finished", EventTypeToString(Tmp->Type).c_str(), pSerial.c_str());
      pEvents.pop_front();
    }
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
    DB.ExecSentenceAsyncResult("SELECT json_build_array(starttime, evttype, samplerate, reference, peak, change, duration, notes) AS result FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " ORDER BY starttime ASC");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerSoftEvents::GetEventsRange(const json &params, Response &response) {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(starttime), 'From', min(starttime)) AS ranges FROM events");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftEvents::DelEventsRange(const json &params, Response &response) {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From = params["From"];
  uint64_t To   = params["To"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftEvents::GetEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false) || ((params["Samples"] != "RMS") && (params["Samples"] != "RAW"))) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  string   Samples   = params["Samples"];
  DB.ExecSentenceAsyncResult("SELECT jsonb_build_object('StartTime', starttime, 'Type', evttype, 'SampleRate', samplerate, 'Reference', reference, 'Peak', peak, 'Change', change, 'Duration', duration, 'Notes', notes, "
                             "'Length', (SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = " + DB.Bind((uint8_t)((Samples == "RMS") ? SAMPLES_RMS_PRESTART : SAMPLES_RAW_PRESTART)) + "), "
                             "'Samples', (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime) SELECT "
                             " (SELECT samples FROM ext WHERE sampletype = " + DB.Bind((uint8_t)((Samples == "RMS") ? SAMPLES_RMS_PRESTART : SAMPLES_RAW_PRESTART)) + ") || "
                             " (SELECT samples FROM ext where sampletype = " + DB.Bind((uint8_t)((Samples == "RMS") ? SAMPLES_RMS_POSSTART : SAMPLES_RAW_POSSTART)) + ") || "
                             " (SELECT samples FROM ext WHERE sampletype = " + DB.Bind((uint8_t)((Samples == "RMS") ? SAMPLES_RMS_PREEND   : SAMPLES_RAW_PREEND)) + ") || "
                             " (SELECT samples FROM ext WHERE sampletype = " + DB.Bind((uint8_t)((Samples == "RMS") ? SAMPLES_RMS_POSEND   : SAMPLES_RAW_POSEND)) + ")) "
                             ") AS result FROM events WHERE starttime = " + DB.Bind(StartTime));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoftEvents::SetEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Notes") == false) || (params["Notes"].is_string() == false)) return HTTPServer::FillResponse(response, "'Notes' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  string   Notes     = params["Notes"];
  bool     Result    = DB.ExecSentenceNoResult("UPDATE events SET Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(StartTime));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoftEvents::DelEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  bool     Result    = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(StartTime));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------