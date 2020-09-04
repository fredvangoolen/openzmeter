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
#include "Status.h"
#include "Telegram.h"
#include "MessageLoop.h"
#include "URL.h"
#include "AnalyzerSoftAlarms.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
AnalyzerSoft::AnalyzerSoft(const string &id, const Analyzer::AnalyzerMode mode, const string &optionPath, const float samplerate) : Analyzer(id), pSamplerate(samplerate), pConfigPath(optionPath), pVoltageChannels((mode == PHASE1) ? 1 : 3), pCurrentChannels((mode == PHASE1) ? 1 : (mode == PHASE3) ? 3 : 4), pMode(mode) {
  Database DB(id);
  LogFile::Debug("Creating AnalyzerSoft database tables");
  DB.ExecResource("SQL/Database_AnalyzerSoft.sql");
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pFFTs           = new AnalyzerSoftFFTs(mode);
  pMQTT           = new AnalyzerSoftMQTT(pSerial);
  pRates          = new AnalyzerSoftRates(pSerial);
  pAlarms         = new AnalyzerSoftAlarms(pMQTT, pSerial);
  pEvents         = new AnalyzerSoftEvents(pSerial, mode, samplerate);
  pSync           = new AnalyzerSoftSync(pSerial);
  pInputBlock     = new AnalyzerSoftBlock(mode, samplerate);
  pFrequency      = new AnalyzerSoftFrequency(samplerate);
  pAggreg200MS    = new AnalyzerSoftAggreg200MS(pSerial, mode);
  pAggreg3S       = new AnalyzerSoftAggreg3S(pSerial, mode);
  pAggreg1M       = new AnalyzerSoftAggreg1M(pSerial, mode);
  pAggreg10M      = new AnalyzerSoftAggreg10M(pSerial, mode);
  pAggreg15M      = new AnalyzerSoftAggreg15M(pSerial, mode);
  pAggreg1H       = new AnalyzerSoftAggreg1H(pSerial, mode);
  pStorage        = new AnalyzerSoftStorage(pSerial, mode);
  pAnalyzerWorker = new Worker((mode == PHASE1) ? "Analyzer_1P" : (mode == PHASE3) ? "Analyzer_3P" : "Analyzer_3P+N", [&](void *a) { this->Analyzer_Func(); });
  MessageLoop::RegisterListener("AnalyzerSoft_" + pSerial + "_PeriodicTasks", [&]() { this->PeriodicTasks(); }, 60000);
  Configure(CheckConfig(ConfigFile::ReadConfig(optionPath), mode));
}
AnalyzerSoft::~AnalyzerSoft() {
  LogFile::Info("Stopping analyzer with serial '%s'", pSerial.c_str());
  MessageLoop::UnregisterListener("AnalyzerSoft_" + pSerial + "_PeriodicTasks");
  delete pAnalyzerWorker;
  delete pAggreg1H;
  delete pAggreg15M;
  delete pAggreg10M;
  delete pAggreg1M;
  delete pAggreg3S;
  delete pAggreg200MS;
  delete pFrequency;
  delete pInputBlock;
  delete pMQTT;
  delete pSync;
  delete pEvents;
  delete pAlarms;
  delete pRates;
  delete pFFTs;
  LogFile::Info("Analyzer with serial '%s' stopped", pSerial.c_str());
}
bool AnalyzerSoft::BufferAppend(float *voltageSamples, float *currentSamples, const uint32_t buffUsage) {
  if(pAggreg200MS->Timestamp() == 0) pAggreg200MS->setTimestamp(Tools::GetTime64() - (uint64_t)((buffUsage * 1000) / pSamplerate));
  bool Result = pInputBlock->PushSample(voltageSamples, currentSamples);
  if(Result == false) return false;
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
  ConfigOUT["Sync"] = AnalyzerSoftSync::CheckConfig(json::object());
  ConfigOUT["MQTT"] = AnalyzerSoftMQTT::CheckConfig(json::object());
  ConfigOUT["Save200MS"] = false;
  ConfigOUT["EventsEnabled"] = false;
  ConfigOUT["AlarmsEnabled"] = false;
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
  if(config.contains("Tags") && config["Tags"].is_array()) {
    for(auto Tag : config["Tags"]) {
      if(!Tag.is_object() || !Tag.contains("Name") || !Tag["Name"].is_string() || !Tag.contains("Value") || !Tag["Value"].is_string()) continue;
      json Row = json::object();
      Row["Name"] = Tag["Name"];
      Row["Value"] = Tag["Value"];
      ConfigOUT["Tags"].push_back(Row);
    }
  }
  if(config.contains("Sync")) ConfigOUT["Sync"] = AnalyzerSoftSync::CheckConfig(config["Sync"]);
  if(config.contains("MQTT")) ConfigOUT["MQTT"] = AnalyzerSoftMQTT::CheckConfig(config["MQTT"]);
  if(config.contains("Save200MS") && config["Save200MS"].is_boolean()) ConfigOUT["Save200MS"] = config["Save200MS"];
  if(config.contains("EventsEnabled") && config["EventsEnabled"].is_boolean()) ConfigOUT["EventsEnabled"] = config["EventsEnabled"];
  if(config.contains("AlarmsEnabled") && config["AlarmsEnabled"].is_boolean()) ConfigOUT["AlarmsEnabled"] = config["AlarmsEnabled"];
  return ConfigOUT;
}
void AnalyzerSoft::Configure(const json &options) {
  ConfigFile::WriteConfig(pConfigPath, options);
  Name(options["Name"]);
  pthread_mutex_lock(&pMutex);
  float NominalVoltage   = options["NominalVoltage"];
  float NominalFrequency = options["NominalFrequency"];
  pSave200MS             = options["Save200MS"];
  if(NominalFrequency != pFrequency->NominalFrequency()) {
    pAggreg200MS->Reset();
    pFrequency->Config(NominalFrequency);
    pInputBlock->Config(NominalFrequency);
    pFFTs->Config((NominalFrequency == 0.0) ? 0 : (NominalFrequency == 50.0) ? 10 : 12);
  }
  pAggreg1H->Config(NominalVoltage, NominalFrequency);
  pRates->Config(options["TimeZone"]);
  pAlarms->Config(options["AlarmsEnabled"], NominalVoltage, NominalFrequency);
  pEvents->Config(options["EventsEnabled"], NominalVoltage, NominalFrequency);
  pSync->Config(options["Sync"], Name());
  pMQTT->Config(options["MQTT"]);
  pMQTT->PublishConfig(options);
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoft::Analyzer_Func() {
  uint64_t Start     = Tools::GetTime64();
  uint16_t BlockSize = pInputBlock->BlockSamples();
  pFFTs->Run(pInputBlock);
  pEvents->Run(pInputBlock, pAggreg200MS->Timestamp());
  pAggreg200MS->setFrequency(pFrequency->Frequency(pInputBlock));
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    float Voltage     = 0.0;
    float Current     = 0.0;
    float ActivePower = 0.0;
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
  }
  pFFTs->Wait();
  for(uint8_t j = 0; j < pVoltageChannels; j++) {
    float THDv        = 0.0;
    float THDi        = 0.0;
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS->setVoltageHarmonic(j, i, pFFTs->VoltageHarmonicPower(j, i));
      if(i > 0) THDv += pFFTs->VoltageHarmonicPower(j, i);
      pAggreg200MS->setCurrentHarmonic(j, i, pFFTs->CurrentHarmonicPower(j, i));
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
  if(pMode == PHASE3N) {
    float Current = 0.0;
    float THD     = 0.0;
    for(uint16_t i = 0; i < BlockSize; i++) Current += pow(pInputBlock->CurrentSample(3, i), 2.0);
    pAggreg200MS->setCurrent(3, sqrt(Current / BlockSize));
    for(uint8_t i = 0; i < 50; i++) {
      pAggreg200MS->setCurrentHarmonic(3, i, pFFTs->CurrentHarmonicPower(3, i));
      if(i > 0) THD += pFFTs->CurrentHarmonicPower(3, i);
    }
    pAggreg200MS->setCurrentTHD(3, (THD / pAggreg200MS->CurrentHarmonic(3, 0)) * 100.0);
  }
  pAggreg200MS->setFlag(pEvents->Wait());
  if(pInputBlock->Sequence() < 100) {
    LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in ommited block %" PRIu64 "", pSerial.c_str(), Tools::GetTime64() - Start, pInputBlock->Sequence());
    pAggreg200MS->Reset();
    return;
  }
  Online(true);
  pNowDataLock.ProviderWrite();
  if((pInputBlock->Sequence() % 15) == 0) {
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
    LogFile::Debug("Storing 200MS aggregation for analyzer '%s'\n", pSerial.c_str());
    pAggreg200MS->Save();
  }
  pAlarms->Wait();
  if(pAggreg200MS->Flag() == true) Status::SetValue("/Last_Event", pAggreg200MS->Timestamp());
  pNowDataLock.ProviderWait();
  LogFile::Debug("Analyzer '%s' take %" PRIu64 "mS in block %" PRIu64 " (Sampletime: %" PRIu64 ")", pSerial.c_str(), Tools::GetTime64() - Start, pInputBlock->Sequence(), pAggreg200MS->Timestamp());
  pAggreg200MS->Reset();
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
  json Response = ConfigFile::ReadConfig(pConfigPath);
  Response["Online"]     = Online();
  Response["Mode"]       = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Configure(CheckConfig(params, pMode));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerSoft::GetSeries(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pStorage->GetSeries(params, response);
}
void AnalyzerSoft::GetSeriesNow(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  json Result  = json::object();
  Result["Time"] = pAggreg200MS->Timestamp();
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) {
      pNowDataLock.ConsumerEnds();
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    if(pAggreg200MS->GetSeriesJSON(Serie, Result) == true) continue;
    if(pFFTs->GetSeriesJSON(Serie, Result) == true) continue;
    if(pInputBlock->GetSeriesJSON(Serie, Result) == true) continue;
    if(pGA->GetSeriesNow(Serie, Result) == true) continue;
    // == "GA_POWER") {
//      Result["GA_Power"] = CalcGA();
 //     continue;
 //   }
    pNowDataLock.ConsumerEnds();
    return HTTPServer::FillResponse(response, "Unknown serie name", "application/json", HTTP_BAD_REQUEST);
  }
  pNowDataLock.ConsumerEnds();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetStatus(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pStorage->GetStatus(params, response);
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
  auto Func = [this, Params](uint64_t offset, DataSink &sink) {
    if(pNowDataLock.ConsumerWait(5) == false) {
      sink.done();
      return false;
    }
    Params->Block = (Params->Block == 0) ? pInputBlock->Sequence() : Params->Block + 1;
    if(Params->Block != pInputBlock->Sequence()) {
      pNowDataLock.ConsumerEnds();
      sink.done();
      return false;
    }
    json Result = json::object();
//    Result["Samples"] = pBlockSamples;
    if(Params->Voltage) {
      Result["Voltage_Samples"] = json::array();
      for(uint8_t n = 0; n < pVoltageChannels; n++) {
        Result["Voltage_Samples"].push_back(json::array());
//        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Voltage_Samples"][n].push_back(pVoltageSamples[n * pMaxSamples + i]);
      }
    }
    if(Params->Current) {
      Result["Current_Samples"] = json::array();
      for(uint8_t n = 0; n < pCurrentChannels; n++) {
        Result["Current_Samples"].push_back(json::array());
//        for(uint16_t i = 0; i < pBlockSamples; i++) Result["Current_Samples"][n].push_back(pCurrentSamples[n * pMaxSamples + i]);
      }
    }
    pNowDataLock.ConsumerEnds();
    string Data = Result.dump() + "\n\n";
    sink.write(Data.c_str(), Data.size());
    sink.done();
    return true;
  };
  auto Free = [Params]() {
    free(Params);
  };
  response.set_chunked_content_provider("application/json", Func, Free);
}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pStorage->GetSeriesRange(params, response);
}
void AnalyzerSoft::DelSeriesRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pStorage->DelSeriesRange(params, response);
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
void AnalyzerSoft::GetAlarmsDef(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarmsDef(params, response);
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if(pNowDataLock.ConsumerWait(5) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  pAlarms->TestAlarm(params, response, pAggreg200MS);
  pNowDataLock.ConsumerEnds();
}
void AnalyzerSoft::AddAlarmDef(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->AddAlarmDef(params, response);
}
void AnalyzerSoft::GetAlarmDef(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarmDef(params, response);
}
void AnalyzerSoft::SetAlarmDef(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->SetAlarmDef(params, response);
}
void AnalyzerSoft::DelAlarmDef(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->DelAlarmDef(params, response);
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarms(params, response);
}
void AnalyzerSoft::GetAlarmsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->GetAlarmsRange(params, response);
}
void AnalyzerSoft::DelAlarm(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->DelAlarm(params, response);
}
void AnalyzerSoft::DelAlarmsRange(const json &params, Response &response) {
  if(CheckRights(params["UserName"], WRITE) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pAlarms->DelAlarmsRange(params, response);
}
void AnalyzerSoft::GetCosts(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->GetCosts(params, response);
}
void AnalyzerSoft::GetBills(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  pRates->GetBills(params, response);
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