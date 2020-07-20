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
#include <malloc.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>
#include <endian.h>
#include "cctz/time_zone.h"
#include "muParser/muParser.h"
#include "AnalyzerSoft.h"
#include "ConfigManager.h"
#include "LogFile.h"
#include "SysSupport.h"
#include "Telegram.h"
#include "AnalyzerSoftGA.h"
#include "URL.h"
#include "AnalyzerSoftAlarms.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3), pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pMode(mode) {
  Database DB(id);
  LogFile::Debug("Creating AnalyzerSoft database tables");
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFT[i] = new AnalyzerSoftFFT("FFT_V" + to_string(i));
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFT[i] = new AnalyzerSoftFFT("FFT_I" + to_string(i));
  pRates          = new AnalyzerSoftRates(pSerial);
  pAlarms         = new AnalyzerSoftAlarms(pSerial);
  pEvents         = new AnalyzerSoftEvents(pSerial);
  pSync           = new AnalyzerSoftSync();
  pMQTT           = new AnalyzerSoftMQTT();
  pInputBlock     = new AnalyzerSoftBlock(mode, samplerate);
  pFrequency      = new AnalyzerSoftFrequency(samplerate);
  pAggreg200MS    = new AnalyzerSoftAggreg200MS(pSerial, mode);
  pAggreg3S       = new AnalyzerSoftAggreg3S(pSerial, mode);
  pAggreg1M       = new AnalyzerSoftAggreg1M(pSerial, mode);
  pAggreg10M      = new AnalyzerSoftAggreg10M(pSerial, mode);
  pAggreg15M      = new AnalyzerSoftAggreg15M(pSerial, mode);
  pAggreg1H       = new AnalyzerSoftAggreg1H(pSerial, mode);
  pAnalyzerWorker = new Worker((mode == PHASE1) ? "Analyzer_1P" : (mode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N", [&](void *a) { this->Analyzer_Func(); });
  Configure(CheckConfig(ConfigManager::ReadConfig(optionPath), mode));
}
AnalyzerSoft::~AnalyzerSoft() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());

//  pSyncTerminate = true;
//  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
//  pMQTTTerminate = true;
//  if(pMQTTThread != 0) pthread_join(pMQTTThread, NULL);
  if(pAnalyzerWorker != NULL) delete pAnalyzerWorker;
//  if(pEventsWorker != NULL) delete pEventsWorker;
  if(pAlarms != NULL) delete pAlarms;

