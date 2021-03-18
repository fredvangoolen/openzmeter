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
AnalyzerSoftSync::AnalyzerSoftSync(const string &serial, const Analyzer::AnalyzerMode mode, float samplerate) {
  pMutex      = PTHREAD_MUTEX_INITIALIZER;
  pSerial     = serial;
  pThread     = 0;
  pStage      = DISABLED;
  pMode       = mode;
  pSampleRate = samplerate;
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
void AnalyzerSoftSync::Config(const json &config, const string &path, function<void(const string &name, const json &params)> handleAction, function<json(const vector<string> &series)> status, function<json(const vector<string> &series)> now) {
  pthread_mutex_lock(&pMutex);
  if((pThread != 0) && (pStage == DISABLED)) {
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  pHandleAction    = handleAction;
  pStatus          = status;
  pNow             = now;
  pConfigPath      = path;
  pServer          = config["Server"];
  pUser            = config["User"];
  pPass            = config["Pass"];
  pStage           = (config["Enabled"] == true) ? LOGIN : DISABLED;
  pTerminate       = false;
  if((pStage != DISABLED) && (pThread == 0)) {
    if(pthread_create(&pThread, NULL, [](void *params) { return ((AnalyzerSoftSync*)params)->Thread(); }, this) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pThread = 0;
    } else {
      Worker::SetThreadProperties(pThread, "Sync", Worker::LOW);
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftSync::Push(const string name, const json &params) {
  json Action = json::object();
  Action["Type"] = name;
  Action["Params"] = params;
  if(Action["Params"].contains("UserName")) Action["Params"].erase("UserName");
  pthread_mutex_lock(&pMutex);
  pPendingActions.push(Action);
  pthread_mutex_unlock(&pMutex);
}
void *AnalyzerSoftSync::Thread() {
  string         Cookie;
  string         Path;
  vector<string> RequestNow;
  vector<string> RequestStatus;
  Client        *HTTPClient      = NULL;
  uint64_t       LastSample200MS = UINT64_MAX;
  uint64_t       LastSample3S    = UINT64_MAX;
  uint64_t       LastSample1M    = UINT64_MAX;
  uint64_t       LastSample10M   = UINT64_MAX;
  uint64_t       LastSample15M   = UINT64_MAX;
  uint64_t       LastSample1H    = UINT64_MAX;
  uint64_t       LastEvent       = UINT64_MAX;
  uint64_t       LastAlarm       = UINT64_MAX;
  bool           Wait            = false;
  pthread_mutex_lock(&pMutex);
  while((pTerminate == false) && (pStage != DISABLED)) {
    if(Wait) {
      pthread_mutex_unlock(&pMutex);
      usleep(100000);
      pthread_mutex_lock(&pMutex);
    }
    if(pStage == LOGIN) {
      URL Url(pServer);
      json    Content = json::object();
      Content["UserName"]        = pUser;
      Content["Password"]        = pPass;
      Content["UpdateLastLogin"] = false;
      if(HTTPClient != NULL) delete HTTPClient;
      HTTPClient = NULL;
      Path       = Url.Path();
      HTTPClient = new Client(Url.Host(), Url.Port());
      HTTPClient->set_compress(true);
      HTTPClient->set_keep_alive(true);
      HTTPClient->set_read_timeout(30);
      HTTPClient->set_write_timeout(30);
      HTTPClient->set_connection_timeout(30);
      pthread_mutex_unlock(&pMutex);
      auto Response = HTTPClient->Post((Path + "/logIn").c_str(), { {"Accept", "application/json"} }, Content.dump(), "application/json");
      pthread_mutex_lock(&pMutex);
      if((pStage == LOGIN) && (Response != nullptr) && (Response->status == HTTP_OK) && (Response->has_header("Set-Cookie") == true)) {
        LogFile::Debug("Analyzer '%s' logged on remote server", pSerial.c_str());
        Cookie = Response->get_header_value("Set-Cookie");
        pStage = CONFIG;
        Wait = false;
      } else {
        Wait = true;
      }
    } else {
      json Content        = json::object();
      Content["Analyzer"] = pSerial;
      if(pStage == CONFIG) {
        Content["Config"]     = ConfigFile::ReadConfig(pConfigPath);
        Content["SampleRate"] = pSampleRate;
        Content["Mode"]       = (pMode == Analyzer::PHASE1) ? "1PHASE" : (pMode == Analyzer::PHASE3) ? "3PHASE" : "3PHASE+N";
        Content["Synced"]     = false;
        Wait = false;
      } else {
        Database DB(pSerial);
        json Actions = json::array();
        bool Synced  = true;
        if((LastSample1H != UINT64_MAX) && Synced) {
          static const uint16_t Max1HSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "1H"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, active_power, reactive_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) FROM aggreg_1h WHERE sampletime > " + DB.Bind(LastSample1H) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max1HSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max1HSamples) Synced = false;
        }
        if((LastSample15M != UINT64_MAX) && Synced) {
          static const uint16_t Max15MSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "15M"}, {"Samples", json::array()}, {"Headers", {"Time", "Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, active_power, ARRAY(SELECT UNNEST(active_power) / 4.0), reactive_power, ARRAY(SELECT UNNEST(reactive_power) / 4.0), NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')) FROM aggreg_15m WHERE sampletime > " + DB.Bind(LastSample15M) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max15MSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max15MSamples) Synced = false;
        }
        if((LastSample10M != UINT64_MAX) && Synced) {
          static const uint16_t Max10MSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "10M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i) FROM aggreg_10m WHERE sampletime > " + DB.Bind(LastSample10M) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max10MSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max10MSamples) Synced = false;
        }
        if((LastSample1M != UINT64_MAX) && Synced) {
          static const uint16_t Max1MSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "1M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i, harmonics_p) FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m WHERE sampletime > " + DB.Bind(LastSample1M) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max1MSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max1MSamples) Synced = false;
        }
        if((LastSample3S != UINT64_MAX) && Synced) {
          static const uint16_t Max3SSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "3S"}, {"Samples", json::array()}, {"Headers", { "Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')) FROM aggreg_3s WHERE sampletime > " + DB.Bind(LastSample3S) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max3SSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max3SSamples) Synced = false;
        }
        if((LastSample200MS != UINT64_MAX) && Synced) {
          static const uint16_t Max200MSSamples = 10;
          json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "200MS"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics", "Current_Harmonics_Complex", "Power_Harmonics"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), power_factor, thd_v, thd_i, phi, (SELECT array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3), harmonics_v, (SELECT array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3), harmonics_i, harmonics_p) FROM aggreg_200ms WHERE sampletime > " + DB.Bind(LastSample200MS) + " ORDER BY sampletime ASC LIMIT " + DB.Bind(Max200MSSamples));
          while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Samples"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Samples"].size() == Max200MSSamples) Synced = false;
        }
        if((LastAlarm != UINT64_MAX) && Synced) {
          static const uint16_t MaxAlarmsSamples = 10;
          json Action = {{"Type", "SetAlarms"}, {"Params", {{"Alarms", json::array()}, {"Headers", {"ChangeTime", "AlarmID", "Trigger", "Release"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, id, trigger, release) as result FROM alarms WHERE changetime > " + DB.Bind(LastAlarm) + " ORDER BY trigger ASC LIMIT " + DB.Bind(MaxAlarmsSamples));
          while(DB.FecthRow() == true) Action["Params"]["Alarms"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Alarms"].size() > 0) Actions.push_back(Action);
          if(Action["Params"]["Alarms"].size() == MaxAlarmsSamples) Synced = false;
        }
        if((LastEvent != UINT64_MAX) && Synced) {
          json Action = {{"Type", "SetEvents"}, {"Params", {{"Events", json::array()}, {"Headers", {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes", "LengthRAW", "SamplesRAW", "LengthRMS", "SamplesRMS"}}}}};
          DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, starttime, evttype, samplerate, reference, peak, change, duration, notes, "
            "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'),"
            " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
            "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RAW_PRESTART') || (SELECT samples FROM ext where sampletype = 'RAW_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RAW_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RAW_POSEND')), "
            "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART'), "
            " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
            "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RMS_PRESTART') || (SELECT samples FROM ext where sampletype = 'RMS_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RMS_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RMS_POSEND'))"
            ") AS result FROM events WHERE changeTime > " + DB.Bind(LastEvent) + " ORDER BY starttime ASC LIMIT 1");
          while(DB.FecthRow() == true) Action["Params"]["Events"].push_back(DB.ResultJSON(0));
          if(Action["Params"]["Events"].size() > 0) Content["Actions"].push_back(Action);
          if(Action["Params"]["Events"].size() == 1) Synced = false;
        }
        while((pPendingActions.empty() == false) && Synced) {
          Actions.push_back(pPendingActions.front());
          pPendingActions.pop();
        }
        if(Actions.size() > 0) Content["Actions"] = Actions;
        if(RequestNow.size()) {
          json Now = pNow(RequestNow);
          if(Now != nullptr) Content["Now"] = Now;
        }
        if(RequestStatus.size()) {
          json Status = pStatus(RequestStatus);
          if(Status != nullptr) Content["Status"] = Status;
        }
        Content["Synced"] = Synced;
        Wait = Synced;
      }
      pthread_mutex_unlock(&pMutex);
      LogFile::Debug("AnalyzerSoft '%s' send %d actions to AnalyzerRemote", pSerial.c_str(), Content["Actions"].size());
      auto Response = HTTPClient->Post((Path + "/setRemote").c_str(), {{"Accept", "application/json"}, {"Cookie", Cookie}}, Content.dump(), "application/json");
      pthread_mutex_lock(&pMutex);
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        Content = json::parse(Response->body, NULL, false);
        if(Content.is_discarded()) {
          LogFile::Error("Malformed JSON params in AnalyzerSoft::Sync");
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
          pStage = CONNECTED;
        }
      } else {
        if(Response != nullptr)
          LogFile::Warning("AnalyzerSoft '%s' receive error code %d from AnalyzerRemote", pSerial.c_str(), Response->status);
        else
          LogFile::Warning("AnalyzerSoft '%s' fail while sending data to AnalyzerRemote", pSerial.c_str());
        pStage = LOGIN;
        Wait   = true;
      }
    }
  }
  pStage = DISABLED;
  pthread_mutex_unlock(&pMutex);
  if(HTTPClient != NULL) delete HTTPClient;
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------