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
  LogFile::Info("VIRTUAL", "Adding 'Virtual' device driver instance with serial 'VIRTUAL'");
  DeviceManager::AddDevice(new DeviceVirtual());
}
DeviceVirtual::DeviceVirtual() : Device("VIRTUAL", "Virtual", "Emulate most of real capture device for testing") {
  pThread   = 0;
  pLock     = PTHREAD_MUTEX_INITIALIZER;
  pAnalyzer = NULL;
  Configure(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
}
DeviceVirtual::~DeviceVirtual() {
  LogFile::Info("VIRTUAL", "Stopping device with serial '%s'", pSerial.c_str());
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  LogFile::Info("VIRTUAL", "Device with serial '%s' stopped", pSerial.c_str());
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
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object(), Analyzer::PHASE1);
  if(config.is_object()) {
    if(config.contains("Voltage") && config["Voltage"].is_number() && (config["Voltage"] >= 10) && (config["Voltage"] <= 1000)) ConfigOUT["Voltage"] = config["Voltage"];
    if(config.contains("Frequency") && config["Frequency"].is_number() && (((config["Frequency"] >= 10) && (config["Frequency"] <= 100)) || (config["Frequency"] == 0.0))) ConfigOUT["Frequency"] = config["Frequency"];
    if(config.contains("SampleRate") && config["SampleRate"].is_number() && (config["SampleRate"] >= 10000) && (config["SampleRate"] <= 100000)) ConfigOUT["SampleRate"] = config["SampleRate"];
    if(config.contains("Current") && config["Current"].is_number() && (config["Current"] >= 0.1) && (config["Current"] <= 1000)) ConfigOUT["Current"] = config["Current"];
    if(config.contains("CurrentPhase") && config["CurrentPhase"].is_number() && (config["CurrentPhase"] >= -180.0) && (config["CurrentPhase"] <= 180.0)) ConfigOUT["CurrentPhase"] = config["CurrentPhase"];
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "1PHASE") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"], (ConfigOUT["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (ConfigOUT["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N);
  }
  return ConfigOUT;
}
uint32_t DeviceVirtual::AnalyzersCount() {
  pthread_mutex_lock(&pLock);
  uint32_t Count = (pAnalyzer != NULL) ? 1 : 0;
  pthread_mutex_unlock(&pLock);
  return Count;
}
void DeviceVirtual::Configure(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  ClearAnalyzers();
  pthread_mutex_lock(&pLock);
  string ModeString = config["Mode"];
  pVoltage          = config["Voltage"];
  pFrequency        = config["Frequency"];
  pSampleRate       = config["SampleRate"];
  pCurrent          = config["Current"];
  pCurrentPhase     = config["CurrentPhase"];
  pMode             = (config["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (config["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
  pEnabled          = config["Enabled"];
  pAnalyzer         = new AnalyzerSoft(pSerial + "_" + ModeString, pMode, "/Devices/" + pSerial + "/Analyzer", pSampleRate);
  AnalyzerManager::AddAnalyzer(pAnalyzer);
  pthread_mutex_unlock(&pLock);
  if(pEnabled == true) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, Capture_Thread, this) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      ClearAnalyzers();
      pThread = 0;
    } else {
      pthread_setname_np(pThread, "DeviceVirtual");
    }
  }
}
void DeviceVirtual::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceVirtual::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
  Config["Analyzer"] = ConfigFile::ReadConfig("/Devices/" + pSerial + "/Analyzer");
  Configure(Config);
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void *DeviceVirtual::Capture_Thread(void *args) {
  DeviceVirtual *This = (DeviceVirtual*)args;
  uint64_t       StartTime        = Tools::GetTime64();
  uint64_t       GeneratedSamples = 0;
  float          Time             = 0;
  float          Phase            = This->pCurrentPhase * M_PI / 180.0;
  float          TimeInc          = 2.0 * M_PI / (This->pSampleRate / This->pFrequency);
  while(!This->pTerminate) {
    float Voltage[3], Current[4];
    uint64_t TimeI = Tools::GetTime64() - StartTime;
    if((This->pAnalyzer->IsWorking() == false) && ((TimeI * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage[0] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(Time) : This->pVoltage;
      Current[0] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(Time + Phase) : This->pCurrent;
      if((TimeI >= 50000) && (TimeI < 60000)) {
        Voltage[0] = Voltage[0] * 1.12;
      } else if((TimeI >= 90000) && (TimeI < 100000)) {
        Voltage[0] = Voltage[0] * 0.88;
      } else if((TimeI >= 120000) && (TimeI < 121000)) {
        Voltage[0] = Voltage[0] * (0.94 + (TimeI - 120000) / 25000.0);
      } else if(TimeI >= 121000) {
        Voltage[0] = Voltage[0] * 0.98;
      }
      if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
        Voltage[1] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(Time - 120.0 * M_PI / 180.0) : This->pVoltage;
        Current[1] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(Time + Phase - 120.0 * M_PI / 180.0) : This->pCurrent;
        Voltage[2] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(Time - 240.0 * M_PI / 180.0)  : This->pVoltage;
        Current[2] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(Time + Phase - 240.0 * M_PI / 180.0) : This->pCurrent;
      }
      if(This->pMode == Analyzer::PHASE3) (This->pFrequency > 0.0) ? Current[3] = Current[0] + Current[1] + Current[2] : 0;
      GeneratedSamples++;
      Time += TimeInc;
      if(Time > (2.0 * M_PI)) Time -= 2.0 * M_PI;
      This->pAnalyzer->BufferAppend(Voltage, Current, 0);
    } else {
      usleep(10000);
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------