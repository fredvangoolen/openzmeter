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
#include <math.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "DeviceS1.h"
#include "DeviceS1Serial.h"
#include "DeviceManager.h"
#include "AnalyzerManager.h"
#include "ConfigFile.h"
#include "LogFile.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceS1::pRegistered = DeviceS1::Register();
//----------------------------------------------------------------------------------------------------------------------
bool DeviceS1::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceS1::GetInstances() {
  DeviceManager::AddDevice(new DeviceS1());
}
DeviceS1::DeviceS1() : Device("S1", "S1", "Fluvius digital meter S1 port (e-MUCS H)") {
  LogFile::Info("Starting DeviceS1...");
  pThread     = 0;
  pLock       = PTHREAD_MUTEX_INITIALIZER;
  pAnalyzer   = NULL;
  pTerminate  = false;
  pConnected  = false;
  pReceiving  = false;
  pFramesOK = pCRCErrors = pLostFrames = pInvalidSamples = pResyncs = 0;
  pMode       = Analyzer::INVALID;
  pNominalRate = 0.0;
  pPerPeriod  = false;
  pHaveHeld   = false;
  pHaveSequence = false;
  pLastSequence = 0;
  pNewSamples   = 0;
  pMeasuredRate = 0.0;
  memset(&pLastFrame, 0, sizeof(pLastFrame));
  memset(pHeldVoltage, 0, sizeof(pHeldVoltage));
  memset(pHeldCurrent, 0, sizeof(pHeldCurrent));
  Config(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
  LogFile::Info("DeviceS1 started");
}
DeviceS1::~DeviceS1() {
  LogFile::Info("Stopping DeviceS1...");
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  ClearAnalyzers();
  LogFile::Info("DeviceS1 stopped");
}
void DeviceS1::ClearAnalyzers() {
  pthread_mutex_lock(&pLock);
  if(pAnalyzer != NULL) {
    AnalyzerManager::DelAnalyzer(pAnalyzer->Serial());
    delete pAnalyzer;
    pAnalyzer = NULL;
  }
  pthread_mutex_unlock(&pLock);
}
json DeviceS1::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Port"]     = "/dev/ttyUSB0";
  ConfigOUT["Mode"]     = "AUTO";
  ConfigOUT["Enabled"]  = false;
  ConfigOUT["Analyzer"] = json::object();
  if(config.is_object()) {
    if(config.contains("Port") && config["Port"].is_string() && (config["Port"].get<string>().rfind("/dev/", 0) == 0)) ConfigOUT["Port"] = config["Port"];
    if(config.contains("Mode") && config["Mode"].is_string() && ((config["Mode"] == "AUTO") || (config["Mode"] == "1PHASE") || (config["Mode"] == "3PHASE") || (config["Mode"] == "3PHASE+N"))) ConfigOUT["Mode"] = config["Mode"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzer") && config["Analyzer"].is_object()) ConfigOUT["Analyzer"] = config["Analyzer"];  //Validated by AnalyzerSoft once mode is known
  }
  return ConfigOUT;
}
uint32_t DeviceS1::AnalyzersCount() {
  pthread_mutex_lock(&pLock);
  uint32_t Count = (pAnalyzer != NULL) ? 1 : 0;
  pthread_mutex_unlock(&pLock);
  return Count;
}
void DeviceS1::Config(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  ClearAnalyzers();
  pthread_mutex_lock(&pLock);
  pPort        = config["Port"];
  pModeConfig  = config["Mode"];
  pEnabled     = config["Enabled"];
  pConnected   = false;
  pReceiving   = false;
  pMeasuredRate = 0.0;
  pFramesOK = pCRCErrors = pLostFrames = pInvalidSamples = pResyncs = 0;
  pthread_mutex_unlock(&pLock);
  if(pEnabled == true) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, [](void *params) { return ((DeviceS1*)params)->Thread(); }, this) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pThread = 0;
      return;
    }
    if(pthread_setname_np(pThread, "DeviceS1") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    TaskPool::SetThreadProperties(pThread, TaskPool::ABOUTNORMAL);
  }
}
void DeviceS1::GetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  Config.erase("Analyzer");
  pthread_mutex_lock(&pLock);
  json Status = json::object();
  Status["Connected"]      = pConnected;
  Status["Receiving"]      = pReceiving;
  Status["FramesOK"]       = pFramesOK;
  Status["CRCErrors"]      = pCRCErrors;
  Status["LostFrames"]     = pLostFrames;
  Status["InvalidSamples"] = pInvalidSamples;
  Status["DiscardedBytes"] = pResyncs;
  if(pFramesOK > 0) {
    Status["MeterID"]      = string(pLastFrame.MeterID);
    Status["MeterType"]    = (pLastFrame.PolyPhase == false) ? "Single phase" : (pLastFrame.FourWires == true) ? "Poly phase, 4 wires" : "Poly phase, 3 wires";
    Status["Neutral"]      = pLastFrame.Neutral;
    Status["Sampling"]     = (pLastFrame.PerPeriod == true) ? to_string(pLastFrame.Sampling) + " samples/period" : to_string(pLastFrame.Sampling * 100) + " Hz";
    Status["Frequency"]    = pLastFrame.Frequency;
    Status["FormatVersion"] = pLastFrame.Version;
  }
  if(pMeasuredRate > 0.0) Status["SampleRate"] = pMeasuredRate;
  if(pAnalyzer != NULL) {
    Status["Analyzer"]     = pAnalyzer->Serial();
    Status["AnalyzerMode"] = Analyzer::ModeToString(pMode);
  }
  pthread_mutex_unlock(&pLock);
  Config["Status"] = Status;
  req.Success(Config);
}
void DeviceS1::SetDevice(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json ConfigIN = req.Params();
  ConfigIN["Analyzer"] = ConfigFile::ReadConfig("/Devices/" + pSerial + "/Analyzer");
  Config(CheckConfig(ConfigIN));
  req.Success(json::object());
}
// Create (or recreate) analyzer matching the stream. Returns false if stream parameters are unusable.
bool DeviceS1::EnsureAnalyzer(const Frame_t &frame) {
  if(frame.Sampling <= 0) return false;
  Analyzer::AnalyzerMode Mode;
  if(pModeConfig == "AUTO") {
    Mode = (frame.PolyPhase == false) ? Analyzer::PHASE1 : (frame.Neutral == true) ? Analyzer::PHASE3N : Analyzer::PHASE3;
  } else {
    Mode = Analyzer::StringToMode(pModeConfig);
  }
  float Rate = (frame.PerPeriod == true) ? frame.Sampling * 50.0 : frame.Sampling * 100.0;
  if((pAnalyzer != NULL) && (pMode == Mode) && (pNominalRate == Rate) && (pPerPeriod == frame.PerPeriod)) return true;
  if(pAnalyzer != NULL) LogFile::Warning("DeviceS1: stream parameters changed, recreating analyzer");
  ClearAnalyzers();
  string ModeString = (Mode == Analyzer::PHASE1) ? "_1P" : (Mode == Analyzer::PHASE3) ? "_3P" : "_3PN";
  LogFile::Info("DeviceS1: meter '%s', %s, %s sampling (%d), creating analyzer in mode %s at %.0f Hz", frame.MeterID, (frame.PolyPhase ? "poly phase" : "single phase"), (frame.PerPeriod ? "per period" : "per second"), frame.Sampling, Analyzer::ModeToString(Mode).c_str(), Rate);
  if(frame.Version != 0) LogFile::Warning("DeviceS1: unknown data format version %u, decoding as version 0", frame.Version);
  json AnalyzerConfig = ConfigFile::ReadConfig("/Devices/" + pSerial + "/Analyzer");
  if(AnalyzerConfig.is_object() == false) AnalyzerConfig = json::object();
  // Defaults for analyzers never configured by the user: S1 ports only exist in Belgium. A configured value is not changed.
  bool Unnamed = (AnalyzerConfig.contains("Name") == false) || (AnalyzerConfig["Name"] == "Unnamed analyzer");
  bool NoZone  = (AnalyzerConfig.contains("TimeZone") == false) || ((Unnamed == true) && (AnalyzerConfig["TimeZone"] == "UTC"));
  if(Unnamed == true) AnalyzerConfig["Name"]     = "Fluvius S1 meter";
  if(NoZone == true)  AnalyzerConfig["TimeZone"] = "Europe/Brussels";
  if((Unnamed == true) || (NoZone == true)) ConfigFile::WriteConfig("/Devices/" + pSerial + "/Analyzer", AnalyzerConfig);
  AnalyzerSoft *New = new AnalyzerSoft(pSerial + ModeString, Mode, "/Devices/" + pSerial + "/Analyzer", Rate);
  pthread_mutex_lock(&pLock);
  pAnalyzer     = New;
  pMode         = Mode;
  pNominalRate  = Rate;
  pPerPeriod    = frame.PerPeriod;
  pHaveHeld     = false;
  pHaveSequence = false;
  pMeasuredRate = 0.0;
  pthread_mutex_unlock(&pLock);
  // Per second sampling: quartz controlled, long window, announced with 100 Hz resolution. Per period sampling: the rate
  // follows the grid frequency, short window, +-15% as in EN 50160 (only used if the meter does not send the frequency).
  pRate.Config(Rate, (frame.PerPeriod == true) ? Rate * 0.15 : 100.0, (frame.PerPeriod == true) ? DeviceS1Rate::MIN_POINTS : 600);
  pNewSamples = 0;
  AnalyzerManager::AddAnalyzer(New);
  return true;
}
void DeviceS1::ProcessFrame(const Frame_t &frame) {
  if(EnsureAnalyzer(frame) == false) return;
  float Frequency = (frame.Frequency > 0.0) ? frame.Frequency : NAN;
  float Rate      = ((frame.PerPeriod == true) && !isnan(Frequency)) ? frame.Sampling * Frequency : pRate.Rate();
  if(pHaveSequence == true) {
    uint8_t Gap = frame.Sequence - pLastSequence - 1;
    pNewSamples  += Gap;
    if(Gap != 0) {
      pthread_mutex_lock(&pLock);
      pLostFrames += Gap;
      pthread_mutex_unlock(&pLock);
      if((Gap <= MAX_GAP_FILL) && (pHaveHeld == true)) {
        for(uint8_t n = 0; n < Gap; n++) pAnalyzer->BufferAppend(pHeldVoltage, pHeldCurrent, Frequency, Rate);
      }
    }
  }
  pLastSequence = frame.Sequence;
  pHaveSequence = true;
  pNewSamples++;
  if((frame.Valid == true) || (pHaveHeld == false)) {
    memcpy(pHeldVoltage, frame.Voltage, sizeof(pHeldVoltage));
    memcpy(pHeldCurrent, frame.Current, sizeof(pHeldCurrent));
    pHaveHeld = true;
  }
  if(frame.Valid == false) {
    pthread_mutex_lock(&pLock);
    pInvalidSamples++;
    pthread_mutex_unlock(&pLock);
  }
  pAnalyzer->BufferAppend(pHeldVoltage, pHeldCurrent, Frequency, Rate);
}
void *DeviceS1::Thread() {
  uint8_t  Raw[4096];
  DeviceS1Parser Parser;
  int      FD            = -1;
  uint64_t LastOpenTry   = 0;
  uint64_t LastFrameTime = 0;
  bool     PortErrLogged = false;
  while(pTerminate == false) {
    uint64_t Now = Tools::GetTime64();
    if(FD < 0) {
      if((Now - LastOpenTry) < 5000) {
        usleep(100000);
        continue;
      }
      LastOpenTry = Now;
      FD = DeviceS1Serial::Open(pPort.c_str(), BAUDRATE, 5);
      if(FD < 0) {
        if(PortErrLogged == false) LogFile::Error("DeviceS1: can not open '%s' at %u baud -> %s", pPort.c_str(), BAUDRATE, strerror(errno));
        PortErrLogged = true;
        continue;
      }
      LogFile::Info("DeviceS1: port '%s' opened", pPort.c_str());
      PortErrLogged = false;
      Parser.Reset();
      pRate.Restart();
      pNewSamples   = 0;
      pHaveSequence = false;
      LastFrameTime = Now;
      pthread_mutex_lock(&pLock);
      pConnected = true;
      pthread_mutex_unlock(&pLock);
    }
    ssize_t Readed = read(FD, Raw, sizeof(Raw));
    if(Readed < 0) {
      if((errno == EINTR) || (errno == EAGAIN)) continue;
      LogFile::Error("DeviceS1: read error on '%s' -> %s", pPort.c_str(), strerror(errno));
      close(FD);
      FD = -1;
      pthread_mutex_lock(&pLock);
      pConnected = false;
      pReceiving = false;
      pthread_mutex_unlock(&pLock);
      continue;
    }
    struct timespec ReadTime;
    clock_gettime(CLOCK_MONOTONIC, &ReadTime);
    uint64_t CRCBefore = Parser.CRCErrors();
    uint64_t DiscBefore = Parser.Discarded();
    Parser.Push(Raw, Readed, [&](const Frame_t &frame) {
      ProcessFrame(frame);
      pthread_mutex_lock(&pLock);
      pLastFrame = frame;
      pFramesOK++;
      if(pReceiving == false) LogFile::Info("DeviceS1: receiving frames");
      pReceiving = true;
      pthread_mutex_unlock(&pLock);
      LastFrameTime = Now;
    });
    if(pNewSamples > 0) {
      bool Known = pRate.Valid();
      pRate.Push(ReadTime.tv_sec + ReadTime.tv_nsec / (double)1e9, pNewSamples);
      pNewSamples = 0;
      if(pRate.Valid() == true) {
        if(Known == false) LogFile::Info("DeviceS1: announced sample rate %.0f Hz, measured %.2f Hz", pNominalRate, pRate.Rate());
        pthread_mutex_lock(&pLock);
        pMeasuredRate = pRate.Rate();
        pthread_mutex_unlock(&pLock);
      }
    }
    if((Parser.CRCErrors() != CRCBefore) || (Parser.Discarded() != DiscBefore)) {
      pthread_mutex_lock(&pLock);
      pCRCErrors += Parser.CRCErrors() - CRCBefore;
      pResyncs   += Parser.Discarded() - DiscBefore;
      pthread_mutex_unlock(&pLock);
    }
    if((pReceiving == true) && ((Now - LastFrameTime) > 3000)) {
      LogFile::Warning("DeviceS1: no valid frames received for 3 seconds (is the S1 port activated by Fluvius?)");
      pthread_mutex_lock(&pLock);
      pReceiving = false;
      pthread_mutex_unlock(&pLock);
    }
  }
  if(FD >= 0) close(FD);
  pthread_mutex_lock(&pLock);
  pConnected = false;
  pReceiving = false;
  pthread_mutex_unlock(&pLock);
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------
