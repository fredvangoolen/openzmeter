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
#define EVENT_RMSENVELOPETIME           10.0        //Time in seconds before and after event to store RMS samples
//----------------------------------------------------------------------------------------------------------------------
const float   AnalyzerSoft::FrequencyRanges[];
const uint8_t AnalyzerSoft::FrequencyRangesLen;
const float   AnalyzerSoft::VoltageRanges[];
const uint8_t AnalyzerSoft::VoltageRangesLen;
const float   AnalyzerSoft::THDRanges[];
const uint8_t AnalyzerSoft::THDRangesLen;
const float   AnalyzerSoft::HarmonicsAllowed[];
const uint8_t AnalyzerSoft::HarmonicsAllowedLen;
const float   AnalyzerSoft::UnbalanceRanges[];
const uint8_t AnalyzerSoft::UnbalanceRangesLen;
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t     AnalyzerSoft::pFFTMutex = PTHREAD_MUTEX_INITIALIZER;
uint32_t            AnalyzerSoft::pFFTRunning = 0;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3), pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pMode(mode) {
  LOGFile::Debug("Creating AnalyzerSoft database tables\n");
  Database DB(id);
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Tools::GetTime64() - 86400000));   //Delete 1 day old
  DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(Tools::GetTime64() - 3600000));     //Delete 1 hour old
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
//  pPrevTimestamp    = 0;
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
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFTWorker[i] = new Worker("FFT_V" + to_string(i), this, &FFT_Func);
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFTWorker[i] = new Worker("FFT_I" + to_string(i), this, &FFT_Func);
  Configure(CheckConfig(ConfigManager::ReadConfig(optionPath), mode));
  pAnalyzerWorker = new Worker((pMode == PHASE1) ? "Analyzer_1P" : (pMode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N", this, &Analyzer_Func);
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
    pAnalyzerWorker->Run(&pAggreg200MS);
    pAnalyzerLock.Unlock();
    return true;
  }
  pAnalyzerLock.Unlock();
  return false;
}
bool AnalyzerSoft::IsWorking() {
  return pAnalyzerWorker->IsWorking();
}
json AnalyzerSoft::CheckConfig(const json &config, const Analyzer::AnalyzerMode mode) {
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
    for(auto configRate : config["Rates"]) {
      if((configRate.contains("StartDate") == false) || (configRate["StartDate"].is_number_unsigned() == false)) continue;
      if((configRate.contains("EndDate") == false) || (configRate["EndDate"].is_number_unsigned() == false)) continue;
      if((configRate.contains("Name") == false) || (configRate["Name"].is_string() == false)) continue;
      if((configRate.contains("Config") == false) || (configRate["Config"].is_object() == false)) continue;
      if((configRate.contains("Template") == false) || (configRate["Template"].is_object() == false)) continue;
      //Tariff T(configRate["StartDate"], configRate["EndDate"], ConfigOUT["TimeZone"]);
      //if(T.SetTemplate(configRate["Template"]) == false) continue;
      //if(T.SetConfig(configRate["Config"]) == false) continue;
      //ConfigOUT["Rates"].push_back(configRate);
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
  pTimeZone         = options["TimeZone"];
  pSave200MS        = options["Save200MS"];
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
  //TODO VERIFICO RATESpRates = options["Rates"];
  bool AlarmsEnabled = options["AlarmsEnabled"];
  bool EventsEnabled = options["EventsEnabled"];
  if((EventsEnabled == false) && (pEventsWorker != NULL)) {
    delete pEventsWorker;
    pEventsWorker = NULL;
  } else if((EventsEnabled == true) && (pEventsWorker == NULL)) {
    pPrevStationary = false;
    pEventsWorker = new Worker("EventsDet", this, &Events_Func);
  }
  if((AlarmsEnabled == false) && (pAlarmsWorker != NULL)) {
    delete pAlarmsWorker;
    pAlarmsWorker = NULL;
  } else if((AlarmsEnabled == true) && (pAlarmsWorker == NULL)) {
    pAlarmsWorker = new Worker("AlarmsDet", this, &Alarms_Func);
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
//      mqtt_reinit(&pMQTTClient, -1, pMQTTSendBuff, pMQTTBuffSize, pMQTTRecvBuff, pMQTTBuffSize);
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
bool AnalyzerSoft::FindRunningEvent(const uint32_t mask) {
  for(list<Event_t>::iterator it = pEvents.begin(); it != pEvents.end(); it++) {
    if((it->Type & mask) && (it->State == EVT_RUNNING)) return true;
  }
  return false;
}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  if(params.AggregSamples == 0) return;
  SetOnline();
  Database DB(Serial);
  if(params.Type == AGGREG_200MS)
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
    DB.Bind(params.StatsVoltage, VoltageRangesLen) + ", " +
    DB.Bind(params.StatsFrequency, FrequencyRangesLen) + ", " +
    DB.Bind(params.StatsTHD, THDRangesLen) + ", " +
    DB.Bind(params.StatsHarmonics, 2) + ", " +
    DB.Bind(params.StatsUnbalance, UnbalanceRangesLen) + ")");
  if(params.Type == AGGREG_1M) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(params.Timestamp - 86400000));   //Delete 1 day old
  if((params.Type == AGGREG_1M) && (pSave200MS == true)) DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(params.Timestamp - 3600000));     //Delete 1 hour old
}
void AnalyzerSoft::SaveEvent(Event_t &evt) {
  Database DB(Serial);
  string Type = (evt.Type == EVT_RVC) ? "RVC" : (evt.Type == EVT_INTERRUPTION) ? "INTERRUPTION" : (evt.Type == EVT_DIP) ? "DIP" : "SWELL";
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
  evt.SamplesRAW_A = deque<vector<float> >();
  evt.SamplesRMS_A = deque<vector<float> >();
  evt.SamplesRAW_B = deque<vector<float> >();
  evt.SamplesRMS_B = deque<vector<float> >();
}
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
  AppendArray("Active_Power", Params->Active_Power, pVoltageChannels);
  AppendArray("Reactive_Power", Params->Reactive_Power, pVoltageChannels);
  AppendArray("Apparent_Power", Params->Apparent_Power, pVoltageChannels);
  AppendArray("Power_Factor", Params->Power_Factor, pVoltageChannels);
  AppendArray("Voltage_THD", Params->Voltage_THD, pVoltageChannels);
  AppendArray("Current_THD", Params->Current_THD, pCurrentChannels);
  if(pVoltageChannels > 1) AppendArray("Voltage_Phase", Params->Voltage_Phase, pVoltageChannels);
  AppendArray("Phi", Params->Phi, pVoltageChannels);
  try {
    parser.SetExpr(expr);
  } catch(Parser::exception_type &e) {
    LOGFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", Serial.c_str());
  }
}
void AnalyzerSoft::Analyzer_Func(void *creation, void *run) {
  AnalyzerSoft *This  = (AnalyzerSoft*)creation;
  Params_t     *Aggreg = (Params_t*)run;
  This->pAnalyzerLock.ReadLock();
  uint64_t Start = Tools::GetTime64();
  Aggreg->AggregSamples = 1;
  if(This->pEventsWorker != NULL) This->pEventsWorker->Run(Aggreg);
  This->pFFTSamples = (This->pBlockSamples / 2) + 1;
  for(uint8_t i = 0; i < This->pVoltageChannels; i++) This->pVoltageFFTWorker[i]->Run(&This->pVoltageFFTChannels[i]);
  for(uint8_t i = 0; i < This->pCurrentChannels; i++) This->pCurrentFFTWorker[i]->Run(&This->pCurrentFFTChannels[i]);
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
    Aggreg->Frequency = (Cycles == This->pFreqCyclesLen) ? This->pFreqCyclesLen / ((2.0 * Samples) / This->pSamplerate) : 0;
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
      Aggreg->Voltage_Harmonics[j * 50 + i] = BinPower(This->pVoltageFFT[j * This->pMaxFFTSamples + (10 + i * 10)]);
      Aggreg->Current_Harmonics[j * 50 + i] = BinPower(This->pCurrentFFT[j * This->pMaxFFTSamples + (10 + i * 10)]);
      Aggreg->Power_Harmonics[j * 50 + i]   = Aggreg->Current_Harmonics[j * 50 + i] * Aggreg->Voltage_Harmonics[j * 50 + i] * cos(CalcAngle(This->pCurrentFFT[j * This->pMaxFFTSamples + (10 + i * 10)], This->pVoltageFFT[j * This->pMaxFFTSamples + (10 + i * 10)]));
      if(i > 0) Aggreg->Voltage_THD[j] += Aggreg->Voltage_Harmonics[j * 50 + i];
      if(i > 0) Aggreg->Current_THD[j] += Aggreg->Current_Harmonics[j * 50 + i];
    }
    Aggreg->Voltage_Phase[j] = (j == 0) ? 0.0 : CalcAngle(This->pVoltageFFT[j * This->pMaxFFTSamples + 10], This->pVoltageFFT[10]) * 180.0 / M_PI;
    Aggreg->Voltage_THD[j] = (Aggreg->Voltage_THD[j] / Aggreg->Voltage_Harmonics[j * 50]) * 100.0;
    Aggreg->Apparent_Power[j] = Aggreg->Voltage[j] * Aggreg->Current[j];
    float PHI = CalcAngle(This->pCurrentFFT[j * This->pMaxFFTSamples + 10], This->pVoltageFFT[j * This->pMaxFFTSamples + 10]);
    Aggreg->Reactive_Power[j] = Aggreg->Voltage_Harmonics[j * 50] * Aggreg->Current_Harmonics[j * 50] * sin(-PHI);
    Aggreg->Phi[j] = PHI * 180.0 / M_PI;
    Aggreg->Current_THD[j] = (Aggreg->Current_THD[j] / Aggreg->Current_Harmonics[j * 50]) * 100.0;
    Aggreg->Active_Energy[j] = (Aggreg->Active_Power[j] * This->pBlockSamples) / (This->pSamplerate * 3600.0);
    Aggreg->Power_Factor[j] = Aggreg->Active_Power[j] / Aggreg->Apparent_Power[j];
    Aggreg->Reactive_Energy[j] = (Aggreg->Reactive_Power[j] * This->pBlockSamples) / (This->pSamplerate * 3600.0);
  }
  if(This->pMode != PHASE1) {
    complex<float> A(cos(120.0 * M_PI / 180.0), sin(120.0 * M_PI / 180.0));
    complex<float> V1(This->pVoltageFFT[0 * This->pMaxFFTSamples + 10].Re * M_SQRT2, This->pVoltageFFT[0 * This->pMaxFFTSamples + 10].Im * M_SQRT2);
    complex<float> V2(This->pVoltageFFT[1 * This->pMaxFFTSamples + 10].Re * M_SQRT2, This->pVoltageFFT[1 * This->pMaxFFTSamples + 10].Im * M_SQRT2);
    complex<float> V3(This->pVoltageFFT[2 * This->pMaxFFTSamples + 10].Re * M_SQRT2, This->pVoltageFFT[2 * This->pMaxFFTSamples + 10].Im * M_SQRT2);
    Aggreg->Unbalance = ((abs(V1 + A * A * V2 + A * V3) / 3.0) / (abs(V1 + A * V2 + A * A * V3) / 3.0)) * 100.0;
  }
  if(This->pMode == PHASE3N) {
    This->pCurrentFFTWorker[3]->Wait();
    for(uint i = 0; i < 50; i++) {
      Aggreg->Current_Harmonics[3 * 50 + i] = BinPower(This->pCurrentFFT[3 * This->pFFTSamples + (10 + i * 10)]);
      if(i > 0) Aggreg->Current_THD[3] += Aggreg->Current_Harmonics[3 * 50 + i];
    }
    Aggreg->Current_THD[3] = (Aggreg->Current_THD[3] / Aggreg->Current_Harmonics[3 * 50]) * 100.0;
  }
  if(This->pEventsWorker != NULL) This->pEventsWorker->Wait();
  if(This->pSequence < 100) {
    LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "\n", This->Serial.c_str(), Tools::GetTime64() - Start, This->pSequence);
    This->InitAggreg(This->pAggreg200MS, AGGREG_200MS);
    This->pBlockSamples     = 0;
    This->pBlockCyclesCount = 0;
    memset(This->pBlockCycles, 0, sizeof(pBlockCycles));
    This->pSequence++;
    This->pAnalyzerLock.Unlock();
    return;
  }
  This->pNowDataLock.ProviderWrite();
  if((This->pSequence % 15) == 0) {
    LOGFile::Debug("Storing 3S aggregation for analyzer '%s'\n", This->Serial.c_str());
    This->NormalizeAggreg(This->pAggreg3S);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg3S);
    if(This->pAggreg3S.Flag == false) {
      float Variation = This->CalcChange(This->pAggreg3S.Frequency, This->pNominalFrequency);
      for(uint8_t Range = 0; Range < FrequencyRangesLen; Range++) {
        if((Variation >= FrequencyRanges[Range]) && (Variation < FrequencyRanges[Range + 1])) This->pAggreg1H.StatsFrequency[Range]++;
      }
    }
    This->SaveAggreg(This->pAggreg3S);
    if(This->pMQTT3S) This->PublishAggreg(This->pAggreg3S, "3S");
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg3S, AGGREG_3S);
  }
  This->AccumulateAggreg(This->pAggreg3S, This->pAggreg200MS);

  if((This->pAggreg1M.Timestamp / 60000) < (This->pAggreg200MS.Timestamp / 60000)) {
    LOGFile::Debug("Storing 1M aggregation for analyzer '%s'\n", This->Serial.c_str());
    This->pAggreg1M.Timestamp = This->pAggreg1M.Timestamp / 60000 * 60000 + 59999;
    This->NormalizeAggreg(This->pAggreg1M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg1M);
    This->SaveAggreg(This->pAggreg1M);
    if(This->pMQTT1M) This->PublishAggreg(This->pAggreg1M, "1M");
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg1M, AGGREG_1M);
  }
  This->AccumulateAggreg(This->pAggreg1M, This->pAggreg200MS);

  if((This->pAggreg10M.Timestamp / 600000) < (This->pAggreg200MS.Timestamp / 600000)) {
    LOGFile::Debug("Storing 10M aggregation for analyzer '%s\n", This->Serial.c_str());
    This->pAggreg10M.Timestamp = This->pAggreg10M.Timestamp / 600000 * 600000 + 599999;
    This->NormalizeAggreg(This->pAggreg10M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg10M);
    if(This->pAggreg10M.Flag == false) {
      for(uint8_t Range = 0; Range < UnbalanceRangesLen; Range++) {
        if((This->pAggreg10M.Unbalance >= UnbalanceRanges[Range]) && (This->pAggreg10M.Unbalance < UnbalanceRanges[Range + 1])) This->pAggreg1H.StatsUnbalance[Range]++;
      }
      for(uint8_t i = 0; i < This->pVoltageChannels; i++) {
        float Variation = This->CalcChange(This->pAggreg10M.Voltage[i], This->pNominalVoltage);
        for(uint8_t Range = 0; Range < VoltageRangesLen; Range++) {
          if((Variation >= VoltageRanges[Range]) && (Variation < VoltageRanges[Range + 1])) This->pAggreg1H.StatsVoltage[Range]++;
        }
        for(uint8_t Range = 0; Range < THDRangesLen; Range++) {
          if((This->pAggreg10M.Voltage_THD[i] >= THDRanges[Range]) && (This->pAggreg10M.Voltage_THD[i] < THDRanges[Range + 1])) This->pAggreg1H.StatsTHD[Range]++;
        }
        bool HarmonicsInRange = true;
        for(uint8_t n = 0; n < HarmonicsAllowedLen; n++) {
          float Variation = This->CalcChange(This->pAggreg10M.Voltage_Harmonics[i * 50 + n + 1], This->pNominalVoltage);
            if(Variation > HarmonicsAllowed[n]) {
              HarmonicsInRange = false;
              break;
            }
        }
        if(HarmonicsInRange == true) {
          This->pAggreg1H.StatsHarmonics[0]++;
        } else {
          This->pAggreg1H.StatsHarmonics[1]++;
        }
      }
    }
    This->SaveAggreg(This->pAggreg10M);
    if(This->pMQTT10M) This->PublishAggreg(This->pAggreg10M, "10M");
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg10M, AGGREG_10M);
  }
  This->AccumulateAggreg(This->pAggreg10M, This->pAggreg200MS);

  if((This->pAggreg15M.Timestamp / 900000) < (This->pAggreg200MS.Timestamp / 900000)) {
    LOGFile::Debug("Storing 15M aggregation for analyzer '%s'\n", This->Serial.c_str());
    This->pAggreg15M.Timestamp = This->pAggreg15M.Timestamp / 900000 * 900000 + 899999;
    This->NormalizeAggreg(This->pAggreg15M);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg15M);
    This->SaveAggreg(This->pAggreg15M);
    if(This->pMQTT15M) This->PublishAggreg(This->pAggreg15M, "15M");
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg15M, AGGREG_15M);
  }
  This->AccumulateAggreg(This->pAggreg15M, This->pAggreg200MS);

  if((This->pAggreg1H.Timestamp / 3600000) < (This->pAggreg200MS.Timestamp / 3600000)) {
    LOGFile::Debug("Storing 1H aggregation for analyzer '%s'\n", This->Serial.c_str());
    This->pAggreg1H.Timestamp = This->pAggreg1H.Timestamp / 3600000 * 3600000 + 3599999;
    This->NormalizeAggreg(This->pAggreg1H);
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg1H);
    This->SaveAggreg(This->pAggreg1H);
    if(This->pMQTT1H) This->PublishAggreg(This->pAggreg1H, "1H");
    if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();
    This->InitAggreg(This->pAggreg1H, AGGREG_1H);
  }
  This->AccumulateAggreg(This->pAggreg1H, This->pAggreg200MS);

  if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Run(&This->pAggreg200MS);
  if(This->pSave200MS) {
    LOGFile::Debug("Storing 200MS aggregation for analyzer '%s'\n", This->Serial.c_str());
    This->SaveAggreg(This->pAggreg200MS);
  }
  if(This->pAlarmsWorker != NULL) This->pAlarmsWorker->Wait();

  //  SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
  //  SysStatus->Lock.WriteLock();
  //  SysStatus->Events = This->pAggreg200MS.Flag;
  //  SysStatus->Lock.Unlock();
  This->pNowDataLock.ProviderWait();
  LOGFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (Sampletime: %" PRIu64 ")\n", This->Serial.c_str(), Tools::GetTime64() - Start, This->pSequence, This->pAggreg200MS.Timestamp);
  This->InitAggreg(This->pAggreg200MS, AGGREG_200MS);
  This->pBlockSamples     = 0;
  This->pBlockCyclesCount = 0;
  memset(This->pBlockCycles, 0, sizeof(pBlockCycles));
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
  for(uint16_t i = 0; i < This->pFFTSamples; i++) {
    Ch->OutPtr[i].Re = Ch->OutPtr[i].Re / This->pBlockSamples;
    Ch->OutPtr[i].Im = Ch->OutPtr[i].Im / This->pBlockSamples;
  }
}
void AnalyzerSoft::Alarms_Func(void *creation, void *run) {
  AnalyzerSoft *This   = (AnalyzerSoft*)creation;
  Params_t     *Aggreg = (Params_t*)run;
  for(Alarm_t &Alarm : This->pAlarms) {
    if(Alarm.Aggreg != Aggreg->Type) continue;
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
      LOGFile::Error("Alarm expresion evaluation failed on analyzer '%s'\n", This->Serial.c_str());
    }
    Alarm.ResultPos = (Alarm.ResultPos + 1) % Alarm.Window;
    if((Alarm.ResultCount == Alarm.Samples) && (Alarm.Triggered == false)) {
      Alarm.Triggered = true;
      LOGFile::Debug("Alarm triggered on analyzer '%s'\n", This->Serial.c_str());
      Database DB(This->Serial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(Aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", true, " + DB.Bind(Alarm.Priority) + ")");
      if(This->pMQTTAlarms == true) This->PublishAlarm(Aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
      Telegram::SendAlarm(This->Serial, This->pTimeZone, Aggreg->Timestamp, Alarm.Name, Alarm.Priority, true);
    } else if((Alarm.ResultCount == 0) && (Alarm.Triggered == true)) {
      Alarm.Triggered = false;
      LOGFile::Debug("Alarm reseted on analyzer '%s'\n", This->Serial.c_str());
      Database DB(This->Serial);
      DB.ExecSentenceNoResult("INSERT INTO alarms(alarmtime, name, trigger, priority) VALUES(" + DB.Bind(Aggreg->Timestamp) + ", " + DB.Bind(Alarm.Name) + ", false, " + DB.Bind(Alarm.Priority) + ")");
      if(This->pMQTTAlarms == true) This->PublishAlarm(Aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
      Telegram::SendAlarm(This->Serial, This->pTimeZone, Aggreg->Timestamp, Alarm.Name, Alarm.Priority, false);
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
        Sync_t *Request = (Sync_t*)calloc(1, sizeof(Sync_t));
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
          free(Request);
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
      if(This->pMQTTConfig == true) This->PublishConfig(CheckConfig(ConfigManager::ReadConfig(This->pConfigPath), This->pMode));
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
//HAGO COSAS
    }
    usleep(100000);


/*    if(true) {
      mqtt_init(&Client, Socket, SendBuff, BuffSize, RecvBuff, BuffSize, [](void** unused, struct mqtt_response_publish *published) {});

      if(Client.error != MQTT_OK) {
        LOGFile::Error("Analyzer '%s' MQTT broker error: %s\n", mqtt_error_str(Client.error));
        close(Socket);
        Socket = -1;
        continue;
      }
      pthread_mutex_lock(&This->pMQTTMutex);
      LOGFile::Debug("Analyzer '%s' logged on MQTT broker\n", This->Serial.c_str());
      This->pMQTTClient = &Client;
      pthread_mutex_unlock(&This->pMQTTMutex);
    } else {
    }*/
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
  if((params.contains("Alarms") == false) || (params["Alarms"].is_boolean() == false) || (params["Alarms"] == false)) Response.erase("Alarms");
  if((params.contains("Rates") == false) || (params["Rates"].is_boolean() == false) || (params["Rates"] == false)) Response.erase("Rates");
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
  Configure(CheckConfig(Config, pMode));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
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
                              "'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                              "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), "
                              "'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), "
                              "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), "
                              "'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), "
                              "'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}'), "
                              "'Notes', COALESCE(ARRAY_AGG(notes ORDER BY starttime ASC), '{}')) AS evt "
                              "FROM events WHERE starttime >= " + DB.Bind(From) + " AND starttime <= " + DB.Bind(To));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("evt"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Type") == false) || (params["Type"].is_string() == false)) return HTTPServer::FillResponse(response, "'Type' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false)) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
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
  uint32_t PeriodSeconds   = 900000;
  uint64_t BillFrom        = From;
  uint64_t BillTo          = To;
  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if(((To - From) / PeriodSeconds) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    if(params["Series"][i] == "POWER") {
      Result["Power"]               = json::object();
      Result["Power"]["Sampletime"] = json::array();
      Result["Power"]["Active"]     = json::array();
      Result["Power"]["Reactive"]   = json::array();
      Result["Power"]["Expenses"]   = json::array();
    } else if(params["Series"][i] == "PERIODS") {
      Result["Periods"] = json::array();
    } else if(params["Series"][i] == "BILLS") {
      Result["Bills"] = json::array();
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  Database DB(Serial);
  pAnalyzerLock.ReadLock();
  RateList Rates(pRates, pTimeZone, BillFrom, BillTo, params["Precise"]);
  pAnalyzerLock.Unlock();
  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + " ORDER BY sampletime ASC");
  bool  MoreData    = DB.FecthRow();
  float AccActive   = 0;
  float AccReactive = 0;
  float AccCost     = 0;
  for(uint64_t Time = From; Time != To; Time = Time + 900000) {
    float Active   = 0;
    float Reactive = 0;
    if((MoreData == true) && (DB.ResultUInt64("sampletime") == Time)) {
      Active   = DB.ResultFloat("active_power");
      Reactive = DB.ResultFloat("reactive_power");
      MoreData = DB.FecthRow();
    }
    bool BillDate = Rates.EvaluateSample(Time, Active, Reactive);
    //si quiero periodos y hay cambio de periodo añado a la lista
      //      string   Color      = "";
      //string   Period     = "";
      //uint64_t Time       = time;
      //float    Cost       = NAN;
      //float    Contracted = NAN;
    //Si quiero potencias añado a la lista {
      //float    Cost       = NAN;

    //}
    //si billdate == true
      //  Si quiero facturas y , añado factura
      //Reinicio contadores
    //



    //Recover/group values
//    do {
//      string   Color      = "";
//      string   Period     = "";
//      uint64_t Time       = time;
//      float    Cost       = NAN;
//      float    Contracted = NAN;
//      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
//        if(SaveBills == true) Result["Bills"].push_back(Tariff->GetResume(Now));
//        break;
//      }
//      if(Time <= Now) {
//        AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
//        AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
//        AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
//      }
//      if(Aggreg == "1H") {
//        if((Time % 3600000) != 3599999) {
//          if(Tariff != NULL) continue;
//          break;
//        }
//        AccActive = AccActive / 4.0;
//        AccReactive = AccReactive / 4.0;
//      }
//      if((Time >= From) && (Time <= To)) {
//        if(SaveTime == true)        Result["Sampletime"].push_back(Time);
//        if(SaveActive == true)      Result["Active_Power"].push_back(AccActive);
//        if(SaveReactive == true)    Result["Reactive_Power"].push_back(AccReactive);
//        if(SaveContracted == true)  Result["Period_Power"].push_back(Contracted);
//        if(SavePeriodColor == true) {
//          if(Color.empty() == true)
//            Result["Period_Color"].push_back(nullptr);
//          else
//            Result["Period_Color"].push_back(Color);
//        }
//        if(SavePeriodName == true) {
//          if(Period.empty() == true)
//            Result["Period_Name"].push_back(nullptr);
//          else
//            Result["Period_Name"].push_back(Period);
//        }
//        if(SaveExpense == true)     Result["Period_Expense"].push_back(AccCost);
//      }
//      AccActive = NAN;
//      AccReactive = NAN;
//      AccCost = NAN;
//    } while(Tariff != NULL);
  }



//  uint64_t Now             = Tools::GetTime64();
//  pAnalyzerLock.ReadLock();
//  TariffList Tariffs(pRates, pTimeZone);
//  pAnalyzerLock.Unlock();
  //Find start and end date to retrieve data
//  uint64_t BillingFrom = Tariffs.BillFrom(From);
//  uint64_t BillingTo = Tariffs.BillTo(To);



  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
//void AnalyzerSoft::GetPrices(const json &params, Response &response) {
//  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("From") == false) || (params["From"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  uint64_t To              = params["To"];
//  uint64_t From            = params["From"];
//  string   Aggreg          = params["Aggreg"];
//  json     Result          = json::object();
//  bool     SaveActive      = false;
//  bool     SaveReactive    = false;
//  bool     SavePeriodName  = false;
//  bool     SavePeriodColor = false;
//  bool     SaveContracted  = false;
//  bool     SaveExpense     = false;
//  bool     SaveBills       = false;
//  bool     SaveTime        = false;
//  uint64_t Now             = Tools::GetTime64();
//  for(uint8_t i = 0; i < params["Series"].size(); i++) {
//    if(params["Series"][i].is_string() == false) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
//    if(params["Series"][i] == "ACTIVE_POWER") {
//      SaveActive = true;
//    } else if(params["Series"][i] == "REACTIVE_POWER") {
//      SaveReactive = true;
//    } else if(params["Series"][i] == "PERIOD_NAME") {
//      SavePeriodName = true;
//    } else if(params["Series"][i] == "PERIOD_COLOR") {
//      SavePeriodColor = true;
//    } else if(params["Series"][i] == "PERIOD_POWER") {
//      SaveContracted = true;
//    } else if(params["Series"][i] == "PERIOD_EXPENSE") {
//      SaveExpense = true;
//    } else if(params["Series"][i] == "BILLS") {
//      SaveBills = true;
//    } else {
//      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
//    }
//  }
//  if(SaveActive || SaveReactive || SavePeriodName || SavePeriodColor || SaveContracted || SaveExpense) SaveTime = true;
//  if(From > To) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
//  if((Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
//  if(((To - From) / ((Aggreg == "1H") ? 3600000 : 900000)) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
//  if(SaveTime == true)        Result["Sampletime"] = json::array();
//  if(SaveActive == true)      Result["Active_Power"]   = json::array();
//  if(SaveReactive == true)    Result["Reactive_Power"] = json::array();
//  if(SavePeriodName == true)  Result["Period_Name"]    = json::array();
//  if(SavePeriodColor == true) Result["Period_Color"]   = json::array();
//  if(SaveContracted == true)  Result["Period_Power"]   = json::array();
//  if(SaveExpense == true)     Result["Period_Expense"] = json::array();
//  if(SaveBills == true)       Result["Bills"]          = json::array();
//  //Load involved tariffs, currency and timezone
//  Database DB(Serial);
////  DB.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + DB.Bind(From) + " AND startdate <= " + DB.Bind(To));
////  json DBTariffs = json::array();
////  if(DB.FecthRow()) DBTariffs = DB.ResultJSON("rate");
//  pAnalyzerLock.ReadLock();
//  TariffList Tariffs(pRates, pTimeZone);
//  pAnalyzerLock.Unlock();
//  //Find start and end date to retrieve data
//  uint64_t BillingFrom = Tariffs.BillFrom(From);
//  uint64_t BillingTo = Tariffs.BillTo(To);
//  //Retrieve active and reactive samples
//  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + DB.Bind(BillingFrom) + " AND sampletime <= " + DB.Bind(BillingTo) + " ORDER BY sampletime ASC");
//  bool MoreData = DB.FecthRow();
//  //Traverse all samples
//  float AccActive = NAN;
//  float AccReactive = NAN;
//  float AccCost = NAN;
//  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
//    float Active = NAN, Reactive = NAN;
//    if((MoreData == true) && (DB.ResultUInt64("sampletime") == time)) {
//      Active = DB.ResultFloat("active_power");
//      Reactive = DB.ResultFloat("reactive_power");
//      MoreData = DB.FecthRow();
//    }
//    Tariff *Tariff = Tariffs.Find(time);
//    if((Tariff != NULL) && (Tariff->PushSample(time, Active, Reactive) == true) && ((time + 900000) != BillingTo)) continue;  //If under a tariff push samples until bill cycle ends or no more data!
//    //Recover/group values
//    do {
//      string   Color      = "";
//      string   Period     = "";
//      uint64_t Time       = time;
//      float    Cost       = NAN;
//      float    Contracted = NAN;
//      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
//        if(SaveBills == true) Result["Bills"].push_back(Tariff->GetResume(Now));
//        break;
//      }
//      if(Time <= Now) {
//        AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
//        AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
//        AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
//      }
//      if(Aggreg == "1H") {
//        if((Time % 3600000) != 3599999) {
//          if(Tariff != NULL) continue;
//          break;
//        }
//        AccActive = AccActive / 4.0;
//        AccReactive = AccReactive / 4.0;
//      }
//      if((Time >= From) && (Time <= To)) {
//        if(SaveTime == true)        Result["Sampletime"].push_back(Time);
//        if(SaveActive == true)      Result["Active_Power"].push_back(AccActive);
//        if(SaveReactive == true)    Result["Reactive_Power"].push_back(AccReactive);
//        if(SaveContracted == true)  Result["Period_Power"].push_back(Contracted);
//        if(SavePeriodColor == true) {
//          if(Color.empty() == true)
//            Result["Period_Color"].push_back(nullptr);
//          else
//            Result["Period_Color"].push_back(Color);
//        }
//        if(SavePeriodName == true) {
//          if(Period.empty() == true)
//            Result["Period_Name"].push_back(nullptr);
//          else
//            Result["Period_Name"].push_back(Period);
//        }
//        if(SaveExpense == true)     Result["Period_Expense"].push_back(AccCost);
//      }
//      AccActive = NAN;
//      AccReactive = NAN;
//      AccCost = NAN;
//    } while(Tariff != NULL);
//  }
//  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
//}
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