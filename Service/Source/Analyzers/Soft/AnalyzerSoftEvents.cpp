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
AnalyzerSoftEvents::AnalyzerSoftEvents(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate, AnalyzerSoftCyclicHistogram *cyclicHistogram) : AnalyzerEvents(serial, mode), pSamplerate(samplerate), pVoltageChannels(Analyzer::VoltageChannels(mode)) {
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
  pCyclicHistogram  = cyclicHistogram;
}
AnalyzerSoftEvents::~AnalyzerSoftEvents() {
  if(pTask != NULL) TaskPool::Wait(pTask);
  if(pEvent != NULL) delete pEvent;
  if(pPrevSamples != NULL) free(pPrevSamples);
  if(pPrevCycles != NULL) free(pPrevCycles);
  for(auto &Prev : pEvents) delete Prev;
  pEvents.clear();
}
json AnalyzerSoftEvents::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["SaveRMS"] = 1000;
  ConfigOUT["SaveRAW"] = 200;
  ConfigOUT["RVC"] = json::object();
  ConfigOUT["RVC"]["Enabled"] = true;
  ConfigOUT["RVC"]["Threshold"] = 5.0;
  ConfigOUT["RVC"]["Hysteresis"] = 2.0;
  ConfigOUT["DIP"] = json::object();
  ConfigOUT["DIP"]["Enabled"] = true;
  ConfigOUT["DIP"]["Threshold"] = 10.0;
  ConfigOUT["DIP"]["Hysteresis"] = 2.0;
  ConfigOUT["SWELL"] = json::object();
  ConfigOUT["SWELL"]["Enabled"] = true;
  ConfigOUT["SWELL"]["Threshold"] = 10.0;
  ConfigOUT["SWELL"]["Hysteresis"] = 2.0;
  ConfigOUT["INTERRUPTION"] = json::object();
  ConfigOUT["INTERRUPTION"]["Enabled"] = true;
  ConfigOUT["INTERRUPTION"]["Threshold"] = 90.0;
  ConfigOUT["INTERRUPTION"]["Hysteresis"] = 2.0;
  if(config.is_object()) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("SaveRMS") && config["SaveRMS"].is_number() && (config["SaveRMS"] >= 0.0) && (config["SaveRMS"] <= 5000.0)) ConfigOUT["SaveRMS"] = config["SaveRMS"];
    if(config.contains("SaveRAW") && config["SaveRAW"].is_number() && (config["SaveRAW"] >= 0.0) && (config["SaveRAW"] <= 1000.0)) ConfigOUT["SaveRAW"] = config["SaveRAW"];
    if(config.contains("RVC") && config["RVC"].is_object()) {
      if(config["RVC"].contains("Enabled") && config["RVC"]["Enabled"].is_boolean()) ConfigOUT["RVC"]["Enabled"] = config["RVC"]["Enabled"];
      if(config["RVC"].contains("Threshold") && config["RVC"]["Threshold"].is_number() && (config["RVC"]["Threshold"] >= 2.0) && (config["RVC"]["Threshold"] <= 10.0)) ConfigOUT["RVC"]["Threshold"] = config["RVC"]["Threshold"];
      if(config["RVC"].contains("Hysteresis") && config["RVC"]["Hysteresis"].is_number() && (config["RVC"]["Hysteresis"] >= 1.0) && (config["RVC"]["Hysteresis"] < config["RVC"]["Threshold"])) ConfigOUT["RVC"]["Hysteresis"] = config["RVC"]["Hysteresis"];
    }
    if(config.contains("DIP") && config["DIP"].is_object()) {
      if(config["DIP"].contains("Enabled") && config["DIP"]["Enabled"].is_boolean()) ConfigOUT["DIP"]["Enabled"] = config["DIP"]["Enabled"];
      if(config["DIP"].contains("Threshold") && config["DIP"]["Threshold"].is_number() && (config["DIP"]["Threshold"] >= 5.0) && (config["DIP"]["Threshold"] <= 20.0)) ConfigOUT["DIP"]["Threshold"] = config["DIP"]["Threshold"];
      if(config["DIP"].contains("Hysteresis") && config["DIP"]["Hysteresis"].is_number() && (config["DIP"]["Hysteresis"] >= 1.0) && (config["DIP"]["Hysteresis"] < config["DIP"]["Threshold"])) ConfigOUT["DIP"]["Hysteresis"] = config["DIP"]["Hysteresis"];
    }
    if(config.contains("SWELL") && config["SWELL"].is_object()) {
      if(config["SWELL"].contains("Enabled") && config["SWELL"]["Enabled"].is_boolean()) ConfigOUT["SWELL"]["Enabled"] = config["SWELL"]["Enabled"];
      if(config["SWELL"].contains("Threshold") && config["SWELL"]["Threshold"].is_number() && (config["SWELL"]["Threshold"] >= 5.0) && (config["SWELL"]["Threshold"] <= 20.0)) ConfigOUT["SWELL"]["Threshold"] = config["SWELL"]["Threshold"];
      if(config["SWELL"].contains("Hysteresis") && config["SWELL"]["Hysteresis"].is_number() && (config["SWELL"]["Hysteresis"] >= 1.0) && (config["SWELL"]["Hysteresis"] < config["SWELL"]["Threshold"])) ConfigOUT["SWELL"]["Hysteresis"] = config["SWELL"]["Hysteresis"];
    }
    if(config.contains("INTERRUPTION") && config["INTERRUPTION"].is_object()) {
      if(config["INTERRUPTION"].contains("Enabled") && config["INTERRUPTION"]["Enabled"].is_boolean()) ConfigOUT["INTERRUPTION"]["Enabled"] = config["INTERRUPTION"]["Enabled"];
      if(config["INTERRUPTION"].contains("Threshold") && config["INTERRUPTION"]["Threshold"].is_number() && (config["INTERRUPTION"]["Threshold"] >= 50.0) && (config["INTERRUPTION"]["Threshold"] <= 100.0)) ConfigOUT["INTERRUPTION"]["Threshold"] = config["INTERRUPTION"]["Threshold"];
      if(config["INTERRUPTION"].contains("Hysteresis") && config["INTERRUPTION"]["Hysteresis"].is_number() && (config["INTERRUPTION"]["Hysteresis"] >= 1.0) && (config["INTERRUPTION"]["Hysteresis"] < config["INTERRUPTION"]["Threshold"])) ConfigOUT["INTERRUPTION"]["Hysteresis"] = config["INTERRUPTION"]["Hysteresis"];
    }
  }
  return ConfigOUT;
}
void AnalyzerSoftEvents::Config(const json &config, const float nominalVoltage, const float nominalFreq) {
  pthread_mutex_lock(&pMutex);
  pEnabled = config["Enabled"];
  pRVC.Enabled = config["RVC"]["Enabled"];
  pRVC.Threshold = config["RVC"]["Threshold"];
  pRVC.Hysteresis = config["RVC"]["Hysteresis"];
  pDip.Enabled = config["DIP"]["Enabled"];
  pDip.Threshold = config["DIP"]["Threshold"];
  pDip.Hysteresis = config["DIP"]["Hysteresis"];
  pSwell.Enabled = config["SWELL"]["Enabled"];
  pSwell.Threshold = config["SWELL"]["Threshold"];
  pSwell.Hysteresis = config["SWELL"]["Hysteresis"];
  pInterruption.Enabled = config["INTERRUPTION"]["Enabled"];
  pInterruption.Threshold = config["INTERRUPTION"]["Threshold"];
  pInterruption.Hysteresis = config["INTERRUPTION"]["Hysteresis"];
  if(pPrevCycles != NULL) free(pPrevCycles);
  pPrevCycles = NULL;
  if(pPrevSamples != NULL) free(pPrevSamples);
  pPrevSamples = NULL;
  if(pEvent != NULL) delete pEvent;
  pEvent = NULL;
  for(auto &Prev : pEvents) delete Prev;
  pEvents.clear();
  pNominalVoltage    = nominalVoltage;
  pNominalFrequency  = nominalFreq;
  pPrevStationary    = false;
  pPrevSamplesPos    = 0;
  pPrevCyclesPos     = 0;
  pPrevSamplesLen    = config["SaveRAW"].get<float>() * pSamplerate / 1000.0;
  pPrevCyclesSaveLen = config["SaveRMS"].get<float>() * pNominalFrequency * 2 / 1000.0;
  pPrevCyclesAvgLen  = (nominalFreq * 2) - 1;
  pPrevCyclesLen     = (pPrevCyclesSaveLen > pPrevCyclesLen) ? pPrevCyclesSaveLen : pPrevCyclesLen;
  pPrevSamples       = (float*)calloc(pPrevSamplesLen * pVoltageChannels, sizeof(float));
  pPrevCycles        = (float*)calloc(pPrevCyclesLen * pVoltageChannels, sizeof(float));
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftEvents::Run(const AnalyzerSoftBlock *block) {
  pthread_mutex_lock(&pMutex);
  pHaveEvent = false;
  if((pEnabled == true) && (pNominalFrequency != 0.0)) {
    pTask = TaskPool::Run([&, block]() { this->Events_Func(block); }, TaskPool::NORMAL);
  } else {
    pTask = NULL;
  }
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerSoftEvents::Wait() {
  pthread_mutex_lock(&pMutex);
  if(pTask != NULL) TaskPool::Wait(pTask);
  bool Result = pHaveEvent;
  pthread_mutex_unlock(&pMutex);
  return Result;
}
float AnalyzerSoftEvents::CalcChange(const float in, const float reference) const {
  return ((in - reference) / reference) * 100;
}
string AnalyzerSoftEvents::EventTypeToString(const AnalyzerSoftEvents::EventType type) const {
  return (type == EVT_RVC) ? "RVC" : (type == EVT_INTERRUPTION) ? "INTERRUPTION" : (type == EVT_DIP) ? "DIP" : "SWELL";
}
void AnalyzerSoftEvents::Events_Func(const AnalyzerSoftBlock *block) {
  uint16_t Pos = 0;
  for(uint8_t i = 0; i < block->Cycles(); i++) {
    float    CycleRMS[3];
    float    StationaryRMS[3];
    bool     Stationary = true;
    uint16_t CycleLen = block->CycleLen(i);
    uint64_t Timestamp = block->Timestamp() + (uint32_t)(1000 * Pos / pSamplerate);
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      CycleRMS[n] = 0;
      for(uint16_t j = 0; j < CycleLen; j++) CycleRMS[n] += pow(block->VoltageSample(n, Pos + j), 2.0);
      CycleRMS[n] = sqrt(CycleRMS[n] / CycleLen);
    }
    for(uint8_t n = 0; n < pVoltageChannels; n++) StationaryRMS[n] = CycleRMS[n];
    if(pRVC.Enabled == true) {
      float *Ptr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels];
      for(uint16_t c = 0; c < pPrevCyclesAvgLen; c++) {
        Ptr = (Ptr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : Ptr - pVoltageChannels;
        for(uint8_t n = 0; n < pVoltageChannels; n++) StationaryRMS[n] += Ptr[n];
      }
      for(uint8_t n = 0; n < pVoltageChannels; n++) StationaryRMS[n] = StationaryRMS[n] / (pPrevCyclesAvgLen + 1);
      float Tolerance = (pPrevStationary == true) ? pRVC.Threshold : pRVC.Hysteresis;
      for(uint8_t n = 0; (n < pVoltageChannels) && (Stationary == true); n++) {
        if(fabs(CalcChange(CycleRMS[n], StationaryRMS[n])) > Tolerance)
        Stationary = false;
      }
      Ptr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels];
      for(uint16_t c = 0; (c < pPrevCyclesAvgLen) && (Stationary == true); c++) {
        Ptr = (Ptr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : Ptr - pVoltageChannels;
        for(uint8_t n = 0; (n < pVoltageChannels) && (Stationary == true); n++) {
          if(fabs(CalcChange(Ptr[n], StationaryRMS[n])) > Tolerance)
          Stationary = false;
        }
      }
    }
    if(block->Sequence() > 100) {
      EventType Type = EVT_NONE;
      if((pInterruption.Enabled == true) && ((pEvent == NULL) || (pEvent->Type != EVT_INTERRUPTION))) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) {
          if(CalcChange(pNominalVoltage, CycleRMS[n]) > pInterruption.Threshold) Type = EVT_INTERRUPTION;
        }
      }
      if((pDip.Enabled == true) && ((pEvent == NULL) || ((pEvent->Type != EVT_DIP) && (pEvent->Type != EVT_INTERRUPTION)))) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) {
          if(CalcChange(pNominalVoltage, CycleRMS[n]) > pDip.Threshold) Type = EVT_DIP;
        }
      }
      if((pSwell.Enabled == true) && ((pEvent == NULL) || ((pEvent->Type != EVT_SWELL) && (pEvent->Type != EVT_INTERRUPTION)))) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) {
          if(CalcChange(CycleRMS[n], pNominalVoltage) > pSwell.Threshold) Type = EVT_SWELL;
        }
      }
      if((pRVC.Enabled == true) && (pEvent == NULL) && (Stationary == false) && (pPrevStationary == true)) Type = EVT_RVC;
      if(Type != EVT_NONE) {
        if(pEvent == NULL) {
          pEvent = new Event_t;
          pEvent->SamplesDuration = 0;
          pEvent->CyclesDuration = 0;
          pEvent->Type = Type;
          pEvent->Start = Timestamp;
          for(uint8_t n = 0; n < pVoltageChannels; n++) {
            pEvent->Reference[n] = (Type != EVT_RVC) ? pNominalVoltage : StationaryRMS[n];
            pEvent->PeakValue[n] = (Type != EVT_RVC) ? pNominalVoltage : CycleRMS[n];
            pEvent->ChangeValue[n] = 0.0;
          }
          LogFile::Debug("New event '%s' started on analyzer '%s'", EventTypeToString(pEvent->Type).c_str(), pSerial.c_str());
          Database DB(pSerial);
          float *DstRMS = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevCyclesSaveLen);
          for(float *DstPtr = &DstRMS[pVoltageChannels * (pPrevCyclesSaveLen - 1)], *SrcPtr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels]; DstPtr >= DstRMS; DstPtr -= pVoltageChannels) {
            SrcPtr = (SrcPtr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
            for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
          }
          float *DstRAW = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevSamplesLen);
          for(float *DstPtr = &DstRAW[pVoltageChannels * (pPrevSamplesLen - 1)], *SrcPtr = &pPrevSamples[pPrevSamplesPos * pVoltageChannels]; DstPtr >= DstRAW; DstPtr -= pVoltageChannels) {
            SrcPtr = (SrcPtr == pPrevSamples) ? &pPrevSamples[(pPrevSamplesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
            for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
          }
          if(this->pCyclicHistogram != NULL){
            LogFile::Warning("[Events] Event Start Time: %u", pEvent->Start);
            LogFile::Warning("[Events] Sending command to write CH to database.");
            this->pCyclicHistogram->WriteToDatabase(DB.Bind(pEvent->Start));
            LogFile::Warning("[Events] Sending command to clear CH.");
            this->pCyclicHistogram->Clear();
          }
          DB.ExecSentenceNoResult("INSERT INTO events(changetime, starttime, evttype, samplerate, reference, peak, change, notes, duration, size_raw, size_rms, samples_rms, samples_raw) "
                                  "VALUES (" + DB.Bind(Tools::GetTime64()) + ", " + DB.Bind(pEvent->Start) + ", " + DB.Bind(EventTypeToString(pEvent->Type)) + ", " + DB.Bind(pSamplerate) + ", " +
                                              DB.Bind(pEvent->Reference, pVoltageChannels) + ", " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", " +
                                              DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", '', 0, " + DB.Bind(pPrevSamplesLen) + ", " + DB.Bind(pPrevCyclesLen) + ", " +
                                              DB.Bind(DstRMS, pPrevCyclesSaveLen, pVoltageChannels) + ", " + DB.Bind(DstRAW, pPrevSamplesLen, pVoltageChannels) + ")");
          if(DstRMS != NULL) free(DstRMS);
          if(DstRAW != NULL) free(DstRAW);
        }
        if(pEvent->Type != Type) {
          LogFile::Debug("Event '%s' running on analyzer '%s' promoted to '%s'", EventTypeToString(pEvent->Type).c_str(), pSerial.c_str(), EventTypeToString(Type).c_str());
          pEvent->Type = Type;
          for(uint8_t n = 0; n < pVoltageChannels; n++) {
            pEvent->Reference[n] = (Type != EVT_RVC) ? pNominalVoltage : StationaryRMS[n];
            pEvent->PeakValue[n] = (Type != EVT_RVC) ? pNominalVoltage : CycleRMS[n];
          }
        }
      }
    }
    if(pEvent != NULL) {
      bool Close = true;
      if(pEvent->Type == EVT_RVC) {
        if(Stationary == false) Close = false;
      } else if(pEvent->Type == EVT_INTERRUPTION) {
        for(uint8_t n = 0; (n < pVoltageChannels) && (Close == true); n++) {
          if(CalcChange(pNominalVoltage, CycleRMS[n]) > pInterruption.Hysteresis) Close = false;
        }
      } else if(pEvent->Type == EVT_DIP) {
        for(uint8_t n = 0; (n < pVoltageChannels) && (Close == true); n++) {
          if(CalcChange(pNominalVoltage, CycleRMS[n]) > pDip.Hysteresis) Close = false;
        }
      } else if(pEvent->Type == EVT_SWELL) {
        for(uint8_t n = 0; (n < pVoltageChannels) && (Close == true); n++) {
          if(CalcChange(CycleRMS[n], pNominalVoltage) > pSwell.Hysteresis) Close = false;
        }
      }
      if(Close == true) {
        LogFile::Debug("Event '%s' closed on analyzer '%s'", EventTypeToString(pEvent->Type).c_str(), pSerial.c_str());
        Database DB(pSerial);
        if(pEvent->Type == EVT_RVC) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->ChangeValue[n] = CycleRMS[n];
        }
        uint16_t LenRMS = (pEvent->CyclesDuration < pPrevCyclesSaveLen) ? pEvent->CyclesDuration : (pEvent->CyclesDuration < (pPrevCyclesSaveLen * 2)) ? pEvent->CyclesDuration - pPrevCyclesSaveLen : pPrevCyclesSaveLen;
        float   *DstRMS = (float*)malloc(sizeof(float) * pVoltageChannels * LenRMS);
        for(float *DstPtr = &DstRMS[pVoltageChannels * (LenRMS - 1)], *SrcPtr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels]; DstPtr >= DstRMS; DstPtr -= pVoltageChannels) {
          SrcPtr = (SrcPtr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
          for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
        }
        uint16_t LenRAW = (pEvent->SamplesDuration < pPrevSamplesLen) ? pEvent->SamplesDuration : (pEvent->SamplesDuration < (pPrevSamplesLen * 2)) ? pEvent->SamplesDuration - pPrevSamplesLen : pPrevSamplesLen;
        float   *DstRAW = (float*)malloc(sizeof(float) * pVoltageChannels * LenRAW);
        for(float *DstPtr = &DstRAW[pVoltageChannels * (LenRAW - 1)], *SrcPtr = &pPrevSamples[pPrevSamplesPos * pVoltageChannels]; DstPtr >= DstRAW; DstPtr -= pVoltageChannels) {
          SrcPtr = (SrcPtr == pPrevSamples) ? &pPrevSamples[(pPrevSamplesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
          for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
        }
        DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", evttype = " + DB.Bind(EventTypeToString(pEvent->Type)) + ", reference = " + DB.Bind(pEvent->Reference, pVoltageChannels) +
                                ", peak = " + DB.Bind(pEvent->PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent->ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent->SamplesDuration) +
                                ", samples_rms = samples_rms || " + DB.Bind(DstRMS, LenRMS, pVoltageChannels) + ", samples_raw = samples_raw || " + DB.Bind(DstRAW, LenRAW, pVoltageChannels) +
                                " WHERE starttime = " + DB.Bind(pEvent->Start));
        if(DstRMS != NULL) free(DstRMS);
        if(DstRAW != NULL) free(DstRAW);
        pEvent->CyclesDuration  = 0;
        pEvent->SamplesDuration = 0;
        pEvents.push_back(pEvent);
        pEvent = NULL;
      } else {
        if(pEvent->Type == EVT_INTERRUPTION) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmin(pEvent->PeakValue[n], CycleRMS[n]);
        } else if(pEvent->Type == EVT_DIP) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmin(pEvent->PeakValue[n], CycleRMS[n]);
        } else if(pEvent->Type == EVT_SWELL) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = fmax(pEvent->PeakValue[n], CycleRMS[n]);
        } else if(pEvent->Type == EVT_RVC) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent->PeakValue[n] = (fabs(pEvent->Reference[n] - CycleRMS[n]) > fabs(pEvent->Reference[n] - pEvent->PeakValue[n])) ? CycleRMS[n] : pEvent->PeakValue[n];
        }
      }
      pHaveEvent = (pEvent != NULL);
    }
    for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevCycles[pPrevCyclesPos * pVoltageChannels + n] = CycleRMS[n];
    pPrevCyclesPos  = (pPrevCyclesPos + 1) % pPrevCyclesLen;
    if(pEvent != NULL) {
      pEvent->CyclesDuration++;
      if(pEvent->CyclesDuration == pPrevCyclesSaveLen) {
        Database DB(pSerial);
        float *DstRMS = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevCyclesSaveLen);
        for(float *DstPtr = &DstRMS[pVoltageChannels * (pPrevCyclesSaveLen - 1)], *SrcPtr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels]; DstPtr >= DstRMS; DstPtr -= pVoltageChannels) {
          SrcPtr = (SrcPtr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
          for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
        }
        DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_rms = samples_rms || " + DB.Bind(DstRMS, pPrevCyclesSaveLen, pVoltageChannels) + " WHERE starttime = " + DB.Bind(pEvent->Start));
        if(DstRMS != NULL) free(DstRMS);
      }
    }
    for(auto &Event : pEvents) {
      Event->CyclesDuration++;
      if(Event->CyclesDuration == pPrevCyclesSaveLen) {
        Database DB(pSerial);
        float *DstRMS = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevCyclesSaveLen);
        for(float *DstPtr = &DstRMS[pVoltageChannels * (pPrevCyclesSaveLen - 1)], *SrcPtr = &pPrevCycles[pPrevCyclesPos * pVoltageChannels]; DstPtr >= DstRMS; DstPtr -= pVoltageChannels) {
          SrcPtr = (SrcPtr == pPrevCycles) ? &pPrevCycles[(pPrevCyclesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
          for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
        }
        DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_rms = samples_rms || " + DB.Bind(DstRMS, pPrevCyclesSaveLen, pVoltageChannels) + " WHERE starttime = " + DB.Bind(Event->Start));
        if(DstRMS != NULL) free(DstRMS);
      }
    }
    for(uint16_t j = 0; j < CycleLen; j++) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) pPrevSamples[pPrevSamplesPos * pVoltageChannels + n] = block->VoltageSample(n, Pos + j);
      pPrevSamplesPos = (pPrevSamplesPos + 1) % pPrevSamplesLen;
      if(pEvent != NULL) {
        pEvent->SamplesDuration++;
        if(pEvent->SamplesDuration == pPrevSamplesLen) {
          float *DstRAW = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevSamplesLen);
          for(float *DstPtr = &DstRAW[pVoltageChannels * (pPrevSamplesLen - 1)], *SrcPtr = &pPrevSamples[pPrevSamplesPos * pVoltageChannels]; DstPtr >= DstRAW; DstPtr -= pVoltageChannels) {
            SrcPtr = (SrcPtr == pPrevSamples) ? &pPrevSamples[(pPrevSamplesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
            for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
          }
          Database DB(pSerial);
          DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_raw = samples_raw || " + DB.Bind(DstRAW, pPrevSamplesLen, pVoltageChannels) + " WHERE starttime = " + DB.Bind(pEvent->Start));
          if(DstRAW != NULL) free(DstRAW);
        }
      }
      for(auto &Event : pEvents) {
        Event->SamplesDuration++;
        if(Event->SamplesDuration == pPrevSamplesLen) {
          float *DstRAW = (float*)malloc(sizeof(float) * pVoltageChannels * pPrevSamplesLen);
          for(float *DstPtr = &DstRAW[pVoltageChannels * (pPrevSamplesLen - 1)], *SrcPtr = &pPrevSamples[pPrevSamplesPos * pVoltageChannels]; DstPtr >= DstRAW; DstPtr -= pVoltageChannels) {
            SrcPtr = (SrcPtr == pPrevSamples) ? &pPrevSamples[(pPrevSamplesLen - 1) * pVoltageChannels] : SrcPtr - pVoltageChannels;
            for(uint8_t n = 0; n < pVoltageChannels; n++) DstPtr[n] = SrcPtr[n];
          }
          Database DB(pSerial);
          DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", samples_raw = samples_raw || " + DB.Bind(DstRAW, pPrevSamplesLen, pVoltageChannels) + " WHERE starttime = " + DB.Bind(Event->Start));
          if(DstRAW != NULL) free(DstRAW);
        }
      }
    }
    for(auto Tmp = pEvents.begin(); Tmp != pEvents.end(); ) {
      if(((*Tmp)->SamplesDuration >= pPrevSamplesLen) && ((*Tmp)->CyclesDuration > pPrevCyclesSaveLen)) {
        LogFile::Debug("Event '%s' on analyzer '%s' finished", EventTypeToString((*Tmp)->Type).c_str(), pSerial.c_str());
        delete *Tmp;
        pEvents.erase(Tmp++);
      } else {
        Tmp++;
      }
    }
    pPrevStationary = Stationary;
    Pos = Pos + CycleLen;
  }
}
// ---------------------------------------------------------------------------------------------------------------------