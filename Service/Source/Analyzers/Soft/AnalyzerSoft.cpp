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
#include "Telegram.h"
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
//----------------------------------------------------------------------------------------------------------------------
constexpr float   AnalyzerSoft::FrequencyRanges[];
const     uint8_t AnalyzerSoft::FrequencyRangesLen;
constexpr float   AnalyzerSoft::VoltageRanges[];
const     uint8_t AnalyzerSoft::VoltageRangesLen;
constexpr float   AnalyzerSoft::THDRanges[];
const     uint8_t AnalyzerSoft::THDRangesLen;
constexpr float   AnalyzerSoft::HarmonicsAllowed[];
const     uint8_t AnalyzerSoft::HarmonicsAllowedLen;
constexpr float   AnalyzerSoft::UnbalanceRanges[];
const     uint8_t AnalyzerSoft::UnbalanceRangesLen;
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t     AnalyzerSoft::pFFTMutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t            AnalyzerSoft::pFFTRunning = 0;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3), pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pMode(mode) {
  LOGFile::Debug("Creating AnalyzerSoft database tables\n");
  Database DB(id);
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  LOGFile::Debug("AnalyzerSoft database tables created\n");
  pDelayVoltage     = NULL;
  pDelayCurrent     = NULL;
  pVoltageSamples   = NULL;
  pCurrentSamples   = NULL;
  pVoltageFFT       = NULL;
  pCurrentFFT       = NULL;
  pAlarmsWorker     = NULL;
  pEventsWorker     = NULL;
  pFreqCycles       = NULL;
  pRateList         = NULL;
  pPrevSamples      = NULL;
  pPrevCycles       = NULL;
  pSyncThread       = 0;
  pMQTTThread       = 0;
  pMQTTSendBuff     = NULL;
  pMQTTRecvBuff     = NULL;
  pMQTTBuffSize     = 0x00020000;
  InitAggreg(pAggreg200MS, AGGREG_200MS);
  InitAggreg(pAggreg3S, AGGREG_3S);
  InitAggreg(pAggreg1M, AGGREG_1M);
  InitAggreg(pAggreg10M, AGGREG_10M);
  InitAggreg(pAggreg15M, AGGREG_15M);
  InitAggreg(pAggreg1H, AGGREG_1H);
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFTWorker[i] = new Worker("FFT_V" + to_string(i), [&](void *a) { this->FFT_Func((FFTChannel_t*)a); });
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFTWorker[i] = new Worker("FFT_I" + to_string(i), [&](void *a) { this->FFT_Func((FFTChannel_t*)a); });
  string Error;
  Configure(CheckConfig(ConfigManager::ReadConfig(optionPath), mode, &Error));
  if(Error.empty() == false) LOGFile::Error("Analyzer '%s' rates failed to parse (%s)\n", Serial.c_str(), Error.c_str());
  pAnalyzerWorker = new Worker((pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N", [&](void *a) { this->Analyzer_Func(); });
}
AnalyzerSoft::~AnalyzerSoft() {
  pSyncTerminate = true;
  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
  pMQTTTerminate = true;
  if(pMQTTThread != 0) pthread_join(pMQTTThread, NULL);
  if(pAnalyzerWorker != NULL) delete pAnalyzerWorker;
  if(pEventsWorker != NULL) delete pEventsWorker;
  if(pAlarmsWorker != NULL) delete pAlarmsWorker;
  if(pFreqCycles != NULL) free(pFreqCycles);
  if(pDelayVoltage != NULL) free(pDelayVoltage);
  if(pDelayCurrent != NULL) free(pDelayCurrent);
  if(pPrevSamples != NULL) free(pPrevSamples);
  if(pPrevCycles != NULL) free(pPrevCycles);
  if(pVoltageSamples != NULL) free(pVoltageSamples);
  if(pCurrentSamples != NULL) free(pCurrentSamples);
  if(pVoltageFFT != NULL) free(pVoltageFFT);
  if(pCurrentFFT != NULL) free(pCurrentFFT);
  pAnalyzerLock.WriteLock();
  if(pRateList != NULL) delete pRateList;
  pRateList = NULL;
  pAnalyzerLock.Unlock();
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
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples, const uint32_t buffUsage) {
  pAnalyzerLock.ReadLock();
  if(pBlockSamples == 0) {
    pAggreg200MS.Timestamp = Tools::GetTime64();
    pAggreg200MS.Timestamp -= (uint64_t)((buffUsage * 1000) / pSamplerate);
  }
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
    pAnalyzerWorker->Run(NULL);
    pAnalyzerLock.Unlock();
    return true;
  }
  pAnalyzerLock.Unlock();
  return false;
}
bool AnalyzerSoft::IsWorking() {
  return pAnalyzerWorker->IsWorking();
}
json AnalyzerSoft::CheckConfig(const json &config, const Analyzer::AnalyzerMode mode, string *error) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed analyzer";
  ConfigOUT["NominalVoltage"] = 230.0;
  ConfigOUT["NominalFrequency"] = 50.0;
  ConfigOUT["MaxCurrent"] = 35.0;
  ConfigOUT["MinCurrent"] = 0.1;
  if(mode != Analyzer::PHASE1) {
    ConfigOUT["PhaseNames"] = json::array();
    ConfigOUT["PhaseNames"].push_back("R");
    ConfigOUT["PhaseNames"].push_back("S");
    ConfigOUT["PhaseNames"].push_back("T");
    if(mode == Analyzer::PHASE3N) ConfigOUT["PhaseNames"].push_back("N");
  }
  ConfigOUT["TimeZone"] = "UTC";
  ConfigOUT["Tags"] = json::array();
  ConfigOUT["SyncEnabled"] = false;
  ConfigOUT["Sync"] = json::object();
  ConfigOUT["Sync"]["Server"] = "";
  ConfigOUT["Sync"]["User"] = "";
  ConfigOUT["Sync"]["Pass"] = "";
  ConfigOUT["MQTTEnabled"] = false;
  ConfigOUT["MQTT"] = json::object();
  ConfigOUT["MQTT"]["Prefix"] = "";
  ConfigOUT["MQTT"]["Server"] = "";
  ConfigOUT["MQTT"]["User"] = "";
  ConfigOUT["MQTT"]["Pass"] = "";
  ConfigOUT["MQTT"]["Export3S"] = false;
  ConfigOUT["MQTT"]["Export1M"] = false;
  ConfigOUT["MQTT"]["Export10M"] = false;
  ConfigOUT["MQTT"]["Export15M"] = false;
  ConfigOUT["MQTT"]["Export1H"] = false;
  ConfigOUT["MQTT"]["ExportAlarms"] = false;
  ConfigOUT["MQTT"]["ExportConfig"] = false;
  ConfigOUT["Save200MS"] = false;
  ConfigOUT["EventsEnabled"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
  ConfigOUT["Alarms"] = json::array();
  ConfigOUT["Rates"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFrequency") && config["NominalFrequency"].is_number() && (config["NominalFrequency"] >= 0) && (config["NominalFrequency"] <= 400)) ConfigOUT["NominalFrequency"] = config["NominalFrequency"];
  if(config.contains("MaxCurrent") && config["MaxCurrent"].is_number() && (config["MaxCurrent"] >= 1) && (config["MaxCurrent"] <= 100000)) ConfigOUT["MaxCurrent"] = config["MaxCurrent"];
  if(config.contains("MinCurrent") && config["MinCurrent"].is_number() && (config["MinCurrent"] >= 0.01) && (config["MinCurrent"] <= 100)) ConfigOUT["MinCurrent"] = config["MinCurrent"];
  if((mode != Analyzer::PHASE1) && config.contains("PhaseNames") && config["PhaseNames"].is_array() && (config["PhaseNames"].size() == ConfigOUT["PhaseNames"].size())) {
    for(uint8_t n = 0; n < ConfigOUT["PhaseNames"].size(); n++) {
      if(config["PhaseNames"][n].is_string() && (config["PhaseNames"][n].size() < 3)) ConfigOUT["PhaseNames"][n] = config["PhaseNames"][n];
    }
  }
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
  if(config.contains("SyncEnabled") && config["SyncEnabled"].is_boolean()) ConfigOUT["SyncEnabled"] = config["SyncEnabled"];
  if(config.contains("Sync") && config["Sync"].is_object()) {
    if(config["Sync"].contains("Server") && config["Sync"]["Server"].is_string()) ConfigOUT["Sync"]["Server"] = config["Sync"]["Server"];
    if(config["Sync"].contains("User") && config["Sync"]["User"].is_string()) ConfigOUT["Sync"]["User"] = config["Sync"]["User"];
    if(config["Sync"].contains("Pass") && config["Sync"]["Pass"].is_string()) ConfigOUT["Sync"]["Pass"] = config["Sync"]["Pass"];
  }
  if(config.contains("MQTTEnabled") && config["MQTTEnabled"].is_boolean()) ConfigOUT["MQTTEnabled"] = config["MQTTEnabled"];
  if(config.contains("MQTT") && config["MQTT"].is_object()) {
    if(config["MQTT"].contains("Prefix") && config["MQTT"]["Prefix"].is_string()) ConfigOUT["MQTT"]["Prefix"] = config["MQTT"]["Prefix"];
    if(config["MQTT"].contains("Server") && config["MQTT"]["Server"].is_string()) ConfigOUT["MQTT"]["Server"] = config["MQTT"]["Server"];
    if(config["MQTT"].contains("User") && config["MQTT"]["User"].is_string()) ConfigOUT["MQTT"]["User"] = config["MQTT"]["User"];
    if(config["MQTT"].contains("Pass") && config["MQTT"]["Pass"].is_string()) ConfigOUT["MQTT"]["Pass"] = config["MQTT"]["Pass"];
    if(config["MQTT"].contains("Export3S") && config["MQTT"]["Export3S"].is_boolean()) ConfigOUT["MQTT"]["Export3S"] = config["MQTT"]["Export3S"];
    if(config["MQTT"].contains("Export1M") && config["MQTT"]["Export1M"].is_boolean()) ConfigOUT["MQTT"]["Export1M"] = config["MQTT"]["Export1M"];
    if(config["MQTT"].contains("Export10M") && config["MQTT"]["Export10M"].is_boolean()) ConfigOUT["MQTT"]["Export10M"] = config["MQTT"]["Export10M"];
    if(config["MQTT"].contains("Export15M") && config["MQTT"]["Export15M"].is_boolean()) ConfigOUT["MQTT"]["Export15M"] = config["MQTT"]["Export15M"];
    if(config["MQTT"].contains("Export1H") && config["MQTT"]["Export1H"].is_boolean()) ConfigOUT["MQTT"]["Export1H"] = config["MQTT"]["Export1H"];
    if(config["MQTT"].contains("ExportAlarms") && config["MQTT"]["ExportAlarms"].is_boolean()) ConfigOUT["MQTT"]["ExportAlarms"] = config["MQTT"]["ExportAlarms"];
    if(config["MQTT"].contains("ExportConfig") && config["MQTT"]["ExportConfig"].is_boolean()) ConfigOUT["MQTT"]["ExportConfig"] = config["MQTT"]["ExportConfig"];
  }
  if(config.contains("Save200MS") && config["Save200MS"].is_boolean()) ConfigOUT["Save200MS"] = config["Save200MS"];
  if(config.contains("EventsEnabled") && config["EventsEnabled"].is_boolean()) ConfigOUT["EventsEnabled"] = config["EventsEnabled"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
  if(config.contains("Alarms") && config["Alarms"].is_array()) {
    for(auto configAlarm : config["Alarms"]) {
      if((configAlarm.contains("Name") == false) || (configAlarm["Name"].is_string() == false)) continue;
      if((configAlarm.contains("Priority") == false) || (configAlarm["Priority"].is_number_unsigned() == false) || (configAlarm["Priority"] > 2)) continue;
      if((configAlarm.contains("Expression") == false) || (configAlarm["Expression"].is_string() == false)) continue;
      if((configAlarm.contains("Aggreg") == false) || (configAlarm["Aggreg"].is_string() == false) || ((configAlarm["Aggreg"] != "200MS") && (configAlarm["Aggreg"] != "3S") && (configAlarm["Aggreg"] != "1M") && (configAlarm["Aggreg"] != "10M") && (configAlarm["Aggreg"] != "15M") && (configAlarm["Aggreg"] != "1H"))) continue;
      if((configAlarm.contains("Samples") == false) || (configAlarm["Samples"].is_number_unsigned() == false) || (configAlarm["Samples"] > 1000)) continue;
      if((configAlarm.contains("Window") == false) || (configAlarm["Window"].is_number_unsigned() == false)) continue;
      if(configAlarm["Window"] < configAlarm["Samples"]) continue;
      ConfigOUT["Alarms"].push_back(configAlarm);
    }
  }
  if(config.contains("Rates") && config["Rates"].is_array()) {
    json Tmp = Rates::CheckConfig(config["Rates"]);
    if(Tmp.is_string()) {
      if(error != NULL) *error = Tmp;
    } else {
      ConfigOUT["Rates"] = Tmp;
    }
  }
  return ConfigOUT;
}
void AnalyzerSoft::Configure(const json &options) {
  ConfigManager::WriteConfig(pConfigPath, options);
  SetName(options["Name"]);
  pAnalyzerLock.WriteLock();
  pNominalVoltage   = options["NominalVoltage"];
  pMaxCurrent       = options["MaxCurrent"];
  pMinCurrent       = options["MinCurrent"];
  pSave200MS        = options["Save200MS"];
  load_time_zone(options["TimeZone"], &pTimeZone);
  if(pNominalFrequency != options["NominalFrequency"]) {
    pNominalFrequency = options["NominalFrequency"];
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
    pMaxFFTSamples    = (pMaxSamples / 2) + 1;
    pFreqCyclesPos    = 0;
    pSequence         = 0;
    pBlockSamples     = 0;
    pBlockCyclesCount = 0;
    InitAggreg(pAggreg200MS, AGGREG_200MS);
    memset(pBlockCycles, 0, sizeof(pBlockCycles));
    pFreqCycles       = (uint32_t*)realloc(pFreqCycles, pFreqCyclesLen * sizeof(uint32_t));
    pVoltageSamples   = (float*)realloc(pVoltageSamples, sizeof(float) * pVoltageChannels * pMaxSamples);
    pCurrentSamples   = (float*)realloc(pCurrentSamples, sizeof(float) * pCurrentChannels * pMaxSamples);
    pVoltageFFT       = (Complex_t*)realloc(pVoltageFFT, sizeof(Complex_t) * pVoltageChannels * pMaxFFTSamples);
    pCurrentFFT       = (Complex_t*)realloc(pCurrentFFT, sizeof(Complex_t) * pCurrentChannels * pMaxFFTSamples);
    for(uint8_t n = 0; n < pVoltageChannels; n++) pLastSamples[n] = 0.0;
    for(uint8_t i = 0; i < pVoltageChannels; i++) {
      pVoltageFFTChannels[i].OutPtr = &pVoltageFFT[i * pMaxFFTSamples];
      pVoltageFFTChannels[i].InPtr  = &pVoltageSamples[i * pMaxSamples];
    }
    for(uint8_t i = 0; i < pCurrentChannels; i++) {
      pCurrentFFTChannels[i].OutPtr = &pCurrentFFT[i * pMaxFFTSamples];
      pCurrentFFTChannels[i].InPtr  = &pCurrentSamples[i * pMaxSamples];
    }
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
  }
  if(pRateList != NULL) delete pRateList;
  pRateList = new Rates(options["Rates"], pTimeZone);
  bool AlarmsEnabled = options["AlarmsEnabled"];
  bool EventsEnabled = options["EventsEnabled"];
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
    pPrevCyclesLen  = pNominalFrequency * 2;
    pPrevCyclesPos  = 0;
    pPrevCycles     = (EventCycle_t*)calloc(pPrevCyclesLen, sizeof(EventCycle_t));
    pEventsWorker   = new Worker("EventsDet", [&](void *a) { this->Events_Func(); });
  }
  if((AlarmsEnabled == false) && (pAlarmsWorker != NULL)) {
    delete pAlarmsWorker;
    pAlarmsWorker = NULL;
  } else if((AlarmsEnabled == true) && (pAlarmsWorker == NULL)) {
    pAlarmsWorker = new Worker("AlarmsDet", [&](void *a) { this->Alarms_Func((Params_t*)a); });
  }
  for(Alarm_t &Alarm : pAlarms) {
    if(Alarm.Results != NULL) free(Alarm.Results);
  }
  pAlarms.clear();
  if(AlarmsEnabled == true) {
    pAlarms.resize(options["Alarms"].size());
    for(uint32_t n = 0; n < pAlarms.size(); n++) {
      pAlarms[n].Name        = options["Alarms"][n]["Name"];
      pAlarms[n].Priority    = options["Alarms"][n]["Priority"];
      pAlarms[n].Samples     = options["Alarms"][n]["Samples"];
      pAlarms[n].Window      = options["Alarms"][n]["Window"];
      pAlarms[n].ResultPos   = 0;
      pAlarms[n].ResultCount = 0;
      pAlarms[n].Aggreg      = (options["Alarms"][n]["Aggreg"] == "200MS") ? AnalyzerSoft::AGGREG_200MS :
                               (options["Alarms"][n]["Aggreg"] == "3S") ? AnalyzerSoft::AGGREG_3S :
                               (options["Alarms"][n]["Aggreg"] == "1M") ? AnalyzerSoft::AGGREG_1M :
                               (options["Alarms"][n]["Aggreg"] == "10M") ? AnalyzerSoft::AGGREG_10M :
                               (options["Alarms"][n]["Aggreg"] == "15M") ? AnalyzerSoft::AGGREG_15M : AnalyzerSoft::AGGREG_1H;
      pAlarms[n].Results     = (uint8_t*)calloc(ceil(pAlarms[n].Window / 8), 1);
      pAlarms[n].Triggered   = false;
      InitAlarmParser(pAlarms[n].Evaluator, options["Alarms"][n]["Expression"], pAlarms[n].Aggreg);
    }
  }
  bool SyncEnabled = options["SyncEnabled"];
  if(SyncEnabled == true) {
    pSyncServer    = options["Sync"]["Server"];
    pSyncUser      = options["Sync"]["User"];
    pSyncPass      = options["Sync"]["Pass"];
    pSyncTerminate = false;
    if(pSyncThread == 0) {
      if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
        pSyncThread = 0;
      } else {
        pthread_setname_np(pSyncThread, "Sync");
      }
    }
  } else if((SyncEnabled == false) && (pSyncThread != 0)) {
    pSyncTerminate = true;
    pthread_join(pSyncThread, NULL);
    pSyncThread = 0;
  }
  bool MQTTEnabled = options["MQTTEnabled"];
  if(MQTTEnabled == true) {
    pMQTTPrefix    = options["MQTT"]["Prefix"];
    pMQTTServer    = options["MQTT"]["Server"];
    pMQTTUser      = options["MQTT"]["User"];
    pMQTTPass      = options["MQTT"]["Pass"];
    pMQTT3S        = options["MQTT"]["Export3S"];
    pMQTT1M        = options["MQTT"]["Export1M"];
    pMQTT10M       = options["MQTT"]["Export10M"];
    pMQTT15M       = options["MQTT"]["Export15M"];
    pMQTT1H        = options["MQTT"]["Export1H"];
    pMQTTAlarms    = options["MQTT"]["ExportAlarms"];
    pMQTTConfig    = options["MQTT"]["ExportConfig"];
    pMQTTTerminate = false;
    if(pMQTTThread == 0) {
      pMQTTSendBuff = (uint8_t*)calloc(pMQTTBuffSize, 1);
      pMQTTRecvBuff = (uint8_t*)calloc(pMQTTBuffSize, 1);
      mqtt_init_reconnect(&pMQTTClient, NULL, NULL, NULL);
      if(pthread_create(&pMQTTThread, NULL, MQTT_Thread, this) != 0) {
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
        pMQTTThread = 0;
      } else {
        pthread_setname_np(pMQTTThread, "MQTT");
      }
    }
  } else if(MQTTEnabled == false) {
    pMQTT3S        = false;
    pMQTT1M        = false;
    pMQTT10M       = false;
    pMQTT15M       = false;
    pMQTT1H        = false;
    pMQTTAlarms    = false;
    pMQTTConfig    = false;
    if(pMQTTThread != 0) {
      pthread_join(pMQTTThread, NULL);
      free(pMQTTSendBuff);
      free(pMQTTRecvBuff);
      pMQTTThread = 0;
    }
  }
  pAnalyzerLock.Unlock();
  if(pMQTTConfig == true) PublishConfig(options);
}
void AnalyzerSoft::InitAggreg(Params_t &params, AggregType type) {
  memset(&params, 0, sizeof(params));
  params.Type = type;
}
//void AnalyzerSoft::InitEvent(const EventType type, const vector<float> &reference, const uint32_t pos) {
//  Event_t Event;
//  Event.Type = type;
//  Event.Duration = 0;
//  Event.State = EVT_RUNNING;
//  Event.Reference = reference;
//  Event.PeakValue = reference;
//  Event.ChangeValue = reference;
//  Event.SamplesRAW_A = deque<vector<float> >();
//  Event.SamplesRMS_A = deque<vector<float> >();
//  Event.SamplesRAW_B = deque<vector<float> >();
//  Event.SamplesRMS_B = deque<vector<float> >();
//  for(deque<EventCycle_t>::reverse_iterator it = pPrevCycles.rbegin(); it != pPrevCycles.rend(); it++) {
//    Event.SamplesRMS_A.push_front(it->RMS);
//    if(Event.SamplesRMS_A.size() == (pNominalFrequency * 2 * EVENT_RMSENVELOPETIME)) break;
//  }
//  for(deque<vector<float> >::reverse_iterator it = pPrevSamples.rbegin(); it != pPrevSamples.rend(); it++) {
//    Event.SamplesRAW_A.push_front(*it);
//    if(Event.SamplesRAW_A.size() == (EVENT_RAWENVELOPETIME * pSamplerate)) break;
//  }
  //A new event destroy any running RVC
//  if(type != EVT_RVC) {
//    for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); ) {
//      if((it->Type == EVT_RVC) && (it->State == EVT_RUNNING)) {
//        Database DB(Serial);
//        DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(it->Start) + " AND evttype = 'RVC'");
//        pEvents.erase(it++);
//      } else {
//        it++;
//      }
//    }
//  }
//  Event.Start = pAggreg200MS.Timestamp + (uint32_t)((pos * 1000) / pSamplerate);
//  SaveEvent(Event);
//  pEvents.push_back(Event);
//}
void AnalyzerSoft::AccumulateAggreg(Params_t &dst, const Params_t &src) {
  if(src.AggregSamples == 0) return;
  if(dst.Timestamp == 0) dst.Timestamp = src.Timestamp;
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
  if(params.AggregSamples == 0) return;
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
//bool AnalyzerSoft::FindRunningEvent(const uint32_t mask) {
//  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
//    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
//  }
//  return false;
//}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  if(params.AggregSamples == 0) return;
  SetOnline();
  Database DB(Serial);
  if(params.Type == AGGREG_200MS) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance) VALUES(" +
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
    DB.Bind(params.Phi, pVoltageChannels) + ", " +
    DB.Bind(params.Unbalance) + ")");
  } else if(params.Type == AGGREG_3S) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  } else if(params.Type == AGGREG_1M) {
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
  } else if(params.Type == AGGREG_10M) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ")");
  } else if(params.Type == AGGREG_15M) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  } else if(params.Type == AGGREG_1H) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.StatsVoltage, VoltageRangesLen) + ", " +
    DB.Bind(params.StatsFrequency, FrequencyRangesLen) + ", " +
    DB.Bind(params.StatsTHD, THDRangesLen) + ", " +
    DB.Bind(params.StatsHarmonics, 2) + ", " +
    DB.Bind(params.StatsUnbalance, UnbalanceRangesLen) + ")");
  }
}
//void AnalyzerSoft::SaveEvent(Event_t &evt) {
//  Database DB(Serial);
//  string Type = (evt.Type == EVT_RVC) ? "RVC" : (evt.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (evt.Type == EVT_DIP) ? "DIP" : "SWELL";
//  vector<vector<float> > RMS = vector<vector< float> >(evt.Reference.size(), vector<float>());
//  vector<vector<float> > RAW = vector<vector< float> >(evt.Reference.size(), vector<float>());
//  for(deque<vector<float> >::iterator it = evt.SamplesRMS_A.begin(); it != evt.SamplesRMS_A.end(); it++) {
//    for(uint8_t ch = 0; ch < it->size(); ch++) RMS[ch].push_back((*it)[ch]);
//  }
//  for(deque<vector<float> >::iterator it = evt.SamplesRMS_B.begin(); it != evt.SamplesRMS_B.end(); it++) {
//    for(uint8_t ch = 0; ch < it->size(); ch++) RMS[ch].push_back((*it)[ch]);
//  }
//  for(deque<vector<float> >::iterator it = evt.SamplesRAW_A.begin(); it != evt.SamplesRAW_A.end(); it++) {
//    for(uint8_t ch = 0; ch < it->size(); ch++) RAW[ch].push_back((*it)[ch]);
//  }
//  for(deque<vector<float> >::iterator it = evt.SamplesRAW_B.begin(); it != evt.SamplesRAW_B.end(); it++) {
//    for(uint8_t ch = 0; ch < it->size(); ch++) RAW[ch].push_back((*it)[ch]);
//  }
//  if(evt.State == EVT_RUNNING) {
//    DB.ExecSentenceNoResult("INSERT INTO events(starttime, duration, samplerate, finished, evttype, reference, peak, change, notes, samples1, rms1) VALUES (" + DB.Bind(evt.Start) + ", 0, " + DB.Bind(pSamplerate) + ", false, " + DB.Bind(Type) + ", " + DB.Bind(evt.Reference) + ", " + DB.Bind(evt.PeakValue) + ", " + DB.Bind(evt.ChangeValue) + ", '', " + DB.Bind(RAW) + ", " + DB.Bind(RMS) + ")");
//  } else if(evt.State == EVT_CLOSING) {
//    DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", duration = " + DB.Bind(evt.Duration) + ", peak = " + DB.Bind(evt.PeakValue) + ", change = " + DB.Bind(evt.ChangeValue) + ", samples2 = " + DB.Bind(RAW) + ", rms2 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
//  } else {
//    DB.ExecSentenceNoResult("UPDATE events SET finished = TRUE, samples3 = " + DB.Bind(RAW) + ", rms3 = " + DB.Bind(RMS) + " WHERE starttime = " + DB.Bind(evt.Start));
//  }
//  evt.SamplesRAW_A = deque<vector<float> >();
//  evt.SamplesRMS_A = deque<vector<float> >();
//  evt.SamplesRAW_B = deque<vector<float> >();
//  evt.SamplesRMS_B = deque<vector<float> >();
//}
void AnalyzerSoft::PublishAggreg(const Params_t &params, const string &topic) {
  if(pMQTTClient.socketfd == -1) return;
  json Msg = json::object();
  Msg["Sampletime"] = params.Timestamp;
  Msg["Frequency"] = params.Frequency;
  Msg["Flag"] = params.Flag;
  Msg["Unbalance"] = params.Unbalance;
  Msg["Voltage"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage"].push_back(params.Voltage[n]);
  Msg["Current"] = json::array();
  for(uint8_t n = 0; n < pCurrentChannels; n++) Msg["Current"].push_back(params.Current[n]);
  Msg["Active_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Active_Power"].push_back(params.Active_Power[n]);
  Msg["Reactive_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Reactive_Power"].push_back(params.Reactive_Power[n]);
  Msg["Active_Energy"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Active_Energy"].push_back(params.Active_Energy[n]);
  Msg["Reactive_Energy"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Reactive_Energy"].push_back(params.Reactive_Energy[n]);
  Msg["Apparent_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Apparent_Power"].push_back(params.Apparent_Power[n]);
  Msg["Power_Factor"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Power_Factor"].push_back(params.Power_Factor[n]);
  Msg["Voltage_THD"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage_THD"].push_back(params.Voltage_THD[n]);
  Msg["Current_THD"] = json::array();
  for(uint8_t n = 0; n < pCurrentChannels; n++) Msg["Current_THD"].push_back(params.Current_THD[n]);
  Msg["Phi"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Phi"].push_back(params.Phi[n]);
  Msg["Voltage_Phase"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage_Phase"].push_back(params.Voltage_Phase[n]);
  string TextMsg = Msg.dump();
  string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + Serial + "/" + topic : Serial + "/" + topic;
  mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
  LOGFile::Debug("Analyzer '%s' publish message to MQTT broker\n", Serial.c_str());
}
void AnalyzerSoft::PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered) {
  if(pMQTTClient.socketfd == -1) return;
  json Msg = json::object();
  Msg["AlarmTime"] = time;
  Msg["Name"] = name;
  Msg["Priority"] = prio;
  Msg["Triggered"] = triggered;
  string TextMsg = Msg.dump();
  string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + Serial + "/Alarms" : Serial + "/Alarms";
  mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1);
  LOGFile::Debug("Analyzer '%s' publish message to MQTT broker\n", Serial.c_str());
}
void AnalyzerSoft::PublishConfig(const json &options) {
  if(pMQTTClient.socketfd != -1) {
    string TextMsg = options.dump();
    string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + Serial + "/Config" : Serial + "/Config";
    mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
    LOGFile::Debug("Analyzer '%s' publish message to MQTT broker\n", Serial.c_str());
  }
}
float AnalyzerSoft::CalcChange(const float in, const float reference) {
  return ((in - reference) / reference) * 100;
}
float AnalyzerSoft::CalcAngle(const Complex_t &a, const Complex_t &b) {
  float Tmp;
  if((a.Re == 0.0) && (a.Im == 0.0))
    Tmp = 0.0;
  else if(a.Re < 0.0)
    Tmp = atan(a.Im / a.Re);
  else
    Tmp = atan(a.Im / a.Re) + M_PI;
  if((b.Re == 0.0) && (b.Im == 0.0))
    Tmp = Tmp;
  else if(b.Re < 0.0)
    Tmp = Tmp - atan(b.Im / b.Re);
  else
    Tmp = Tmp - atan(b.Im / b.Re) + M_PI;
  Tmp = fmod(Tmp, 2.0 * M_PI);
  if(Tmp > M_PI) Tmp = Tmp -  2.0 * M_PI;
  return Tmp;
}
float AnalyzerSoft::BinPower(const Complex_t &a) {
  return sqrt(pow(a.Re, 2.0) + pow(a.Im, 2.0)) / M_SQRT2 * 2.0;
}
void AnalyzerSoft::InitAlarmParser(Parser &parser, const string &expr, const AggregType &type) {
  auto AppendArray = [&](const string &serie, float *data, const uint8_t len) {
    if(len == 1) {
      parser.DefineVar(serie, &data[0]);
    } else {
      for(uint8_t n = 0; n < len; n++) parser.DefineVar(serie + "_" + to_string(n + 1), &data[n]);
    }
  };
  Params_t *Params = NULL;
  if(type == AGGREG_200MS) Params = &pAggreg200MS;
  if(type == AGGREG_3S)    Params = &pAggreg3S;
  if(type == AGGREG_1M)    Params = &pAggreg1M;
  if(type == AGGREG_10M)   Params = &pAggreg10M;
  if(type == AGGREG_15M)   Params = &pAggreg15M;
  if(type == AGGREG_1H)    Params = &pAggreg1H;
  parser.DefineVar("Frequency", &Params->Frequency);
  parser.DefineVar("Nominal_Voltage", &pNominalVoltage);
  parser.DefineVar("Nominal_Frequency", &pNominalFrequency);
  if(pVoltageChannels > 1) parser.DefineVar("Unbalance", &Params->Unbalance);
  AppendArray("Voltage", Params->Voltage, pVoltageChannels);
  AppendArray("Current", Params->Current, pCurrentChannels);
  AppendArray("ActivePower", Params->Active_Power, pVoltageChannels);
  AppendArray("ReactivePower", Params->Reactive_Power, pVoltageChannels);
  AppendArray("ApparentPower", Params->Apparent_Power, pVoltageChannels);
  AppendArray("PowerFactor", Params->Power_Factor, pVoltageChannels);
  AppendArray("VoltageTHD", Params->Voltage_THD, pVoltageChannels);
  AppendArray("CurrentTHD", Params->Current_THD, pCurrentChannels);
  if(pVoltageChannels > 1) AppendArray("VoltagePhase", Params->Voltage_Phase, pVoltageChannels);
  AppendArray("Phi", Params->Phi, pVoltageChannels);
  try {
    parser.SetExpr(expr);
  } catch(Parser::exception_type &e) {
    LOGFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", Serial.c_str());
  }
}
void AnalyzerSoft::Analyzer_Func() {
  pAnalyzerLock.ReadLock();
  uint64_t Start = Tools::GetTime64();
  pAggreg200MS.AggregSamples = 1;
  if(pEventsWorker != NULL) pEventsWorker->Run(NULL);
  pFFTSamples = (pBlockSamples / 2) + 1;
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFTWorker[i]->Run(&pVoltageFFTChannels[i]);
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFTWorker[i]->Run(&pCurrentFFTChannels[i]);
  if(pNominalFrequency > 0) {
    for(uint8_t n = 0; n < pBlockCyclesCount; n++) {
      pFreqCycles[pFreqCyclesPos] = pBlockCycles[n];
      pFreqCyclesPos = (pFreqCyclesPos + 1) % pFreqCyclesLen;
    }
    uint32_t Samples  = 0;
    uint16_t Cycles   = 0;
    uint16_t MinCycle = pSamplerate / (pNominalFrequency * 1.15 * 2.0);
    uint16_t MaxCycle = pSamplerate / (pNominalFrequency * 0.85 * 2.0);
    for(uint16_t i = 0; i < pFreqCyclesLen; i++) {
      if((pFreqCycles[i] < MinCycle) && (pFreqCycles[i] > MaxCycle)) break;
      Samples += pFreqCycles[i];
      Cycles++;
    }
    pAggreg200MS.Frequency = (Cycles == pFreqCyclesLen) ? pFreqCyclesLen / ((2.0 * Samples) / pSamplerate) : 0;
  }
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    float *ChV = &pVoltageSamples[j * pMaxSamples];
    float *ChC = &pCurrentSamples[j * pMaxSamples];
    for(uint16_t i = 0; i < pBlockSamples; i++) {
      pAggreg200MS.Voltage[j] += pow(ChV[i], 2.0);
      pAggreg200MS.Active_Power[j] += ChV[i] * ChC[i];
    }
    pAggreg200MS.Voltage[j] = sqrt(pAggreg200MS.Voltage[j] / pBlockSamples);
    pAggreg200MS.Active_Power[j] = pAggreg200MS.Active_Power[j] / pBlockSamples;
  }
  for(uint8_t j = 0; j < pCurrentChannels; j++) {
    float *Ch = &pCurrentSamples[j * pMaxSamples];
    for(uint16_t i = 0; i < pBlockSamples; i++) pAggreg200MS.Current[j] += pow(Ch[i], 2.0);
    pAggreg200MS.Current[j] = sqrt(pAggreg200MS.Current[j] / pBlockSamples);
  }
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    pVoltageFFTWorker[j]->Wait();
    pCurrentFFTWorker[j]->Wait();
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS.Voltage_Harmonics[j * 50 + i] = BinPower(pVoltageFFT[j * pMaxFFTSamples + (10 + i * 10)]);
      pAggreg200MS.Current_Harmonics[j * 50 + i] = BinPower(pCurrentFFT[j * pMaxFFTSamples + (10 + i * 10)]);
      pAggreg200MS.Power_Harmonics[j * 50 + i]   = pAggreg200MS.Current_Harmonics[j * 50 + i] * pAggreg200MS.Voltage_Harmonics[j * 50 + i] * cos(CalcAngle(pCurrentFFT[j * pMaxFFTSamples + (10 + i * 10)], pVoltageFFT[j * pMaxFFTSamples + (10 + i * 10)]));
      if(i > 0) pAggreg200MS.Voltage_THD[j] += pAggreg200MS.Voltage_Harmonics[j * 50 + i];
      if(i > 0) pAggreg200MS.Current_THD[j] += pAggreg200MS.Current_Harmonics[j * 50 + i];
    }
    pAggreg200MS.Voltage_Phase[j] = (j == 0) ? 0.0 : CalcAngle(pVoltageFFT[j * pMaxFFTSamples + 10], pVoltageFFT[10]) * 180.0 / M_PI;
    pAggreg200MS.Voltage_THD[j] = (pAggreg200MS.Voltage_THD[j] / pAggreg200MS.Voltage_Harmonics[j * 50]) * 100.0;
    pAggreg200MS.Apparent_Power[j] = pAggreg200MS.Voltage[j] * pAggreg200MS.Current[j];
    float PHI = CalcAngle(pCurrentFFT[j * pMaxFFTSamples + 10], pVoltageFFT[j * pMaxFFTSamples + 10]);
    pAggreg200MS.Reactive_Power[j] = pAggreg200MS.Voltage_Harmonics[j * 50] * pAggreg200MS.Current_Harmonics[j * 50] * sin(-PHI);
    pAggreg200MS.Phi[j] = PHI * 180.0 / M_PI;
    pAggreg200MS.Current_THD[j] = (pAggreg200MS.Current_THD[j] / pAggreg200MS.Current_Harmonics[j * 50]) * 100.0;
    pAggreg200MS.Active_Energy[j] = (pAggreg200MS.Active_Power[j] * pBlockSamples) / (pSamplerate * 3600.0);
    pAggreg200MS.Power_Factor[j] = pAggreg200MS.Active_Power[j] / pAggreg200MS.Apparent_Power[j];
    pAggreg200MS.Reactive_Energy[j] = (pAggreg200MS.Reactive_Power[j] * pBlockSamples) / (pSamplerate * 3600.0);
  }
  if(pMode != PHASE1) {
    complex<float> A(cos(120.0 * M_PI / 180.0), sin(120.0 * M_PI / 180.0));
    complex<float> V1(pVoltageFFT[0 * pMaxFFTSamples + 10].Re * M_SQRT2, pVoltageFFT[0 * pMaxFFTSamples + 10].Im * M_SQRT2);
    complex<float> V2(pVoltageFFT[1 * pMaxFFTSamples + 10].Re * M_SQRT2, pVoltageFFT[1 * pMaxFFTSamples + 10].Im * M_SQRT2);
    complex<float> V3(pVoltageFFT[2 * pMaxFFTSamples + 10].Re * M_SQRT2, pVoltageFFT[2 * pMaxFFTSamples + 10].Im * M_SQRT2);
    pAggreg200MS.Unbalance = ((abs(V1 + A * A * V2 + A * V3) / 3.0) / (abs(V1 + A * V2 + A * A * V3) / 3.0)) * 100.0;
  }
  if(pMode == PHASE3N) {
    pCurrentFFTWorker[3]->Wait();
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS.Current_Harmonics[3 * 50 + i] = BinPower(pCurrentFFT[3 * pFFTSamples + (10 + i * 10)]);
      if(i > 0) pAggreg200MS.Current_THD[3] += pAggreg200MS.Current_Harmonics[3 * 50 + i];
    }
    pAggreg200MS.Current_THD[3] = (pAggreg200MS.Current_THD[3] / pAggreg200MS.Current_Harmonics[3 * 50]) * 100.0;
  }
  if(pEventsWorker != NULL) pEventsWorker->Wait();
  if(pSequence < 100) {
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "\n", Serial.c_str(), Tools::GetTime64() - Start, pSequence);
    InitAggreg(pAggreg200MS, AGGREG_200MS);
    pBlockSamples     = 0;
    pBlockCyclesCount = 0;
    memset(pBlockCycles, 0, sizeof(pBlockCycles));
    pSequence++;
    pAnalyzerLock.Unlock();
    return;
  }
  pNowDataLock.ProviderWrite();
  if((pSequence % 15) == 0) {
    LOGFile::Debug("Storing 3S aggregation for analyzer '%s'\n", Serial.c_str());
    NormalizeAggreg(pAggreg3S);
    if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg3S);
    if(pAggreg3S.Flag == false) {
      float Variation = CalcChange(pAggreg3S.Frequency, pNominalFrequency);
      for(uint8_t Range = 0; Range < FrequencyRangesLen; Range++) {
        if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) pAggreg1H.StatsFrequency[Range]++;
      }
    }
    SaveAggreg(pAggreg3S);
    if(pMQTT3S) PublishAggreg(pAggreg3S, "3S");
    if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();
    InitAggreg(pAggreg3S, AGGREG_3S);
  }
  AccumulateAggreg(pAggreg3S, pAggreg200MS);

  if((pAggreg1M.Timestamp != 0) && ((pAggreg1M.Timestamp / 60000) < (pAggreg200MS.Timestamp / 60000))) {
    LOGFile::Debug("Storing 1M aggregation for analyzer '%s'\n", Serial.c_str());
    pAggreg1M.Timestamp = pAggreg1M.Timestamp / 60000 * 60000 + 59999;
    NormalizeAggreg(pAggreg1M);
    if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg1M);
    SaveAggreg(pAggreg1M);
    if(pMQTT1M) PublishAggreg(pAggreg1M, "1M");
    if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();
    InitAggreg(pAggreg1M, AGGREG_1M);
  }
  AccumulateAggreg(pAggreg1M, pAggreg200MS);

  if((pAggreg10M.Timestamp != 0) && ((pAggreg10M.Timestamp / 600000) < (pAggreg200MS.Timestamp / 600000))) {
    LOGFile::Debug("Storing 10M aggregation for analyzer '%s\n", Serial.c_str());
    pAggreg10M.Timestamp = pAggreg10M.Timestamp / 600000 * 600000 + 599999;
    NormalizeAggreg(pAggreg10M);
    if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg10M);
    if(pAggreg10M.Flag == false) {
      for(uint8_t Range = 0; Range < UnbalanceRangesLen; Range++) {
        if((pAggreg10M.Unbalance >= UnbalanceRanges[Range]) && (pAggreg10M.Unbalance < UnbalanceRanges[Range + 1])) pAggreg1H.StatsUnbalance[Range]++;
      }
      for(uint8_t i = 0; i < pVoltageChannels; i++) {
        float Variation = CalcChange(pAggreg10M.Voltage[i], pNominalVoltage);
        for(uint8_t Range = 0; Range < VoltageRangesLen; Range++) {
          if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) pAggreg1H.StatsVoltage[Range]++;
        }
        for(uint8_t Range = 0; Range < THDRangesLen; Range++) {
          if((pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) pAggreg1H.StatsTHD[Range]++;
        }
        bool HarmonicsInRange = true;
        for(uint8_t n = 0; n < HarmonicsAllowedLen; n++) {
          float Variation = CalcChange(pAggreg10M.Voltage_Harmonics[i * 50 + n + 1], pNominalVoltage);
            if(Variation > HarmonicsAllowed[n]) {
              HarmonicsInRange = false;
              break;
            }
        }
        if(HarmonicsInRange == true) {
          pAggreg1H.StatsHarmonics[0]++;
        } else {
          pAggreg1H.StatsHarmonics[1]++;
        }
      }
    }
    SaveAggreg(pAggreg10M);
    if(pMQTT10M) PublishAggreg(pAggreg10M, "10M");
    if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();
    InitAggreg(pAggreg10M, AGGREG_10M);
  }
  AccumulateAggreg(pAggreg10M, pAggreg200MS);

  if((pAggreg15M.Timestamp != 0) && ((pAggreg15M.Timestamp / 900000) < (pAggreg200MS.Timestamp / 900000))) {
    LOGFile::Debug("Storing 15M aggregation for analyzer '%s'\n", Serial.c_str());
    pAggreg15M.Timestamp = pAggreg15M.Timestamp / 900000 * 900000 + 899999;
    NormalizeAggreg(pAggreg15M);
    if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg15M);
    SaveAggreg(pAggreg15M);
    if(pMQTT15M) PublishAggreg(pAggreg15M, "15M");
    if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();
    InitAggreg(pAggreg15M, AGGREG_15M);
  }
  AccumulateAggreg(pAggreg15M, pAggreg200MS);

  if((pAggreg1H.Timestamp != 0) && ((pAggreg1H.Timestamp / 3600000) < (pAggreg200MS.Timestamp / 3600000))) {
    LOGFile::Debug("Storing 1H aggregation for analyzer '%s'\n", Serial.c_str());
    pAggreg1H.Timestamp = pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
    NormalizeAggreg(pAggreg1H);
    if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg1H);
    SaveAggreg(pAggreg1H);
    if(pMQTT1H) PublishAggreg(pAggreg1H, "1H");
    if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();
    InitAggreg(pAggreg1H, AGGREG_1H);
  }
  AccumulateAggreg(pAggreg1H, pAggreg200MS);

  if(pAlarmsWorker != NULL) pAlarmsWorker->Run(&pAggreg200MS);
  if(pSave200MS) {
    LOGFile::Debug("Storing 200MS aggregation for analyzer '%s'\n", Serial.c_str());
    SaveAggreg(pAggreg200MS);
  }
  if(pAlarmsWorker != NULL) pAlarmsWorker->Wait();

  //  SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
  //  SysStatus->Lock.WriteLock();
  //  SysStatus->Events = This->pAggreg200MS.Flag;
  //  SysStatus->Lock.Unlock();
  pNowDataLock.ProviderWait();
  LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (Sampletime: %" PRIu64 ")\n", Serial.c_str(), Tools::GetTime64() - Start, pSequence, pAggreg200MS.Timestamp);
  InitAggreg(pAggreg200MS, AGGREG_200MS);
  pBlockSamples     = 0;
  pBlockCyclesCount = 0;
  memset(pBlockCycles, 0, sizeof(pBlockCycles));
  pSequence++;
  pAnalyzerLock.Unlock();
}
void AnalyzerSoft::Events_Func() {
  uint32_t Pos = 0;
  for(uint8_t i = 0; i < pBlockCyclesCount; i++) {
    float         Tolerance     = EVENT_RVC - ((pPrevStationary == true) ? 0.0 : EVENT_HYSTERESIS);
    EventCycle_t *Cycle         = &pPrevCycles[pPrevCyclesPos];
    uint16_t      CycleLen      = pBlockCycles[i];
    bool          Stationary    = true;
    float         RMS_Avg[3];
    for(uint8_t n = 0; n < pVoltageChannels; n++) {
      Cycle->RMS[n] = 0.0;
      RMS_Avg[n]    = 0.0;
      float *Ptr    = &pVoltageSamples[n * pMaxSamples + Pos];
      for(uint16_t j = 0; j < CycleLen; j++) Cycle->RMS[n] += pow(Ptr[j], 2);
      Cycle->RMS[n] = sqrt(Cycle->RMS[n] / CycleLen);
      for(uint16_t j = 0; j < pPrevCyclesLen; j++) RMS_Avg[n] += pPrevCycles[j].RMS[n];
      RMS_Avg[n] = RMS_Avg[n] / pPrevCyclesLen;
    }
    for(uint8_t n = 0; (n < pVoltageChannels) && (Stationary == true); n++) {
      for(int j = 0; j < pPrevCyclesLen; j++) {
        if(fabs(CalcChange(pPrevCycles[j].RMS[n], RMS_Avg[n])) > Tolerance) Stationary = false;
      }
    }
    if(pEvent.Type != EVT_NONE) {
      if(pEvent.CyclesDuration == (pPrevCyclesLen - 1)) {
        Database DB(Serial);
        float *CyclesValues = (float*)malloc(sizeof(float) * pVoltageChannels * (pPrevCyclesLen - 1));
        for(uint16_t n = 0; n < (pPrevCyclesLen - 1); n++) {
          for(uint8_t i = 0; i < pVoltageChannels; i++) CyclesValues[i * (pPrevCyclesLen - 1) + n] = pPrevCycles[(pPrevCyclesPos + 1 + n) % pPrevCyclesLen].RMS[i];
        }
        DB.ExecSentenceNoResult("UPDATE events_ext SET samples2 = " + DB.Bind(CyclesValues, pVoltageChannels, pPrevCyclesLen - 1) + " WHERE starttime = " + DB.Bind(pEvent.Start) + " AND sampletype = 'RMS'");
        free(CyclesValues);
      }
      bool IsRunning = false;
      if(pEvent.Type == EVT_INTERRUPTION) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(Cycle->RMS[n], pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS));
      } else if(pEvent.Type == EVT_DIP) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(Cycle->RMS[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
      } else if(pEvent.Type == EVT_SWELL) {
        for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (CalcChange(Cycle->RMS[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
      } else if(pEvent.Type == EVT_RVC) {
        if(Stationary == true) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.ChangeValue[n] = RMS_Avg[n];
        } else {
          IsRunning = true;
        }
      }
      if(IsRunning == false) {



        string Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
        LOGFile::Debug("Event '%s' finished on analyzer '%s'\n", Type.c_str(), Serial.c_str());
        Database DB(Serial);
        DB.ExecSentenceNoResult("UPDATE events SET evttype = " + DB.Bind(Type) + ", reference = " + DB.Bind(pEvent.Reference, pVoltageChannels) + ", peak = " + DB.Bind(pEvent.PeakValue, pVoltageChannels) + ", change = " + DB.Bind(pEvent.ChangeValue, pVoltageChannels) + ", duration = " + DB.Bind(pEvent.SamplesDuration) + " WHERE starttime = " + DB.Bind(pEvent.Start));

//        float *CyclesValues = (float*)malloc(sizeof(float) * pVoltageChannels * (pPrevCyclesLen - 1));
//        for(uint16_t n = 0; n < (pPrevCyclesLen - 1); n++) {
//          for(uint8_t i = 0; i < pVoltageChannels; i++) CyclesValues[i * (pPrevCyclesLen - 1) + n] = pPrevCycles[(pPrevCyclesPos + 1 + n) % pPrevCyclesLen].RMS[i];
//        }
//        DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RMS', " + DB.Bind(CyclesValues, pVoltageChannels, pPrevCyclesLen - 1) + ")");
//        free(CyclesValues);

        //Como se guarda un evento?
        pEvents.push_back(pEvent);
        memset(&pEvent, 0, sizeof(pEvent));
      }
    }
    if(pSequence > 100) {
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        if((pEvent.Type != EVT_INTERRUPTION) && (-CalcChange(Cycle->RMS[n], pNominalVoltage) > EVENT_INTERRUPTION)) {
          pEvent.Type = EVT_INTERRUPTION;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent.Reference[i] = pNominalVoltage;
        } else if((pEvent.Type != EVT_DIP) && (pEvent.Type != EVT_INTERRUPTION) && (-CalcChange(Cycle->RMS[n], pNominalVoltage) > EVENT_DIP_SWELL)) {
          pEvent.Type = EVT_DIP;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent.Reference[i] = pNominalVoltage;
        } else if((pEvent.Type != EVT_SWELL) && (CalcChange(Cycle->RMS[n], pNominalVoltage) > EVENT_DIP_SWELL)) {
          pEvent.Type = EVT_SWELL;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent.Reference[i] = pNominalVoltage;
        } else if((pEvent.Type == EVT_NONE) && (Stationary == false) && (pPrevStationary == true)) {
          pEvent.Type = EVT_RVC;
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent.Reference[i] = RMS_Avg[i];
        }
      }
      if(pEvent.Type != EVT_NONE) {
        pAggreg200MS.Flag = true;
        if(pEvent.SamplesDuration == 0) {
          string Type = (pEvent.Type == EVT_RVC) ? "RVC" : (pEvent.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (pEvent.Type == EVT_DIP) ? "DIP" : "SWELL";
          LOGFile::Debug("New event '%s' started on analyzer '%s'\n", Type.c_str(), Serial.c_str());
          pEvent.SamplesDuration = CycleLen;
          pEvent.CyclesDuration = 1;
          pEvent.Start = pAggreg200MS.Timestamp + (uint32_t)((Pos * 1000) / pSamplerate);
          for(uint8_t i = 0; i < pVoltageChannels; i++) pEvent.PeakValue[i] = Cycle->RMS[i];
          Database DB(Serial);
          DB.ExecSentenceNoResult("INSERT INTO events(starttime, evttype, samplerate, reference, peak, change, notes, duration) VALUES (" + DB.Bind(pEvent.Start) + ", " + DB.Bind(Type) + ", " + DB.Bind(pSamplerate) + ", " + DB.Bind(pEvent.Reference, pVoltageChannels) + ", " + DB.Bind(pEvent.PeakValue, pVoltageChannels) + ", " + DB.Bind(pEvent.ChangeValue, pVoltageChannels) + ", '', 0)");
          float *CyclesValues = (float*)malloc(sizeof(float) * pVoltageChannels * (pPrevCyclesLen - 1));
          for(uint16_t n = 0; n < (pPrevCyclesLen - 1); n++) {
            for(uint8_t i = 0; i < pVoltageChannels; i++) CyclesValues[i * (pPrevCyclesLen - 1) + n] = pPrevCycles[(pPrevCyclesPos + 1 + n) % pPrevCyclesLen].RMS[i];
          }
          DB.ExecSentenceNoResult("INSERT INTO events_ext(starttime, sampletype, samples1) VALUES (" + DB.Bind(pEvent.Start) + ", 'RMS', " + DB.Bind(CyclesValues, pVoltageChannels, pPrevCyclesLen - 1) + ")");
          free(CyclesValues);
          //Copio muestras de inicio y ciclos
          //El inicio en cycles esta claro, como estoy escribiendo en POS, tengo que leer



        } else {
          pEvent.SamplesDuration += CycleLen;
          pEvent.CyclesDuration = pEvent.CyclesDuration + ((pEvent.CyclesDuration != (pPrevCyclesLen * 2)) ? 1 : 0);
          if(pEvent.Type == EVT_INTERRUPTION) {
            for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmin(pEvent.PeakValue[n], Cycle->RMS[n]);
          } else if(pEvent.Type == EVT_DIP) {
            for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmin(pEvent.PeakValue[n], Cycle->RMS[n]);
          } else if(pEvent.Type == EVT_SWELL) {
            for(uint8_t n = 0; n < pVoltageChannels; n++) pEvent.PeakValue[n] = fmax(pEvent.PeakValue[n], Cycle->RMS[n]);
          } else if(pEvent.Type == EVT_RVC) {
            for(uint8_t n = 0; n < pVoltageChannels; n++) {
              if(fabs(pEvent.Reference[n] - Cycle->RMS[n]) > fabs(pEvent.Reference[n] - pEvent.PeakValue[n])) pEvent.PeakValue[n] = Cycle->RMS[n];
            }
          }
        }
      }
    }





//          bool Found = false;
//          for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
//            if((it->Type != EVT_DIP) || (it->State != EVT_RUNNING) || (it->Duration > (EVENT_RAWENVELOPETIME * This->pSamplerate))) continue;
//            it->Type = EVT_INTERRUPTION;
//            Found = true;
//            break;
//          }
//          if(Found == false) This->InitEvent(EVT_INTERRUPTION, vector<float>(RMS_Avg.size(), This->pNominalVoltage), Pos);
//          Type = EVT_INTERRUPTION;
//        }




//        if(it->State == EVT_RUNNING) {
//          it->Duration += CycleLen;
//          if(it->Type != EVT_RVC) This->pAggreg200MS.Flag = true;
//        } else {
//          This->SaveEvent(*it);
//        }
//        it++;
//      }
//    }

    //Check events state
//    for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); ) {
      //If closed evend finish post event period save it and remove from list
//      if((it->State == EVT_CLOSING) && (it->SamplesRAW_A.size() == RequiredRAW) && (it->SamplesRMS_A.size() == RequiredRMS)) {
//        it->State = EVT_CLOSED;
//        This->SaveEvent(*it);
//        This->pEvents.erase(it++);
//      } else if(it->State == EVT_CLOSING) {
//        it++;
//      } else {
//        //Check if RVC event can be closed, or update values
//

    //Insert current cycle samples in prev data buffers and in running events than need its
//    for(uint n = 0; n < CycleLen; n++) {
//      vector<float> Sample;
//      for(uint j = 0; j < This->pVoltageSamples.size(); j++) Sample.push_back(This->pAggreg200MS.VoltageSamples[j][n + Pos]);
//      This->pPrevSamples.push_back(Sample);
//      if(This->pPrevSamples.size() > RequiredRAW) This->pPrevSamples.pop_front();
//      for(list<Event_t>::iterator it = This->pEvents.begin(); it != This->pEvents.end(); it++) {
//        if(it->SamplesRAW_A.size() < RequiredRAW) {
//          it->SamplesRAW_A.push_back(Sample);
//        } else if(it->State == EVT_RUNNING) {
//          it->SamplesRAW_B.push_back(Sample);
//          if(it->SamplesRAW_B.size() > RequiredRAW) it->SamplesRAW_B.pop_front();
//        }
//      }
//    }
    //Insert current cycle in prev data buffers and in running events than need its

//    for(auto &Event : pEvents) {
//      if(it->SamplesRMS_A.size() < RequiredRMS) {
//        it->SamplesRMS_A.push_back(Cycle.RMS);
//      } else if(it->State == EVT_RUNNING) {
//        it->SamplesRMS_B.push_back(Cycle.RMS);
//        if(it->SamplesRMS_B.size() > RequiredRMS) it->SamplesRMS_B.pop_front();
//      }
//    }
    pPrevCyclesPos = (pPrevCyclesPos + 1) % pPrevCyclesLen;
    Pos = Pos + CycleLen;
    pPrevStationary = Stationary && (pEvent.Type == EVT_NONE);
  }
}
void AnalyzerSoft::FFT_Func(FFTChannel_t *ch) {
  pthread_mutex_lock(&pFFTMutex);
  fftwf_plan Plan = fftwf_plan_dft_r2c_1d(pBlockSamples, ch->InPtr, (fftwf_complex*)ch->OutPtr, FFTW_ESTIMATE | FFTW_PRESERVE_INPUT);
  pFFTRunning++;
  pthread_mutex_unlock(&pFFTMutex);
  fftwf_execute_dft_r2c(Plan, ch->InPtr, (fftwf_complex*)ch->OutPtr);
  pthread_mutex_lock(&pFFTMutex);
  fftwf_destroy_plan(Plan);
  pFFTRunning--;
  pthread_mutex_unlock(&pFFTMutex);
  for(uint16_t i = 0; i < pFFTSamples; i++) {
    ch->OutPtr[i].Re = ch->OutPtr[i].Re / pBlockSamples;
    ch->OutPtr[i].Im = ch->OutPtr[i].Im / pBlockSamples;
  }
}
void AnalyzerSoft::Alarms_Func(Params_t *aggreg) {
  for(Alarm_t &Alarm : pAlarms) {
    if(Alarm.Aggreg != aggreg->Type) continue;
    uint8_t *Byte   = &Alarm.Results[Alarm.ResultPos / 8];
    uint8_t  Bit    = Alarm.ResultPos % 8;
    if((*Byte) & (1 << Bit)) Alarm.ResultCount--;
    *Byte &= ~(1 << Bit);
    try {
      if((Alarm.Evaluator.Eval() > 0.0) == true) {
        Alarm.ResultCount++;
        *Byte |= (1 << Bit);
      }
    } catch (Parser::exception_type &e) {
      LOGFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", Serial.c_str());
    }
    Alarm.ResultPos = (Alarm.ResultPos + 1) % Alarm.Window;
    if((Alarm.ResultCount == Alarm.Samples) && (Alarm.Triggered == false)) {
      Alarm.Triggered = true;
      LOGFile::Debug("Alarm triggered on analyzer '%s'\n", Serial.c_str());
      Database DB(Serial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", true, " + DB.Bind(Alarm.Priority) + ")");
      if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
      Telegram::SendAlarm(Serial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
    } else if((Alarm.ResultCount == 0) && (Alarm.Triggered == true)) {
      Alarm.Triggered = false;
      LOGFile::Debug("Alarm reseted on analyzer '%s'\n", Serial.c_str());
      Database DB(Serial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", false, " + DB.Bind(Alarm.Priority) + ")");
      if(pMQTTAlarms == true) PublishAlarm(aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
      Telegram::SendAlarm(Serial, pTimeZone, aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
    }
  }
}
void *AnalyzerSoft::Sync_Thread(void *params) {
  AnalyzerSoft *This = (AnalyzerSoft*)params;
  nice(10);
  string Cookie, User, Pass, Server;
  uint32_t ThreadsCount = 0;
  pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  Cond  = PTHREAD_COND_INITIALIZER;
  while(This->pSyncTerminate == false) {
    This->pAnalyzerLock.ReadLock();
    if((This->pSyncUser != User) || (This->pSyncPass != Pass) || (This->pSyncServer != Server)) {
      Cookie = "";
      User   = This->pSyncUser;
      Pass   = This->pSyncPass;
      Server = This->pSyncServer;
    }
    This->pAnalyzerLock.Unlock();
    URL Url(Server);
    Client HTTPClient(Url.Host, Url.Port);
    HTTPClient.set_compress(true);
    HTTPClient.set_read_timeout(3, 0);
    HTTPClient.set_timeout_sec(5);
    if(Cookie == "") {
      Headers Header  = {{"Accept", "application/json"}};
      json    Content = json::object();
      Content["Username"] = User;
      Content["Password"] = Pass;
      Content["UpdateLastLogin"] = false;
      auto Response = HTTPClient.Post(string(Url.Path + "/logIn").c_str(), Header, Content.dump(), "application/json");
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if((Response->has_header("Set-Cookie") == true) && (Content.is_discarded() == false) && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          string Tmp = Response->get_header_value("Set-Cookie");
          for(uint8_t n = 0; (n < Tmp.length()) && (Tmp.length() < 100); n++) {
            if(Tmp[n] == ';') break;
            Cookie += Tmp[n];
          }
        }
      }
      if(Cookie != "") {
        LOGFile::Debug("Analyzer '%s' logged on remote server\n", This->Serial.c_str());
      } else {
        LOGFile::Debug("Analyzer '%s' can not login on remote server\n", This->Serial.c_str());
        usleep(1000000);
      }
    } else {
      json   Content = json::object();
      Content["Serial"] = "REMOTE";
      Content["Analyzer"] = This->Serial;
      Content["Name"] = This->GetName();
      Request Req;
      Req.method = "POST";
      Req.path = Url.Path + "/getRemote";
      Req.headers =  {{"Accept", "application/json"}, {"Cookie", Cookie}, { "Accept", "text/event-stream" }, {"Content-Type", "application/json"}};
      Req.body = Content.dump();
      Req.progress = nullptr;
      Req.response_handler = [&](const Response &response) -> bool {
        if(This->pSyncTerminate == true) return false;
        if(response.status == HTTP_UNAUTHORIZED) {
          Cookie = "";
          LOGFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...\n", This->Serial.c_str());
          usleep(1000000);
          return false;
        } else if(response.status == HTTP_OK) {
          return true;
        } else {
          LOGFile::Debug("Failed request to remote server in analyzer '%s' (ERROR: %d)\n", This->Serial.c_str(), response.status);
          usleep(1000000);
          return false;
        }
      };
      Req.content_receiver = [&](const char *data, uint64_t len) -> bool {
        if(This->pSyncTerminate == true) return false;
        if((len <= 5) || (strncmp("data:", data, 5) != 0)) return false;
        string Msg(&data[5], len - 5);
        json Content = json::parse(Msg, NULL, false);
        if((Content.is_discarded() == true) || (Content.is_object() == false)) return true;
        LOGFile::Debug("Analyzer '%s' receive querie from remote server\n", This->Serial.c_str());
        if((Content["Sequence"].is_number() == false) || (Content["Method"].is_string() == false) || (Content["Params"].is_object() == false)) return false;
        Sync_t *Request = new Sync_t;
        Request->Count        = &ThreadsCount;
        Request->Mutex        = &Mutex;
        Request->Cond         = &Cond;
        Request->Cookie       = Cookie;
        Request->Server       = Server;
        Request->Analyzer     = This;
        Request->Method       = Content["Method"];
        Request->Params       = Content["Params"];
        Request->Sequence     = Content["Sequence"];
        pthread_t Thread;
        pthread_mutex_lock(&Mutex);
        if(pthread_create(&Thread, NULL, Sync_RespThread, (void*)Request) != 0) {
          LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
          delete Request;
        } else {
          pthread_setname_np(Thread, "SyncWorker");
          ThreadsCount++;
          pthread_detach(Thread);
        }
        pthread_mutex_unlock(&Mutex);
        return true;
      };
      auto Res = std::make_shared<Response>();
      if(HTTPClient.send(Req, *Res) == false) usleep(1000000);
    }
  }
  pthread_mutex_lock(&Mutex);
  while(ThreadsCount > 0) {
    LOGFile::Debug("Analyzer '%s' flusing remote request (%d pending)\n", This->Serial.c_str(), ThreadsCount);
    pthread_cond_wait(&Cond, &Mutex);
  }
  pthread_mutex_unlock(&Mutex);
  return NULL;
}
void *AnalyzerSoft::MQTT_Thread(void *params) {
  AnalyzerSoft *This = (AnalyzerSoft*)params;
  nice(10);
  string User, Pass, Server;
  while(This->pMQTTTerminate == false) {
    This->pAnalyzerLock.ReadLock();
    if((This->pMQTTUser != User) || (This->pMQTTPass != Pass) || (This->pMQTTServer != Server)) {
      if(This->pMQTTClient.socketfd != -1) {
        close(This->pMQTTClient.socketfd);
        This->pMQTTClient.socketfd = -1;
      }
      User   = This->pMQTTUser;
      Pass   = This->pMQTTPass;
      Server = This->pMQTTServer;
    }
    This->pAnalyzerLock.Unlock();
    mqtt_sync(&This->pMQTTClient);
    if(This->pMQTTClient.error == MQTT_ERROR_CONNECT_NOT_CALLED) {
      mqtt_connect(&This->pMQTTClient, NULL, NULL, NULL, 0, (User == "") ? NULL : User.c_str(), (Pass == "") ? NULL : Pass.c_str(), MQTT_CONNECT_CLEAN_SESSION, 400);
      if(This->pMQTTConfig == true) This->PublishConfig(CheckConfig(ConfigManager::ReadConfig(This->pConfigPath), This->pMode, NULL));
    } else if(This->pMQTTClient.error == MQTT_ERROR_SOCKET_ERROR) {
      if(This->pMQTTClient.socketfd != -1) {
        close(This->pMQTTClient.socketfd);
        This->pMQTTClient.socketfd = -1;
      }
      URL  Url(Server);
      char Port[16];
      addrinfo Hints = { .ai_flags = 0, .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
      addrinfo *ServInfo;
      sprintf(Port, "%u", Url.Port);
      int Error = getaddrinfo(Url.Host.c_str(), Port, &Hints, &ServInfo);
      if(Error != 0) {
        LOGFile::Error("%s:%d -> %d\n", __func__, __LINE__, Error);
        usleep(1000000);
        continue;
      }
      for(addrinfo *p = ServInfo; p != NULL; p = p->ai_next) {
        This->pMQTTClient.socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(This->pMQTTClient.socketfd == -1) {
          LOGFile::Error("%s:%d -> (%s)\n", __func__, __LINE__, strerror(errno));
          continue;
        }
        if(connect(This->pMQTTClient.socketfd, p->ai_addr, p->ai_addrlen) == -1) {
          close(This->pMQTTClient.socketfd);
          This->pMQTTClient.socketfd = -1;
          continue;
        }
        fcntl(This->pMQTTClient.socketfd, F_SETFL, fcntl(This->pMQTTClient.socketfd, F_GETFL) | O_NONBLOCK);
        break;
      }
      freeaddrinfo(ServInfo);
      if(This->pMQTTClient.socketfd == -1) {
        LOGFile::Debug("Analyzer '%s' can not connect to MQTT broker\n", This->Serial.c_str());
        usleep(1000000);
        continue;
      } else {
        mqtt_reinit(&This->pMQTTClient, This->pMQTTClient.socketfd, This->pMQTTSendBuff, This->pMQTTBuffSize, This->pMQTTRecvBuff, This->pMQTTBuffSize);
      }
    } else if(This->pMQTTClient.error != MQTT_OK) {
      LOGFile::Error("MQTT ERROR: %s (%d)\n", mqtt_error_str(This->pMQTTClient.error), This->pMQTTClient.error);
    }
    usleep(100000);
  }
  if(This->pMQTTClient.socketfd != -1) {
    close(This->pMQTTClient.socketfd);
    This->pMQTTClient.socketfd = -1;
  }
  return NULL;
}
void *AnalyzerSoft::Sync_RespThread(void *args) {
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
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(This->Params, Resp);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(This->Params, Resp);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(This->Params, Resp);
  }
  URL Url(This->Server);
  Headers Header  = {{"Accept", "application/json"}, {"Cookie", This->Cookie}};
  json    Content = json::object();
  Content["Code"] = Resp.status;
  Content["Sequence"] = This->Sequence;
  Content["Serial"] = "REMOTE";
  Content["Analyzer"] = This->Analyzer->Serial;
  Content["Response"] = Resp.body;
  Content["Mime"] = Resp.has_header("Content-Type") ? Resp.get_header_value("Content-Type") : "text/plain";
  Client HTTPClient(Url.Host.c_str(), Url.Port);
  HTTPClient.set_compress(true);
  auto Response = HTTPClient.Post(string(Url.Path + "/setRemote").c_str(), Header, Content.dump(), "application/json");
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    LOGFile::Debug("Remote request '%s' for analyzer '%s' completed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  } else {
    if(Response != nullptr)
      LOGFile::Error("Remote request '%s' for analyzer '%s' failed (ERROR: %d)\n", This->Method.c_str(), This->Analyzer->Serial.c_str(), Response->status);
    else
      LOGFile::Error("Remote request '%s' for analyzer '%s' failed (can not connect)\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  }
  pthread_mutex_lock(This->Mutex);
  *This->Count = *This->Count - 1;
  pthread_cond_signal(This->Cond);
  pthread_mutex_unlock(This->Mutex);
  delete This;
  return NULL;
}
void AnalyzerSoft::PeriodicTasks() {
  LOGFile::Debug("Removing old data for analyzer '%s'\n", Serial.c_str());
  Database DB(Serial);
  uint64_t Now = Tools::GetTime64();
  if(Now > 86400000) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Now - 86400000));   //Delete 1 day old
  if(Now > 3600000)  DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(Now - 3600000));     //Delete 1 hour old
}
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Response = ConfigManager::ReadConfig(pConfigPath);
  Response["Online"] = IsOnline();
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  if((params.contains("Alarms") == false) || (params["Alarms"].is_boolean() == false) || (params["Alarms"] == false)) Response.erase("Alarms");
  if((params.contains("Rates") == false) || (params["Rates"].is_boolean() == false) || (params["Rates"] == false)) {
    Response.erase("Rates");
  } else {
    Response["RateModules"] = PriceModule::GetVarsInfo();
  }
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename))) AS size from pg_tables WHERE schemaname = " + DB.Bind(Serial));
  if(DB.FecthRow()) Response["Size"] = DB.ResultUInt64("size");
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
  if((Config.contains("Alarms") == false) || (Config["Alarms"].is_array() == false)) Config["Alarms"] = ConfigManager::ReadConfig(pConfigPath + "/Alarms");
  if((Config.contains("Rates") == false) || (Config["Rates"].is_array() == false)) Config["Rates"] = ConfigManager::ReadConfig(pConfigPath + "/Rates");
  string Error;
  json   Conf = CheckConfig(Config, pMode, &Error);
  if(Error.empty() == true) {
    Configure(Conf);
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
  } else {
    HTTPServer::FillResponse(response, Error, "text/plain", HTTP_BAD_REQUEST);
  }
}
void AnalyzerSoft::GetSeries(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "200MS") && (params["Aggreg"] != "3S") && (params["Aggreg"] != "1M") && (params["Aggreg"] != "10M") && (params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To = params["To"];
  uint64_t From = params["From"];
  string Aggreg = params["Aggreg"];
  uint32_t PeriodSeconds = 200;
  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if(Aggreg == "3S") PeriodSeconds = 3000;
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if(((To - From) / PeriodSeconds) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  string Query = "SELECT ROW_TO_JSON(Result) AS result FROM ( SELECT ";
  float Default[200];
  for(uint8_t n = 0; n < 200; n++) Default[n] = 0.0;
  for(uint i = 0; i < params["Series"].size(); i++) {
    if(!params["Series"][i].is_string()) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    string Serie = params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency") + ", ";
    } else if((Serie == "UNBALANCE") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Query += "COALESCE(ARRAY_AGG(unbalance ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Unbalance") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      }
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      }
    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + DB.Bind(Default, pCurrentChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + DB.Bind(Default, pVoltageChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + DB.Bind(Default, pCurrentChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_p, " + DB.Bind(Default, pVoltageChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Harmonics") + ", ";
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics_Stats") + ", ";
    } else if((Serie == "UNBALANCE_STATS") && (Aggreg == "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_unbalance ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Unbalance_Stats") + ", ";
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Sampletime") + " FROM ";
  if(params["Aggreg"] == "1M") {
    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
  } else {
    Query += "aggreg_" + Aggreg;
  }
  Query += " WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + ") AS result";
  DB.ExecSentenceAsyncResult(Query);
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetSeriesNow(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  json Result = json::object();
  Result["Sampletime"] = pAggreg200MS.Timestamp;
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) {
      pNowDataLock.ConsumerEnds();
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    string Serie = params["Series"][i];
    if(Serie == "FREQUENCY") {
      Result["Frequency"] = pAggreg200MS.Frequency;
    } else if(Serie == "FLAG") {
      Result["Flag"] = pAggreg200MS.Flag;
    } else if(Serie == "UNBALANCE") {
      Result["Unbalance"] = pAggreg200MS.Unbalance;
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
          Result["Voltage_Harmonics"][n][i].push_back(pVoltageFFT[n * pMaxFFTSamples + (10 + i * 10)].Re * M_SQRT2);
          Result["Voltage_Harmonics"][n][i].push_back(pVoltageFFT[n * pMaxFFTSamples + (10 + i * 10)].Im * M_SQRT2);
        }
      }
    } else if(Serie == "CURRENT_HARMONICS") {
      Result["Current_Harmonics"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_Harmonics"].push_back(json::array());
        for(uint8_t i = 0; i < 50; i++) {
          Result["Current_Harmonics"][n].push_back(json::array());
          Result["Current_Harmonics"][n][i].push_back(pCurrentFFT[n * pMaxFFTSamples + (10 + i * 10)].Re * M_SQRT2);
          Result["Current_Harmonics"][n][i].push_back(pCurrentFFT[n * pMaxFFTSamples + (10 + i * 10)].Im * M_SQRT2);
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
        for(uint16_t i = 0; i < pFFTSamples; i++) Result["Voltage_FFT"][n].push_back(BinPower(pVoltageFFT[n * pMaxFFTSamples + i]));
      }
    } else if(Serie == "CURRENT_FFT") {
      Result["Current_FFT"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_FFT"].push_back(json::array());
        for(uint16_t i = 0; i < pFFTSamples; i++) Result["Current_FFT"][n].push_back(BinPower(pCurrentFFT[n * pMaxFFTSamples + i]));
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
void AnalyzerSoft::GetWaveStream(const json &params, Response &response) {

}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetEvents(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To = params["To"];
  uint64_t From = params["From"];
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object("
                             "'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), "
                             "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), "
                             "'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), "
                             "'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), "
                             "'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), "
                             "'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                             "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), "
                             "'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                             "FROM events WHERE starttime >= " + DB.Bind(From) + " AND starttime <= " + DB.Bind(To));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("evt"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false)) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
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
void AnalyzerSoft::SetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Type") == false) || (params["Type"].is_string() == false)) return HTTPServer::FillResponse(response, "'Type' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Notes") == false) || (params["Notes"].is_string() == false)) return HTTPServer::FillResponse(response, "'Notes' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  uint64_t ID    = params["StartTime"];
  string   Type  = params["Type"];
  string   Notes = params["Notes"];
  bool Result = DB.ExecSentenceNoResult("UPDATE events SET Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::DelEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Type") == false) || (params["Type"].is_string() == false)) return HTTPServer::FillResponse(response, "'Type' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  uint64_t ID = params["StartTime"];
  string Type = params["Type"];
  bool Result = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(ID) + " AND evttype = " + DB.Bind(Type));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Precise") == false) || (params["Precise"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Precise' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json     Result          = json::object();
  uint64_t To              = params["To"];
  uint64_t From            = params["From"];
  string   Aggreg          = params["Aggreg"];
  bool     SaveActive      = false;
  bool     SaveReactive    = false;
  bool     SaveExpense     = false;
  bool     SavePeriodName  = false;
  bool     SavePeriodColor = false;
  bool     SaveContracted  = false;
  bool     SaveBills       = false;
  bool     SaveEnergyCost  = false;
  bool     SavePowerCost   = false;
  bool     SaveSampleTime  = false;
  bool     SavePeriodTime  = false;
  float    AccActive       = 0.0;
  float    AccReactive     = 0.0;
  float    AccCost         = 0.0;
  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if(((To - From) / ((Aggreg == "1H") ? 3600000 : 900000)) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    if(params["Series"][i] == "ACTIVE_POWER") {
      Result["Active_Power"] = json::array();
      SaveActive = true;
    } else if(params["Series"][i] == "REACTIVE_POWER") {
      Result["Reactive_Power"] = json::array();
      SaveReactive = true;
    } else if(params["Series"][i] == "EXPENSE") {
      Result["Expense"] = json::array();
      SaveExpense = true;
    } else if(params["Series"][i] == "PERIOD_POWER_COST") {
      Result["Period_Power_Cost"] = json::array();
      SavePowerCost = true;
    } else if(params["Series"][i] == "PERIOD_ENERGY_COST") {
      Result["Period_Energy_Cost"] = json::array();
      SaveEnergyCost = true;
    } else if(params["Series"][i] == "PERIOD_NUMBER") {
      Result["Period_Number"] = json::array();
      SavePeriodName = true;
    } else if(params["Series"][i] == "PERIOD_COLOR") {
      Result["Period_Color"] = json::array();
      SavePeriodColor = true;
    } else if(params["Series"][i] == "PERIOD_POWER_CONTRACTED") {
      Result["Period_Power_Contracted"] = json::array();
      SaveContracted = true;
    } else if(params["Series"][i] == "BILLS") {
      Result["Bills"] = json::array();
      SaveBills = true;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  if((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg != "15M")) {
    Result["Period_Sampletime"] = json::array();
    SavePeriodTime = true;
  }
  if((SaveActive || SaveReactive || SaveExpense) || ((SavePeriodName || SavePeriodColor || SaveContracted || SavePowerCost || SaveEnergyCost) && (Aggreg == "15M"))) {
    Result["Sampletime"] = json::array();
    SaveSampleTime = true;
  }
  pAnalyzerLock.ReadLock();
  RatesEval Eval(pRateList, From, To, params["Precise"]);
  Eval.SetSampleCallback([&](const RatesEval::SampleOut_t &out) {
    AccActive   += out.ActivePower;
    AccReactive += out.ReactivePower;
    AccCost     += out.Cost;
    if(SavePeriodTime) Result["Period_Sampletime"].push_back(out.SampleTime);
    if(SaveContracted) Result["Period_Power_Contracted"].push_back(out.ContractedPower);
    if(SavePowerCost)  Result["Period_Power_Cost"].push_back(out.PowerCost);
    if(SaveEnergyCost) Result["Period_Energy_Cost"].push_back(out.EnergyCost);
    if(SavePeriodName) {
      if(out.Period == UINT16_MAX) {
        Result["Period_Number"].push_back(nullptr);
      } else {
        Result["Period_Number"].push_back(out.Period);
      }
    }
    if(SavePeriodColor) {
      if(out.Color.empty() == true) {
        Result["Period_Color"].push_back(nullptr);
      } else {
        Result["Period_Color"].push_back(out.Color);
      }
    }
    if(Aggreg == "1H") {
      if((out.SampleTime % 3600000) != 3599999) return;
      AccActive = AccActive / 4.0;
      AccReactive = AccReactive / 4.0;
    }
    if(SaveSampleTime) Result["Sampletime"].push_back(out.SampleTime);
    if(SaveActive)     Result["Active_Power"].push_back(AccActive);
    if(SaveReactive)   Result["Reactive_Power"].push_back(AccReactive);
    if(SaveExpense)    Result["Expense"].push_back(AccCost);
    AccActive   = 0.0;
    AccReactive = 0.0;
    AccCost     = 0.0;
  });
  Eval.SetBillCallback([&](const json &out) {
    if(SaveBills == true) Result["Bills"].push_back(out);
  });
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime >= " + DB.Bind(Eval.StartDate()) + " AND sampletime <= " + DB.Bind(Eval.EndDate()) + " ORDER BY sampletime ASC");
  while(DB.FecthRow() == true) Eval.PushSample(DB.ResultUInt64("sampletime"), DB.ResultFloat("active_power"), DB.ResultFloat("reactive_power"));
  Eval.Finish();
  pAnalyzerLock.Unlock();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Expression") == false) || (params["Expression"].is_string() == false)) return HTTPServer::FillResponse(response, "'Expression' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Parser Evaluator;
  json   Result = json::object();
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  InitAlarmParser(Evaluator, params["Expression"], AGGREG_200MS);
  Result["Variables"] = json::object();
  for(auto Var : Evaluator.GetVar()) Result["Variables"][Var.first] = *Var.second;
  try {
    Result["Result"] = Evaluator.Eval();
  } catch (Parser::exception_type &e) {
    Result["Error"] = e.GetMsg();
  }
  pNowDataLock.ConsumerEnds();
  return HTTPServer::FillResponse(response, Result.dump(), "application/json", (Result.contains("Error")) ? HTTP_BAD_REQUEST : HTTP_OK);
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To = params["To"];
  uint64_t From = params["From"];
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object("
                              "'AlarmTime', COALESCE(ARRAY_AGG(alarmtime ORDER BY alarmtime ASC), '{}'), "
                              "'Name', COALESCE(ARRAY_AGG(name ORDER BY alarmtime ASC), '{}'), "
                              "'Priority', COALESCE(ARRAY_AGG(priority ORDER BY alarmtime ASC), '{}'), "
                              "'Triggered', COALESCE(ARRAY_AGG(trigger ORDER BY alarmtime ASC), '{}')) AS result "
                              "FROM alarms WHERE alarmtime >= " + DB.Bind(From) + " AND alarmtime <= " + DB.Bind(To));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerSoft::SetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}