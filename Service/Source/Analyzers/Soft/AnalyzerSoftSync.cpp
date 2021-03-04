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
  pRequestNow      = false;
  pLastSample200MS = UINT64_MAX;
  pLastSample3S    = UINT64_MAX;
  pLastSample1M    = UINT64_MAX;
  pLastSample10M   = UINT64_MAX;
  pLastSample15M   = UINT64_MAX;
  pLastSample1H    = UINT64_MAX;
  pLastEvent       = UINT64_MAX;
  pLastAlarm       = UINT64_MAX;
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
void AnalyzerSoftSync::PushAction(const string name, const json &params) {
  json Action = json::object();
  Action["Type"] = name;
  Action["Params"] = params;
  if(Action["Params"].contains("UserName")) Action["Params"].erase("UserName");
  pthread_mutex_lock(&pMutex);
  pPendingActions.push(Action);
  pthread_mutex_unlock(&pMutex);
}
void *AnalyzerSoftSync::Thread() {
  string  Cookie, Path;
  Client *HTTPClient = NULL;
  pthread_mutex_lock(&pMutex);
  while((pTerminate == false) && (pStage != DISABLED)) {
    pthread_mutex_unlock(&pMutex);
    usleep(1000000);
    pthread_mutex_lock(&pMutex);
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
      }
    } else {
      Database DB(pSerial);
      json Content        = json::object();
      Content["Actions"]  = json::array();
      Content["Analyzer"] = pSerial;
      if(pStage == CONFIG) {
        Content["Config"]     = ConfigFile::ReadConfig(pConfigPath);
        Content["SampleRate"] = pSampleRate;
        Content["Mode"]       = (pMode == Analyzer::PHASE1) ? "1PHASE" : (pMode == Analyzer::PHASE3) ? "3PHASE" : "3PHASE+N";
      }
      if(pRequestNow == true) {
        json Now = pNow({"Frequency", "Flag", "Unbalance", "Voltage", "Current", "Active_Power", "Reactive_Power", "Active_Energy", "Reactive_Energy", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Phase", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Voltage_FFT", "Current_FFT", "Voltage_Samples", "Current_Samples"});
        if(Now != nullptr) Content["Now"] = Now;
      }
      if(pRequestStatus == true) {
        json Status = pStatus({"Log", "System", "Events", "Alarms", "Storage"});
        if(Status != nullptr) Content["Status"] = Status;
      }
      if(pLastSample200MS != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "200MS"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics", "Current_Harmonics_Complex", "Power_Harmonics"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), power_factor, thd_v, thd_i, phi, (SELECT array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3), harmonics_v, (SELECT array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3), harmonics_i, harmonics_p) FROM aggreg_200ms WHERE sampletime >= " + DB.Bind(pLastSample200MS) + " ORDER BY sampletime ASC LIMIT 100");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastSample3S != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "3S"}, {"Samples", json::array()}, {"Headers", { "Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')) FROM aggreg_3s WHERE sampletime >= " + DB.Bind(pLastSample3S) + " ORDER BY sampletime ASC LIMIT 50");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastSample1M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "1M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, unbalance, flag, rms_v, rms_i, active_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), power_factor, thd_v, thd_i, phi, harmonics_v, harmonics_i, harmonics_p) FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m WHERE sampletime >= " + DB.Bind(pLastSample1M) + " ORDER BY sampletime ASC LIMIT 10");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastSample10M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{ "Aggreg", "10M"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i) FROM aggreg_10m WHERE sampletime >= " + DB.Bind(pLastSample10M) + " ORDER BY sampletime ASC LIMIT 10");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastSample15M != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "15M"}, {"Samples", json::array()}, {"Headers", {"Time", "Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, active_power, ARRAY(SELECT UNNEST(active_power) / 4.0), reactive_power, ARRAY(SELECT UNNEST(reactive_power) / 4.0), NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')) FROM aggreg_15m WHERE sampletime >= " + DB.Bind(pLastSample15M) + " ORDER BY sampletime ASC LIMIT 10");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastSample1H != UINT64_MAX) {
        json Action = {{"Type", "SetSeries"}, {"Params", {{"Aggreg", "1H"}, {"Samples", json::array()}, {"Headers", {"Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats"}}}}};
        DB.ExecSentenceAsyncResult("SELECT json_build_array(sampletime, freq, flag, rms_v, rms_i, active_power, active_power, reactive_power, reactive_power, NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}'), stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) FROM aggreg_1h WHERE sampletime >= " + DB.Bind(pLastSample1H) + " ORDER BY sampletime ASC LIMIT 10");
        while(DB.FecthRow() == true) Action["Params"]["Samples"].push_back(DB.ResultJSON(0));
        if(Action["Params"]["Samples"].size() > 1) Content["Actions"].push_back(Action);
      }
      if(pLastAlarm != UINT64_MAX) {
//        Content["Alarms"] = json::array();
      }
      if(pLastEvent != UINT64_MAX) {
//        Content["Events"] = json::array();
      }
      while((Content["Actions"].size() < 10) && (pPendingActions.empty() == false)) {
        Content["Actions"].push_back(pPendingActions.front());
        pPendingActions.pop();
      }
      pthread_mutex_unlock(&pMutex);
      auto Response = HTTPClient->Post((Path + "/setRemote").c_str(), {{"Accept", "application/json"}, {"Cookie", Cookie}}, Content.dump(), "application/json");
      pthread_mutex_lock(&pMutex);
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if(Content.is_discarded()) {
          LogFile::Error("Malformed JSON params in AnalyzerSoft::Sync");
        } else {
          if(Content.contains("Now") && Content["Now"].is_boolean())                pRequestNow      = Content["Now"];
          if(Content.contains("Status") && Content["Status"].is_boolean())          pRequestStatus   = Content["Status"];
          if(Content.contains("Time_200MS")  && Content["Time_200MS"].is_number())  pLastSample200MS = Content["Time_200MS"];
          if(Content.contains("Time_3S")     && Content["Time_3S"].is_number())     pLastSample3S    = Content["Time_3S"];
          if(Content.contains("Time_1M")     && Content["Time_1M"].is_number())     pLastSample1M    = Content["Time_1M"];
          if(Content.contains("Time_10M")    && Content["Time_10M"].is_number())    pLastSample10M   = Content["Time_10M"];
          if(Content.contains("Time_15M")    && Content["Time_15M"].is_number())    pLastSample15M   = Content["Time_15M"];
          if(Content.contains("Time_1H")     && Content["Time_1H"].is_number())     pLastSample1H    = Content["Time_1H"];
          if(Content.contains("Time_Alarms") && Content["Time_Alarms"].is_number()) pLastAlarm       = Content["Time_Alarms"];
          if(Content.contains("Time_Events") && Content["Time_Events"].is_number()) pLastEvent       = Content["Time_Events"];
          if(Content.contains("Actions") && Content["Actions"].is_array()) {
            for(auto &Action : Content["Actions"]) {
              if(Action.contains("Type") && Action["Type"].is_string() && Action.contains("Params")) pHandleAction(Action["Type"], Action["Params"]);
            }
          }
          pStage = CONNECTED;
        }
      } else {
        pStage = LOGIN;
      }
    }
  }
  pStage = DISABLED;
  pthread_mutex_unlock(&pMutex);
  if(HTTPClient != NULL) delete HTTPClient;
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------