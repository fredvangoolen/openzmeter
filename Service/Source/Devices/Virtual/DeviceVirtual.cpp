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
#include "DeviceVirtual.h"
#include "DeviceManager.h"
#include "AnalyzerManager.h"
#include "Tools.h"
#include "LogFile.h"
#include "ConfigFile.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceVirtual::pRegistered = DeviceVirtual::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceVirtual::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceVirtual::GetInstances() {
  DeviceManager::AddDevice(new DeviceVirtual());
}
DeviceVirtual::DeviceVirtual() : Device("VIRTUAL", "Virtual", "Emulate most of real capture device for testing") {
  LogFile::Info("Starting DeviceVirtual...");
  pThread   = 0;
  pLock     = PTHREAD_MUTEX_INITIALIZER;
  pAnalyzer = NULL;
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  LogFile::Info("DeviceVirtual started");
}
DeviceVirtual::~DeviceVirtual() {
  LogFile::Info("Stopping DeviceVirtual...");
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  LogFile::Info("DeviceVirtual stopped");
}
void DeviceVirtual::ClearAnalyzers() {
  pthread_mutex_lock(&pLock);
  if(pAnalyzer != NULL) {
    AnalyzerManager::DelAnalyzer(pAnalyzer->Serial());
    delete pAnalyzer;
    pAnalyzer = NULL;
  }
  pthread_mutex_unlock(&pLock);
}
json DeviceVirtual::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Voltage"] = 230.0;
  ConfigOUT["Frequency"] = 50.0;
  ConfigOUT["SampleRate"] = 10000.0;
  ConfigOUT["Current"] = 5.0;
  ConfigOUT["CurrentPhase"] = 0.0;
  ConfigOUT["Mode"] = "1PHASE";
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1, false);
  if(config.is_object()) {
    if(config.contains("Voltage") && config["Voltage"].is_number() && (config["Voltage"] >= 10) && (config["Voltage"] <= 100000)) ConfigOUT["Voltage"] = config["Voltage"];
    if(config.contains("Frequency") && config["Frequency"].is_number() && (((config["Frequency"] >= 10) && (config["Frequency"] <= 100)) || (config["Frequency"] == 0.0))) ConfigOUT["Frequency"] = config["Frequency"];
    if(config.contains("SampleRate") && config["SampleRate"].is_number() && (config["SampleRate"] >= 5000) && (config["SampleRate"] <= 1000000)) ConfigOUT["SampleRate"] = config["SampleRate"];
    if(config.contains("Current") && config["Current"].is_number() && (config["Current"] >= 0.1) && (config["Current"] <= 10000)) ConfigOUT["Current"] = config["Current"];
    if(config.contains("CurrentPhase") && config["CurrentPhase"].is_number() && (config["CurrentPhase"] >= -180.0) && (config["CurrentPhase"] <= 180.0)) ConfigOUT["CurrentPhase"] = config["CurrentPhase"];
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "1PHASE") || (config["Mode"] == "2PHASE") || (config["Mode"] == "2PHASE+N") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"], Analyzer::StringToMode(ConfigOUT["Mode"]), false);
  }
  return ConfigOUT;
}
uint32_t DeviceVirtual::AnalyzersCount() {
  pthread_mutex_lock(&pLock);
  uint32_t Count = (pAnalyzer != NULL) ? 1 : 0;
  pthread_mutex_unlock(&pLock);
  return Count;
}
void DeviceVirtual::Config(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  ClearAnalyzers();
  pthread_mutex_lock(&pLock);
  string ModeString = (config["Mode"] == "1PHASE") ? "_1P" : (config["Mode"] == "2PHASE") ? "_2P" : (config["Mode"] == "2PHASE+N") ? "_2PN" : (config["Mode"] == "3PHASE") ? "_3P" : "_3PN";
  pVoltage          = config["Voltage"];
  pFrequency        = config["Frequency"];
  pSampleRate       = config["SampleRate"];
  pCurrent          = config["Current"];
  pCurrentPhase     = config["CurrentPhase"];
  pMode             = Analyzer::StringToMode(config["Mode"]);
  pEnabled          = config["Enabled"];
  pAnalyzer         = new AnalyzerSoft(pSerial + ModeString, pMode, "/Devices/" + pSerial + "/Analyzer", pSampleRate);
  AnalyzerManager::AddAnalyzer(pAnalyzer);
  pthread_mutex_unlock(&pLock);
  if(pEnabled == true) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, [](void *params) { return ((DeviceVirtual*)params)->Thread(); }, this) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      ClearAnalyzers();
      pThread = 0;
      return;
    }
    if(pthread_setname_np(pThread, "DeviceVirtual") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    TaskPool::SetThreadProperties(pThread, TaskPool::NORMAL);
  }
}
void DeviceVirtual::GetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  req.Success(Config);
}
void DeviceVirtual::SetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json ConfigIN = req.Params();
  ConfigIN["Analyzer"] = ConfigFile::ReadConfig("/Devices/" + pSerial + "/Analyzer");
  Config(CheckConfig(ConfigIN));
  req.Success(json::object());
}
void *DeviceVirtual::Thread() {
  uint64_t       StartTime        = Tools::GetTime64();
  uint64_t       GeneratedSamples = 0;
  float          Time             = 0;
  float          Phase            = pCurrentPhase * M_PI / 180.0;
  float          TimeInc          = 2.0 * M_PI / (pSampleRate / pFrequency);
  while(!pTerminate) {
    float Voltage[3], Current[4];
    uint64_t TimeI = Tools::GetTime64() - StartTime;
    if((TimeI * pSampleRate / 1000) > GeneratedSamples) {
      Voltage[0] = (pFrequency > 0.0) ? pVoltage * M_SQRT2 * sin(Time) : pVoltage;
      Current[0] = (pFrequency > 0.0) ? pCurrent * M_SQRT2 * sin(Time + Phase) : pCurrent;
      if((TimeI >= 50000) && (TimeI < 60000)) {
        Voltage[0] = Voltage[0] * 1.12;
      } else if((TimeI >= 90000) && (TimeI < 100000)) {
        Voltage[0] = Voltage[0] * 0.88;
      } else if((TimeI >= 120000) && (TimeI < 121000)) {
        Voltage[0] = Voltage[0] * (0.94 + (TimeI - 120000) / 25000.0);
      } else if(TimeI >= 121000) {
        Voltage[0] = Voltage[0] * 0.98;
      }
      if((pMode == Analyzer::PHASE3) || (pMode == Analyzer::PHASE3N)) {
        Voltage[1] = (pFrequency > 0.0) ? pVoltage * M_SQRT2 * sin(Time - 120.0 * M_PI / 180.0) : pVoltage;
        Current[1] = (pFrequency > 0.0) ? pCurrent * M_SQRT2 * sin(Time + Phase - 120.0 * M_PI / 180.0) : pCurrent;
        Voltage[2] = (pFrequency > 0.0) ? pVoltage * M_SQRT2 * sin(Time - 240.0 * M_PI / 180.0)  : pVoltage;
        Current[2] = (pFrequency > 0.0) ? pCurrent * M_SQRT2 * sin(Time + Phase - 240.0 * M_PI / 180.0) : pCurrent;
        if(pMode == Analyzer::PHASE3N) Current[3] = (pFrequency > 0.0) ? Current[0] + Current[1] + Current[2] : 0;
      } else if((pMode == Analyzer::PHASE2) || (pMode == Analyzer::PHASE2N)) {
        Voltage[1] = (pFrequency > 0.0) ? pVoltage * M_SQRT2 * sin(Time - 180.0 * M_PI / 180.0) : pVoltage;
        Current[1] = (pFrequency > 0.0) ? pCurrent * M_SQRT2 * sin(Time + Phase - 180.0 * M_PI / 180.0) : pCurrent;
        if(pMode == Analyzer::PHASE2N) Current[2] = (pFrequency > 0.0) ? Current[0] + Current[1] : 0;
      }
      GeneratedSamples++;
      Time += TimeInc;
      if(Time > (2.0 * M_PI)) Time -= 2.0 * M_PI;
      pAnalyzer->BufferAppend(Voltage, Current);
    } else {
      usleep(10000);
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------