//  if(pPrevSamples != NULL) free(pPrevSamples);
//  if(pPrevCycles != NULL) free(pPrevCycles);
  for(uint8_t n = 0; n < pVoltageChannels; n++) delete pVoltageFFT[n];
  for(uint8_t n = 0; n < pCurrentChannels; n++) delete pCurrentFFT[n];
  delete pInputBlock;
  delete pRates;
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples, const uint32_t buffUsage) {
  pAnalyzerLock.ReadLock();
  if(pAggreg200MS->Timestamp() == 0) pAggreg200MS->setTimestamp(Tools::GetTime64() - (uint64_t)((buffUsage * 1000) / pSamplerate));
  AnalyzerSoftBlock::AppendResult Result = pInputBlock->PushSample(voltageSamples, currentSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) pVoltageFFT[n]->PushSample(voltageSamples[n]);
  for(uint8_t n = 0; n < pCurrentChannels; n++) pCurrentFFT[n]->PushSample(currentSamples[n]);
  pAnalyzerLock.Unlock();
  if(Result == AnalyzerSoftBlock::NONE) {
    return false;
  } else if(Result == AnalyzerSoftBlock::FULL_CYCLE) {
    pFrequency->PushCycle(pInputBlock->LastCycleLen());
    return false;
  }
  pFrequency->PushCycle(pInputBlock->LastCycleLen());
  pAnalyzerWorker->Run(NULL);
  return true;
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
//  ConfigOUT["Alarms"] = json::array();
//  ConfigOUT["Rates"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFrequency") && config["NominalFrequency"].is_number() && ((config["NominalFrequency"] == 50.0) || (config["NominalFrequency"] == 60.0) || (config["NominalFrequency"] == 0.0))) ConfigOUT["NominalFrequency"] = config["NominalFrequency"];
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
//  if(config.contains("Rates") && config["Rates"].is_array()) {
//    json Tmp = RatesEval::CheckConfig(config["Rates"]);
//    if(Tmp.is_string()) {
//      if(error != NULL) *error = Tmp;
//    } else {
//      ConfigOUT["Rates"] = Tmp;
//    }
//  }
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
    pFrequency->Config(pNominalFrequency);
    pSequence          = 0;
//    InitAggreg(pAggreg200MS, AGGREG_200MS);
    pInputBlock->Config(pNominalFrequency);
    for(uint8_t n = 0; n < pVoltageChannels; n++) pVoltageFFT[n]->Config((pNominalFrequency == 0.0) ? 1 : (pNominalFrequency == 50.0) ? 10 : 12);
    for(uint8_t n = 0; n < pCurrentChannels; n++) pCurrentFFT[n]->Config((pNominalFrequency == 0.0) ? 1 : (pNominalFrequency == 50.0) ? 10 : 12);
  }
  bool AlarmsEnabled = options["AlarmsEnabled"];
  bool EventsEnabled = options["EventsEnabled"];
  bool SyncEnabled = options["SyncEnabled"];
  if(SyncEnabled == true) {
//    pSyncServer    = options["Sync"]["Server"];
//    pSyncUser      = options["Sync"]["User"];
//    pSyncPass      = options["Sync"]["Pass"];
//    pSyncTerminate = false;
//    if(pSyncThread == 0) {
//      if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
//        LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
//        pSyncThread = 0;
//      } else {
//        pthread_setname_np(pSyncThread, "Sync");
//      }
//    }
//  } else if((SyncEnabled == false) && (pSyncThread != 0)) {
//    pSyncTerminate = true;
//    pthread_join(pSyncThread, NULL);
//    pSyncThread = 0;
  }
  bool MQTTEnabled = options["MQTTEnabled"];
  if(MQTTEnabled == true) {
//    pMQTTPrefix    = options["MQTT"]["Prefix"];
//    pMQTTServer    = options["MQTT"]["Server"];
//    pMQTTUser      = options["MQTT"]["User"];
//    pMQTTPass      = options["MQTT"]["Pass"];
//    pMQTT3S        = options["MQTT"]["Export3S"];
//    pMQTT1M        = options["MQTT"]["Export1M"];
//    pMQTT10M       = options["MQTT"]["Export10M"];
//    pMQTT15M       = options["MQTT"]["Export15M"];
//    pMQTT1H        = options["MQTT"]["Export1H"];
//    pMQTTAlarms    = options["MQTT"]["ExportAlarms"];
//    pMQTTConfig    = options["MQTT"]["ExportConfig"];
//    pMQTTTerminate = false;
//    if(pMQTTThread == 0) {
//      pMQTTSendBuff = (uint8_t*)calloc(pMQTTBuffSize, 1);
//      pMQTTRecvBuff = (uint8_t*)calloc(pMQTTBuffSize, 1);
//      mqtt_init_reconnect(&pMQTTClient, NULL, NULL, NULL);
//      if(pthread_create(&pMQTTThread, NULL, MQTT_Thread, this) != 0) {
//        LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
//        pMQTTThread = 0;
//      } else {
//        pthread_setname_np(pMQTTThread, "MQTT");
//      }
//    }
  } else if(MQTTEnabled == false) {
//    pMQTT3S        = false;
//    pMQTT1M        = false;
//    pMQTT10M       = false;
//    pMQTT15M       = false;
//    pMQTT1H        = false;
//    pMQTTAlarms    = false;
//    pMQTTConfig    = false;
//    if(pMQTTThread != 0) {
//      pthread_join(pMQTTThread, NULL);
//      free(pMQTTSendBuff);
//      free(pMQTTRecvBuff);
//      pMQTTThread = 0;
//    }
  }
  pAnalyzerLock.Unlock();
