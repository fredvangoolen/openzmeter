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
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#include <fftw3.h>
#include "cctz/time_zone.h"
#include "muParser/muParser.h"
#include "AnalyzerSoft.h"
#include "Tools.h"
#include "SysSupport.h"
//#include "FFT.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
#define EVENT_RVC                        5.0
#define EVENT_DIP_SWELL                 10.0        //Variacion en % para generar subida o bajada
#define EVENT_INTERRUPTION              90.0        //Bajada en % para generar interrupcion
#define EVENT_INTERRUPTION_HYSTERESIS   40.0        //Interruption hysteresis
#define EVENT_HYSTERESIS                02.0        //Variacion en % para cerrar evento (DIP/SWELL/INTERRUPTION)
#define EVENT_RAWENVELOPETIME            0.5        //Time in seconds before and after event to store RAW samples
#define EVENT_RMSENVELOPETIME           10.0        //Time in seconds before and after event to store RMS samples
//----------------------------------------------------------------------------------------------------------------------
const vector<float> AnalyzerSoft::FrequencyRanges  = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
const vector<float> AnalyzerSoft::VoltageRanges    = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
const vector<float> AnalyzerSoft::THDRanges        = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
const vector<float> AnalyzerSoft::HarmonicsAllowed = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
const vector<float> AnalyzerSoft::UnbalanceRanges  = {0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.50, 1.75, 2.0, 2.25, 2.50, 2.75, 3.0, 3.25, 3.50, 3.75, 4.0, 4.25, 4.50, 4.75, 5.0, 5.5, 6.0, 6.5, 7.0, 7.5, 8.0, 8.5, 9.0, 9.5, 10.0, INFINITY};
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t     AnalyzerSoft::pFFTMutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t            AnalyzerSoft::pFFTRunning = 0;
uint32_t            AnalyzerSoft::pAlarmsID = 0;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3), pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pMode(mode) {
  LOGFile::Debug("Creating AnalyzerSoft database tables\n");
  Database DB(id);
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Tools::GetTime() - 86400000));   //Delete 1 day old
  DB.ExecSentenceNoResult("DELETE FROM aggreg_10c WHERE sampletime < " + DB.Bind(Tools::GetTime() - 3600000));       //Delete 1 hour old
  pDelayVoltage     = NULL;
  pDelayCurrent     = NULL;
  pVoltageSamples   = NULL;
  pCurrentSamples   = NULL;
  pVoltageFFT       = NULL;
  pCurrentFFT       = NULL;
  pAlarmsWorker     = NULL;
  pEventsWorker     = NULL;
  pFreqCycles       = NULL;
  pPrevTimestamp    = 0;
  pSyncThread       = 0;
  InitAggreg(pAggreg200MS, AGGREG_200MS);
  InitAggreg(pAggreg3S, AGGREG_3S);
  InitAggreg(pAggreg1M, AGGREG_1M);
  InitAggreg(pAggreg10M, AGGREG_10M);
  InitAggreg(pAggreg15M, AGGREG_15M);
  InitAggreg(pAggreg1H, AGGREG_1H);
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFTWorker[i] = new Worker("FFT_V" + to_string(i), this, &FFT_Func);
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFTWorker[i] = new Worker("FFT_I" + to_string(i), this, &FFT_Func);
  Configure(CheckConfig(ConfigManager::ReadConfig(optionPath)));
  pAnalyzerWorker = new Worker((pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N", this, &Analyzer_Func);
}
AnalyzerSoft::~AnalyzerSoft() {
  pSyncTerminate = true;
  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
  if(pAnalyzerWorker != NULL) delete pAnalyzerWorker;
  if(pEventsWorker != NULL) delete pEventsWorker;
  if(pAlarmsWorker != NULL) delete pAlarmsWorker;
  if(pFreqCycles != NULL) free(pFreqCycles);
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  if(pVoltageSamples != NULL) free(pVoltageSamples);
  if(pCurrentSamples != NULL) free(pCurrentSamples);
  if(pVoltageFFT != NULL) free(pVoltageFFT);
  if(pCurrentFFT != NULL) free(pCurrentFFT);
  for(uint8_t n = 0; n < pVoltageChannels; n++) {
    if(pVoltageFFTWorker[n] != NULL) delete pVoltageFFTWorker[n];
  }
  for(uint8_t n = 0; n < pCurrentChannels; n++) {
    if(pCurrentFFTWorker[n] != NULL) delete pCurrentFFTWorker[n];
  }
  pthread_mutex_lock(&pFFTMutex);
  if(pFFTRunning == 0) {
    LOGFile::Debug("Clearing FFT cached plans\n");
    fftwf_cleanup();
  }
  pthread_mutex_unlock(&pFFTMutex);
}
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples) {
  pAnalyzerLock.ReadLock();
  if(pNominalFrequency == 0) {
    for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageSamples[i * pMaxSamples + pBlockSamples] = voltageSamples[i];
    for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentSamples[i * pMaxSamples + pBlockSamples] = currentSamples[i];
    pBlockSamples++;
  } else {
    float *Ptr = &pDelayVoltage[pDelayPos * pVoltageChannels];
    for(uint8_t i = 0; i < pVoltageChannels; i++) {
      pVoltageSamples[i * pMaxSamples + pBlockSamples] = Ptr[i];
      Ptr[i] = voltageSamples[i];
    }
    Ptr = &pDelayCurrent[pDelayPos * pCurrentChannels];
    for(uint8_t i = 0; i < pCurrentChannels; i++) {
      pCurrentSamples[i * pMaxSamples + pBlockSamples] = Ptr[i];
      Ptr[i] = currentSamples[i];
    }
    pDelayPos = (pDelayPos + 1) % pDelaySize;
    pBlockSamples++;
    for(unsigned i = 0; i < pVoltageChannels; i++) {
      for(unsigned n = 0; n < FILTERSTAGES; n++) {
        float Out = voltageSamples[i] * pFilters[n].A0 + pFilters[n].ChannelsZ1[i];
        pFilters[n].ChannelsZ1[i] = voltageSamples[i] * pFilters[n].A1 + pFilters[n].ChannelsZ2[i] - pFilters[n].B1 * Out;
        pFilters[n].ChannelsZ2[i] = voltageSamples[i] * pFilters[n].A2 - pFilters[n].B2 * Out;
        voltageSamples[i] = Out;
      }
    }
    pBlockCycles[pBlockCyclesCount]++;
    if((((pLastSamples[0] >= 0.0) && (voltageSamples[0] < 0.0)) || ((pLastSamples[0] <= 0.0) && (voltageSamples[0] > 0.0))) && (pBlockCycles[pBlockCyclesCount] > ((pSamplerate * 0.85) / (2.0 * pNominalFrequency)))) pBlockCyclesCount++;
    for(uint8_t i = 0; i < pVoltageChannels; i++) pLastSamples[i] = voltageSamples[i];
  }
  if((pBlockSamples == pMaxSamples) || ((pNominalFrequency == 50.0) && (pBlockCyclesCount == 20)) || ((pNominalFrequency == 60.0) && (pBlockCyclesCount == 24))) {
    if(pSequence == 0) {
      InitAggreg(pAggreg200MS, AGGREG_200MS);
      pBlockSamples     = 0;
      pBlockCyclesCount = 0;
      pSequence ++;
      memset(pBlockCycles, 0, sizeof(pBlockCycles));
      pAnalyzerLock.Unlock();
      return false;
    } else {
      pAnalyzerWorker->Run(&pAggreg200MS);
      pAnalyzerLock.Unlock();
      return true;
    }
  }
  pAnalyzerLock.Unlock();
  return false;
}
bool AnalyzerSoft::IsWorking() {
  return pAnalyzerWorker->IsWorking();
}
json AnalyzerSoft::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed analyzer";
  ConfigOUT["NominalVoltage"] = 230.0;
  ConfigOUT["NominalFrequency"] = 50.0;
  ConfigOUT["MaxCurrent"] = 35.0;
  ConfigOUT["MinCurrent"] = 0.1;
  ConfigOUT["TimeZone"] = "UTC";
  ConfigOUT["Tags"] = json::array();
  ConfigOUT["RemoteEnabled"] = false;
  ConfigOUT["RemoteServer"] = "";
  ConfigOUT["RemoteUser"] = "";
  ConfigOUT["RemotePass"] = "";
  ConfigOUT["Save200MS"] = false;
  ConfigOUT["EventsEnabled"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
  ConfigOUT["Alarms"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFrequency") && config["NominalFrequency"].is_number() && (config["NominalFrequency"] >= 0) && (config["NominalFrequency"] <= 400)) ConfigOUT["NominalFrequency"] = config["NominalFrequency"];
  if(config.contains("MaxCurrent") && config["MaxCurrent"].is_number() && (config["MaxCurrent"] >= 1) && (config["MaxCurrent"] <= 100000)) ConfigOUT["MaxCurrent"] = config["MaxCurrent"];
  if(config.contains("MinCurrent") && config["MinCurrent"].is_number() && (config["MinCurrent"] >= 0.01) && (config["MinCurrent"] <= 100)) ConfigOUT["MinCurrent"] = config["MinCurrent"];
  if(config.contains("TimeZone") && config["TimeZone"].is_string()) {
    time_zone TimeZone;
    string TZ = config["TimeZone"];
    if(cctz::load_time_zone(TZ, &TimeZone) == true) ConfigOUT["TimeZone"] = TZ;
  }
  if(config.contains("Tags") && config["Tags"].is_array()) {
    for(auto Tag : config["Tags"]) {
      if(!Tag.is_object() || !Tag.contains("Name") || !Tag["Name"].is_string() || !Tag.contains("Value") || !Tag["Value"].is_string()) continue;
      json Row = json::object();
      Row["Name"] = Tag["Name"];
      Row["Value"] = Tag["Value"];
      ConfigOUT["Tags"].push_back(Row);
    }
  }
  if(config.contains("RemoteEnabled") && config["RemoteEnabled"].is_boolean()) ConfigOUT["RemoteEnabled"] = config["RemoteEnabled"];
  if(config.contains("RemoteServer") && config["RemoteServer"].is_string()) ConfigOUT["RemoteServer"] = config["RemoteServer"];
  if(config.contains("RemoteUser") && config["RemoteUser"].is_string()) ConfigOUT["RemoteUser"] = config["RemoteUser"];
  if(config.contains("RemotePass") && config["RemotePass"].is_string()) ConfigOUT["RemotePass"] = config["RemotePass"];
  if(config.contains("Save200MS") && config["Save200MS"].is_boolean()) ConfigOUT["Save200MS"] = config["Save200MS"];
  if(config.contains("EventsEnabled") && config["EventsEnabled"].is_boolean()) ConfigOUT["EventsEnabled"] = config["EventsEnabled"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
//  if(config.contains("Alarms") && config["Alarms"].is_array()) ConfigOUT["Alarms"] = Alarms::CheckConfig(config["Alarms"]);
  return ConfigOUT;
}
void AnalyzerSoft::Configure(const json &options) {
  ConfigManager::WriteConfig(pConfigPath, options);
  SetName(options["Name"]);
  pAnalyzerLock.WriteLock();
  pNominalVoltage   = options["NominalVoltage"];
  pNominalFrequency = options["NominalFrequency"];
  pMaxCurrent       = options["MaxCurrent"];
  pMinCurrent       = options["MinCurrent"];
  pTimeZone         = options["TimeZone"];
  pSave200MS        = options["Save200MS"];
  if(pNominalFrequency == 0.0) {
    pFreqCyclesLen  = 1;
    pMaxSamples = pSamplerate * 0.2;
  } else if(pNominalFrequency == 50.0) {
    pFreqCyclesLen  = ZEROCROSSING_SECONDS * 50;
    pMaxSamples = (10.0 * pSamplerate * 1.15) / pNominalFrequency;
  } else if(pNominalFrequency == 60.0) {
    pFreqCyclesLen = ZEROCROSSING_SECONDS * 60;
    pMaxSamples = (12.0 * pSamplerate * 1.15) / pNominalFrequency;
  }
  pFreqCyclesPos    = 0;
  pSequence         = 0;
  pBlockSamples     = 0;
  pBlockCyclesCount = 0;
  InitAggreg(pAggreg200MS, AGGREG_200MS);
  memset(pBlockCycles, 0, sizeof(pBlockCycles));
  pFreqCycles       = (uint32_t*)realloc(pFreqCycles, pFreqCyclesLen * sizeof(uint32_t));
  pVoltageSamples   = (float*)realloc(pVoltageSamples, sizeof(float) * pVoltageChannels * pMaxSamples);
  pCurrentSamples   = (float*)realloc(pCurrentSamples, sizeof(float) * pCurrentChannels * pMaxSamples);
  pVoltageFFT       = (float*)realloc(pVoltageFFT, sizeof(float) * 2 * pVoltageChannels * ((pMaxSamples / 2) + 1));
  pCurrentFFT       = (float*)realloc(pCurrentFFT, sizeof(float) * 2 * pCurrentChannels * ((pMaxSamples / 2) + 1));
  for(uint8_t n = 0; n < pVoltageChannels; n++) pLastSamples[n] = 0.0;
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFTChannels[i].InPtr = &pVoltageSamples[i * pMaxSamples];
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFTChannels[i].InPtr = &pCurrentSamples[i * pMaxSamples];
  if(pNominalFrequency != 0.0) {
    for(uint8_t n = 0; n < FILTERSTAGES; n++) {
      float w0 = 2 * M_PI * (pNominalFrequency / pSamplerate);
      float alpha = sin(w0) / 2.0;
      pFilters[n].A0 = alpha / (1.0 + alpha);
      pFilters[n].A1 = 0.0;
      pFilters[n].A2 = -alpha / (1.0 + alpha);
      pFilters[n].B1 = (-2.0 * cos(w0)) / (1.0 + alpha);
      pFilters[n].B2 = (1.0 - alpha) / (1.0 + alpha);
      memset(pFilters[n].ChannelsZ1, 0, sizeof(Filter_t::ChannelsZ1));
      memset(pFilters[n].ChannelsZ2, 0, sizeof(Filter_t::ChannelsZ2));
    }
    pDelaySize = ((pSamplerate / pNominalFrequency) / 2.0) * FILTERSTAGES + 2;
    pDelayPos = 0;
    pDelayVoltage = (float*)realloc(pDelayVoltage, sizeof(float) * pDelaySize * pVoltageChannels);
    pDelayCurrent = (float*)realloc(pDelayCurrent, sizeof(float) * pDelaySize * pCurrentChannels);
  } else {
    if(pDelayVoltage != NULL) {
      free(pDelayVoltage);
      pDelayVoltage = NULL;
    }
    if(pDelayCurrent != NULL) {
      free(pDelayCurrent);
      pDelayCurrent = NULL;
    }
  }
  bool EventsEnabled = options["EventsEnabled"];
  if((EventsEnabled == false) && (pEventsWorker != NULL)) {
    delete pEventsWorker;
    pEventsWorker = NULL;
  } else if((EventsEnabled == true) && (pEventsWorker == NULL)) {
    pPrevStationary = false;
    pEventsWorker = new Worker("EventsDet", this, &Events_Func);
  }
  bool AlarmsEnabled = options["AlarmsEnabled"];
  if((AlarmsEnabled == true) && (pAlarmsWorker != NULL)) {
    delete pAlarmsWorker;
    pAlarmsWorker = NULL;
  } else if((AlarmsEnabled == true) && (pAlarmsWorker == NULL)) {
    pAlarmsWorker = new Worker("AlarmsDet", this, &Alarms_Func);
  }
  bool ExportEnabled = options["RemoteEnabled"];
  if(ExportEnabled == true) {
    pSyncServer    = options["RemoteServer"];
    pSyncUser      = options["RemoteUser"];
    pSyncPass      = options["RemotePass"];
    pSyncTerminate = false;
    if(pSyncThread == 0) {
      if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
        pSyncThread = 0;
      } else {
        pthread_setname_np(pSyncThread, "Sync");
      }
    }
  } else if((ExportEnabled == false) && (pSyncThread != 0)) {
    pSyncTerminate = true;
    pthread_join(pSyncThread, NULL);
    pSyncThread = 0;
  }
  pAnalyzerLock.Unlock();
}
void AnalyzerSoft::InitAggreg(Params_t &params, AggregType type) {
  params.Type = type;
  params.AggregSamples = 0;
  params.Timestamp = 0;
  params.Frequency = 0;
  params.Unbalance = 0;
  params.Flag = false;
//  if((params.Type == AGGREG_200MS) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage[n] = 0;
    for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current[n] = 0;
//  }
//  if((params.Type == AGGREG_200MS) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1H)) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Active_Power[n] = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Reactive_Power[n] = 0;
//  }
//  if((params.Type == AGGREG_200MS) || (params.Type == AGGREG_1M)) {
    for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current_THD[n] = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Phi[n] = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Power_Factor[n] = 0;
    for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) params.Current_Harmonics[n] = 0;
    for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Power_Harmonics[n] = 0;
//  }
//  if((params.Type == AGGREG_200MS) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M)) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_THD[n] = 0;
    for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Voltage_Harmonics[n] = 0;
//  }
//  if(params.Type == AGGREG_200MS) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Active_Energy[n] = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_Phase[n]  = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Reactive_Energy[n] = 0;
    for(uint8_t n = 0; n < pVoltageChannels; n++) params.Apparent_Power[n] = 0;
//  }
//  if(params.Type == AGGREG_1H) {
    params.StatsFrequency = vector<uint16_t>(FrequencyRanges.size() - 1, 0);
    params.StatsVoltage = vector<uint16_t>(VoltageRanges.size() - 1, 0);
    params.StatsTHD = vector<uint16_t>(THDRanges.size() - 1, 0);
    params.StatsUnbalance = vector<uint16_t>(UnbalanceRanges.size() - 1, 0);
    params.StatsHarmonics = vector<uint16_t>(2, 0);
//  }
}
void AnalyzerSoft::InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos) {
  Event_t Event;
  Event.Type = type;
  Event.Duration = 0;
  Event.State = EVT_RUNNING;
  Event.Reference = reference;
  Event.PeakValue = reference;
  Event.ChangeValue = reference;
  Event.SamplesRAW_A = deque<vector<float> >();
  Event.SamplesRMS_A = deque<vector<float> >();
  Event.SamplesRAW_B = deque<vector<float> >();
  Event.SamplesRMS_B = deque<vector<float> >();
  for(deque<EventCycle_t>::reverse_iterator it = pPrevCycles.rbegin(); it != pPrevCycles.rend(); it++) {
    Event.SamplesRMS_A.push_front(it->RMS);
    if(Event.SamplesRMS_A.size() == (pNominalFrequency * 2 * EVENT_RMSENVELOPETIME)) break;
  }
  for(deque<vector<float> >::reverse_iterator it = pPrevSamples.rbegin(); it != pPrevSamples.rend(); it++) {
    Event.SamplesRAW_A.push_front(*it);
    if(Event.SamplesRAW_A.size() == (EVENT_RAWENVELOPETIME * pSamplerate)) break;
  }
  //A new event destroy any running RVC
  if(type != EVT_RVC) {
    for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); ) {
      if((it->Type == EVT_RVC) && (it->State == EVT_RUNNING)) {
        Database DB(Serial);
        DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(it->Start) + " AND evttype = 'RVC'");
        pEvents.erase(it++);
      } else {
        it++;
      }
    }
  }
  Event.Start = pAggreg200MS.Timestamp + (uint32_t)((pos * 1000) / pSamplerate);
  SaveEvent(Event);
  pEvents.push_back(Event);
}
void AnalyzerSoft::AccumulateAggreg(Params_t &dst, const Params_t &src) {
  dst.Timestamp = src.Timestamp;
  dst.AggregSamples += src.AggregSamples;
  dst.Frequency += src.Frequency;
  dst.Unbalance += src.Unbalance;
  dst.Flag |= src.Flag;
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage[n] += pow(src.Voltage[n], 2.0);
  for(uint8_t n = 0; n < pCurrentChannels; n++) dst.Current[n] += pow(src.Current[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Active_Power[n] += src.Active_Power[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Reactive_Power[n] += src.Reactive_Power[n];
  for(uint8_t n = 0; n < pCurrentChannels; n++) dst.Current_THD[n] += pow(src.Current_THD[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Phi[n] += src.Phi[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Power_Factor[n] += src.Power_Factor[n];
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) dst.Current_Harmonics[n] += pow(src.Current_Harmonics[n], 2.0);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) dst.Power_Harmonics[n] += src.Power_Harmonics[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage_THD[n] += pow(src.Voltage_THD[n], 2.0);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) dst.Voltage_Harmonics[n] += pow(src.Voltage_Harmonics[n], 2.0);
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Active_Energy[n] += src.Active_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage_Phase[n] += src.Voltage_Phase[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Reactive_Energy[n] += src.Reactive_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Apparent_Power[n] += src.Apparent_Power[n];
}
void AnalyzerSoft::NormalizeAggreg(Params_t &params) {
  params.Frequency = params.Frequency / params.AggregSamples;
  params.Unbalance = params.Unbalance / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage[n] = sqrt(params.Voltage[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current[n] = sqrt(params.Current[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Active_Power[n] = params.Active_Power[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Reactive_Power[n] = params.Reactive_Power[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current_THD[n] = sqrt(params.Current_THD[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Phi[n] = params.Phi[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Power_Factor[n] = params.Power_Factor[n] / params.AggregSamples;
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) params.Current_Harmonics[n] = sqrt(params.Current_Harmonics[n] / params.AggregSamples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Power_Harmonics[n] = params.Power_Harmonics[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_THD[n] = sqrt(params.Voltage_THD[n] / params.AggregSamples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Voltage_Harmonics[n] = sqrt(params.Voltage_Harmonics[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_Phase[n] = params.Voltage_Phase[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Apparent_Power[n] = params.Apparent_Power[n] / params.AggregSamples;
  params.AggregSamples = 1;
}
bool AnalyzerSoft::FindRunningEvent(const uint32_t mask) {
  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
  }
  return false;
}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  Database DB(Serial);
  if(params.Type == AGGREG_200MS)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10c(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Power_Factor, pVoltageChannels) + ", " +
    DB.Bind(params.Voltage_THD, pVoltageChannels) + ", " +
    DB.Bind(params.Current_THD, pCurrentChannels) + ", " +
    DB.Bind(params.Phi, pVoltageChannels) + ")");
  else if(params.Type == AGGREG_3S)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  else if(params.Type == AGGREG_1M)
    DB.ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) SELECT sampletime, " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Power_Factor, pVoltageChannels) + ", " +
    DB.Bind(params.Voltage_THD, pVoltageChannels) + ", " +
    DB.Bind(params.Current_THD, pCurrentChannels) + ", " +
    DB.Bind(params.Phi, pVoltageChannels) + ", " +
    DB.Bind(params.Unbalance) + ", " +
    DB.Bind(params.Voltage_Harmonics, pVoltageChannels, 50) + ", " +
    DB.Bind(params.Current_Harmonics, pCurrentChannels, 50) + ", " +
    DB.Bind(params.Power_Harmonics, pVoltageChannels, 50) + " FROM Sample");
  else if(params.Type == AGGREG_10M)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ")");
  else if(params.Type == AGGREG_15M)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  else if(params.Type == AGGREG_1H)
    DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.StatsVoltage) + ", " +
    DB.Bind(params.StatsFrequency) + ", " +
    DB.Bind(params.StatsTHD) + ", " +
    DB.Bind(params.StatsHarmonics) + ", " +
    DB.Bind(params.StatsUnbalance) + ")");
  if(params.Type == AGGREG_1M) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(params.Timestamp - 86400000));   //Delete 1 day old
  if((params.Type == AGGREG_1M) && (pSave200MS == true)) DB.ExecSentenceNoResult("DELETE FROM aggreg_10c WHERE sampletime < " + DB.Bind(params.Timestamp - 3600000));     //Delete 1 hour old
  SetOnline();
}
void AnalyzerSoft::SaveEvent(Event_t &evt) {
  Database DB(Serial);
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
    DB.ExecSentenceNoResult("INSERT INTO events(starttime, duration, samplerate, finished, evttype, reference, peak, change, notes, samples1, rms1) VALUES (" + DB.Bind(evt.Start) + ", 0, " + DB.Bind(pSamplerate) + ", false, " + DB.Bind(Type) + ", " + DB.Bind(evt.Reference) + ", " + DB.Bind(evt.PeakValue) + ", " + DB.Bind(evt.ChangeValue) + ", '', " + DB.Bind(RAW) + ", " + DB.Bind(RMS) + ")");
  } else if(evt.State == EVT_CLOSING) {
    DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", duration = " + DB.Bind(evt.Duration) + ", peak = " + DB.Bind(evt.PeakValue) + ", change = " + DB.Bind(evt.ChangeValue) + ", samples2 = " + DB.Bind(RAW) + ", rms2 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
  } else {
    DB.ExecSentenceNoResult("UPDATE events SET finished = TRUE, samples3 = " + DB.Bind(RAW) + ", rms3 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
  }
  evt.SamplesRAW_A = deque<vector<float> >();
  evt.SamplesRMS_A = deque<vector<float> >();
  evt.SamplesRAW_B = deque<vector<float> >();
  evt.SamplesRMS_B = deque<vector<float> >();
}
json AnalyzerSoft::GetAlarmJSON(const Alarm_t &alarm) {
  json Alarm = json::object();
  string Expresion     = alarm.Evaluator.GetExpr();
  Alarm["Name"]        = alarm.Name;
  Alarm["Priority"]    = alarm.Priority;
  Alarm["Expression"]  = Expresion.erase(Expresion.find_last_not_of(" \n\r\t")+1);
  Alarm["Aggreg"]      = (alarm.Aggreg == AnalyzerSoft::AGGREG_3S) ? "3S" : (alarm.Aggreg == AnalyzerSoft::AGGREG_1M) ? "1M" : (alarm.Aggreg == AnalyzerSoft::AGGREG_10M) ? "10M" : (alarm.Aggreg == AnalyzerSoft::AGGREG_15M) ? "15M" : "1H";
  Alarm["Samples"]     = alarm.Samples;
  Alarm["Window"]      = alarm.Window;
  return Alarm;
}
void AnalyzerSoft::SaveAlarmsConf() {
  json Config = json::array();
  for(auto &Alarm : pAlarms) {
    Config["Alarms"].push_back(GetAlarmJSON(Alarm));
  }
  ConfigManager::WriteConfig(pConfigPath + "/Alarms", Config);
}
void AnalyzerSoft::InitAlarm(Alarm_t &alarm, const json &config) {
  alarm.Name = config["Name"];
  alarm.Priority = config["Priority"];
  if(alarm.Priority > 2) alarm.Priority = 2;
  try {
    alarm.Evaluator.SetExpr(config["Expression"]);
  } catch (Parser::exception_type &e) {
  }
  alarm.Aggreg = (config["Aggreg"] == "3S") ? AnalyzerSoft::AGGREG_3S : (config["Aggreg"] == "1M") ? AnalyzerSoft::AGGREG_1M : (config["Aggreg"] == "10M") ? AnalyzerSoft::AGGREG_10M : (config["Aggreg"] == "15M") ? AnalyzerSoft::AGGREG_15M : AnalyzerSoft::AGGREG_1H;
  alarm.Samples = config["Samples"];
  alarm.Window = config["Window"];
  alarm.Results = queue<bool>();
}
json AnalyzerSoft::CheckAlarmConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed alarm";
  ConfigOUT["Priority"] = 0;
  ConfigOUT["Expression"] = "";
  ConfigOUT["Aggreg"] = "3S";
  ConfigOUT["Samples"] = 1;
  ConfigOUT["Window"] = 10;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("Priority") && config["Priority"].is_number_unsigned() && config["Priority"] < 3) ConfigOUT["Priority"] = config["Priority"];
  if(config.contains("Expression") && config["Expression"].is_string()) ConfigOUT["Expression"] = config["Expression"];
  if(config.contains("Aggreg") && config["Aggreg"].is_string() && ((config["Aggreg"] == "3S") || (config["Aggreg"] == "1M") || (config["Aggreg"] == "10M") || (config["Aggreg"] == "15M") || (config["Aggreg"] == "1H"))) ConfigOUT["Aggreg"] = config["Aggreg"];
  if(config.contains("Samples") && config["Samples"].is_number_unsigned() && config["Samples"] < 1000) ConfigOUT["Samples"] = config["Samples"];
  if(config.contains("Window") && config["Window"].is_number_unsigned()) ConfigOUT["Window"] = config["Window"];
  if(ConfigOUT["Window"] < ConfigOUT["Samples"]) ConfigOUT["Window"] = ConfigOUT["Samples"];
  return ConfigOUT;
}
float AnalyzerSoft::CalcChange(const float in, const float reference) {
  return ((in - reference) / reference) * 100;
}
float AnalyzerSoft::CalcAngle(const float *a, const float *b) {
  float Tmp;
  if((a[0] == 0.0) && (a[1] == 0.0))
    Tmp = 0.0;
  else if(a[0] < 0.0)
    Tmp = atan(a[1] / a[0]);
  else
    Tmp = atan(a[1] / a[0]) + M_PI;
  if((b[0] == 0.0) && (b[1] == 0.0))
    Tmp = Tmp;
  else if(b[0] < 0.0)
    Tmp = Tmp - atan(b[1] / b[0]);
  else
    Tmp = Tmp - atan(b[1] / b[0]) + M_PI;
  Tmp = fmod(Tmp, 2.0 * M_PI);
  if(Tmp > M_PI) Tmp = Tmp -  2.0 * M_PI;
  return Tmp;

}
float AnalyzerSoft::BinPower(const float *a) {
  return sqrt(pow(a[0], 2.0) + pow(a[1], 2.0)) / M_SQRT2 * 2.0;
}
void AnalyzerSoft::Analyzer_Func(void *creation, void *run) {
  AnalyzerSoft *This  = (AnalyzerSoft*)creation;
  Params_t     *Aggreg = (Params_t*)run;
  This->pAnalyzerLock.ReadLock();
  uint64_t Start = Tools::GetTime();
  Aggreg->Timestamp = Start;
  Aggreg->AggregSamples = 1;
  if(This->pEventsWorker != NULL) This->pEventsWorker->Run(Aggreg);
  This->pFFTSamples = (This->pBlockSamples / 2) + 1;
  for(uint8_t i = 0; i < This->pVoltageChannels; i++) {
    This->pVoltageFFTChannels[i].OutPtr = &This->pVoltageFFT[i * 2 * This->pFFTSamples];
    This->pVoltageFFTWorker[i]->Run(&This->pVoltageFFTChannels[i]);
  }
  for(uint8_t i = 0; i < This->pCurrentChannels; i++) {
    This->pCurrentFFTChannels[i].OutPtr = &This->pCurrentFFT[i * 2 * This->pFFTSamples];
    This->pCurrentFFTWorker[i]->Run(&This->pCurrentFFTChannels[i]);
  }
  if(This->pNominalFrequency > 0) {
    for(uint8_t n = 0; n < This->pBlockCyclesCount; n++) {
      This->pFreqCycles[This->pFreqCyclesPos] = This->pBlockCycles[n];
      This->pFreqCyclesPos = (This->pFreqCyclesPos + 1) % This->pFreqCyclesLen;
    }
    uint32_t Samples  = 0;
    uint16_t Cycles   = 0;
    uint16_t MinCycle = This->pSamplerate / (This->pNominalFrequency * 1.15 * 2.0);
    uint16_t MaxCycle = This->pSamplerate / (This->pNominalFrequency * 0.85 * 2.0);
    for(uint16_t i = 0; i < This->pFreqCyclesLen; i++) {
      if((This->pFreqCycles[i] < MinCycle) && (This->pFreqCycles[i] > MaxCycle)) break;
      Samples += This->pFreqCycles[i];
      Cycles++;
    }
    Aggreg->Frequency = (Cycles == This->pFreqCyclesLen) ? This->pFreqCyclesLen / ((2.0 * Samples) / This->pSamplerate) : NAN;
  }
  for(uint8_t j = 0; j < This->pVoltageChannels; j++) {
    float *ChV = &This->pVoltageSamples[j * This->pMaxSamples];
    float *ChC = &This->pCurrentSamples[j * This->pMaxSamples];
    for(uint16_t i = 0; i < This->pBlockSamples; i++) {
      Aggreg->Voltage[j] += pow(ChV[i], 2.0);
      Aggreg->Active_Power[j] += ChV[i] * ChC[i];
    }
    Aggreg->Voltage[j] = sqrt(Aggreg->Voltage[j] / This->pBlockSamples);
    Aggreg->Active_Power[j] = Aggreg->Active_Power[j] / This->pBlockSamples;
  }
  for(uint8_t j = 0; j < This->pCurrentChannels; j++) {
    float *Ch = &This->pCurrentSamples[j * This->pMaxSamples];
    for(uint16_t i = 0; i < This->pBlockSamples; i++) Aggreg->Current[j] += pow(Ch[i], 2.0);
    Aggreg->Current[j] = sqrt(Aggreg->Current[j] / This->pBlockSamples);
  }
  for(uint8_t j = 0; j < This->pVoltageChannels; j++) {
    This->pVoltageFFTWorker[j]->Wait();
    This->pCurrentFFTWorker[j]->Wait();
    for(uint8_t i = 0; i < 50; i++) {
      Aggreg->Voltage_Harmonics[j * 50 + i] = BinPower(&This->pVoltageFFT[j * This->pFFTSamples * 2 + (20 + i * 20)]);
      Aggreg->Current_Harmonics[j * 50 + i] = BinPower(&This->pCurrentFFT[j * This->pFFTSamples * 2 + (20 + i * 20)]);
      Aggreg->Power_Harmonics[j *50 + i]    = Aggreg->Current_Harmonics[j * 50 + i] * Aggreg->Voltage_Harmonics[j * 50 + i] * cos(CalcAngle(&This->pCurrentFFT[j * This->pFFTSamples * 2 + (20 + i * 20)], &This->pVoltageFFT[j * This->pFFTSamples * 2 + (20 + i * 20)]));
      if(i > 0) Aggreg->Voltage_THD[j] += Aggreg->Voltage_Harmonics[j * 50 + i];
      if(i > 0) Aggreg->Current_THD[j] += Aggreg->Current_Harmonics[j * 50 + i];
    }
    Aggreg->Voltage_Phase[j] = (j == 0) ? 0.0 : CalcAngle(&This->pVoltageFFT[j * This->pFFTSamples * 2 + 20], &This->pVoltageFFT[10 * 2]) * 180.0 / M_PI;
    Aggreg->Voltage_THD[j] = (Aggreg->Voltage_THD[j] / Aggreg->Voltage_Harmonics[j * 50]) * 100.0;
    Aggreg->Apparent_Power[j] = Aggreg->Voltage[j] * Aggreg->Current[j];
    float PHI = CalcAngle(&This->pCurrentFFT[j * This->pFFTSamples * 2 + 20], &This->pVoltageFFT[j * This->pFFTSamples * 2 + 20]);
    Aggreg->Reactive_Power[j] = Aggreg->Voltage_Harmonics[j * 50] * Aggreg->Current_Harmonics[j * 50] * sin(-PHI);
    Aggreg->Phi[j] = PHI * 180.0 / M_PI;
    Aggreg->Current_THD[j] = (Aggreg->Current_THD[j] / Aggreg->Current_Harmonics[j * 50]) * 100.0;
    Aggreg->Active_Energy[j] = (Aggreg->Active_Power[j] * This->pBlockSamples) / (This->pSamplerate * 3600.0);
    Aggreg->Power_Factor[j] = Aggreg->Active_Power[j] / Aggreg->Apparent_Power[j];
    Aggreg->Reactive_Energy[j] = (Aggreg->Reactive_Power[j] * This->pBlockSamples) / (This->pSamplerate * 3600.0);
  }
  if(This->pMode == PHASE3N) {
    This->pCurrentFFTWorker[3]->Wait();
    for(uint i = 0; i < 50; i++) {
      Aggreg->Current_Harmonics[3 * 50 + i] = BinPower(&This->pCurrentFFT[3 * This->pFFTSamples * 2 + (20 + i * 20)]);
      if(i > 0) Aggreg->Current_THD[3] += Aggreg->Current_Harmonics[3 * 50 + i];
    }
    Aggreg->Current_THD[3] = (Aggreg->Current_THD[3] / Aggreg->Current_Harmonics[3 * 50]) * 100.0;
  }
  if(This->pEventsWorker != NULL) This->pEventsWorker->Wait();
  if(((This->pSequence % 15) == 0) && (This->pPrevTimestamp != 0)) {
    LOGFile::Debug("Storing 3S aggregation\n");
    This->NormalizeAggreg(This->pAggreg3S);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg3S);
    if(This->pAggreg3S.Flag == false) {
      float Variation = This->CalcChange(This->pAggreg3S.Frequency, This->pNominalFrequency);
      for(uint8_t Range = 0; Range < 40; Range++) {
        if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) This->pAggreg1H.StatsFrequency[Range]++;
      }
    }
    This->SaveAggreg(This->pAggreg3S);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg3S, AGGREG_3S);
  }
  This->AccumulateAggreg(This->pAggreg3S, This->pAggreg200MS);
  if(((This->pAggreg200MS.Timestamp / 60000) > (This->pPrevTimestamp / 60000)) && (This->pPrevTimestamp != 0)) {
    LOGFile::Debug("Storing 1M aggregation\n");
    This->pAggreg1M.Timestamp = This->pAggreg1M.Timestamp / 60000 * 60000 + 59999;
    This->NormalizeAggreg(This->pAggreg1M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg1M);
    This->SaveAggreg(This->pAggreg1M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg1M, AGGREG_1M);
  }
  This->AccumulateAggreg(This->pAggreg1M, This->pAggreg200MS);
  if(((This->pAggreg200MS.Timestamp / 600000) > (This->pPrevTimestamp / 600000)) && (This->pPrevTimestamp != 0)) {
    LOGFile::Debug("Storing 10M aggregation\n");
    This->pAggreg10M.Timestamp = This->pAggreg10M.Timestamp / 600000 * 600000 + 599999;
    This->NormalizeAggreg(This->pAggreg10M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg10M);
    if(This->pAggreg10M.Flag == false) {
      for(uint i = 0; i < This->pVoltageChannels; i++) {
        float Variation = This->CalcChange(This->pAggreg10M.Voltage[i], This->pNominalVoltage);
        for(uint Range = 0; Range < (VoltageRanges.size() - 1); Range++) {
          if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) This->pAggreg1H.StatsVoltage[Range]++;
        }
        for(uint Range = 0; Range < (THDRanges.size() - 1); Range++) {
          if((This->pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (This->pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) This->pAggreg1H.StatsTHD[Range]++;
        }
        bool HarmonicsInRange = true;
        for(uint n = 0; n < HarmonicsAllowed.size(); n++) {
//          float Variation = This->CalcChange(sqrt(norm(This->pAggreg10M.Voltage_Harmonics[i][n + 1])), This->pNominalVoltage);
//            if(Variation > HarmonicsAllowed[n]) {
//              HarmonicsInRange = false;
//              break;
//             }
        }
        if(HarmonicsInRange == true) {
          This->pAggreg1H.StatsHarmonics[0]++;
        } else {
          This->pAggreg1H.StatsHarmonics[1]++;
        }
      }
    }
    This->SaveAggreg(This->pAggreg10M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg10M, AGGREG_10M);
  }
  This->pNowDataLock.ProviderWrite();
  This->AccumulateAggreg(This->pAggreg10M, This->pAggreg200MS);
  if(((This->pAggreg200MS.Timestamp / 900000) > (This->pPrevTimestamp / 900000)) && (This->pPrevTimestamp != 0)) {
    LOGFile::Debug("Storing 15M aggregation\n");
    This->pAggreg15M.Timestamp = This->pAggreg15M.Timestamp / 900000 * 900000 + 899999;
    This->NormalizeAggreg(This->pAggreg15M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg15M);
    This->SaveAggreg(This->pAggreg15M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg15M, AGGREG_15M);
  }
  This->AccumulateAggreg(This->pAggreg15M, This->pAggreg200MS);
  if(((This->pAggreg200MS.Timestamp / 3600000) > (This->pPrevTimestamp / 3600000)) && (This->pPrevTimestamp != 0)) {
    LOGFile::Debug("Storing 1H aggregation\n");
    This->pAggreg1H.Timestamp = This->pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
    This->NormalizeAggreg(This->pAggreg1H);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg1H);
    This->SaveAggreg(This->pAggreg1H);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg1H, AGGREG_1H);
  }
  This->AccumulateAggreg(This->pAggreg1H, This->pAggreg200MS);
  This->pPrevTimestamp = This->pAggreg200MS.Timestamp;
  if(This->pSave200MS) This->SaveAggreg(This->pAggreg200MS);
  SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
  SysStatus->Lock.WriteLock();
  SysStatus->Events = This->pAggreg200MS.Flag;
  SysStatus->Lock.Unlock();
  This->pNowDataLock.ProviderWait();
  This->InitAggreg(This->pAggreg200MS, AGGREG_200MS);
  This->pBlockSamples     = 0;
  This->pBlockCyclesCount = 0;
  memset(This->pBlockCycles, 0, sizeof(pBlockCycles));
  LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 "\n", This->Serial.c_str(), Tools::GetTime() - Start, This->pSequence);
  This->pSequence++;
  This->pAnalyzerLock.Unlock();
}
void AnalyzerSoft::Events_Func(void *creation, void *run) {
  AnalyzerSoft *This = (AnalyzerSoft*)creation;
  uint32_t RequiredRAW = This->pSamplerate * EVENT_RAWENVELOPETIME;
  uint32_t RequiredRMS = This->pNominalFrequency * 2 * EVENT_RMSENVELOPETIME;
  uint32_t Pos = 0;
  This->pAggreg200MS.Flag = false;
  for(uint32_t i = 0; i < This->pBlockCyclesCount; i++) {
    //Calc cycle stats
    EventCycle_t Cycle;
    uint32_t CycleLen = This->pBlockCycles[i];
//    for(uint n = 0; n < This->pAggreg200MS.VoltageSamples.size(); n++) {
//      float RMS = 0.0;
//      for(uint32_t j = 0; j < CycleLen; j++) RMS += pow(This->pAggreg200MS.VoltageSamples[n][Pos + j], 2);
//      Cycle.RMS.push_back(sqrt(RMS / CycleLen));
//    }
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
      if(fabs(This->CalcChange(Cycle.RMS[n], RMS_Avg[n])) > Tolerance) {
        Stationary = false;
        break;
      }
      for(int j = 0; j < 99; j++) {
        if(fabs(This->CalcChange(it->RMS[n], RMS_Avg[n])) > Tolerance) {
          Stationary = false;
          break;
        }
      }
    }
    //Check for new events
    if((This->pPrevSamples.size() >= RequiredRAW) && (This->pPrevCycles.size() >= RequiredRMS)) {
      for(uint8_t n = 0; n < RMS_Avg.size(); n++) {
        //Check for interruption new events
        if((-This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_INTERRUPTION) && (This->FindRunningEvent(EVT_INTERRUPTION) == false)) {
          bool Found = false;
          for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
            if((it->Type != EVT_DIP) || (it->State != EVT_RUNNING) || (it->Duration > (EVENT_RAWENVELOPETIME * This->pSamplerate))) continue;
            it->Type = EVT_INTERRUPTION;
            Found = true;
            break;
          }
          if(Found == false) This->InitEvent(EVT_INTERRUPTION, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
        }
        //Check for DIP new events
        if((-This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_DIP | EVT_INTERRUPTION) == false)) This->InitEvent(EVT_DIP, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
        //Check for SWELL new events
        if((This->CalcChange(Cycle.RMS[n], This->pNominalVoltage) > EVENT_DIP_SWELL) && (This->FindRunningEvent(EVT_SWELL) == false)) This->InitEvent(EVT_SWELL, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
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
            if(-This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS)) it->State = EVT_RUNNING;
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
            if(-This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
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
            if(This->CalcChange(Cycle.RMS[ch], This->pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS)) it->State = EVT_RUNNING;
          }
          if(it->State == EVT_RUNNING) {
            for(uint8_t ch = 0; ch < Cycle.RMS.size(); ch++) {
              if(Cycle.RMS[ch] > it->PeakValue[ch]) it->PeakValue[ch] = Cycle.RMS[ch];
            }
          }
        }
        if(it->State == EVT_RUNNING) {
          it->Duration += CycleLen;
          if(it->Type != EVT_RVC) This->pAggreg200MS.Flag = true;
        } else {
          This->SaveEvent(*it);
        }
        it++;
      }
    }
    //Insert current cycle samples in prev data buffers and in running events than need its
    for(uint n = 0; n < CycleLen; n++) {
      vector<float> Sample;
//      for(uint j = 0; j < This->pVoltageSamples.size(); j++) Sample.push_back(This->pAggreg200MS.VoltageSamples[j][n + Pos]);
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
}
void AnalyzerSoft::FFT_Func(void *creation, void *run) {
  AnalyzerSoft *This  = (AnalyzerSoft*)creation;
  FFTChannel_t *Ch = (FFTChannel_t*)run;
  pthread_mutex_lock(&pFFTMutex);
  fftwf_plan Plan = fftwf_plan_dft_r2c_1d(This->pBlockSamples, Ch->InPtr, (fftwf_complex*)Ch->OutPtr, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  pFFTRunning++;
  pthread_mutex_unlock(&pFFTMutex);
  fftwf_execute_dft_r2c(Plan, Ch->InPtr, (fftwf_complex*)Ch->OutPtr);
  pthread_mutex_lock(&pFFTMutex);
  fftwf_destroy_plan(Plan);
  pFFTRunning--;
  pthread_mutex_unlock(&pFFTMutex);
  for(uint16_t i = 0; i < (2 * This->pFFTSamples); i++) Ch->OutPtr[i] = Ch->OutPtr[i] / This->pBlockSamples;
}
void AnalyzerSoft::Alarms_Func(void *creation, void *run) {

}
void *AnalyzerSoft::Sync_Thread(void *params) {
  AnalyzerSoft *This = (AnalyzerSoft*)params;
  string Cookie, User, Pass, Server;
  uint32_t ThreadsCount = 0;
  pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  Cond  = PTHREAD_COND_INITIALIZER;
  Client *HTTPClient = NULL;
  while(This->pSyncTerminate == false) {
    This->pAnalyzerLock.ReadLock();
    if((This->pSyncUser != User) || (This->pSyncPass != Pass) || (This->pSyncServer != Server)) {
      Cookie = "";
      User   = This->pSyncUser;
      Pass   = This->pSyncPass;
      Server = This->pSyncServer;
      if(HTTPClient != NULL) {
        delete HTTPClient;
        HTTPClient = NULL;
      }
    }
    This->pAnalyzerLock.Unlock();
    URL Url(Server);
    if(Cookie == "") {
      Headers Header  = {{"Accept", "application/json"}, {"Connection", "keep-alive"}};
      json    Content = json::object();
      Content["Username"] = User;
      Content["Password"] = Pass;
      Content["UpdateLastLogin"] = false;
      if(HTTPClient == NULL) HTTPClient = new Client(Url.Host.c_str(), (Url.Port == "") ? 80 : atoi(Url.Port.c_str()) , 30);
//      HTTPClient->set_read_timeout(1,0);
      auto Response = HTTPClient->Post(string(Url.Path + "/LogIn").c_str(), Header, Content.dump(), "application/json", false);
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if((Response->has_header("Set-Cookie") == true) && (Content.is_discarded() == false) && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          string Tmp = Response->get_header_value("Set-Cookie");
          for(uint8_t n = 0; (n < Tmp.length()) && (Tmp.length() < 100); n++) {
            if(Tmp[n] == ';') break;
            Cookie += Tmp[n];
          }
        }
        if(Cookie != "") LOGFile::Info("Analyzer '%s' logged on remote server\n", This->Serial.c_str());
      } else {
        LOGFile::Debug("Analyzer '%s' can not login on remote server\n", This->Serial.c_str());
      }
      if(Cookie.empty()) usleep(1000000);
    } else {
      Headers Header = {{"Accept", "application/json"}, {"Connection", "keep-alive"}, {"Cookie", Cookie}};
      json   Content = json::object();
      Content["Analyzer"] = This->Serial;
      Content["Name"] = This->GetName();
      LOGFile::Debug("Analyzer '%s' request pending queries to server\n", This->Serial.c_str());
      auto Response = HTTPClient->Post(string(Url.Path + "/getRemote").c_str(), Header, Content.dump(), "application/json", false);
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if(!Content.is_discarded() && Content.is_array()) {
          LOGFile::Debug("Analyzer '%s' receive %u queries from remote server\n", This->Serial.c_str(), Content.size());
          for(uint32_t n = 0; n < Content.size(); n++) {
            if(Content[n].is_object() && Content[n]["Sequence"].is_number() && Content[n]["Method"].is_string() && Content[n]["Params"].is_object()) {
              Sync_t *Request = (Sync_t*)calloc(1, sizeof(Request));
              Request->Count        = &ThreadsCount;
              Request->Mutex        = &Mutex;
              Request->Cond         = &Cond;
              Request->Cookie       = Cookie;
              Request->Port         = (Url.Port == "") ? 80 : atoi(Url.Port.c_str());
              Request->Host         = Url.Host;
              Request->ResponsePath = Url.Path + "/setRemote";
              Request->Analyzer     = This;
              Request->Method       = Content[n]["Method"];
              Request->Params       = Content[n]["Params"];
              Request->Sequence     = Content[n]["Sequence"];
              pthread_t Thread;
              pthread_mutex_lock(&Mutex);
              if(pthread_create(&Thread, NULL, SyncResp_Thread, (void*)Request) != 0) {
                LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
                free(Request);
              } else {
                pthread_setname_np(Thread, "SyncWorker");
                ThreadsCount++;
                pthread_detach(Thread);
              }
              pthread_mutex_unlock(&Mutex);
            }
          }
        }
      } else if((Response != nullptr) && (Response->status == HTTP_UNAUTHORIZED)) {
        Cookie = "";
        LOGFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...\n", This->Serial.c_str());
        usleep(1000000);
      } else if((Response != nullptr) && (Response->status == HTTP_NO_CONTENT)) {
        LOGFile::Debug("No pending queries in remote server for analyzer '%s'\n", This->Serial.c_str());
      } else {
        if(Response != nullptr) LOGFile::Debug("Failed request to remote server in analyzer '%s' (ERROR: %d)\n", This->Serial.c_str(), Response->status);
        usleep(1000000);
      }
    }
  }
  pthread_mutex_lock(&Mutex);
  while(ThreadsCount > 0) {
    LOGFile::Debug("Analyzer '%s' flusing remote request (%d pending)\n", This->Serial.c_str(), ThreadsCount);
    pthread_cond_wait(&Cond, &Mutex);
  }
  pthread_mutex_unlock(&Mutex);
  if(HTTPClient != NULL) delete HTTPClient;
  return NULL;
}
void *AnalyzerSoft::SyncResp_Thread(void *args) {
  Sync_t *This = (Sync_t*)args;
  This->Params["UserName"] = "admin";
  LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  Response Resp;
  if(This->Method == "getAnalyzer") {
    This->Analyzer->GetAnalyzer(This->Params, Resp);
  } else if(This->Method == "setAnalyzer") {
    This->Analyzer->SetAnalyzer(This->Params, Resp);
  } else if(This->Method == "getSeries") {
    This->Analyzer->GetSeries(This->Params, Resp);
  } else if(This->Method == "getSeriesNow") {
    This->Analyzer->GetSeriesNow(This->Params, Resp);
  } else if(This->Method == "getSeriesRange") {
    This->Analyzer->GetSeriesRange(This->Params, Resp);
  } else if(This->Method == "getEvents") {
    This->Analyzer->GetEvents(This->Params, Resp);
  } else if(This->Method == "getEvent") {
    This->Analyzer->GetEvent(This->Params, Resp);
  } else if(This->Method == "setEvent") {
    This->Analyzer->SetEvent(This->Params, Resp);
  } else if(This->Method == "delEvent") {
    This->Analyzer->DelEvent(This->Params, Resp);
  } else if(This->Method == "getRates") {
    This->Analyzer->GetRates(This->Params, Resp);
  } else if(This->Method == "getRate") {
    This->Analyzer->GetRate(This->Params, Resp);
  } else if(This->Method == "setRate") {
    This->Analyzer->SetRate(This->Params, Resp);
  } else if(This->Method == "delRate") {
    This->Analyzer->DelRate(This->Params, Resp);
  } else if(This->Method == "addRate") {
    This->Analyzer->AddRate(This->Params, Resp);
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(This->Params, Resp);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(This->Params, Resp);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(This->Params, Resp);
  } else if(This->Method == "getAlarm") {
    This->Analyzer->GetAlarm(This->Params, Resp);
  } else if(This->Method == "setAlarm") {
    This->Analyzer->SetAlarm(This->Params, Resp);
  } else if(This->Method == "addAlarm") {
    This->Analyzer->AddAlarm(This->Params, Resp);
  } else if(This->Method == "delAlarm") {
    This->Analyzer->DelAlarm(This->Params, Resp);
  }
  Headers Header  = {{"Accept", "application/json"}, {"Connection", "keep-alive"}, {"Cookie", This->Cookie}};
  json    Content = json::object();
  Content["Code"] = Resp.status;
  Content["Sequence"] = This->Sequence;
  Content["Analyzer"] = This->Analyzer->Serial;
  Content["Response"] = Resp.body;
  Client HTTPClient(This->Host.c_str(), This->Port);
  auto Response = HTTPClient.Post(This->ResponsePath.c_str(), Header, Content.dump(), "application/json", true);
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    LOGFile::Debug("Remote request '%s' for analyzer '%s' completed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  } else {
    LOGFile::Error("Remote request '%s' for analyzer '%s' failed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  }
  pthread_mutex_lock(This->Mutex);
  *This->Count = *This->Count - 1;
  pthread_cond_signal(This->Cond);
  pthread_mutex_unlock(This->Mutex);
  free(This);
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Response = ConfigManager::ReadConfig(pConfigPath);
  Response["Online"] = IsOnline();
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  Response.erase("Alarms");
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename))) AS size from pg_tables WHERE schemaname = " + DB.Bind(Serial));
  if(DB.FecthRow()) Response["Size"] = DB.ResultUInt64("size");
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = CheckConfig(params);
  Config["Alarms"] = ConfigManager::ReadConfig(pConfigPath + "/Alarms");
  Configure(Config);
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerSoft::GetSeries(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' param are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "10C") && (Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetSeries(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
  //Insert defaults
  uint64_t To = params["To"];
  uint64_t Period = params["Period"];
  uint64_t LastSample = params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 200;
  string Aggreg = params["Aggreg"];
  if(Aggreg == "3S") PeriodSeconds = 3000;
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    response.set_content("Returned period to large for this aggregation level", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  Database DB(Serial);
//  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Serial));
  string Query = "SELECT ROW_TO_JSON(Result) AS result FROM ( SELECT " + DB.Bind(From) + " AS " + DB.BindID("From") + ", " + DB.Bind(To) +" AS " + DB.BindID("To") + ", ";
  Query += DB.Bind((To - From) / PeriodSeconds) + " AS " + DB.BindID("MaxSamples") + ", " + DB.Bind(Aggreg) + " AS " + DB.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < params["Series"].size(); i++) {
    if(!params["Series"][i].is_string()) {
      response.set_content("Series must be an array of valid serie names", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
    string Serie = params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      }
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      }
    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (((Aggreg == "10C") || Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_p, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Harmonics") + ", ";
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics_Stats") + ", ";
    } else {
      response.set_content("Invalid serie in selected aggregation", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
  }
  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Sampletime") + " FROM ";
  if(params["Aggreg"] == "1M") {
    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
  } else {
    Query += "aggreg_" + Aggreg;
  }
  Query += " WHERE sampletime > " + DB.Bind(LastSample) + " AND sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + ") AS result";
  DB.ExecSentenceAsyncResult(Query);
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("Result"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::GetSeriesNow(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(pNowDataLock.ConsumerWait(5000) == false) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  }
  json Result = json::object();
  Result["Sampletime"] = pAggreg200MS.Timestamp;
  for(uint i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) {
      pNowDataLock.ConsumerEnds();
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    string Serie = params["Series"][i];
    if(Serie == "FREQUENCY") {
      Result["Frequency"] = pAggreg200MS.Frequency;
    } else if(Serie == "FLAG") {
      Result["Flag"] = pAggreg200MS.Flag;
    } else if(Serie == "VOLTAGE") {
      Result["Voltage"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Voltage"].push_back(pAggreg200MS.Voltage[n]);
    } else if(Serie == "CURRENT") {
      Result["Current"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) Result["Current"].push_back(pAggreg200MS.Current[n]);
    } else if(Serie == "ACTIVE_POWER") {
      Result["Active_Power"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Active_Power"].push_back(pAggreg200MS.Active_Power[n]);
    } else if(Serie == "REACTIVE_POWER") {
      Result["Reactive_Power"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Reactive_Power"].push_back(pAggreg200MS.Reactive_Power[n]);
    } else if(Serie == "ACTIVE_ENERGY") {
      Result["Active_Energy"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Active_Energy"].push_back(pAggreg200MS.Active_Energy[n]);
    } else if(Serie == "REACTIVE_ENERGY") {
      Result["Rective_Energy"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Reactive_Energy"].push_back(pAggreg200MS.Reactive_Energy[n]);
    } else if(Serie == "APPARENT_POWER") {
      Result["Apparent_Power"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Apparent_Power"].push_back(pAggreg200MS.Apparent_Power[n]);
    } else if(Serie == "POWER_FACTOR") {
      Result["Power_Factor"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Power_Factor"].push_back(pAggreg200MS.Power_Factor[n]);
    } else if(Serie == "VOLTAGE_THD") {
      Result["Voltage_THD"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Voltage_THD"].push_back(pAggreg200MS.Voltage_THD[n]);
    } else if(Serie == "CURRENT_THD") {
      Result["Current_THD"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) Result["Current_THD"].push_back(pAggreg200MS.Current_THD[n]);
    } else if(Serie == "PHI") {
      Result["Phi"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Phi"].push_back(pAggreg200MS.Phi[n]);
    } else if(Serie == "VOLTAGE_PHASE") {
       Result["Voltage_Phase"] = json::array();
       for(uint8_t n = 0; n < pVoltageChannels; n++) Result["Voltage_Phase"].push_back(pAggreg200MS.Voltage_Phase[n]);
    } else if(Serie == "VOLTAGE_HARMONICS") {
      Result["Voltage_Harmonics"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        Result["Voltage_Harmonics"].push_back(json::array());
        for(uint8_t i = 0; i < 50; i++) {
          Result["Voltage_Harmonics"][n].push_back(json::array());
          Result["Voltage_Harmonics"][n][i].push_back(pVoltageFFT[n * pFFTSamples * 2 + (20 + i * 20) + 0] * M_SQRT2);
          Result["Voltage_Harmonics"][n][i].push_back(pVoltageFFT[n * pFFTSamples * 2 + (20 + i * 20) + 1] * M_SQRT2);
        }
      }
    } else if(Serie == "CURRENT_HARMONICS") {
      Result["Current_Harmonics"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_Harmonics"].push_back(json::array());
        for(uint8_t i = 0; i < 50; i++) {
          Result["Current_Harmonics"][n].push_back(json::array());
          Result["Current_Harmonics"][n][i].push_back(pCurrentFFT[n * pFFTSamples * 2 + (20 + i * 20) + 0] * M_SQRT2);
          Result["Current_Harmonics"][n][i].push_back(pCurrentFFT[n * pFFTSamples * 2 + (20 + i * 20) + 1] * M_SQRT2);
        }
      }
    } else if(Serie == "POWER_HARMONICS") {
      Result["Power_Harmonics"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        Result["Power_Harmonics"].push_back(json::array());
        for(uint8_t i = 0; i < 50; i++) Result["Power_Harmonics"][n].push_back(pAggreg200MS.Power_Harmonics[n * 50 + i]);
      }
    } else if(Serie == "VOLTAGE_FFT") {
      Result["Voltage_FFT"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        Result["Voltage_FFT"].push_back(json::array());
        for(uint16_t i = 0; i < pFFTSamples; i++) Result["Voltage_FFT"][n].push_back(BinPower(&pVoltageFFT[n * 2 * pFFTSamples + (2 * i)]));
      }
    } else if(Serie == "CURRENT_FFT") {
      Result["Current_FFT"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_FFT"].push_back(json::array());
        for(uint16_t i = 0; i < pFFTSamples; i++) Result["Current_FFT"][n].push_back(BinPower(&pCurrentFFT[n * 2 * pFFTSamples + (2 * i)]));
      }
    } else if(Serie == "VOLTAGE_SAMPLES") {
      Result["Voltage_Samples"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        Result["Voltage_Samples"].push_back(json::array());
        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Voltage_Samples"][n].push_back(pVoltageSamples[n * pMaxSamples + i]);
      }
    } else if(Serie == "CURRENT_SAMPLES") {
      Result["Current_Samples"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_Samples"].push_back(json::array());
        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Current_Samples"][n].push_back(pCurrentSamples[n * pMaxSamples + i]);
      }
    } else if(Serie == "SYS_STATS") {
      Result["Sys_Stats"] = SysSupport::GetSystemStatusJSON();
    } else {
      pNowDataLock.ConsumerEnds();
      return HTTPServer::FillResponse(response, "Unknown serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  pNowDataLock.ConsumerEnds();
  return HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '10C', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetEvents(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetEvents(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
//  pMembersLocks.ReadLock();
//  if(pEventsDetector != NULL) {
//    pEventsDetector->GetEvents(params, response);
//  } else {
//    response.set_content("Events detection disabled", "text/plain");
//    response.status = HTTP_SERVICE_UNAVAILABLE;
//  }
//  pMembersLocks.ReadUnlock();
    uint64_t To = params["To"];
  uint64_t Period = params["Period"];
  uint64_t LastSample = params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('From', " + DB.Bind(From) + ", 'To', " + DB.Bind(To) + ", 'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), 'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                              "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), 'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), 'Finished', COALESCE(ARRAY_AGG(finished ORDER BY starttime ASC), '{}'), "
                              "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), 'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), 'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), 'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                              "FROM events WHERE starttime > " + DB.Bind(LastSample) + " AND starttime >= " + DB.Bind(From) + " AND starttime <= " + DB.Bind(To));
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("evt"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetEvent(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
//  pMembersLocks.ReadLock();
//  if(pEventsDetector != NULL) {
//    pEventsDetector->GetEvent(params, response);
//  } else {
//    response.set_content("Events detection disabled", "text/plain");
//    response.status = HTTP_SERVICE_UNAVAILABLE;
//  }
//  pMembersLocks.ReadUnlock();
  Database DB(Serial);
  if((params["Samples"] != "RMS") && (params["Samples"] != "RAW")) {
    response.set_content("'Samples' params must be 'RMS' or 'RAW'", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  uint64_t ID = params["StartTime"];
  string Type = params["Type"];
  string Samples = params["Samples"];
  if(Samples == "RAW") {
    DB.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', samples1, 'Samples', samples2, 'Post_Samples', samples3) AS evt FROM events WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  } else {
    DB.ExecSentenceAsyncResult("SELECT json_build_object('Prev_Samples', rms1, 'Samples', rms2, 'Post_Samples', rms3) AS evt FROM events WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  }
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("evt"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::SetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB(Serial);
  uint64_t ID    = params["StartTime"];
  string   Type  = params["Type"];
  string   Notes = params["Notes"];
  bool Result = DB.ExecSentenceNoResult("UPDATE events SET Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::DelEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB(Serial);
  uint64_t ID = params["StartTime"];
  string Type = params["Type"];
  bool Result = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetRates(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') AS result FROM rates");
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID = params["RateID"];
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT JSON_BUILD_OBJECT('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) AS result FROM rates WHERE rate = " + DB.Bind(RateID));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("StartDate") == false) || (params["StartDate"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'StartDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("EndDate") == false) || (params["EndDate"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'EndDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Config") == false) || (params["Config"].is_object() == false)) return HTTPServer::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID   = params["RateID"];
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  Tariff T(Start, End, pTimeZone);  //THREAD SAFE ISSUE!
  if(T.SetTemplate(params["Template"]) == false) return HTTPServer::FillResponse(response, "Invalid template", "text/plain", HTTP_BAD_REQUEST);
  if(T.SetConfig(params["Config"]) == false) return HTTPServer::FillResponse(response, "Invalid 'Config' format", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("UPDATE rates SET name = " + DB.Bind(Name) + ", template = " + DB.Bind(T.GetTemplate().dump()) + ", config = " + DB.Bind(T.GetConfig().dump()) + ", startdate = " + DB.Bind(Start) + ", enddate = " + DB.Bind(End) + " WHERE rate = " + DB.Bind(RateID));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::DelRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID = params["RateID"];
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + DB.Bind(RateID));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::AddRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartDate") == false) || (params["StartDate"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'StartDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("EndDate") == false) || (params["EndDate"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'EndDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Config") == false) || (params["Config"].is_object() == false)) return HTTPServer::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  Tariff T(Start, End, pTimeZone);  //THREAD SAFE ISSUE!
  if(T.SetTemplate(params["Template"]) == false) return HTTPServer::FillResponse(response, "Invalid template", "text/plain", HTTP_BAD_REQUEST);
  if(T.SetConfig(params["Config"]) == false) return HTTPServer::FillResponse(response, "Invalid 'Config' format", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("INSERT INTO rates(startdate, enddate, config, template, name) VALUES (" + DB.Bind(Start) + ", " + DB.Bind(End) + ", " + DB.Bind(T.GetConfig().dump()) + ", " + DB.Bind(T.GetTemplate().dump()) + ", " + DB.Bind(Name) + ")");
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To         = params["To"];
  uint64_t From       = params["From"];
  string   Aggreg     = params["Aggreg"];
  json     Result     = json::object();
  uint64_t Now        = Tools::GetTime();
  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  if(((To - From) / ((Aggreg == "1H") ? 3600000 : 900000)) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Power"]       = json::array();
  Result["Reactive_Power"]     = json::array();
  Result["Period_Name"]        = json::array();
  Result["Period_Color"]       = json::array();
  Result["Contracted_Power"]   = json::array();
  Result["Expense"]            = json::array();
  Result["Bills"]              = json::array();
  //Load involved tariffs, currency and timezone
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + DB.Bind(From) + " AND startdate <= " + DB.Bind(To));
  json DBTariffs = json::array();
  if(DB.FecthRow()) {
    Result["Currency"] = "€";//req.ResultString("currency");
    DBTariffs = DB.ResultJSON("rate");
  }
  TariffList Tariffs(DBTariffs, pTimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom(From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + DB.Bind(BillingFrom) + " AND sampletime <= " + DB.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = DB.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (DB.ResultUInt64("sampletime") == time)) {
      Active = DB.ResultFloat("active_power");
      Reactive = DB.ResultFloat("reactive_power");
      MoreData = DB.FecthRow();
    }
    Tariff *Tariff = Tariffs.Find(time);
    if((Tariff != NULL) && (Tariff->PushSample(time, Active, Reactive) == true) && ((time + 900000) != BillingTo)) continue;  //If under a tariff push samples until bill cycle ends or no more data!
    //Recover/group values
    do {
      string   Color      = "";
      string   Period     = "";
      uint64_t Time       = time;
      float    Cost       = NAN;
      float    Contracted = NAN;
      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
        Result["Bills"].push_back(Tariff->GetResume(Now));
        break;
      }
      if(Time <= Now) AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
      if(Time <= Now) AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
      if(Time <= Now) AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
      if(Aggreg == "1H") {
        if((Time % 3600000) != 3599999) {
          if(Tariff != NULL) continue;
          break;
        }
        AccActive = AccActive / 4.0;
        AccReactive = AccReactive / 4.0;
      }
      if((Time >= From) && (Time <= To)) {
        Result["Sampletime"].push_back(Time);
        Result["Active_Power"].push_back(AccActive);
        Result["Reactive_Power"].push_back(AccReactive);
        Result["Contracted_Power"].push_back(Contracted);
        if(Color.empty() == true)
          Result["Period_Color"].push_back(nullptr);
        else
          Result["Period_Color"].push_back(Color);
        if(Period.empty() == true)
          Result["Period_Name"].push_back(nullptr);
        else
          Result["Period_Name"].push_back(Period);
        Result["Expense"].push_back(AccCost);
      }
      AccActive = NAN;
      AccReactive = NAN;
      AccCost = NAN;
    } while(Tariff != NULL);
  }
  response.set_content(Result.dump(), "application/json");
  response.status = HTTP_OK;
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  json Result = json::object();
  Result["Variables"] = json::array();
  pAnalyzerLock.ReadLock();
  json NominalVoltage = json::object();
  NominalVoltage["Name"] = "NominalVoltage";
  NominalVoltage["Value"] = pNominalVoltage;
  NominalVoltage["Units"] = "V";
  Result["Variables"].push_back(NominalVoltage);
  json NominalFreq = json::object();
  NominalFreq["Name"] = "NominalFrequency";
  NominalFreq["Value"] = pNominalFrequency;
  NominalFreq["Units"] = "Hz";
  Result["Variables"].push_back(NominalFreq);
  pAnalyzerLock.Unlock();
  Result["Aggregation_Source"] = Aggreg;
  Database DB(Serial);
  if(Aggreg == "3S") {
    DB.ExecSentenceAsyncResult("SELECT COALESCE(json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag, 'Active_Power', active_power, 'Reactive_Power', reactive_power), '{}') as result FROM aggreg_3s ORDER BY sampletime DESC LIMIT 1");
  } else if(Aggreg == "1M") {
    DB.ExecSentenceAsyncResult("SELECT COALESCE(json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag, 'Power_Factor', power_factor, 'Voltage_THD', thd_v, 'Current_THD', thd_i, 'Active_Power', active_power, 'Reactive_Power', reactive_power, 'Phi', phi), '{}') as result FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m ORDER BY sampletime DESC LIMIT 1");
  } else if(Aggreg == "10M") {
    DB.ExecSentenceAsyncResult("SELECT COALESCE(json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag), '{}') as result FROM aggreg_10m ORDER BY sampletime DESC LIMIT 1");
  } else if(Aggreg == "15M") {
    DB.ExecSentenceAsyncResult("SELECT COALESCE(json_build_object('Sampletime', sampletime, 'Active_Power', active_power, 'Reactive_Power', reactive_power), '{}') as result FROM aggreg_15m ORDER BY sampletime DESC LIMIT 1");
  } else if(Aggreg == "1H") {
    DB.ExecSentenceAsyncResult("SELECT COALESCE(json_build_object('Sampletime', sampletime, 'Voltage', rms_v, 'Current', rms_i, 'Frequency', freq, 'Flag', flag, 'Active_Power', active_power, 'Reactive_Power', reactive_power), '{}') as result FROM aggreg_1h ORDER BY sampletime DESC LIMIT 1");
  }
  if(DB.FecthRow() == false) {
    Result["Error"] = "No data avaliable";
    return HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_BAD_REQUEST);
  }
  json Query = DB.ResultJSON("result");
  auto AppendArray = [&Query, &Result](const string &serie, const string &units) {
    for(unsigned n = 0; n < Query[serie].size(); n++) {
      float  Tmp = 0.0;
      string Name = serie;
      if(Query[serie].size() > 1) Name = Name + "_" + to_string(n + 1);
      if(Query[serie][n].is_number()) Tmp = Query[serie][n];
      json Row = json::object();
      Row["Name"] = Name;
      Row["Value"] = Tmp;
      Row["Units"] = units;
      Result["Variables"].push_back(Row);
    }
  };
  if(Query["Sampletime"].is_number()) {
    json Sampletime = json::object();
    Sampletime["Name"] = "Sampletime";
    Sampletime["Value"] = Query["Sampletime"];
    Sampletime["Units"] = "ms";
    Result["Variables"].push_back(Sampletime);
  }
  if(Query["Flag"].is_boolean()) {
    bool Tmp = Query["Flag"];
    json Flag = json::object();
    Flag["Name"] = "Flag";
    Flag["Value"] = Tmp ? 1 : 0;
    Flag["Units"] = "";
    Result["Variables"].push_back(Flag);
  }
  if(Query["Frequency"].is_number()) {
    float Tmp = 0.0;
    if(Query["Frequency"].is_number()) Tmp = Query["Frequency"];
    json Frequency = json::object();
    Frequency["Name"] = "Frequency";
    Frequency["Value"] = Tmp;
    Frequency["Units"] = "Hz";
    Result["Variables"].push_back(Frequency);
  }
  if(Query["Voltage"].is_array()) AppendArray("Voltage", "V");
  if(Query["Current"].is_array()) AppendArray("Current", "A");
  if(Query["Active_Power"].is_array()) AppendArray("Active_Power", "W");
  if(Query["Reactive_Power"].is_array()) AppendArray("Reactive_Power", "var");
  if(Query["Power_Factor"].is_array()) AppendArray("Power_Factor", "");
  if(Query["Voltage_THD"].is_array()) AppendArray("Voltage_THD", "%");
  if(Query["Current_THD"].is_array()) AppendArray("Current_THD", "%");
  if(Query["Phi"].is_array()) AppendArray("Phi", "°");
  if(params.contains("Expression") && params["Expression"].is_string()) {
    string Expression = params["Expression"];
    try {
      Parser Exp;
      for(auto &Var : Result["Variables"]) Exp.DefineConst(Var["Name"], Var["Value"]);
      Exp.SetExpr(Expression);
      float Tmp = Exp.Eval();
      Result["Result"] = Tmp;
    } catch (Parser::exception_type &e) {
      Result["Error"] = e.GetMsg();
    }
  }
  return HTTPServer::FillResponse(response, Result.dump(), "application/json", (Result.contains("Error")) ? HTTP_BAD_REQUEST : HTTP_OK);
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::array();
  pAlarmsLock.ReadLock();
  for(auto Alarm : pAlarms) {
    json Tmp = json::object();
    Tmp["AlarmID"] = Alarm.AlarmID;
    Tmp["Name"] = Alarm.Name;
    Tmp["Priority"] = Alarm.Priority;
    Result.push_back(Tmp);
  }
  pAlarmsLock.Unlock();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetAlarmsLog(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::array();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t AlarmID = params["AlarmID"];
  pAlarmsLock.ReadLock();
  for(auto &Alarm : pAlarms) {
    if(Alarm.AlarmID != AlarmID) continue;
    HTTPServer::FillResponse(response, GetAlarmJSON(Alarm).dump(), "aplication/json", HTTP_OK);
    pAlarmsLock.Unlock();
    return;
  }
  pAlarmsLock.Unlock();
  HTTPServer::FillResponse(response, "AlarmID not found", "text/plain", HTTP_BAD_REQUEST);
}
void AnalyzerSoft::SetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned() || !Params["Name"].is_string() || !Params["Priority"].is_number_unsigned() || !Params["Expression"].is_string() || !Params["Aggreg"].is_string() || !Params["Message"].is_string() || !Params["Samples"].is_number_unsigned() || !Params["Window"].is_number_unsigned()) return req.FromString("'Analyzer', 'AlarmID', 'Name', 'Priority', 'Expression', 'Aggreg', 'Message', 'Samples' and 'Window' params are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->SetAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
//  pMembersLocks.ReadLock();
//  if(pAlarms != NULL) {
//    pAlarms->SetAlarm(params, response);
//  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
//  }
//  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::AddAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Alarm_t Alarm;
  InitAlarm(Alarm, CheckAlarmConfig(params));
  pAlarmsLock.WriteLock();
  Alarm.AlarmID = pAlarmsID++;
  pAlarms.push_back(Alarm);
  SaveAlarmsConf();
  pAlarmsLock.Unlock();
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerSoft::DelAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t AlarmID = params["AlarmId"];
  pAlarmsLock.WriteLock();
  for(auto it = pAlarms.begin(); it != pAlarms.end(); it++) {
    if(it->AlarmID != AlarmID) continue;
    pAlarms.erase(it);
    SaveAlarmsConf();
    pAlarmsLock.Unlock();
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
  }
  pAlarmsLock.Unlock();
  HTTPServer::FillResponse(response, "AlarmID not found", "text/plain", HTTP_BAD_REQUEST);
}
