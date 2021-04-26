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
#include "AnalyzerRemoteNow.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemoteNow::AnalyzerRemoteNow(const string &serial) : pSerial(serial) {
  pNow           = json::object();
  pStatus        = json::object();
  pNowMutex      = PTHREAD_MUTEX_INITIALIZER;
  pNowCond       = PTHREAD_COND_INITIALIZER;
  pStatusMutex   = PTHREAD_MUTEX_INITIALIZER;
  pStatusCond    = PTHREAD_COND_INITIALIZER;
  pNowRequest    = {{"Frequency", 0}, {"Flag", 0}, {"Unbalance", 0}, {"Voltage", 0}, {"Current", 0}, {"Active_Power", 0}, {"Reactive_Power", 0}, {"Active_Energy", 0}, {"Reactive_Energy", 0}, {"Apparent_Power", 0}, {"Power_Factor", 0}, {"Voltage_THD", 0}, {"Current_THD", 0}, {"Phi", 0}, {"Voltage_Phase", 0}, {"Voltage_Harmonics", 0}, {"Current_Harmonics", 0}, {"Power_Harmonics", 0}, {"Voltage_Harmonics_Complex", 0}, {"Current_Harmonics_Complex", 0}, {"Voltage_FFT", 0}, {"Current_FFT", 0}, {"Voltage_Samples", 0}, {"Current_Samples", 0}};
  pStatusRequest = {{"Log", 0}, {"System", 0}, {"SystemLog", 0}, {"Storage", 0}};
}
AnalyzerRemoteNow::~AnalyzerRemoteNow() {
}
void AnalyzerRemoteNow::SetRemote(const json &params, json &response) {
  json     Now    = json::array();
  json     Status = json::array();
  uint64_t Time   = Tools::GetTime64() - pTimeout;
  if(pthread_mutex_lock(&pNowMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(params.contains("Now") && params["Now"].is_object()) {
    pNow = params["Now"];
    if(pthread_cond_broadcast(&pNowCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  for(auto &Serie : pNowRequest) {
    if(Serie.second > Time) Now.push_back(Serie.first);
  }
  if(pthread_mutex_unlock(&pNowMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_lock(&pStatusMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(params.contains("Status") && params["Status"].is_object()) {
    pStatus = params["Status"];
    if(pthread_cond_broadcast(&pStatusCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  for(auto &Serie : pStatusRequest) {
    if(Serie.second > Time) Status.push_back(Serie.first);
  }
  if(pthread_mutex_unlock(&pStatusMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(Now.size() > 0) response["Now"] = Now;
  if(Status.size() > 0) response["Status"] = Status;
}
bool AnalyzerRemoteNow::CheckSeries(const json &params, const vector<string> &series, const uint64_t time) {
  if((params.contains("Time") == false) || (params["Time"].is_number() == false) || (params["Time"].get<uint64_t>() < (time - 30000))) return false;
  for(auto &Serie : series) {
    if(pNow.contains(Serie) == false) return false;
  }
  return true;
}
void AnalyzerRemoteNow::GetSeriesNow(HTTPRequest &req) {
  uint64_t       Time = Tools::GetTime64();
  timespec       To;
  vector<string> Series;
  json           Result = json::object();
  if(clock_gettime(CLOCK_REALTIME, &To) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  To.tv_sec += 10;
  if(pthread_mutex_lock(&pNowMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  for(auto &Serie : req.Param("Series")) {
    Series.push_back(Serie);
    pNowRequest.find(Serie)->second = Time;
  }
  json Tmp = Series;
  while((Series.size() > 0) && (CheckSeries(pNow, Series, Time) == false)) {
    int Error = pthread_cond_timedwait(&pNowCond, &pNowMutex, &To);
    if(Error != 0) {
      if(Error != ETIMEDOUT) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      if(pthread_mutex_unlock(&pNowMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return req.Error("", HTTP_METHOD_NOT_ALLOWED);
    }
  }
  Result["Time"] = pNow["Time"];
  for(auto &Serie : Series) Result[(string)Serie] = pNow[(string)Serie];
  if(pthread_mutex_unlock(&pNowMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  req.Success(Result);
}
void AnalyzerRemoteNow::GetStatus(HTTPRequest &req, const json extraProps) {
  uint64_t       Time = Tools::GetTime64();
  timespec       To;
  vector<string> Series;
  json           Result = extraProps;
  if(clock_gettime(CLOCK_REALTIME, &To) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  To.tv_sec += 10;
  if(pthread_mutex_lock(&pStatusMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  for(auto &Serie : req.Param("Series")) {
    if(Serie == "Events") {
      Result["Events"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE changetime > eventsread) AS events FROM permissions WHERE username = " + DB.Bind(req.UserName()) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == false) {
        Result["Events"]["LastRead"] = 0;
        Result["Events"]["Count"]    = 0;
      } else {
        Result["Events"]["LastRead"] = DB.ResultUInt64("eventsread");
        Result["Events"]["Count"]    = DB.ResultUInt64("events");
      }
    } else if(Serie == "Alarms") {
      Result["Alarms"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT alarmsread, (SELECT count(*) FROM alarms WHERE changetime > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(req.UserName()) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == false) {
        Result["Alarms"]["LastRead"] = 0;
        Result["Alarms"]["Count"]    = 0;
      } else {
        Result["Alarms"]["LastRead"] = DB.ResultUInt64("alarmsread");
        Result["Alarms"]["Count"]    = DB.ResultUInt64("alarms");
      }
    } else {
      pStatusRequest.find(Serie)->second = Time;
      Series.push_back(Serie);
    }
  }
  while((Series.size() > 0) && (CheckSeries(pStatus, Series, Time) == false)) {
    int Error = pthread_cond_timedwait(&pStatusCond, &pStatusMutex, &To);
    if(Error != 0) {
      if(Error != ETIMEDOUT) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      if(pthread_mutex_unlock(&pStatusMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return req.Error("", HTTP_METHOD_NOT_ALLOWED);
    }
  }
  Result["Time"] = pStatus["Time"];
  for(auto &Serie : Series) Result[(string)Serie] = pStatus[(string)Serie];
  if(pthread_mutex_unlock(&pStatusMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  req.Success(Result);
}
// ---------------------------------------------------------------------------------------------------------------------