//  if(pMQTTConfig == true) PublishConfig(options);
}
void AnalyzerSoft::Analyzer_Func() {
  pAnalyzerLock.ReadLock();
  uint64_t Start     = Tools::GetTime64();
  uint16_t BlockSize = pInputBlock->BlockSamples();
//  pAggreg200MS.AggregSamples = 1;
//  if(pEventsWorker != NULL) pEventsWorker->Run(NULL);
  for(uint8_t i = 0; i < pVoltageChannels; i++) pVoltageFFT[i]->Run();
  for(uint8_t i = 0; i < pCurrentChannels; i++) pCurrentFFT[i]->Run();
  pAggreg200MS->setFrequency(pFrequency->Frequency());
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    float Voltage     = 0.0;
    float Current     = 0.0;
    float ActivePower = 0.0;
    float THDv        = 0.0;
    float THDi        = 0.0;
    for(uint16_t i = 0; i < BlockSize; i++) {
      Voltage     += pow(pInputBlock->VoltageSample(j, i), 2.0);
      Current     += pow(pInputBlock->CurrentSample(j, i), 2.0);
      ActivePower += pInputBlock->VoltageSample(j, i) * pInputBlock->CurrentSample(j, i);
    }
    pAggreg200MS->setVoltage(j, sqrt(Voltage / BlockSize));
    pAggreg200MS->setCurrent(j, sqrt(Current / BlockSize));
    pAggreg200MS->setActivePower(j, ActivePower / BlockSize);
    pAggreg200MS->setApparentPower(j, pAggreg200MS->Voltage(j) * pAggreg200MS->Current(j));
    pAggreg200MS->setActiveEnergy(j, (pAggreg200MS->ActivePower(j) * BlockSize) / (pSamplerate * 3600.0));
    pAggreg200MS->setPowerFactor(j, pAggreg200MS->ActivePower(j) / pAggreg200MS->ApparentPower(j));
    pAggreg200MS->setReactiveEnergy(j, (pAggreg200MS->ReactivePower(j) * BlockSize) / (pSamplerate * 3600.0));
    pVoltageFFT[j]->Wait();
    pCurrentFFT[j]->Wait();
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS->setVoltageHarmonic(j, i, pVoltageFFT[j]->HarmonicPower(i));
      if(i > 0) THDv += pVoltageFFT[j]->HarmonicPower(i);
      pAggreg200MS->setCurrentHarmonic(j, i, pCurrentFFT[j]->HarmonicPower(i));
      if(i > 0) THDi += pCurrentFFT[j]->HarmonicPower(i);
      pAggreg200MS->setPowerHarmonic(j, i, pVoltageFFT[j]->HarmonicPower(i) * pCurrentFFT[j]->HarmonicPower(i) * cos(AnalyzerSoftFFT::CalcAngle(pCurrentFFT[j]->Harmonic(i), pVoltageFFT[j]->Harmonic(i))));
    }
    pAggreg200MS->setVoltageTHD(j, (THDv / pAggreg200MS->VoltageHarmonic(j, 0)) * 100.0);
    pAggreg200MS->setCurrentTHD(j, (THDi / pAggreg200MS->CurrentHarmonic(j, 0)) * 100.0);
    pAggreg200MS->setVoltagePhase(j, (j == 0) ? 0.0 : AnalyzerSoftFFT::CalcAngle(pVoltageFFT[j]->Harmonic(0), pVoltageFFT[0]->Harmonic(0)) * 180.0 / M_PI);
    pAggreg200MS->setReactivePower(j, pVoltageFFT[j]->HarmonicPower(0) * pCurrentFFT[j]->HarmonicPower(0) * sin(-AnalyzerSoftFFT::CalcAngle(pCurrentFFT[j]->Harmonic(0), pVoltageFFT[j]->Harmonic(0))));
    pAggreg200MS->setPhi(j, AnalyzerSoftFFT::CalcAngle(pCurrentFFT[j]->Harmonic(0), pVoltageFFT[j]->Harmonic(0)) * 180.0 / M_PI);
  }
  if((pMode == PHASE3N) || (pMode == PHASE3)) {
    complex<float> A(cos(120.0 * M_PI / 180.0), sin(120.0 * M_PI / 180.0));
    complex<float> V1(pVoltageFFT[0]->Bin(10).Re, pVoltageFFT[0]->Bin(10).Im);
    complex<float> V2(pVoltageFFT[1]->Bin(10).Re, pVoltageFFT[1]->Bin(10).Im);
    complex<float> V3(pVoltageFFT[2]->Bin(10).Re, pVoltageFFT[2]->Bin(10).Im);
    pAggreg200MS->setUnbalance(((abs(V1 + A * A * V2 + A * V3) / 3.0) / (abs(V1 + A * V2 + A * A * V3) / 3.0)) * 100.0);
  }
  if(pMode == PHASE3N) {
    float Current = 0.0;
    float THD     = 0.0;
    for(uint16_t i = 0; i < BlockSize; i++) Current += pow(pInputBlock->CurrentSample(3, i), 2.0);
    pAggreg200MS->setCurrent(3, sqrt(Current / BlockSize));
    pCurrentFFT[3]->Wait();
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS->setCurrentHarmonic(3, i, pCurrentFFT[3]->HarmonicPower(i));
      if(i > 0) THD += pCurrentFFT[3]->HarmonicPower(i);
    }
    pAggreg200MS->setCurrentTHD(3, (THD / pAggreg200MS->CurrentHarmonic(3, 0)) * 100.0);
  }
