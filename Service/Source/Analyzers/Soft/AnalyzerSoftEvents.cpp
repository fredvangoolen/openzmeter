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
#include "Tools.h"
#include "Database.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
constexpr float AnalyzerSoftEvents::Event_RVC;
constexpr float AnalyzerSoftEvents::Event_DIP_SWELL;
constexpr float AnalyzerSoftEvents::Event_INT;
constexpr float AnalyzerSoftEvents::Event_Hysteresis;
constexpr float AnalyzerSoftEvents::Event_RAW_Time;
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftEvents::AnalyzerSoftEvents(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate) : AnalyzerEvents(serial, mode), pSamplerate(samplerate), pVoltageChannels(Analyzer::VoltageChannels(mode)) {
  pMutex            = PTHREAD_MUTEX_INITIALIZER;
  pNominalVoltage   = 230.0;
  pEvent            = NULL;
  pPrevStationary   = false;
  pPrevSamplesLen   = 0;
  pPrevSamplesPos   = 0;
  pPrevSamples      = NULL;
  pPrevCyclesLen    = 0;
  pPrevCyclesPos    = 0;
  pPrevCycles       = NULL;
  pTask             = NULL;
  pHaveEvent        = false;
  pEnabled          = false;
}
AnalyzerSoftEvents::~AnalyzerSoftEvents() {
  if(pTask != NULL) TaskPool::Wait(pTask);
  if(pEvent != NULL) delete pEvent;
  if(pPrevSamples != NULL) free(pPrevSamples);
  if(pPrevCycles != NULL) free(pPrevCycles);
  for(auto &Prev : pEvents) delete Prev;
  pEvents.clear();
}
void AnalyzerSoftEvents::Config(const bool enabled, const float nominalVoltage, const float nominalFreq) {
  pthread_mutex_lock(&pMutex);
  pEnabled = enabled;
  if((pNominalVoltage != nominalVoltage) || (pNominalFrequency != nominalFreq)) {
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
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftEvents::Run(const AnalyzerSoftBlock *block, const uint64_t timestamp) {
  pthread_mutex_lock(&pMutex);
  pHaveEvent = false;
  if(pEnabled) pTask = TaskPool::Run([&, block, timestamp]() { this->Events_Func(block, timestamp); }, TaskPool::ABOUTNORMAL);
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerSoftEvents::Wait() {
  pthread_mutex_lock(&pMutex);
  if(pTask != NULL) TaskPool::Wait(pTask);
  bool Result = (pEnabled && pHaveEvent);
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
  Database DB(pSerial);
  if(type == SAMPLES_RAW) {
    float   *Ptr    = pPrevSamples;
    string   Result = "'{";
    for(uint16_t i = 0, ReadPos = (pPrevSamplesPos + (pPrevSamplesLen - len)) %  pPrevSamplesLen; i < len; i++) {
      float *Row = &Ptr[ReadPos * pVoltageChannels];
      Result.append((i == 0) ? "{" : ", {");
      for(uint32_t n = 0; n < pVoltageChannels; n++) Result.append(((n == 0) ? "" : ", ") + to_string(Row[n]));
      Result.append("}");
      ReadPos = (ReadPos + 1) % pPrevSamplesLen;
    }
    Result.append("}'");
    DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_raw = samples_raw || " + Result + " WHERE starttime = " + DB.Bind(time));
  } else {
    float   *Ptr     = pPrevCycles;
    string   Result  = "'{";
    for(uint16_t i = 0, ReadPos = (pPrevCyclesPos + (pPrevCyclesLen - len)) % pPrevCyclesLen; i < len; i++) {
      float *Row = &Ptr[ReadPos * pVoltageChannels];
      Result.append((i == 0) ? "{" : ", {");
      for(uint32_t n = 0; n < pVoltageChannels; n++) Result.append(((n == 0) ? "" : ", ") + to_string(Row[n]));
      Result.append("}");
      ReadPos = (ReadPos + 1) % pPrevCyclesLen;
    }
    Result.append("}'");
    DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_rms = samples_rms || " + Result + " WHERE starttime = " + DB.Bind(time));
  }
}
void AnalyzerSoftEvents::Events_Func(const AnalyzerSoftBlock *block, const uint64_t timestamp) {
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
        if(pEvent->CyclesDuration  < pPrevCyclesLen)  SaveSamples(pEvent->Start, SAMPLES_RMS, 0);
        if(pEvent->SamplesDuration < pPrevSamplesLen) SaveSamples(pEvent->Start, SAMPLES_RAW, 0);
        SaveSamples(pEvent->Start, SAMPLES_RMS, (pEvent->CyclesDuration < pPrevCyclesLen) ? pEvent->CyclesDuration : (pEvent->CyclesDuration < (pPrevCyclesLen * 2)) ? pEvent->CyclesDuration - pPrevCyclesLen : pPrevCyclesLen);
        SaveSamples(pEvent->Start, SAMPLES_RAW, (pEvent->SamplesDuration < pPrevSamplesLen) ? pEvent->SamplesDuration : (pEvent->SamplesDuration < (pPrevSamplesLen * 2)) ? pEvent->SamplesDuration - pPrevSamplesLen : pPrevSamplesLen);
        DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", evttype = " + DB.Bind(Type) + ", reference = " + DB.Bind(pEvent->Reference, pVoltageChannels) + ", peak = " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent->SamplesDuration) + " WHERE starttime = " + DB.Bind(pEvent->Start));
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
        pEvent->Start = timestamp + (uint32_t)((Pos * 1000) / pSamplerate);
        Database DB(pSerial);
        DB.ExecSentenceNoResult("INSERT INTO events(changetime, starttime, evttype, samplerate, reference, peak, change, notes, duration, size_raw, size_rms) VALUES (" + DB.Bind(Tools::GetTime64()) + ", " + DB.Bind(pEvent->Start) + ", " + DB.Bind(Type) + ", " + DB.Bind(pSamplerate) + ", " + DB.Bind(pEvent->Reference, pVoltageChannels) + ", " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", '', 0, " + DB.Bind(pPrevSamplesLen) + ", " + DB.Bind(pPrevCyclesLen) + ")");
        SaveSamples(pEvent->Start, SAMPLES_RMS, pPrevCyclesLen + 1);
        SaveSamples(pEvent->Start, SAMPLES_RAW, pPrevSamplesLen);
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
      if(pEvent->CyclesDuration == pPrevCyclesLen) SaveSamples(pEvent->Start, SAMPLES_RMS, pPrevCyclesLen);
    }
    for(auto &Event : pEvents) {
      Event->CyclesDuration++;
      if(Event->CyclesDuration == pPrevCyclesLen) SaveSamples(Event->Start, SAMPLES_RMS, pPrevCyclesLen);
    }
    for(uint16_t j = 0; j < CycleLen; j++) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevSamples[pPrevSamplesPos * pVoltageChannels + n] = block->VoltageSample(n, Pos + j);
      pPrevSamplesPos = (pPrevSamplesPos + 1) % pPrevSamplesLen;
      if(pEvent != NULL) {
        pEvent->SamplesDuration++;
        if(pEvent->SamplesDuration == pPrevSamplesLen) SaveSamples(pEvent->Start, SAMPLES_RAW, pPrevSamplesLen);
      }
      for(auto &Event : pEvents) {
        Event->SamplesDuration++;
        if(Event->SamplesDuration == pPrevSamplesLen) SaveSamples(Event->Start, SAMPLES_RAW, pPrevSamplesLen);
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
// ---------------------------------------------------------------------------------------------------------------------