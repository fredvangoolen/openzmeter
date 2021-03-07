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
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) :
  Analyzer(id, mode), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3),
  pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pAccess(id), pZeroCross(mode, samplerate), pFrequency(samplerate), pFFTs(mode),
  pAggreg200MS(id, mode), pAggreg3S(id, mode), pAggreg1M(id, mode), pAggreg10M(id, mode), pAggreg15M(id, mode), pAggreg1H(id, mode),
  pRates(id), pSeries(id), pEvents(id, mode, samplerate), pAlarms(id, &pMQTT), pSync(id, mode, samplerate), pMQTT(id) {
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(pSerial, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
  pTerminate      = false;
  pWorkingBlock   = NULL;
  if(pthread_create(&pThread, NULL, [](void *params) -> void* { ((AnalyzerSoft*)params)->Thread(); return NULL; }, this) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pThread = 0;
  } else {
    Worker::SetThreadProperties(pThread, "Analyzer", Worker::ABOUTNORMAL);
  }
  Config(CheckConfig(ConfigFile::ReadConfig(optionPath), mode, false));
}
AnalyzerSoft::~AnalyzerSoft() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
  pthread_join(pThread, NULL);
  while(pPendingBlocks.size() > 0) {
    delete pPendingBlocks.front();
    pPendingBlocks.pop_front();
  }
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
void AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples) {
  AnalyzerSoftBlock *Result = pZeroCross.PushSample(voltageSamples, currentSamples);
  if(Result == NULL) return;
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
  if(mode != Analyzer::PHASE1) {
    ConfigOUT["PhaseNames"] = json::array();
    ConfigOUT["PhaseNames"].push_back("R");
    ConfigOUT["PhaseNames"].push_back("S");
    ConfigOUT["PhaseNames"].push_back("T");
    if(mode == Analyzer::PHASE3N) ConfigOUT["PhaseNames"].push_back("N");
  }
  ConfigOUT["TimeZone"] = "UTC";
  ConfigOUT["Tags"] = json::object();
  ConfigOUT["Sync"] = AnalyzerSoftSync::CheckConfig(json::object());
  ConfigOUT["MQTT"] = AnalyzerSoftMQTT::CheckConfig(json::object());
  ConfigOUT["Save200MS"] = false;
  ConfigOUT["EventsEnabled"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
  ConfigOUT["Alarms"] = json::array();
  ConfigOUT["Rates"] = json::array();
  if(!config.is_object()) return ConfigOUT;
  if(config.contains("Name") && config["Name"].is_string()) ConfigOUT["Name"] = config["Name"];
  if(config.contains("NominalVoltage") && config["NominalVoltage"].is_number() && (config["NominalVoltage"] > 10.0) && (config["NominalVoltage"] < 450.0)) ConfigOUT["NominalVoltage"] = config["NominalVoltage"];
  if(config.contains("NominalFrequency") && config["NominalFrequency"].is_number() && ((config["NominalFrequency"] == 50.0) || (config["NominalFrequency"] == 60.0) || (config["NominalFrequency"] == 0.0))) ConfigOUT["NominalFrequency"] = config["NominalFrequency"];
  if(config.contains("MaxCurrent") && config["MaxCurrent"].is_number() && (config["MaxCurrent"] >= 1) && (config["MaxCurrent"] <= 100000)) ConfigOUT["MaxCurrent"] = config["MaxCurrent"];
  if(config.contains("MinCurrent") && config["MinCurrent"].is_number() && (config["MinCurrent"] >= 0.01) && (config["MinCurrent"] <= 100)) ConfigOUT["MinCurrent"] = config["MinCurrent"];
  if((mode != Analyzer::PHASE1) && config.contains("PhaseNames") && config["PhaseNames"].is_array() && (config["PhaseNames"].size() == ConfigOUT["PhaseNames"].size())) {
    for(uint8_t n = 0; n < ConfigOUT["PhaseNames"].size(); n++) {
      if(config["PhaseNames"][n].is_string() && (config["PhaseNames"][n].size() < 3) && (config["PhaseNames"][n].size() > 0)) ConfigOUT["PhaseNames"][n] = config["PhaseNames"][n];
    }
  }
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
  if(config.contains("Save200MS") && config["Save200MS"].is_boolean()) ConfigOUT["Save200MS"] = config["Save200MS"];
  if(config.contains("EventsEnabled") && config["EventsEnabled"].is_boolean()) ConfigOUT["EventsEnabled"] = config["EventsEnabled"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
  if(config.contains("Alarms") && config["Alarms"].is_array()) ConfigOUT["Alarms"] = AnalyzerSoftAlarms::CheckConfig(config["Alarms"]);
  if(config.contains("Rates") && config["Rates"].is_array()) ConfigOUT["Rates"] = AnalyzerRates::CheckConfig(config["Rates"], except);
  return ConfigOUT;
}
void AnalyzerSoft::Config(const json &options) {
  ConfigFile::WriteConfig(pConfigPath, options);
  Name(options["Name"]);
  pthread_mutex_lock(&pMutex);
  pNominalVoltage   = options["NominalVoltage"];
  pNominalFrequency = options["NominalFrequency"];
  pSave200MS        = options["Save200MS"];
  pZeroCross.Config(pNominalVoltage, pNominalFrequency);
  pRates.Config(options["TimeZone"], options["Rates"]);
  pAlarms.Config(options["AlarmsEnabled"], options["TimeZone"], options["Alarms"]);
  pEvents.Config(options["EventsEnabled"]);
  pSync.Config(options["Sync"], pConfigPath, [&](const string &name, const json &params) { HandleAction(name, params); }, [&](const vector<string> &series) { return pAccess.GetStatus(series); }, [&](const vector<string> &series) { return GetSeriesNow(series); });
  pMQTT.Config(options);
  pthread_mutex_unlock(&pMutex);
  Configured(true);
}
void AnalyzerSoft::Thread() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(pTerminate == false) {
    if(pPendingBlocks.size() == 0) {
      if(pthread_cond_wait(&pCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      pWorkingBlock = pPendingBlocks.front();
      pPendingBlocks.pop_front();
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      uint64_t Start     = Tools::GetTime64();
      uint16_t BlockSize = pWorkingBlock->Samples();
      pAggreg200MS.Reset();
      pAggreg200MS.setTimestamp(pWorkingBlock->Timestamp());
      pFFTs.Run(pWorkingBlock, (pWorkingBlock->NominalFrequency() == 0.0) ? 0 : (pWorkingBlock->NominalFrequency() == 50.0) ? 10 : 12);
      pEvents.Run(pWorkingBlock, pAggreg200MS.Timestamp(), pWorkingBlock->NominalVoltage(), pWorkingBlock->NominalFrequency());
      pAlarms.ConfigNominals(pWorkingBlock->NominalVoltage(), pWorkingBlock->NominalFrequency());
      pAggreg200MS.setFrequency(pFrequency.Frequency(pWorkingBlock));
      for(uint8_t j = 0; j < pVoltageChannels; j++) {
        float Voltage     = 0.0;
        float Current     = 0.0;
        float ActivePower = 0.0;
        for(uint16_t i = 0; i < BlockSize; i++) {
          Voltage     += pWorkingBlock->VoltageSample(j, i) * pWorkingBlock->VoltageSample(j, i);
          Current     += pWorkingBlock->CurrentSample(j, i) * pWorkingBlock->CurrentSample(j, i);
          ActivePower += pWorkingBlock->VoltageSample(j, i) * pWorkingBlock->CurrentSample(j, i);
        }
        pAggreg200MS.setVoltage(j, sqrt(Voltage / BlockSize));
        pAggreg200MS.setCurrent(j, sqrt(Current / BlockSize));
        pAggreg200MS.setActivePower(j, ActivePower / BlockSize);
        pAggreg200MS.setApparentPower(j, pAggreg200MS.Voltage(j) * pAggreg200MS.Current(j));
        pAggreg200MS.setActiveEnergy(j, (pAggreg200MS.ActivePower(j) * BlockSize) / (pSamplerate * 3600.0));
        pAggreg200MS.setPowerFactor(j, pAggreg200MS.ActivePower(j) / pAggreg200MS.ApparentPower(j));
        pAggreg200MS.setReactiveEnergy(j, (pAggreg200MS.ReactivePower(j) * BlockSize) / (pSamplerate * 3600.0));
      }
      pFFTs.Wait();
      for(uint8_t j = 0; j < pVoltageChannels; j++) {
        float THDv        = 0.0;
        float THDi        = 0.0;
        for(uint8_t i = 0; i < 50; i++) {
          pAggreg200MS.setVoltageHarmonic(j, i, pFFTs.VoltageHarmonic(j, i));
          if(i > 0) THDv += pFFTs.VoltageHarmonicPower(j, i);
          pAggreg200MS.setCurrentHarmonic(j, i, pFFTs.CurrentHarmonic(j, i));
          if(i > 0) THDi += pFFTs.CurrentHarmonicPower(j, i);
          pAggreg200MS.setPowerHarmonic(j, i, pFFTs.VoltageHarmonicPower(j, i) * pFFTs.CurrentHarmonicPower(j, i) * cos(AnalyzerSoftFFTs::CalcAngle(pFFTs.CurrentHarmonic(j, i), pFFTs.VoltageHarmonic(j, i))));
        }
        pAggreg200MS.setVoltageTHD(j, (THDv / pAggreg200MS.VoltageHarmonic(j, 0)) * 100.0);
        pAggreg200MS.setCurrentTHD(j, (THDi / pAggreg200MS.CurrentHarmonic(j, 0)) * 100.0);
        pAggreg200MS.setVoltagePhase(j, (j == 0) ? 0.0 : AnalyzerSoftFFTs::CalcAngle(pFFTs.VoltageHarmonic(j, 0), pFFTs.VoltageHarmonic(0, 0)) * 180.0 / M_PI);
        pAggreg200MS.setReactivePower(j, pFFTs.VoltageHarmonicPower(j, 0) * pFFTs.CurrentHarmonicPower(j, 0) * sin(-AnalyzerSoftFFTs::CalcAngle(pFFTs.CurrentHarmonic(j, 0), pFFTs.VoltageHarmonic(j, 0))));
        pAggreg200MS.setPhi(j, AnalyzerSoftFFTs::CalcAngle(pFFTs.CurrentHarmonic(j, 0), pFFTs.VoltageHarmonic(j, 0)) * 180.0 / M_PI);
      }
      if((pMode == PHASE3N) || (pMode == PHASE3)) {
        complex<float> A(cos(120.0 * M_PI / 180.0), sin(120.0 * M_PI / 180.0));
        complex<float> V1(pFFTs.VoltageHarmonic(0, 0).Re, pFFTs.VoltageHarmonic(0, 0).Im);
        complex<float> V2(pFFTs.VoltageHarmonic(1, 0).Re, pFFTs.VoltageHarmonic(1, 0).Im);
        complex<float> V3(pFFTs.VoltageHarmonic(2, 0).Re, pFFTs.VoltageHarmonic(2, 0).Im);
        pAggreg200MS.setUnbalance(((abs(V1 + A * A * V2 + A * V3) / 3.0) / (abs(V1 + A * V2 + A * A * V3) / 3.0)) * 100.0);
      }
      if(pMode == PHASE3N) {
        float Current = 0.0;
        float THD     = 0.0;
        for(uint16_t i = 0; i < BlockSize; i++) Current += pWorkingBlock->CurrentSample(3, i) * pWorkingBlock->CurrentSample(3, i);
        pAggreg200MS.setCurrent(3, sqrt(Current / BlockSize));
        for(uint8_t i = 0; i < 50; i++) {
          pAggreg200MS.setCurrentHarmonic(3, i, pFFTs.CurrentHarmonic(3, i));
          if(i > 0) THD += pFFTs.CurrentHarmonicPower(3, i);
        }
        pAggreg200MS.setCurrentTHD(3, (THD / pAggreg200MS.CurrentHarmonic(3, 0)) * 100.0);
      }
      pAggreg200MS.setFlag(pEvents.Wait());
      if(pWorkingBlock->Sequence() < 100) {
        if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "", pSerial.c_str(), Tools::GetTime64() - Start, pWorkingBlock->Sequence());
      } else {
        Online(true);
        pNowDataLock.ProviderWrite();
        if((pWorkingBlock->Sequence() % 15) == 0) {
          LogFile::Debug("Storing 3S aggregation for analyzer '%s'", pSerial.c_str());
          if(pAggreg3S.Normalize() == true) {
            pAlarms.Run("3S", pAggreg3S);
            pAggreg3S.Save();
            pAggreg1H.AccumulateStats(pAggreg3S, pWorkingBlock->NominalFrequency());
            pMQTT.PublishAggreg("3S", pAggreg3S);
            pAlarms.Wait();
          }
          pAggreg3S.Reset();
        }
        pAggreg3S.Accumulate(pAggreg200MS);
        if((pAggreg1M.Timestamp() != 0) && ((pAggreg1M.Timestamp() / 60000) < (pAggreg200MS.Timestamp() / 60000))) {
          LogFile::Debug("Storing 1M aggregation for analyzer '%s'", pSerial.c_str());
          if(pAggreg1M.Normalize() == true) {
            pAlarms.Run("1M", pAggreg1M);
            pAggreg1M.Save();
            pMQTT.PublishAggreg("1M", pAggreg1M);
            pAlarms.Wait();
          }
          pAggreg1M.Reset();
        }
        pAggreg1M.Accumulate(pAggreg200MS);
        if((pAggreg10M.Timestamp() != 0) && ((pAggreg10M.Timestamp() / 600000) < (pAggreg200MS.Timestamp() / 600000))) {
          LogFile::Debug("Storing 10M aggregation for analyzer '%s", pSerial.c_str());
          if(pAggreg10M.Normalize() == true) {
            pAlarms.Run("10M", pAggreg10M);
            pAggreg1H.AccumulateStats(pAggreg10M, pWorkingBlock->NominalVoltage());
            pAggreg10M.Save();
            pMQTT.PublishAggreg("10M", pAggreg10M);
            pAlarms.Wait();
          }
          pAggreg10M.Reset();
        }
        pAggreg10M.Accumulate(pAggreg200MS);
        if((pAggreg15M.Timestamp() != 0) && ((pAggreg15M.Timestamp() / 900000) < (pAggreg200MS.Timestamp() / 900000))) {
          LogFile::Debug("Storing 15M aggregation for analyzer '%s'", pSerial.c_str());
          if(pAggreg15M.Normalize() == true) {
            pAlarms.Run("15M", pAggreg15M);
            pAggreg15M.Save();
            pMQTT.PublishAggreg("15M", pAggreg15M);
            pAlarms.Wait();
          }
          pAggreg15M.Reset();
        }
        pAggreg15M.Accumulate(pAggreg200MS);
        if((pAggreg1H.Timestamp() != 0) && ((pAggreg1H.Timestamp() / 3600000) < (pAggreg200MS.Timestamp() / 3600000))) {
          LogFile::Debug("Storing 1H aggregation for analyzer '%s'", pSerial.c_str());
          if(pAggreg1H.Normalize() == true) {
            pAlarms.Run("1H", pAggreg1H);
            pAggreg1H.Save();
            pMQTT.PublishAggreg("1H", pAggreg1H);
            pAlarms.Wait();
          }
          pAggreg1H.Reset();
        }
        pAggreg1H.Accumulate(pAggreg200MS);
        pAlarms.Run("200MS", pAggreg200MS);
        if(pSave200MS) {
          LogFile::Debug("Storing 200MS aggregation for analyzer '%s'", pSerial.c_str());
          pAggreg200MS.Save();
        }
        pMQTT.PublishAggreg("200MS", pAggreg200MS);
        pAlarms.Wait();
        pNowDataLock.ProviderWait();
        ResultsReady(pAggreg200MS);
        uint64_t TotalTime = Tools::GetTime64() - Start;
        if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        if(TotalTime > 200) {
          LogFile::Warning("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (%d pending blocks)", pSerial.c_str(), TotalTime, pWorkingBlock->Sequence(), pPendingBlocks.size());
        } else {
          LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (%d pending blocks)", pSerial.c_str(), TotalTime, pWorkingBlock->Sequence(), pPendingBlocks.size());
        }
      }
      delete pWorkingBlock;
    }
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void AnalyzerSoft::ResultsReady(AnalyzerSoftAggreg200MS &params) {
}
json AnalyzerSoft::GetSeriesNow(const vector<string> &series) {
  if(pNowDataLock.ConsumerWait(5) == false) return nullptr;
  json Result  = json::object();
  Result["Time"] = pAggreg200MS.Timestamp();
  for(auto &Serie : series) {
    if(pAggreg200MS.GetSeriesJSON(Serie, Result) == true) continue;
    if(pFFTs.GetSeriesJSON(Serie, Result) == true) continue;
    if(pWorkingBlock->GetSeriesJSON(Serie, Result) == true) continue;
//    if(pGA->GetSeriesNow(Serie, Result) == true) continue;
    // == "GA_POWER") {
//      Result["GA_Power"] = CalcGA();
 //     continue;
 //   }
    pNowDataLock.ConsumerEnds();
    return nullptr;
  }
  pNowDataLock.ConsumerEnds();
  return Result;
}
void AnalyzerSoft::HandleAction(const string &name, const json &params) {
  Response Resp;
  if(name == "SetSeries") {
    pSeries.SetSeries(params, Resp);
  } else if(name == "DelSeriesRange") {
    pSeries.DelSeriesRange(params, Resp);
  } else if(name == "SetEvents") {
    pEvents.SetEvents(params, Resp);
  } else if(name == "DelEventsRange") {
    pEvents.DelEventsRange(params, Resp);
  } else if(name == "DelEvent") {
    pEvents.DelEvent(params, Resp);
  } else if(name == "SetAlarms") {
    pAlarms.SetAlarms(params, Resp);
  } else if(name == "SetAlarm") {
    pAlarms.SetAlarm(params, Resp);
  } else if(name == "DelAlarm") {
    pAlarms.DelAlarm(params, Resp);
  } else if(name == "DelAlarmsRange") {
    pAlarms.DelAlarmsRange(params, Resp);
  } else {
    LogFile::Warning("AnayzerSoft receive invalid action from AnalyzerRemote '%s' (%s)", pSerial.c_str(), name.c_str());
    return;
  }
  if(Resp.status != HTTP_OK) LogFile::Warning("AnayzerSoft action from AnalyzerRemote '%s' (%s) fails", pSerial.c_str(), name.c_str());
}
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  string UserName = params["UserName"];
  json   Response = ConfigFile::ReadConfig(pConfigPath);
  Response["Online"]       = Online();
  Response["OnlineTime"]   = OnlineTime();
  Response["User"]         = pOwner;
  Response["Mode"]         = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"]   = pSamplerate;
  Response["PriceModules"] = PriceSource::VarsInfo();
  Response["Users"]        = pAccess.Users();
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::CONFIG, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(params.contains("Users")) pAccess.Config(AnalyzerAccess::CheckConfig(params["Users"]));
  try {
    Config(CheckConfig(params, pMode, true));
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
  } catch(const string &err) {
    HTTPServer::FillResponse(response, err, "text/plain", HTTP_BAD_REQUEST);
  }
}
void AnalyzerSoft::GetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pSeries.GetSeries(params, response);
}
void AnalyzerSoft::GetSeriesNow(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  vector<string> Series;
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    Series.push_back(Serie);
  }
  json Result = GetSeriesNow(params["Series"]);
  if(Result == nullptr) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetStatus(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  vector<string> Series;
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    Series.push_back(Serie);
  }
  json Result = pAccess.GetStatus(Series, params["UserName"]);
  if(Result == nullptr) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Result["Online"]     = Online();
  Result["OnlineTime"] = OnlineTime();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetWaveStream(const json &params, Response &response) {
  //if(TestUser(params["UserName"], READ) == false)
  return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//  typedef struct {
//    uint64_t Block;
//    bool     Current;
//    bool     Voltage;
//  } WaveStream_t;
//  WaveStream_t *Params = (WaveStream_t *)calloc(sizeof(WaveStream_t), 1);
//  Params->Block = 0;
//  for(uint8_t i = 0; i < params["Series"].size(); i++) {
//    if(params["Series"][i].is_string() == false) {
//      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
//    } else if(params["Series"][i] == "Current_Samples") {
//      Params->Current = true;
//    } else if(params["Series"][i] == "Voltage_Samples") {
//      Params->Voltage = true;
//    } else {
//      return HTTPServer::FillResponse(response, "Unknown serie name", "text/plain", HTTP_BAD_REQUEST);
//    }
//  }
//  auto Func = [this, Params](uint64_t offset, DataSink &sink) {
//    if(pNowDataLock.ConsumerWait(5) == false) {
//      sink.done();
//      return false;
//    }
//    Params->Block = (Params->Block == 0) ? pInputBlock->Sequence() : Params->Block + 1;
//    if(Params->Block != pInputBlock->Sequence()) {
//      pNowDataLock.ConsumerEnds();
//      sink.done();
//      return false;
//    }
//    json Result = json::object();
//    Result["Samples"] = pBlockSamples;
//    if(Params->Voltage) {
//      Result["Voltage_Samples"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Voltage_Samples"].push_back(json::array());
//        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Voltage_Samples"][n].push_back(pVoltageSamples[n * pMaxSamples + i]);
//      }
//    }
//    if(Params->Current) {
//      Result["Current_Samples"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_Samples"].push_back(json::array());
//        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Current_Samples"][n].push_back(pCurrentSamples[n * pMaxSamples + i]);
//      }
//    }
//    pNowDataLock.ConsumerEnds();
//    string Data = Result.dump() + "\n\n";
//    sink.write(Data.c_str(), Data.size());
//    sink.done();
//    return true;
//  };
//  auto Free = [Params]() {
//    free(Params);
//  };
//  response.set_chunked_content_provider("application/json", Func, Free);
}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pSeries.GetSeriesRange(params, response);
}
void AnalyzerSoft::DelSeriesRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.DelSeriesRange(params, response);
  if(response.status == HTTP_OK) pSync.Push("DelSeriesRange", params);
}
void AnalyzerSoft::GetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents.GetEvents(params, response);
}
void AnalyzerSoft::GetEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents.GetEventsRange(params, response);
}
void AnalyzerSoft::DelEventsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.DelEventsRange(params, response);
  if(response.status == HTTP_OK) pSync.Push("DelEventsRange", params);
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pEvents.GetEvent(params, response);
}
void AnalyzerSoft::SetEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.SetEvent(params, response);
  if(response.status == HTTP_OK) pSync.Push("SetEvent", params);
}
void AnalyzerSoft::DelEvent(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.DelEvent(params, response);
  if(response.status == HTTP_OK) pSync.Push("DelEvent", params);
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  pAlarms.TestAlarm(params, response, pAggreg200MS);
  pNowDataLock.ConsumerEnds();
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms.GetAlarms(params, response);
}
void AnalyzerSoft::GetAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms.GetAlarmsRange(params, response);
}
void AnalyzerSoft::DelAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.DelAlarm(params, response);
  if(response.status == HTTP_OK) pSync.Push("DelAlarm", params);
}
void AnalyzerSoft::SetAlarm(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.SetAlarm(params, response);
  if(response.status == HTTP_OK) pSync.Push("SetAlarm", params);
}
void AnalyzerSoft::DelAlarmsRange(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.DelAlarmsRange(params, response);
  if(response.status == HTTP_OK) pSync.Push("DelAlarmsRange", params);
}
void AnalyzerSoft::GetCosts(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates.GetCosts(params, response);
}
void AnalyzerSoft::GetBills(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::READ, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates.GetBills(params, response);
}
void AnalyzerSoft::SetRemote(const json &params, Response &response) {
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void AnalyzerSoft::SetSeries(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pSeries.SetSeries(params, response);
  if(response.status == HTTP_OK) pSync.Push("SetSeries", params);
}
void AnalyzerSoft::SetEvents(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pEvents.SetEvents(params, response);
  if(response.status == HTTP_OK) pSync.Push("SetEvents", params);
}
void AnalyzerSoft::SetAlarms(const json &params, Response &response) {
  if(pAccess.TestUser(AnalyzerAccess::WRITE, params["UserName"]) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pSync.ShouldPush() == false) return HTTPServer::FillResponse(response, "Too much pending actions in queue", "text/plain", HTTP_METHOD_NOT_ALLOWED);
  pAlarms.SetAlarms(params, response);
  if(response.status == HTTP_OK) pSync.Push("SetAlarms", params);
}
bool AnalyzerSoft::HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) {
  if(pNowDataLock.ConsumerWait(5) == false) return false;
  pAggreg200MS.HandleModbus(buff, start, len);
  pNowDataLock.ConsumerEnds();
  return true;
}
//----------------------------------------------------------------------------------------------------------------------