//  if(pEventsWorker != NULL) pEventsWorker->Wait();
  if(pSequence < 100) {
    LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "", pSerial.c_str(), Tools::GetTime64() - Start, pSequence);
    pAggreg200MS->Reset();
    pSequence++;
    pAnalyzerLock.Unlock();
    return;
  }
  SetOnline();
  pNowDataLock.ProviderWrite();
  if((pSequence % 15) == 0) {
    LogFile::Debug("Storing 3S aggregation for analyzer '%s'", pSerial.c_str());
    pAggreg3S->Normalize();
    pAlarms->Run(pAggreg3S);
    pAggreg3S->Save();
    pAggreg1H->AccumulateStats(pAggreg3S);
    pMQTT->PublishAggreg(pAggreg3S);
    pAlarms->Wait();
    pAggreg3S->Reset();
  }
  pAggreg3S->Accumulate(pAggreg200MS);
  if((pAggreg1M->Timestamp() != 0) && ((pAggreg1M->Timestamp() / 60000) < (pAggreg200MS->Timestamp() / 60000))) {
    LogFile::Debug("Storing 1M aggregation for analyzer '%s'", pSerial.c_str());
    pAggreg1M->Normalize();
    pAlarms->Run(pAggreg1M);
    pAggreg1M->Save();
    pMQTT->PublishAggreg(pAggreg1M);
    pAlarms->Wait();
    pAggreg1M->Reset();
  }
  pAggreg1M->Accumulate(pAggreg200MS);
  if((pAggreg10M->Timestamp() != 0) && ((pAggreg10M->Timestamp() / 600000) < (pAggreg200MS->Timestamp() / 600000))) {
    LogFile::Debug("Storing 10M aggregation for analyzer '%s", pSerial.c_str());
    pAggreg10M->Normalize();
    pAlarms->Run(pAggreg10M);
    pAggreg1H->AccumulateStats(pAggreg10M);
    pAggreg10M->Save();
    pMQTT->PublishAggreg(pAggreg10M);
    pAlarms->Wait();
    pAggreg10M->Reset();
  }
  pAggreg10M->Accumulate(pAggreg200MS);
  if((pAggreg15M->Timestamp() != 0) && ((pAggreg15M->Timestamp() / 900000) < (pAggreg200MS->Timestamp() / 900000))) {
    LogFile::Debug("Storing 15M aggregation for analyzer '%s'", pSerial.c_str());
    pAggreg15M->Normalize();
    pAlarms->Run(pAggreg15M);
    pAggreg15M->Save();
    pMQTT->PublishAggreg(pAggreg15M);
    pAlarms->Wait();
    pAggreg15M->Reset();
  }
  pAggreg15M->Accumulate(pAggreg200MS);
  if((pAggreg1H->Timestamp() != 0) && ((pAggreg1H->Timestamp() / 3600000) < (pAggreg200MS->Timestamp() / 3600000))) {
    LogFile::Debug("Storing 1H aggregation for analyzer '%s'", pSerial.c_str());
    pAggreg1H->Normalize();
    pAlarms->Run(pAggreg1H);
    pAggreg1H->Save();
    pMQTT->PublishAggreg(pAggreg1H);
    pAlarms->Wait();
    pAggreg1H->Reset();
  }
  pAggreg1H->Accumulate(pAggreg200MS);
  pAlarms->Run(pAggreg200MS);
  if(pSave200MS) {
    LogFile::Debug("Storing 200MS aggregation for analyzer '%s'\n", pSerial.c_str());
    pAggreg200MS->Save();
  }
  pAlarms->Wait();

  //  SysSupport::SystemStatus_t *SysStatus = SysSupport::GetSystemStatus();
  //  SysStatus->Lock.WriteLock();
  //  SysStatus->Events = This->pAggreg200MS.Flag;
  //  SysStatus->Lock.Unlock();
  pNowDataLock.ProviderWait();
  LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (Sampletime: %" PRIu64 ")", pSerial.c_str(), Tools::GetTime64() - Start, pSequence, pAggreg200MS->Timestamp());
  pAggreg200MS->Reset();
  pSequence++;
  pAnalyzerLock.Unlock();
}

