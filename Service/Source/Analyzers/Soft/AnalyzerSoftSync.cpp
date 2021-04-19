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
#include "LogFile.h"
#include "URL.h"
#include "AnalyzerSoftSync.h"
#include "AnalyzerSoft.h"
#include "ConfigFile.h"
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftSync::AnalyzerSoftSync(const string &id, const string &path, const Analyzer::AnalyzerMode mode, float samplerate, function<void(const string &name, const json &params)> handleAction, function<json(const vector<string> &series)> status, function<json(const vector<string> &series)> now) :  pSerial(id), pConfigPath(path), pMode(mode), pSampleRate(samplerate), pHandleAction(handleAction), pStatus(status), pNow(now) {
  pMutex  = PTHREAD_MUTEX_INITIALIZER;
  pThread = 0;
}
AnalyzerSoftSync::~AnalyzerSoftSync() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
}
bool AnalyzerSoftSync::ShouldPush() {
  pthread_mutex_lock(&pMutex);
  uint16_t Count = pPendingActions.size();
  pthread_mutex_unlock(&pMutex);
  return (Count < pMaxActions);
}
void AnalyzerSoftSync::Push(const string name, const json &params) {
  json Action = json::object();
  Action["Type"] = name;
  Action["Params"] = params;
  pthread_mutex_lock(&pMutex);
  pPendingActions.push(Action);
  pthread_mutex_unlock(&pMutex);
}
json AnalyzerSoftSync::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Server"] = "";
  ConfigOUT["User"] = "";
  ConfigOUT["Pass"] = "";
  if(config.is_object() == true) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Server") && config["Server"].is_string()) ConfigOUT["Server"] = config["Server"];
    if(config.contains("User") && config["User"].is_string()) ConfigOUT["User"] = config["User"];
    if(config.contains("Pass") && config["Pass"].is_string()) ConfigOUT["Pass"] = config["Pass"];
  }
  return ConfigOUT;
}
void AnalyzerSoftSync::Config(const json &config) {
  pthread_mutex_lock(&pMutex);
  pServer = config["Server"];
  pUser   = config["User"];
  pPass   = config["Pass"];
  if(config["Enabled"] == false) {
    pTerminate = true;
    if(pThread != 0) {
      pthread_join(pThread, NULL);
      pThread = 0;
    }
  } else {
    pTerminate = false;
    if(pThread == 0) {
      if(pthread_create(&pThread, NULL, [](void *params) { return ((AnalyzerSoftSync*)params)->Thread(); }, this) != 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        pThread = 0;
      } else {
        if(pthread_setname_np(pThread, "Sync") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        TaskPool::SetThreadProperties(pThread, TaskPool::LOW);
      }
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void *AnalyzerSoftSync::Thread() {
  string         Cookie, Path, Server, User, Pass;
  vector<string> RequestNow, RequestStatus;
  Client        *HTTPClient      = NULL;
  uint64_t       LastSample200MS = UINT64_MAX;
  uint64_t       LastSample3S    = UINT64_MAX;
  uint64_t       LastSample1M    = UINT64_MAX;
  uint64_t       LastSample10M   = UINT64_MAX;
  uint64_t       LastSample15M   = UINT64_MAX;
  uint64_t       LastSample1H    = UINT64_MAX;
  uint64_t       LastEvent       = UINT64_MAX;
  uint64_t       LastAlarm       = UINT64_MAX;
  Stage          Stage           = LOGIN;
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    if((User != pUser) || (Pass != pPass) || (Server != pServer)) {
      User   = pUser;
      Pass   = pPass;
      Server = pServer;
      Stage  = LOGIN;
    }
    pthread_mutex_unlock(&pMutex);
    if(Stage == LOGIN) {
      if(HTTPClient != NULL) delete HTTPClient;
      HTTPClient = NULL;
      pthread_mutex_lock(&pMutex);
      URL  Url(pServer);
      json Content = { {"User", pUser}, {"Password", pPass}, {"UpdateLastLogin", false} };
      Path         = Url.Path();
      HTTPClient   = new Client(Url.Host(), Url.Port());
      pthread_mutex_unlock(&pMutex);
      HTTPClient->set_compress(true);
      HTTPClient->set_keep_alive(true);
      HTTPClient->set_read_timeout(120);
      HTTPClient->set_write_timeout(120);
      vector<uint8_t> Buff = json::to_cbor(Content);
      auto Response = HTTPClient->Post((Path + "/logIn").c_str(), { {"Accept", "application/cbor"} }, (char*)Buff.data(), Buff.size(), "application/cbor");
      if((Response != nullptr) && (Response->status == HTTP_OK) && (Response->has_header("Set-Cookie") == true)) {
        LogFile::Debug("Analyzer '%s' logged on remote server", pSerial.c_str());
        Cookie = Response->get_header_value("Set-Cookie");
        Stage = CONFIG;
      } else {
        usleep(1000000);
      }
      continue;
    }
    json Content = { {"Analyzer", pSerial} };
    bool Synced  = true;
    if(Stage == CONFIG) {
      Content["Config"]     = ConfigFile::ReadConfig(pConfigPath);
      Content["SampleRate"] = pSampleRate;
      Content["Mode"]       = (pMode == Analyzer::PHASE1) ? "1PHASE" : (pMode == Analyzer::PHASE3) ? "3PHASE" : "3PHASE+N";
      Content["Synced"]     = false;
    } else {
      Database DB(pSerial);
      json Actions = json::array();
      if(LastSample1H != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "1H"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) FROM aggreg_1h WHERE sampletime > " + DB.Bind(LastSample1H) + " ORDER BY sampletime ASC LIMIT 2");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 1)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastSample15M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "15M"}, {"Samples", json::array()}, {"Headers", {"Time", "Active_Power", "Reactive_Power"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, active_power, reactive_power) FROM aggreg_15m WHERE sampletime > " + DB.Bind(LastSample15M) + " ORDER BY sampletime ASC LIMIT 5");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 4)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastSample10M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "10M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i) FROM aggreg_10m WHERE sampletime > " + DB.Bind(LastSample10M) + " ORDER BY sampletime ASC LIMIT 7");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 6)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastSample1M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "1M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i, harmonics_p) FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m WHERE sampletime > " + DB.Bind(LastSample1M) + " ORDER BY sampletime ASC LIMIT 61");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 60)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastSample3S != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "3S"}, {"Samples", json::array()}, {"Headers", { "Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, reactive_power) FROM aggreg_3s WHERE sampletime > " + DB.Bind(LastSample3S) + " ORDER BY sampletime ASC LIMIT 1201");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 1200)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastSample200MS != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "200MS"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Power_Harmonics"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i, harmonics_p) FROM aggreg_200ms WHERE sampletime > " + DB.Bind(LastSample200MS) + " ORDER BY sampletime ASC LIMIT 51");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Samples"].size() == 50)
            Synced = false;
          else
            Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
      }
      if(LastAlarm != UINT64_MAX) {
        json Action = {{"Type", "SetAlarms"}, {"Params", {{"Alarms", json::array()}, {"Headers", {"ChangeTime", "AlarmID", "Trigger", "Release", "Notes"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, id, trigger, release, notes) as result FROM alarms WHERE changetime > " + DB.Bind(LastAlarm) + " ORDER BY changetime ASC LIMIT 11");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Alarms"].size() == 10)
            Synced = false;
          else
            Action["Params"]["Alarms"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Alarms"].size() > 0) Actions.push_back(Action);
      }
      if(LastEvent != UINT64_MAX) {
        json Action = {{"Type", "SetEvents"}, {"Params", {{"Events", json::array()}, {"Headers", {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes", "LengthRAW", "SamplesRAW", "LengthRMS", "SamplesRMS"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, starttime, evttype, samplerate, reference, peak, change, duration, notes, size_raw, samples_raw, size_rms, samples_rms) FROM events WHERE changeTime > " + DB.Bind(LastEvent) + " ORDER BY changetime ASC LIMIT 2");
        while(DB.FecthRow() == true) {
          if(Action["Params"]["Events"].size() == 1)
            Synced = false;
          else
            Action["Params"]["Events"].push_back(DB.ResultJSON(0));
        }
        if(Action["Params"]["Events"].size() > 0) Content["Actions"].push_back(Action);
      }
      pthread_mutex_lock(&pMutex);
      while(pPendingActions.empty() == false) {
        Actions.push_back(pPendingActions.front());
        pPendingActions.pop();
      }
      if(Actions.size() > 0) Content["Actions"] = Actions;
      pthread_mutex_unlock(&pMutex);
      if(RequestNow.size()) {
        json Now = pNow(RequestNow);
        if(Now != nullptr) Content["Now"] = Now;
      }
      if(RequestStatus.size()) {
        json Status = pStatus(RequestStatus);
        if(Status != nullptr) Content["Status"] = Status;
      }
      Content["Synced"] = Synced;
    }
    vector<uint8_t> Buff = json::to_cbor(Content);
    LogFile::Debug("AnalyzerSoft '%s' send %d actions to AnalyzerRemote (%d bytes)", pSerial.c_str(), Content["Actions"].size(), Buff.size());
    auto Response = HTTPClient->Post((Path + "/setRemote").c_str(), {{"Accept", "application/cbor"}, {"Cookie", Cookie}}, (char*)Buff.data(), Buff.size(), "application/cbor");
    if((Response != nullptr) && (Response->status == HTTP_OK)) {
      Content = json::from_cbor(Response->body, NULL, false);
      if(Content.is_discarded()) {
        LogFile::Error("Malformed JSON params in AnalyzerSoft::Sync");
        Stage = LOGIN;
        usleep(1000000);
      } else {
        RequestNow.clear();
        if(Content.contains("Now") && Content["Now"].is_array()) {
          for(auto &Item : Content["Now"]) {
            if(Item.is_string()) RequestNow.push_back(Item);
          }
        }
        RequestStatus.clear();
        if(Content.contains("Status") && Content["Status"].is_array()) {
          for(auto &Item : Content["Status"]) {
            if(Item.is_string()) RequestStatus.push_back(Item);
          }
        }
        if(Content.contains("Time_200MS")  && Content["Time_200MS"].is_number())  LastSample200MS = Content["Time_200MS"];
        if(Content.contains("Time_3S")     && Content["Time_3S"].is_number())     LastSample3S    = Content["Time_3S"];
        if(Content.contains("Time_1M")     && Content["Time_1M"].is_number())     LastSample1M    = Content["Time_1M"];
        if(Content.contains("Time_10M")    && Content["Time_10M"].is_number())    LastSample10M   = Content["Time_10M"];
        if(Content.contains("Time_15M")    && Content["Time_15M"].is_number())    LastSample15M   = Content["Time_15M"];
        if(Content.contains("Time_1H")     && Content["Time_1H"].is_number())     LastSample1H    = Content["Time_1H"];
        if(Content.contains("Time_Alarms") && Content["Time_Alarms"].is_number()) LastAlarm       = Content["Time_Alarms"];
        if(Content.contains("Time_Events") && Content["Time_Events"].is_number()) LastEvent       = Content["Time_Events"];
        if(Content.contains("Actions") && Content["Actions"].is_array()) {
          for(auto &Action : Content["Actions"]) {
            if(Action.contains("Type") && Action["Type"].is_string() && Action.contains("Params")) pHandleAction(Action["Type"], Action["Params"]);
          }
        }
        Stage = CONNECTED;
        if(Synced == true) usleep(1000000);
      }
    } else {
      if(Response != nullptr) {
        if(Response->body.empty() == false)
          LogFile::Warning("AnalyzerSoft '%s' receive error from AnalyzerRemote (%d -> %s)", pSerial.c_str(), Response->status, Response->body.c_str());
        else
          LogFile::Warning("AnalyzerSoft '%s' receive error from AnalyzerRemote (%d)", pSerial.c_str(), Response->status);
      } else
        LogFile::Warning("AnalyzerSoft '%s' fail while sending data to AnalyzerRemote", pSerial.c_str());
      Stage = LOGIN;
      usleep(1000000);
    }
  }
  if(HTTPClient != NULL) delete HTTPClient;
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------