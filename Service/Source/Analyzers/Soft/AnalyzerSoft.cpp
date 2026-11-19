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
#include "ConfigFile.h"
#include "LogFile.h"
#include "MessageLoop.h"
#include "URL.h"
#include "PriceSource.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id, mode), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels(Analyzer::VoltageChannels(mode)), pCurrentChannels(Analyzer::CurrentChannels(mode)) {
  LogFile::Info("Starting AnalyzerSoft with serial '%s'...", pSerial.c_str());
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(pSerial, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
  pTerminate      = false;
  pWorkingBlock   = NULL;
  pSourceFreqSum  = 0.0;
  pSourceRateSum  = 0.0;
  pSourceCount    = 0;
  pConsumers      = new ConsumerQueue(16);
  pAccess         = new AnalyzerAccess(id);
  pZeroCross      = new AnalyzerSoftZeroCrossing(mode, samplerate);
  pFrequency      = new AnalyzerSoftFrequency(samplerate);
  pFFTs           = new AnalyzerSoftFFTs(mode);
  pAggreg200MS    = new AnalyzerSoftAggreg200MS(id, mode);
  pAggreg3S       = new AnalyzerSoftAggreg3S(id, mode);
  pAggreg1M       = new AnalyzerSoftAggreg1M(id, mode);
  pAggreg10M      = new AnalyzerSoftAggreg10M(id, mode);
  pAggreg15M      = new AnalyzerSoftAggreg15M(id, mode);
  pAggreg1H       = new AnalyzerSoftAggreg1H(id, mode);
  pRates          = new AnalyzerRates(id);
  pSeries         = new AnalyzerSeries(id, mode);
  pCyclicHistogram = new AnalyzerSoftCyclicHistogram(id); // Setup cyclic histogram manager
  pJoltAlgorithm  = new AnalyzerSoftJoltAlgorithm(id); // Setup jolt manager
  pEvents         = new AnalyzerSoftEvents(id, mode, samplerate, pCyclicHistogram);
  pMQTT           = new AnalyzerSoftMQTT(id);
  pAlarms         = new AnalyzerSoftAlarms(id, pMQTT);
  pSync           = new AnalyzerSoftSync(id, optionPath, mode, samplerate,
    [&](const string &name, const json &params) { HandleAction(name, params); },
    [&](const vector<string> &series) { return pAccess->GetStatus(series); },
    [&](const vector<string> &series) { return GetSeriesNow(series); }
  );
  if(pthread_create(&pThread, NULL, [](void *params) -> void* { ((AnalyzerSoft*)params)->Thread(); return NULL; }, this) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pThread = 0;
  } else {
    if(pthread_setname_np(pThread, "Analyzer") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    TaskPool::SetThreadProperties(pThread, TaskPool::ABOUTNORMAL);
  }
  Config(CheckConfig(ConfigFile::ReadConfig(optionPath), mode, false));
  LogFile::Info("AnalyzerSoft with serial '%s' started", pSerial.c_str());
}
AnalyzerSoft::~AnalyzerSoft() {
  LogFile::Info("Stopping AnalyzerSoft with serial '%s'", pSerial.c_str());
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
  pthread_join(pThread, NULL);
  while(pPendingBlocks.size() > 0) {
    delete pPendingBlocks.front();
    pPendingBlocks.pop_front();
  }
  delete pCyclicHistogram;
  delete pJoltAlgorithm;
  delete pSync;
  delete pAlarms;
  delete pMQTT;
  delete pEvents;
  delete pSeries;
  delete pRates;
  delete pAggreg1H;
  delete pAggreg15M;
  delete pAggreg10M;
  delete pAggreg1M;
  delete pAggreg3S;
  delete pAggreg200MS;
  delete pFFTs;
  delete pFrequency;
  delete pZeroCross;
  delete pAccess;
  delete pConsumers;
  LogFile::Info("AnalyzerSoft with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerSoft::BufferAppend(const float *voltageSamples, const float *currentSamples) {
  AnalyzerSoftBlock *Result = pZeroCross->PushSample(voltageSamples, currentSamples);
  if(Result == NULL) return;
  pthread_mutex_lock(&pMutex);
  pthread_cond_signal(&pCond);
  pPendingBlocks.push_back(Result);
  pthread_mutex_unlock(&pMutex);
}
// Used by sources that measure grid frequency themselves and/or sample at a rate that follows the grid frequency (ex:
// fixed samples per period). Frequency and samplerate are averaged over each block and attached to it, so frequency
// and energy calculations do not rely on the nominal samplerate. Must be called from a single thread (the device one).
void AnalyzerSoft::BufferAppend(const float *voltageSamples, const float *currentSamples, const float frequency, const float samplerate) {
  pSourceFreqSum += frequency;
  pSourceRateSum += samplerate;
  pSourceCount++;
  AnalyzerSoftBlock *Result = pZeroCross->PushSample(voltageSamples, currentSamples);
  if(Result == NULL) return;
  Result->SourceInfo(pSourceFreqSum / pSourceCount, pSourceRateSum / pSourceCount);
  pSourceFreqSum = 0.0;
  pSourceRateSum = 0.0;
  pSourceCount   = 0;
  pthread_mutex_lock(&pMutex);
  pthread_cond_signal(&pCond);
  pPendingBlocks.push_back(Result);
  pthread_mutex_unlock(&pMutex);
}
json AnalyzerSoft::CheckConfig(const json &config, const Analyzer::AnalyzerMode mode, const bool except) {
  json ConfigOUT = json::object();
  ConfigOUT["Name"] = "Unnamed analyzer";
  ConfigOUT["NominalVoltage"] = 230.0;
  ConfigOUT["NominalFrequency"] = 50.0;
  ConfigOUT["MaxCurrent"] = 35.0;
  ConfigOUT["MinCurrent"] = 0.1;
  if(mode == Analyzer::PHASE1) {
    ConfigOUT["PhaseNames"] = json::array();
    ConfigOUT["PhaseNames"].push_back("L");
  } else if((mode == Analyzer::PHASE2) || (mode == Analyzer::PHASE2N)) {
    ConfigOUT["PhaseNames"] = json::array();
    ConfigOUT["PhaseNames"].push_back("R");
    ConfigOUT["PhaseNames"].push_back("S");
    if(mode == Analyzer::PHASE2N) ConfigOUT["NeutralName"] = "N";
  } else if((mode == Analyzer::PHASE3) || (mode == Analyzer::PHASE3N)) {
    ConfigOUT["PhaseNames"] = json::array();
    ConfigOUT["PhaseNames"].push_back("R");
    ConfigOUT["PhaseNames"].push_back("S");
    ConfigOUT["PhaseNames"].push_back("T");
    if(mode == Analyzer::PHASE3N) ConfigOUT["NeutralName"] ="N";
  }
  ConfigOUT["TimeZone"] = "UTC";
  ConfigOUT["Tags"] = json::object();
  ConfigOUT["Sync"] = AnalyzerSoftSync::CheckConfig(json::object());
  ConfigOUT["MQTT"] = AnalyzerSoftMQTT::CheckConfig(json::object());
  ConfigOUT["CyclicHistogram"] = AnalyzerSoftCyclicHistogram::CheckConfig(json::object());
  ConfigOUT["Events"] = AnalyzerSoftEvents::CheckConfig(json::object());
  ConfigOUT["Save200MS"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
  ConfigOUT["Alarms"] = json::array();
  ConfigOUT["Rates"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFrequency") && config["NominalFrequency"].is_number() && ((config["NominalFrequency"] == 50.0) || (config["NominalFrequency"] == 60.0) || (config["NominalFrequency"] == 0.0))) ConfigOUT["NominalFrequency"] = config["NominalFrequency"];
  if(config.contains("MaxCurrent") && config["MaxCurrent"].is_number() && (config["MaxCurrent"] >= 1) && (config["MaxCurrent"] <= 100000)) ConfigOUT["MaxCurrent"] = config["MaxCurrent"];
  if(config.contains("MinCurrent") && config["MinCurrent"].is_number() && (config["MinCurrent"] >= 0.01) && (config["MinCurrent"] <= 100)) ConfigOUT["MinCurrent"] = config["MinCurrent"];
  if(config.contains("PhaseNames") && config["PhaseNames"].is_array() && (config["PhaseNames"].size() == ConfigOUT["PhaseNames"].size())) {
    for(uint8_t n = 0; n < ConfigOUT["PhaseNames"].size(); n++) {
      if(config["PhaseNames"][n].is_string() && (config["PhaseNames"][n].size() < 3) && (config["PhaseNames"][n].size() > 0)) ConfigOUT["PhaseNames"][n] = config["PhaseNames"][n];
    }
  }
  if(config.contains("NeutralName") && config["NeutralName"].is_string() && (config["NeutralName"].size() < 3) && (config["NeutralName"].size() > 0) && ConfigOUT.contains("NeutralName")) ConfigOUT["NeutralName"] = config["NeutralName"];
  if(config.contains("TimeZone") && config["TimeZone"].is_string()) {
    time_zone TimeZone;
    string TZ = config["TimeZone"];
    if(cctz::load_time_zone(TZ, &TimeZone) == true) ConfigOUT["TimeZone"] = TZ;
  }
  if(config.contains("Tags") && config["Tags"].is_object()) {
    for(auto Tag = config["Tags"].begin(); Tag != config["Tags"].end(); Tag++) {
      if(Tag.value().is_string() == true) ConfigOUT["Tags"][Tag.key()] = Tag.value();
    }
  }
  if(config.contains("Sync")) ConfigOUT["Sync"] = AnalyzerSoftSync::CheckConfig(config["Sync"]);
  if(config.contains("MQTT")) ConfigOUT["MQTT"] = AnalyzerSoftMQTT::CheckConfig(config["MQTT"]);
  if(config.contains("CyclicHistogram")) ConfigOUT["CyclicHistogram"] = AnalyzerSoftCyclicHistogram::CheckConfig(config["CyclicHistogram"]);
  if(config.contains("Events")) ConfigOUT["Events"] = AnalyzerSoftEvents::CheckConfig(config["Events"]);
  if(config.contains("Save200MS") && config["Save200MS"].is_boolean()) ConfigOUT["Save200MS"] = config["Save200MS"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
  if(config.contains("Alarms") && config["Alarms"].is_array()) ConfigOUT["Alarms"] = AnalyzerSoftAlarms::CheckConfig(config["Alarms"]);
  if(config.contains("Rates") && config["Rates"].is_array()) ConfigOUT["Rates"] = AnalyzerRates::CheckConfig(config["Rates"], except);
  return ConfigOUT;
}
void AnalyzerSoft::Config(const json &options) {
  ConfigFile::WriteConfig(pConfigPath, options);
  State(SYNCING);
  Name(options["Name"]);
  pthread_mutex_lock(&pMutex);
  pNominalVoltage   = options["NominalVoltage"];
  pNominalFrequency = options["NominalFrequency"];
  pSave200MS        = options["Save200MS"];
  pZeroCross->Config(pNominalVoltage, pNominalFrequency);
  pAggreg1H->Config(pNominalVoltage, pNominalFrequency);
  pRates->Config(options["TimeZone"], options["Rates"]);
  pAlarms->Config(options["AlarmsEnabled"], options["TimeZone"], options["Alarms"], pNominalVoltage, pNominalFrequency);
  pEvents->Config(options["Events"], pNominalVoltage, pNominalFrequency);
  pSync->Config(options["Sync"]);
  pMQTT->Config(options);
  pCyclicHistogram->SetConfig(options["CyclicHistogram"]);
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoft::Thread() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(pTerminate == false) {
    if(pPendingBlocks.size() == 0) {
      if(pthread_cond_wait(&pCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      continue;
    }
    pWorkingBlock = pPendingBlocks.front();
    pPendingBlocks.pop_front();
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pCyclicHistogram->AddPoints(pWorkingBlock, (int)pSamplerate);
    pJoltAlgorithm->AddPoints(pWorkingBlock);

    uint64_t Start     = Tools::GetTime64();
    uint16_t BlockSize = pWorkingBlock->Samples();
    float    Samplerate = isnan(pWorkingBlock->SourceSamplerate()) ? pSamplerate : pWorkingBlock->SourceSamplerate();
    pAggreg200MS->Reset();
    pAggreg200MS->setTimestamp(pWorkingBlock->Timestamp());
    pFFTs->Run(pWorkingBlock, (pWorkingBlock->NominalFrequency() == 0.0) ? 0 : (pWorkingBlock->NominalFrequency() == 50.0) ? 10 : 12);
    pEvents->Run(pWorkingBlock);
    float    Frequency = pFrequency->Frequency(pWorkingBlock);
    if(!isnan(pWorkingBlock->SourceFrequency()) && (pWorkingBlock->NominalFrequency() != 0.0)) Frequency = pWorkingBlock->SourceFrequency();
    pAggreg200MS->setFrequency(Frequency);
    for(uint8_t j = 0; j < pVoltageChannels; j++) {
      float Voltage     = 0.0;
      float Current     = 0.0;
      float ActivePower = 0.0;
      for(uint16_t i = 0; i < BlockSize; i++) {
        Voltage     += pWorkingBlock->VoltageSample(j, i) * pWorkingBlock->VoltageSample(j, i);
        Current     += pWorkingBlock->CurrentSample(j, i) * pWorkingBlock->CurrentSample(j, i);
        ActivePower += pWorkingBlock->VoltageSample(j, i) * pWorkingBlock->CurrentSample(j, i);
      }
      pAggreg200MS->setVoltage(j, sqrt(Voltage / BlockSize));
      pAggreg200MS->setCurrent(j, sqrt(Current / BlockSize));
      pAggreg200MS->setActivePower(j, ActivePower / BlockSize);
      pAggreg200MS->setApparentPower(j, pAggreg200MS->Voltage(j) * pAggreg200MS->Current(j));
      pAggreg200MS->setActiveEnergy(j, (pAggreg200MS->ActivePower(j) * BlockSize) / (Samplerate * 3600.0));
      pAggreg200MS->setPowerFactor(j, pAggreg200MS->ActivePower(j) / pAggreg200MS->ApparentPower(j));
      pAggreg200MS->setReactiveEnergy(j, (pAggreg200MS->ReactivePower(j) * BlockSize) / (Samplerate * 3600.0));
    }
    pFFTs->Wait();
    for(uint8_t j = 0; j < pVoltageChannels; j++) {
      float THDv        = 0.0;
      float THDi        = 0.0;
      for(uint8_t i = 0; i < 50; i++) {
        pAggreg200MS->setVoltageHarmonic(j, i, pFFTs->VoltageHarmonic(j, i));
        if(i > 0) THDv += pFFTs->VoltageHarmonicPower(j, i);
        pAggreg200MS->setCurrentHarmonic(j, i, pFFTs->CurrentHarmonic(j, i));
        if(i > 0) THDi += pFFTs->CurrentHarmonicPower(j, i);
        pAggreg200MS->setPowerHarmonic(j, i, pFFTs->VoltageHarmonicPower(j, i) * pFFTs->CurrentHarmonicPower(j, i) * cos(AnalyzerSoftFFTs::CalcAngle(pFFTs->CurrentHarmonic(j, i), pFFTs->VoltageHarmonic(j, i))));
      }
      pAggreg200MS->setVoltageTHD(j, (THDv / pAggreg200MS->VoltageHarmonic(j, 0)) * 100.0);
      pAggreg200MS->setCurrentTHD(j, (THDi / pAggreg200MS->CurrentHarmonic(j, 0)) * 100.0);
      pAggreg200MS->setVoltagePhase(j, (j == 0) ? 0.0 : AnalyzerSoftFFTs::CalcAngle(pFFTs->VoltageHarmonic(j, 0), pFFTs->VoltageHarmonic(0, 0)) * 180.0 / M_PI);
      pAggreg200MS->setReactivePower(j, pFFTs->VoltageHarmonicPower(j, 0) * pFFTs->CurrentHarmonicPower(j, 0) * sin(-AnalyzerSoftFFTs::CalcAngle(pFFTs->CurrentHarmonic(j, 0), pFFTs->VoltageHarmonic(j, 0))));
      pAggreg200MS->setPhi(j, AnalyzerSoftFFTs::CalcAngle(pFFTs->CurrentHarmonic(j, 0), pFFTs->VoltageHarmonic(j, 0)) * 180.0 / M_PI);
    }
    if((pMode == PHASE3N) || (pMode == PHASE3)) {
      complex<float> A(cos(120.0 * M_PI / 180.0), sin(120.0 * M_PI / 180.0));
      complex<float> V1(pFFTs->VoltageHarmonic(0, 0).Re, pFFTs->VoltageHarmonic(0, 0).Im);
      complex<float> V2(pFFTs->VoltageHarmonic(1, 0).Re, pFFTs->VoltageHarmonic(1, 0).Im);
      complex<float> V3(pFFTs->VoltageHarmonic(2, 0).Re, pFFTs->VoltageHarmonic(2, 0).Im);
      pAggreg200MS->setUnbalance(((abs(V1 + A * A * V2 + A * V3) / 3.0) / (abs(V1 + A * V2 + A * A * V3) / 3.0)) * 100.0);
    }
    if((pMode == PHASE2N) || (pMode == PHASE3N)) {
      float Current = 0.0;
      float THD     = 0.0;
      for(uint16_t i = 0; i < BlockSize; i++) Current += pWorkingBlock->CurrentSample(pCurrentChannels - 1, i) * pWorkingBlock->CurrentSample(pCurrentChannels - 1, i);
      pAggreg200MS->setCurrent(pCurrentChannels - 1, sqrt(Current / BlockSize));
      for(uint8_t i = 0; i < 50; i++) {
        pAggreg200MS->setCurrentHarmonic(pCurrentChannels - 1, i, pFFTs->CurrentHarmonic(pCurrentChannels - 1, i));
        if(i > 0) THD += pFFTs->CurrentHarmonicPower(pCurrentChannels - 1, i);
      }
      pAggreg200MS->setCurrentTHD(pCurrentChannels - 1, (THD / pAggreg200MS->CurrentHarmonic(pCurrentChannels - 1, 0)) * 100.0);
    }
    pAggreg200MS->setFlag(pEvents->Wait());
    pConsumers->Write();
    if(pWorkingBlock->Sequence() < 100) {
      LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "", pSerial.c_str(), Tools::GetTime64() - Start, pWorkingBlock->Sequence());
    } else {
      State(ONLINE);
      if((pWorkingBlock->Sequence() % 15) == 0) {
        LogFile::Debug("Storing 3S aggregation for analyzer '%s'", pSerial.c_str());
        if(pAggreg3S->Normalize() == true) {
          pAlarms->Run("3S", pAggreg3S);
          pAggreg3S->Save();
          pAggreg1H->AccumulateStats(pAggreg3S);
          pMQTT->PublishAggreg("3S", pAggreg3S);
          pAlarms->Wait();
        }
        pAggreg3S->Reset();
      }
      pAggreg3S->Accumulate(pAggreg200MS);
      if((pAggreg1M->Timestamp() != 0) && ((pAggreg1M->Timestamp() / 60000) < (pAggreg200MS->Timestamp() / 60000))) {
        LogFile::Debug("Storing 1M aggregation for analyzer '%s'", pSerial.c_str());
        if(pAggreg1M->Normalize() == true) {
          pAlarms->Run("1M", pAggreg1M);
          pAggreg1M->Save();
          pMQTT->PublishAggreg("1M", pAggreg1M);
          pAlarms->Wait();
        }
        pAggreg1M->Reset();
      }
      pAggreg1M->Accumulate(pAggreg200MS);
      if((pAggreg10M->Timestamp() != 0) && ((pAggreg10M->Timestamp() / 600000) < (pAggreg200MS->Timestamp() / 600000))) {
        LogFile::Debug("Storing 10M aggregation for analyzer '%s", pSerial.c_str());
        if(pAggreg10M->Normalize() == true) {
          pAlarms->Run("10M", pAggreg10M);
          pAggreg1H->AccumulateStats(pAggreg10M);
          pAggreg10M->Save();
          pMQTT->PublishAggreg("10M", pAggreg10M);
          pAlarms->Wait();
        }
        pAggreg10M->Reset();
      }
      pAggreg10M->Accumulate(pAggreg200MS);
      if((pAggreg15M->Timestamp() != 0) && ((pAggreg15M->Timestamp() / 900000) < (pAggreg200MS->Timestamp() / 900000))) {
        LogFile::Debug("Storing 15M aggregation for analyzer '%s'", pSerial.c_str());
        if(pAggreg15M->Normalize() == true) {
          pAlarms->Run("15M", pAggreg15M);
          pAggreg15M->Save();
          pMQTT->PublishAggreg("15M", pAggreg15M);
          pAlarms->Wait();
        }
        pAggreg15M->Reset();
      }
      pAggreg15M->Accumulate(pAggreg200MS);
      if((pAggreg1H->Timestamp() != 0) && ((pAggreg1H->Timestamp() / 3600000) < (pAggreg200MS->Timestamp() / 3600000))) {
        LogFile::Debug("Storing 1H aggregation for analyzer '%s'", pSerial.c_str());
        if(pAggreg1H->Normalize() == true) {
          pAlarms->Run("1H", pAggreg1H);
          pAggreg1H->Save();
          pMQTT->PublishAggreg("1H", pAggreg1H);
          pAlarms->Wait();
        }
        pAggreg1H->Reset();
      }
      pAggreg1H->Accumulate(pAggreg200MS);
      pAlarms->Run("200MS", pAggreg200MS);
      if(pSave200MS) {
        LogFile::Debug("Storing 200MS aggregation for analyzer '%s'", pSerial.c_str());
        pAggreg200MS->Save();
      }
      pMQTT->PublishAggreg("200MS", pAggreg200MS);
      pAlarms->Wait();
      uint64_t TotalTime = Tools::GetTime64() - Start;
      if(TotalTime > 200) {
        LogFile::Warning("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (%d pending blocks)", pSerial.c_str(), TotalTime, pWorkingBlock->Sequence(), pPendingBlocks.size());
      } else {
        LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (%d pending blocks)", pSerial.c_str(), TotalTime, pWorkingBlock->Sequence(), pPendingBlocks.size());
      }
    }
    pConsumers->Wait();
    delete pWorkingBlock;
    if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void AnalyzerSoft::ResultsReady(AnalyzerSoftAggreg200MS *params) {
}
json AnalyzerSoft::GetSeriesNow(const vector<string> &series) {
  json Result = json::object();
  auto Func = [&]() {
    Result["Time"] = pAggreg200MS->Timestamp();
    for(auto &Serie : series) {
      if(pWorkingBlock->GetSeriesJSON(Serie, Result) == true) continue;
      if(pFFTs->GetSeriesJSON(Serie, Result) == true) continue;
      if(pAggreg200MS->GetSeriesJSON(Serie, Result) == true) continue;
    }
    return false;
  };
  if(pConsumers->OnWrite(Func) == false) return nullptr;
  return Result;
}
void AnalyzerSoft::HandleAction(const string &name, const json &params) {
  if(name == "SetAnalyzer") {
    if(params.contains("Users")) pAccess->Config(AnalyzerAccess::CheckConfig(params["Users"]));
    try {
      Config(CheckConfig(params, pMode, true));
    } catch(const string &err) {
      return LogFile::Warning("AnayzerSoft action from AnalyzerRemote '%s' (%s -> %s) fails", pSerial.c_str(), name.c_str(), err.c_str());
    }
  } else {
    HTTPRequest Req(params);
    if(name == "SetSeries")           pSeries->SetSeries(Req);
    else if(name == "DelSeriesRange") pSeries->DelSeriesRange(Req);
    else if(name == "SetEvents")      pEvents->SetEvents(Req);
    else if(name == "DelEventsRange") pEvents->DelEventsRange(Req);
    else if(name == "DelEvent")       pEvents->DelEvent(Req);
    else if(name == "SetAlarms")      pAlarms->SetAlarms(Req);
    else if(name == "SetAlarm")       pAlarms->SetAlarm(Req);
    else if(name == "DelAlarm")       pAlarms->DelAlarm(Req);
    else if(name == "DelAlarmsRange") pAlarms->DelAlarmsRange(Req);
    else {
      LogFile::Warning("AnayzerSoft receive invalid action from AnalyzerRemote '%s' (%s)", pSerial.c_str(), name.c_str());
      return;
    }
    if(Req.Status() != HTTP_OK) {
      if(Req.ErrorBody().empty() == false)
        LogFile::Warning("AnayzerSoft action from AnalyzerRemote '%s' (%s -> (%u) %s) fails", pSerial.c_str(), name.c_str(), Req.Status(), Req.ErrorBody().c_str());
      else
        LogFile::Warning("AnayzerSoft action from AnalyzerRemote '%s' (%s -> %u) fails", pSerial.c_str(), name.c_str(), Req.Status());
    }
  }
}
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  json   Response = ConfigFile::ReadConfig(pConfigPath);
  Response["User"]         = pOwner;
  Response["Mode"]         = ModeToString(pMode);
  Response["SampleRate"]   = pSamplerate;
  Response["PriceModules"] = PriceSource::VarsInfo();
  Response["Users"]        = pAccess->Users();
  req.Success(Response);
}
void AnalyzerSoft::SetAnalyzer(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::CONFIG, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  json Users = nullptr;
  if(req.CheckParamArray("Users")) Users = AnalyzerAccess::CheckConfig(req.Param("Users"));
  try {
    Config(CheckConfig(req.Params(), pMode, true));
    if(Users != nullptr) pAccess->Config(Users);
    req.Success(json::object());
  } catch(const string &err) {
    req.Error(err, HTTP_BAD_REQUEST);
  }
}
void AnalyzerSoft::GetSeries(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pSeries->GetSeries(req);
}
void AnalyzerSoft::GetSeriesNow(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamSet("Series", {"Frequency", "Flag", "Unbalance", "Voltage", "Current", "Active_Power", "Reactive_Power", "Active_Energy", "Reactive_Energy", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Phase", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Voltage_FFT", "Current_FFT", "Voltage_Samples", "Current_Samples"})) return;
  json Result = GetSeriesNow(req.Param("Series"));
  if(Result == nullptr) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  req.Success(Result);
}
void AnalyzerSoft::GetCyclicHistogramConfig(HTTPRequest &req){
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);

  json result = pCyclicHistogram->GetConfig();
  if(result == nullptr) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  req.Success(result);
}
void AnalyzerSoft::SetCyclicHistogramConfig(HTTPRequest &req){
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pCyclicHistogram->SetConfig(AnalyzerSoftCyclicHistogram::CheckConfig(req.Params()));
  req.Success(json::object());
}
void AnalyzerSoft::GetCyclicHistogramData(HTTPRequest &req)
{
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);

  string ID = "";
  string mode = "";

  if (req.Params().contains("ID"))
    ID = req.Params()["ID"];

  if (req.Params().contains("Mode"))
    mode = req.Params()["Mode"];

  json result = json::object();

  if (ID == "")
  {
    // Fetch current (in-memory) cyclic histogram data
    result = pCyclicHistogram->GetCurrentCyclicHistogram(mode);
  }
  else
  {
    // Fetch cyclic histogram for the specified event
    result = pCyclicHistogram->GetCyclicHistogramForEvent(ID);
  }

  if(result == nullptr) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  req.Success(result);
}
void AnalyzerSoft::ClearCyclicHistogramData(HTTPRequest &req)
{
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pCyclicHistogram->Clear();
  req.Success(json::object());
}
void AnalyzerSoft::GetJoltData(HTTPRequest &req)
{
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);

  if (!req.Params().contains("StartTime") || !req.Params()["StartTime"].is_number() || !req.Params().contains("EndTime") || !req.Params()["EndTime"].is_number()){
    req.Error("StartTime and EndTime parameters are required.", HTTP_BAD_REQUEST);
    return;
  }

  uint64_t startTime = req.Params()["StartTime"];
  uint64_t endTime = req.Params()["EndTime"];

  json result = pJoltAlgorithm->GetJoltData(startTime, endTime);

  if(result == nullptr) return req.Error("", HTTP_METHOD_NOT_ALLOWED);
  req.Success(result);
  // return HTTPServer::FillResponse(response, result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetStatus(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamSet("Series", {"Version", "Log", "System", "SystemLog", "Events", "Alarms", "Storage"})) return;
  json Result = pAccess->GetStatus(req.Param("Series"), req.UserName());
  Result["State"] = (State() == UNCONFIGURED) ? "UNCONFIGURED" : (State() == OFFLINE) ? "OFFLINE" : (State() == SYNCING) ? "SYNCING" : "ONLINE";
  req.Success(Result);
}
void AnalyzerSoft::GetWaveStream(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamSet("Series", {"Voltage_Samples", "Current_Samples"})) return;
  if(!req.CheckParamNumber("Length")) return;
  if(!req.CheckParamNumber("Downsample") || (req.Param("Downsample") < 1)) return;
  vector<string> Headers    = req.Param("Series");
  uint8_t        Downsample = req.Param("Downsample").get<uint8_t>();
  uint64_t       Length     = req.Param("Length").get<float>() * pSamplerate / Downsample;
  auto Func = [&, Headers, Length, Downsample](function<bool(const json &content)> Write) {
    Write(Headers);
    float Voltage[3], Current[4];
    uint8_t  Count   = 0;
    uint32_t Pending = Length;
    auto Func2 = [&]() {
      for(uint16_t i = 0; i < pWorkingBlock->Samples(); i++) {
        if(Count == 0) {
          for(uint8_t n = 0; n < pVoltageChannels; n++) Voltage[n] = 0.0;
          for(uint8_t n = 0; n < pCurrentChannels; n++) Current[n] = 0.0;
        }
        for(uint8_t n = 0; n < pVoltageChannels; n++) Voltage[n] += pWorkingBlock->VoltageSample(n, i);
        for(uint8_t n = 0; n < pCurrentChannels; n++) Current[n] += pWorkingBlock->CurrentSample(n, i);
        Count++;
        if(Count == Downsample) {
          Count = 0;
          json Result = json::array();
          for(auto &Serie : Headers) {
            if(Serie == "Voltage_Samples") {
              json Tmp = json::array();
              for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(Voltage[n] / Downsample);
              Result.push_back(Tmp);
            }
            if(Serie == "Current_Samples") {
              json Tmp = json::array();
              for(uint8_t n = 0; n < pCurrentChannels; n++) Tmp.push_back(Current[n] / Downsample);
              Result.push_back(Tmp);
            }
          }
          bool Res = Write(Result);
          Pending--;
          if((Pending == 0) || (Res == false)) return false;
        }
      }
      return true;
    };
    pConsumers->OnWrite(Func2);
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerSoft::GetSeriesRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pSeries->GetSeriesRange(req);
}
void AnalyzerSoft::DelSeriesRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pSeries->DelSeriesRange(req);
  if(req.Status() == HTTP_OK) pSync->Push("DelSeriesRange", req.Params());
}
void AnalyzerSoft::GetEvents(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pEvents->GetEvents(req);
}
void AnalyzerSoft::GetEventsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pEvents->GetEventsRange(req);
}
void AnalyzerSoft::DelEventsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->DelEventsRange(req);
  if(req.Status() == HTTP_OK) pSync->Push("DelEventsRange", req.Params());
}
void AnalyzerSoft::GetEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pEvents->GetEvent(req);
}
void AnalyzerSoft::SetEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->SetEvent(req);
  if(req.Status() == HTTP_OK) pSync->Push("SetEvent", req.Params());
}
void AnalyzerSoft::DelEvent(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->DelEvent(req);
  if(req.Status() == HTTP_OK) pSync->Push("DelEvent", req.Params());
}
void AnalyzerSoft::TestAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  auto Func = [&]() {
    pAlarms->TestAlarm(req, pAggreg200MS);
    return false;
  };
  if(pConsumers->OnWrite(Func) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetAlarms(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pAlarms->GetAlarms(req);
}
void AnalyzerSoft::GetAlarmsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pAlarms->GetAlarmsRange(req);
}
void AnalyzerSoft::DelAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->DelAlarm(req);
  if(req.Status() == HTTP_OK) pSync->Push("DelAlarm", req.Params());
}
void AnalyzerSoft::SetAlarm(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->SetAlarm(req);
  if(req.Status() == HTTP_OK) pSync->Push("SetAlarm", req.Params());
}
void AnalyzerSoft::DelAlarmsRange(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->DelAlarmsRange(req);
  if(req.Status() == HTTP_OK) pSync->Push("DelAlarmsRange", req.Params());
}
void AnalyzerSoft::GetCosts(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pRates->GetCosts(req);
}
void AnalyzerSoft::GetBills(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::READ, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  pRates->GetBills(req);
}
void AnalyzerSoft::SetRemote(HTTPRequest &req) {
  req.Error("", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerSoft::SetSeries(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pSeries->SetSeries(req);
  if(req.Status() == HTTP_OK) pSync->Push("SetSeries", req.Params());
}
void AnalyzerSoft::SetEvents(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pEvents->SetEvents(req);
  if(req.Status() == HTTP_OK) pSync->Push("SetEvents", req.Params());
}
void AnalyzerSoft::SetAlarms(HTTPRequest &req) {
  if(pAccess->TestUser(AnalyzerAccess::WRITE, req.UserName()) == false) return req.Error("", HTTP_FORBIDDEN);
  if(pSync->ShouldPush() == false) return req.Error("Too much pending actions in queue", HTTP_METHOD_NOT_ALLOWED);
  pAlarms->SetAlarms(req);
  if(req.Status() == HTTP_OK) pSync->Push("SetAlarms", req.Params());
}
bool AnalyzerSoft::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  auto Func = [&]() {
    pAggreg200MS->HandleModbus(buff, start, len);
    return false;
  };
  return pConsumers->OnWrite(Func);
}
//----------------------------------------------------------------------------------------------------------------------