json AnalyzerSoft::CalcGA() {
  json Result = json::object();
  GAVector vGA(50 * 2 * pVoltageChannels);
  GAVector iGA(50 * 2 * pVoltageChannels);
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    for(uint8_t i = 0; i < 50; i++) {
      vGA.AddRectPhasor(i * 2, pVoltageFFT[j]->Harmonic(i).Re, pVoltageFFT[j]->Harmonic(i).Im);
      iGA.AddRectPhasor(i * 2, pCurrentFFT[j]->Harmonic(i).Re, pCurrentFFT[j]->Harmonic(i).Im);
    }
  }
  Result["Voltage"] = vGA.ToJSON();
  Result["Current"] = iGA.ToJSON();
  GABivector M = vGA * iGA;
  Result["Power"] = M.ToJSON();
//  float Ma = M.GetActiveTotal();
//  Result["Ma"] = Ma;
//  GaPoTNumBivector Mn = M.GetNonActivePart();
//  Result["Mn"] = Mn.ToString();
//  float Norm_M = M.Norm();
//  Result["Norm(M)"] = Norm_M;
//  float Norm_iGA = iGA.Norm();
//  Result["Norm(iGA)"] = Norm_iGA;
//  float Norm_vGA = vGA.Norm();
//  Result["Norm(vGA)"] = Norm_vGA;
//  float Norm_Ia = (vGA.Inverse() * Ma).Norm();
//  Result["Ia"] = Norm_Ia;
//  float Norm_In = (vGA.Inverse() * Mn).Norm();
//  Result["In"] = Norm_In;
//  auto PF = Ma / Norm_M;
//  Result["PF"] = PF;
  return Result;
}
void AnalyzerSoft::PeriodicTasks() {
  LogFile::Debug("Removing old data for analyzer '%s'", pSerial.c_str());
  Database DB(pSerial);
  uint64_t Now = Tools::GetTime64();
  if(Now > 86400000) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Now - 86400000));   //Delete 1 day old
  if(Now > 3600000)  DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(Now - 3600000));     //Delete 1 hour old
}
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  string UserName = params["UserName"];
  json Response = ConfigManager::ReadConfig(pConfigPath);
  Response["Online"] = IsOnline();
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  if((params.contains("Alarms") == false) || (params["Alarms"].is_boolean() == false) || (params["Alarms"] == false)) Response.erase("Alarms");
//  if((params.contains("Rates") == false) || (params["Rates"].is_boolean() == false) || (params["Rates"] == false)) {
//    Response.erase("Rates");
//  } else {
//    Response["RateModules"] = PriceSource::GetVarsInfo();
//  }
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT json_build_object('Read', read, 'Write', write, 'Configure', config, 'EventsRead', eventsread, 'AlarmsRead', alarmsread) as access FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  if(DB.FecthRow() == true) Response["Access"] = DB.ResultJSON("access");
  Response["DatabaseSize"] = json::array();
  DB.ExecSentenceAsyncResult("WITH sizes AS (SELECT tablename, pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename)) AS size from pg_tables WHERE schemaname = " + DB.Bind(pSerial) + ") "
    "SELECT 'Aggreg_3S' AS name, size FROM sizes WHERE tablename = 'aggreg_3s' UNION "
    "SELECT 'Aggreg_1M' AS name, SUM(size) FROM sizes WHERE tablename IN ('aggreg_1m', 'aggreg_ext_1m') UNION "
    "SELECT 'Aggreg_10M' AS name, size FROM sizes WHERE tablename = 'aggreg_10m' UNION "
    "SELECT 'Aggreg_15M' AS name, size FROM sizes WHERE tablename = 'aggreg_15m' UNION "
    "SELECT 'Aggreg_1H' AS name, size FROM sizes WHERE tablename = 'aggreg_1h' UNION "
    "SELECT 'Events_Stats' AS name, size FROM sizes WHERE tablename = 'events' UNION "
    "SELECT 'Events_Samples' AS name, size FROM sizes WHERE tablename = 'events_ext' UNION "
    "SELECT 'Alarms' AS name, size FROM sizes WHERE tablename = 'alarms'");
  while(DB.FecthRow()) {
    json Row = json::object();
    Row["Table"] = DB.ResultString("name");
    Row["Size"]  = DB.ResultUInt64("size");
    Response["DatabaseSize"].push_back(Row);
  }
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
//  if((Config.contains("Alarms") == false) || (Config["Alarms"].is_array() == false)) Config["Alarms"] = ConfigManager::ReadConfig(pConfigPath + "/Alarms");
//  if((Config.contains("Rates") == false) || (Config["Rates"].is_array() == false)) Config["Rates"] = ConfigManager::ReadConfig(pConfigPath + "/Rates");
  json   Conf = CheckConfig(Config, pMode);
