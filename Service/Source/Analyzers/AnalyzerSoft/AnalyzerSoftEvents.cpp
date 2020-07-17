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
// ---------------------------------------------------------------------------------------------------------------------

    pPrevSamples       = NULL;
  pPrevCycles        = NULL;


  if((EventsEnabled == false) && (pEventsWorker != NULL)) {
    delete pEventsWorker;
    pEventsWorker = NULL;
    free(pPrevCycles);
    pPrevCycles = NULL;
    free(pPrevSamples);
    pPrevSamples = NULL;
  } else if((EventsEnabled == true) && (pEventsWorker == NULL)) {
    memset(&pEvent, 0, sizeof(Event_t));
    pPrevStationary = false;
    pPrevSamplesLen = EVENT_RAWENVELOPETIME * pSamplerate;
    pPrevSamplesPos = 0;
    pPrevSamples    = (float*)calloc(pPrevSamplesLen * pVoltageChannels, sizeof(float));
    pPrevCyclesLen  = (pNominalFrequency * 2) - 1;
    pPrevCyclesPos  = 0;
    pPrevCycles     = (float*)calloc(pPrevCyclesLen * pVoltageChannels, sizeof(float));
    pEventsWorker   = new Worker("EventsDet", [&](void *a) { this->Events_Func(); });
  }


  void AnalyzerSoft::Events_Func() {
  auto CopyCycles = [&](float *buff, const uint16_t count) -> float* {
    buff = (float*)realloc(buff, sizeof(float) * pVoltageChannels * count);
    for(uint16_t n = 0; n < count; n++) {
      for(uint8_t i = 0; i < pVoltageChannels; i++) buff[i * count + n] = pPrevCycles[((pPrevCyclesPos + pPrevCyclesLen - count + n) % pPrevCyclesLen) * pVoltageChannels + i];
    }
    return buff;
  };
  auto CopySamples = [&](float *buff, const uint16_t count) -> float* {
    buff = (float*)realloc(buff, sizeof(float) * pVoltageChannels * count);
    for(uint16_t n = 0; n < count; n++) {
      for(uint8_t i = 0; i < pVoltageChannels; i++) buff[i * count + n] = pPrevSamples[((pPrevSamplesPos + pPrevSamplesLen - count + n) % pPrevSamplesLen) * pVoltageChannels + i];
    }
    return buff;
  };
  uint32_t  Pos  = 0;
  float    *Buff = NULL;
/*  for(uint8_t i = 0; i < pBlockCyclesCount; i++) {
    float    Tolerance  = EVENT_RVC - ((pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
    bool     Stationary = true;
    uint16_t CycleLen   = pBlockCycles[i];
    float    Cycle[3];
    float    RMS_Avg[3];
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      Cycle[n] = 0.0;
      float *Ptr   = &pVoltageSamples[n * pMaxSamples + Pos];
      for(uint16_t j = 0; j < CycleLen; j++) Cycle[n] += Ptr[j] * Ptr[j];
      Cycle[n] = sqrt(Cycle[n] / CycleLen);
      RMS_Avg[n]   = Cycle[n];
      for(uint16_t j = 0; j < pPrevCyclesLen; j++) RMS_Avg[n] += pPrevCycles[j * pVoltageChannels + n];
      RMS_Avg[n] = RMS_Avg[n] / (pPrevCyclesLen + 1);
    }
    for(uint8_t n = 0; (n < pVoltageChannels) && (Stationary == true); n++) {
      Stationary = (fabs(CalcChange(Cycle[n], RMS_Avg[n])) > Tolerance) ? false : Stationary;
      for(int j = 0; j < pPrevCyclesLen; j++) Stationary = (fabs(CalcChange(pPrevCycles[j * pVoltageChannels + n], RMS_Avg[n])) > Tolerance) ? false : Stationary;
    }
    if(pEvent.Type != EVT_NONE) {
      bool IsRunning = false;
      if(pEvent.Type == EVT_INTERRUPTION) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(Cycle[n], pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS));
      } else if(pEvent.Type == EVT_DIP) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(Cycle[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
      } else if(pEvent.Type == EVT_SWELL) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (CalcChange(Cycle[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
      } else if(pEvent.Type == EVT_RVC) {
        if(Stationary == true) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.ChangeValue[n] = RMS_Avg[n];
        } else {
          IsRunning = true;
        }
      }
      if(IsRunning == false) {
        string  Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
        LogFile::Debug("Event '%s' closed on analyzer '%s'", Type.c_str(), pSerial.c_str());
        Database DB(pSerial);
        DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", reference = " + DB.Bind(pEvent.Reference, pVoltageChannels) + ", peak = " + DB.Bind(pEvent.PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent.ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent.SamplesDuration) + " WHERE starttime = " + DB.Bind(pEvent.Start));
        if(pEvent.CyclesDuration < pPrevCyclesLen) {
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RMS'");
        } else if(pEvent.CyclesDuration > pPrevCyclesLen) {
          pEvent.CyclesDuration = (pEvent.CyclesDuration < (pPrevCyclesLen * 2)) ? pEvent.CyclesDuration - pPrevCyclesLen : pPrevCyclesLen;
          DB.ExecSentenceNoResult("WITH tbl as (SELECT starttime as srctime, sampletype as srctype, samples2 as src, " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) +"::real[] as dst, generate_subscripts(samples2, 1) AS idx FROM events_ext) UPDATE events_ext SET samples2 = (SELECT array_agg(array(select unnest(src[idx:idx])) || array(select unnest(dst[idx:idx]))) FROM tbl WHERE srctype = sampletype and srctime = starttime) WHERE sampletype = 'RMS' and starttime = " + DB.Bind(pEvent.Start));
        }
        if(pEvent.SamplesDuration < pPrevSamplesLen) {
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RAW'");
        } else if(pEvent.SamplesDuration > pPrevSamplesLen) {
          pEvent.SamplesDuration = (pEvent.SamplesDuration < (pPrevSamplesLen * 2)) ? pEvent.SamplesDuration - pPrevSamplesLen : pPrevSamplesLen;
          DB.ExecSentenceNoResult("WITH tbl as (SELECT starttime as srctime, sampletype as srctype, samples2 as src, " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) +"::real[] as dst, generate_subscripts(samples2, 1) AS idx FROM events_ext) UPDATE events_ext SET samples2 = (SELECT array_agg(array(select unnest(src[idx:idx])) || array(select unnest(dst[idx:idx]))) FROM tbl WHERE srctype = sampletype and srctime = starttime) WHERE sampletype = 'RAW' and starttime = " + DB.Bind(pEvent.Start));
        }
        pEvent.CyclesDuration  = 0;
        pEvent.SamplesDuration = 0;
        pEvents.push_back(pEvent);
        memset(&pEvent, 0, sizeof(pEvent));
      }
    }
    if(pSequence > 100) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        if((pEvent.Type != EVT_INTERRUPTION) && (-CalcChange(Cycle[n], pNominalVoltage) > EVENT_INTERRUPTION)) {
          pEvent.Type = EVT_INTERRUPTION;
          for(uint8_t i = 0; i < pVoltageChannels; i++) {
            pEvent.Reference[i] = pNominalVoltage;
            pEvent.PeakValue[i] = Cycle[i];
          }
        } else if((pEvent.Type != EVT_DIP) && (pEvent.Type != EVT_INTERRUPTION) && (-CalcChange(Cycle[n], pNominalVoltage) > EVENT_DIP_SWELL)) {
          pEvent.Type = EVT_DIP;
          for(uint8_t i = 0; i < pVoltageChannels; i++) {
            pEvent.Reference[i] = pNominalVoltage;
            pEvent.PeakValue[i] = Cycle[i];
          }
        } else if((pEvent.Type != EVT_SWELL) && (CalcChange(Cycle[n], pNominalVoltage) > EVENT_DIP_SWELL)) {
          pEvent.Type = EVT_SWELL;
          for(uint8_t i = 0; i < pVoltageChannels; i++) {
            pEvent.Reference[i] = pNominalVoltage;
            pEvent.PeakValue[i] = Cycle[i];
          }
        } else if((pEvent.Type == EVT_NONE) && (Stationary == false) && (pPrevStationary == true)) {
          pEvent.Type = EVT_RVC;
          for(uint8_t i = 0; i < pVoltageChannels; i++) {
            pEvent.Reference[i] = RMS_Avg[i];
            pEvent.PeakValue[i] = Cycle[i];
          }
        }
      }
    }
    if(pEvent.Type != EVT_NONE) {
      pAggreg200MS.Flag = true;
      if(pEvent.SamplesDuration == 0) {
        string Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
        LogFile::Debug("New event '%s' started on analyzer '%s'", Type.c_str(), pSerial.c_str());
        pEvent.Start = pAggreg200MS.Timestamp + (uint32_t)((Pos * 1000) / pSamplerate);
        Database DB(pSerial);
        DB.ExecSentenceNoResult("INSERT INTO events(starttime, evttype, samplerate, reference, peak, change, notes, duration) VALUES (" + DB.Bind(pEvent.Start) + ", " + DB.Bind(Type) + ", " + DB.Bind(pSamplerate) + ", " + DB.Bind(pEvent.Reference, pVoltageChannels) + ", " + DB.Bind(pEvent.PeakValue, pVoltageChannels) + ", " + DB.Bind(pEvent.ChangeValue, pVoltageChannels) + ", '', 0)");
        DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RMS', " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + ")");
        DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RAW', " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + ")");
      } else {
        if(pEvent.Type == EVT_INTERRUPTION) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmin(pEvent.PeakValue[n], Cycle[n]);
        } else if(pEvent.Type == EVT_DIP) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmin(pEvent.PeakValue[n], Cycle[n]);
        } else if(pEvent.Type == EVT_SWELL) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmax(pEvent.PeakValue[n], Cycle[n]);
        } else if(pEvent.Type == EVT_RVC) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) {
            if(fabs(pEvent.Reference[n] - Cycle[n]) > fabs(pEvent.Reference[n] - pEvent.PeakValue[n])) pEvent.PeakValue[n] = Cycle[n];
          }
        }
      }
    }
    for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevCycles[pPrevCyclesPos * pVoltageChannels + n] =  Cycle[n];
    pPrevCyclesPos  = (pPrevCyclesPos + 1) % pPrevCyclesLen;
    if(pEvent.Type != EVT_NONE) {
      pEvent.CyclesDuration++;
      if(pEvent.CyclesDuration == pPrevCyclesLen) {
        Database DB(pSerial);
        DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RMS'");
      }
    }
    for(auto &Event : pEvents) {
      Event.CyclesDuration++;
      if(Event.CyclesDuration == pPrevCyclesLen) {
        Database DB(pSerial);
        DB.ExecSentenceNoResult("UPDATE events_ext SET samples3 = " + DB.Bind(CopyCycles(Buff, pPrevCyclesLen), pVoltageChannels, pPrevCyclesLen) + " WHERE starttime = " + DB.Bind(Event.Start) + " AND sampletype = 'RMS'");
      }
    }
    for(uint16_t j = 0; j < CycleLen; j++) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevSamples[pPrevSamplesPos * pVoltageChannels + n] = pVoltageSamples[n * pMaxSamples + Pos + j];
      pPrevSamplesPos = (pPrevSamplesPos + 1) % pPrevSamplesLen;
      if(pEvent.Type != EVT_NONE) {
        pEvent.SamplesDuration++;
        if(pEvent.SamplesDuration == pPrevSamplesLen) {
          Database DB(pSerial);
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RAW'");
        }
      }
      for(auto &Event : pEvents) {
        Event.SamplesDuration++;
        if(Event.SamplesDuration == pPrevSamplesLen) {
          Database DB(pSerial);
          DB.ExecSentenceNoResult("UPDATE events_ext SET samples3 = " + DB.Bind(CopySamples(Buff, pPrevSamplesLen), pVoltageChannels, pPrevSamplesLen) + " WHERE starttime = " + DB.Bind(Event.Start) + " AND sampletype = 'RAW'");
        }
      }
    }
    while(pEvents.size() != 0) {
      Event_t *Tmp = &pEvents.front();
      if((Tmp->SamplesDuration < pPrevSamplesLen) || (Tmp->CyclesDuration < pPrevCyclesLen)) break;
      string Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
      LogFile::Debug("Event '%s' on analyzer '%s' finished", Type.c_str(), pSerial.c_str());
      pEvents.pop_front();
    }
    Pos             = Pos + CycleLen;
    pPrevStationary = Stationary && (pEvent.Type == EVT_NONE);
  }*/
  free(Buff);
}

// ---------------------------------------------------------------------------------------------------------------------
