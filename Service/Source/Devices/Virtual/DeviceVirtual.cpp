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
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceVirtual::pRegistered = DeviceVirtual::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceVirtual::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
vector<Device*> DeviceVirtual::GetInstances() {
  LOGFile::Info("Adding 'Virtual' device driver instance with serial 'VIRTUAL'\n");
  vector<Device*> Result;
  Result.push_back(new DeviceVirtual());
  return Result;
}
DeviceVirtual::DeviceVirtual() : Device("VIRTUAL", "Virtual", "Emulate most of real capture device for testing") {
  pThread = 0;
  pAnalyzer = NULL;
  Configure(CheckConfig(ConfigManager::ReadConfig("/Devices/" + Serial)));
}
DeviceVirtual::~DeviceVirtual() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
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
  ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(json::object());
  if(config.is_object()) {
    if(config.contains("Voltage") && config["Voltage"].is_number() && (config["Voltage"] >= 10) && (config["Voltage"] <= 1000)) ConfigOUT["Voltage"] = config["Voltage"];
    if(config.contains("Frequency") && config["Frequency"].is_number() && (((config["Frequency"] >= 10) && (config["Frequency"] <= 100)) || (config["Frequency"] == 0.0))) ConfigOUT["Frequency"] = config["Frequency"];
    if(config.contains("SampleRate") && config["SampleRate"].is_number() && (config["SampleRate"] >= 10000) && (config["SampleRate"] <= 100000)) ConfigOUT["SampleRate"] = config["SampleRate"];
    if(config.contains("Current") && config["Current"].is_number() && (config["Current"] >= 0.1) && (config["Current"] <= 1000)) ConfigOUT["Current"] = config["Current"];
    if(config.contains("CurrentPhase") && config["CurrentPhase"].is_number() && (config["CurrentPhase"] >= -180.0) && (config["CurrentPhase"] <= 180.0)) ConfigOUT["CurrentPhase"] = config["CurrentPhase"];
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "1PHASE") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = AnalyzerSoft::CheckConfig(config["Analyzer"]);
  }
  return ConfigOUT;
}
void DeviceVirtual::Configure(const json &config) {
  ConfigManager::WriteConfig("/Devices/" + Serial, config);
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  ClearAnalyzers();
  pAnalyzer         = NULL;
  pVoltage          = config["Voltage"];
  pFrequency        = config["Frequency"];
  pSampleRate       = config["SampleRate"];
  pCurrent          = config["Current"];
  pCurrentPhase     = config["CurrentPhase"];
  pMode             = (config["Mode"] == "1PHASE") ? Analyzer::PHASE1 : (config["Mode"] == "3PHASE") ? Analyzer::PHASE3 : Analyzer::PHASE3N;
  string ModeString = config["Mode"];
  pAnalyzer         = new AnalyzerSoft(Serial + "_" + ModeString, pMode, "/Devices/" + Serial + "/Analyzer", pSampleRate);
  bool Enabled      = config["Enabled"];
  AddAnalyzer(pAnalyzer);
  if(Enabled == true) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, Capture_Thread, this) != 0) {
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      ClearAnalyzers();
      pThread = 0;
    } else {
      pthread_setname_np(pThread, "DeviceVirtual");
    }
  }
}
void DeviceVirtual::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/Devices/" + Serial);
  Config.erase("Analyzer");
  HTTPCallback::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceVirtual::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = params;
  Config["Analyzer"] = ConfigManager::ReadConfig("/Devices/" + Serial + "/Analyzer");
  Configure(Config);
  HTTPCallback::FillResponse(response, "", "text/plain", HTTP_OK);
}
void DeviceVirtual::GetRemote(const json &params, Response &response) {
  HTTPCallback::FillResponse(response, "", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void *DeviceVirtual::Capture_Thread(void *args) {
  DeviceVirtual *This = (DeviceVirtual*)args;
  uint64_t       StartTime        = Tools::GetTime();
  uint64_t       GeneratedSamples = 0;
  int32_t        Phase            = (This->pSampleRate / This->pFrequency) * (This->pCurrentPhase / 360.0);
  while(!This->pTerminate) {
    float Voltage[3], Current[4];
    uint64_t Sampletime = Tools::GetTime();
    if((This->pAnalyzer->IsWorking() == false) && (((Sampletime - StartTime) * This->pSampleRate / 1000) > GeneratedSamples)) {
      Voltage[0] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency) : This->pVoltage;
      Current[0] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency) : This->pCurrent;
      if((This->pMode == Analyzer::PHASE3) || (This->pMode == Analyzer::PHASE3N)) {
        Voltage[1] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0) : This->pVoltage;
        Current[1] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency - 2.0 * M_PI / 3.0) : This->pCurrent;
        Voltage[2] = (This->pFrequency > 0.0) ? This->pVoltage * M_SQRT2 * sin(2.0 * M_PI * GeneratedSamples / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0)  : This->pVoltage;
        Current[2] = (This->pFrequency > 0.0) ? This->pCurrent * M_SQRT2 * sin(2.0 * M_PI * (GeneratedSamples + Phase) / This->pSampleRate * This->pFrequency - 4.0 * M_PI / 3.0) : This->pCurrent;
      }
      if(This->pMode == Analyzer::PHASE3) (This->pFrequency > 0.0) ? Current[3] = Current[0] + Current[1] + Current[2] : 0;
      GeneratedSamples++;
      This->pAnalyzer->BufferAppend(Voltage, Current);
    } else {
      usleep(1000);
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------