//  if(Error.empty() == true) {
    Configure(Conf);
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
//  } else {
//    HTTPServer::FillResponse(response, Error, "text/plain", HTTP_BAD_REQUEST);
//  }
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
  Database DB(pSerial);
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
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) {
      pNowDataLock.ConsumerEnds();
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    if(pAggreg200MS->AppendToJSON(params["Series"][i], Result) == true) continue;
  }
  pNowDataLock.ConsumerEnds();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetWaveStream(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  typedef struct {
    uint64_t Block;
    bool     Current;
    bool     Voltage;
  } WaveStream_t;
  WaveStream_t *Params = (WaveStream_t *)calloc(sizeof(WaveStream_t), 1);
  Params->Block = 0;
  for(uint8_t i = 0; i < params["Series"].size(); i++) {
    if(params["Series"][i].is_string() == false) {
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    } else if(params["Series"][i] == "CURRENT_SAMPLES") {
      Params->Current = true;
    } else if(params["Series"][i] == "VOLTAGE_SAMPLES") {
      Params->Voltage = true;
    } else {
      return HTTPServer::FillResponse(response, "Unknown serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  response.set_header("Content-Type", "application/json");
  response.set_chunked_content_provider(
    [this, Params](uint64_t offset, DataSink &sink) {
      if(pNowDataLock.ConsumerWait(5) == false) {
        sink.done();
        return false;
      }
      Params->Block = (Params->Block == 0) ? pSequence : Params->Block + 1;
      if(Params->Block != pSequence) {
        pNowDataLock.ConsumerEnds();
        sink.done();
        return false;
      }
      json Result = json::object();
//      Result["Samples"] = pBlockSamples;
      if(Params->Voltage) {
        Result["Voltage_Samples"] = json::array();
        for(uint8_t n = 0; n < pVoltageChannels; n++) {
          Result["Voltage_Samples"].push_back(json::array());
//          for(uint16_t i = 0; i < pBlockSamples; i++) Result["Voltage_Samples"][n].push_back(pVoltageSamples[n * pMaxSamples + i]);
        }
      }
      if(Params->Current) {
        Result["Current_Samples"] = json::array();
        for(uint8_t n = 0; n < pCurrentChannels; n++) {
          Result["Current_Samples"].push_back(json::array());
//          for(uint16_t i = 0; i < pBlockSamples; i++) Result["Current_Samples"][n].push_back(pCurrentSamples[n * pMaxSamples + i]);
        }
      }
      pNowDataLock.ConsumerEnds();
      string Data = Result.dump() + "\n\n";
      sink.write(Data.c_str(), Data.size());
      return true;
    },
    [Params]() {
      free(Params);
    }
  );
}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoft::DelSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Aggreg = params["Aggreg"];
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetEvents(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->GetEvents(params, response);
}
void AnalyzerSoft::GetEventsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->GetEventsRange(params, response);
}
void AnalyzerSoft::DelEventsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->DelEventsRange(params, response);
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->GetEvent(params, response);
}
void AnalyzerSoft::SetEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->SetEvent(params, response);
}
void AnalyzerSoft::DelEvent(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents->DelEvent(params, response);
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->TestAlarm(params, response);
}
void AnalyzerSoft::ListAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->ListAlarms(params, response);
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarms(params, response);
}
void AnalyzerSoft::AddAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->AddAlarm(params, response);
}
void AnalyzerSoft::GetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarm(params, response);
}
void AnalyzerSoft::SetAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->SetAlarm(params, response);
}
void AnalyzerSoft::DelAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->DelAlarm(params, response);
}
void AnalyzerSoft::GetAlarmsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarmsRange(params, response);
}
void AnalyzerSoft::DelAlarmsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->DelAlarmsRange(params, response);
}
void AnalyzerSoft::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->CalcRate(params, response);
}
void AnalyzerSoft::GetRates(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->GetRates(params, response);
}
void AnalyzerSoft::AddRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->AddRate(params, response);
}
void AnalyzerSoft::GetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->GetRate(params, response);
}
void AnalyzerSoft::SetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->SetRate(params, response);
}
void AnalyzerSoft::DelRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->DelRate(params, response);
}
void AnalyzerSoft::GetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerSoft::SetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
bool AnalyzerSoft::HandleModbus(const uint8_t *buff, const uint16_t start, const uint8_t len) {
  uint8_t  Buffer[4096];
  uint8_t *Ptr = Buffer;
  auto Append_U32 = [&](const uint32_t val) {
    uint32_t *Dest = (uint32_t*)Ptr;
    *Dest = htobe32(val);
    Ptr += 4;
  };
  auto Append_U64 = [&](const uint64_t val) {
    uint64_t *Dest = (uint64_t*)Ptr;
    *Dest = htobe64(val);
    Ptr += 8;
  };
  auto Append_Float = [&](const float val) {
    uint32_t *Dest = (uint32_t*)Ptr;
    uint32_t *Val  = (uint32_t*)&val;
    *Dest = htobe32(*Val);
    Ptr += 4;
  };
  memset(Buffer, 0, sizeof(Buffer));
  strcpy((char*)Ptr, pSerial.c_str()); Ptr += 32;
  if(pNowDataLock.ConsumerWait(5) == false) return false;
//  Append_U32(pAggreg200MS.Timestamp / 1000);
//  Append_U64(pAggreg200MS.Timestamp);
//  Append_Float(pAggreg200MS.Frequency);
//  printf("%f - %f\n", pAggreg200MS.Frequency, pAggreg200MS.Voltage[0]);
//  if(pVoltageChannels > 1) Append_Float(pAggreg200MS.Unbalance);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Voltage[ch]);
//  for(uint8_t ch = 0; ch < pCurrentChannels; ch++) Append_Float(pAggreg200MS.Current[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Active_Power[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Reactive_Power[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Active_Energy[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Reactive_Energy[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Apparent_Power[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Power_Factor[ch]);
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Voltage_THD[ch]);
//  for(uint8_t ch = 0; ch < pCurrentChannels; ch++) Append_Float(pAggreg200MS.Current_THD[ch]);
  if(pVoltageChannels > 1) {
//    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Voltage_Phase[ch]);
  }
//  for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Phi[ch]);
  for(uint8_t h = 0; h < 50; h++) {
//    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Voltage_Harmonics[ch * 50 + h]);
  }
  for(uint8_t h = 0; h < 50; h++) {
//    for(uint8_t ch = 0; ch < pCurrentChannels; ch++) Append_Float(pAggreg200MS.Current_Harmonics[ch * 50 + h]);
  }
  for(uint8_t h = 0; h < 50; h++) {
//    for(uint8_t ch = 0; ch < pVoltageChannels; ch++) Append_Float(pAggreg200MS.Power_Harmonics[ch * 50 + h]);
  }
  pNowDataLock.ConsumerEnds();
  memcpy((void*)buff, &Buffer[start * 2], len * 2);
  return true;
}
//----------------------------------------------------------------------------------------------------------------------
//995