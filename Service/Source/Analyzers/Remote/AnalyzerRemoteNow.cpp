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
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemoteNow::AnalyzerRemoteNow() {
  pNow           = json::object();
  pStatus        = json::object();
  pLock          = PTHREAD_RWLOCK_INITIALIZER;
  pNowRequest    = {{"Frequency", 0}, {"Flag", 0}, {"Unbalance", 0}, {"Voltage", 0}, {"Current", 0}, {"Active_Power", 0}, {"Reactive_Power", 0}, {"Active_Energy", 0}, {"Reactive_Energy", 0}, {"Apparent_Power", 0}, {"Power_Factor", 0}, {"Voltage_THD", 0}, {"Current_THD", 0}, {"Phi", 0}, {"Voltage_Phase", 0}, {"Voltage_Harmonics", 0}, {"Current_Harmonics", 0}, {"Power_Harmonics", 0}, {"Voltage_Harmonics_Complex", 0}, {"Current_Harmonics_Complex", 0}, {"Voltage_FFT", 0}, {"Current_FFT", 0}, {"Voltage_Samples", 0}, {"Current_Samples", 0}};
  pStatusRequest = {{"Log", 0}, {"System", 0}, {"SystemLog", 0}, {"Events", 0}, {"Alarms", 0}, {"Storage", 0}};
}
AnalyzerRemoteNow::~AnalyzerRemoteNow() {
}
void AnalyzerRemoteNow::SetRemote(const json &params, json &response) {
  if(params.contains("Now") && params["Now"].is_object()) {
    pthread_rwlock_wrlock(&pLock);
    pNow = params["Now"];
    pthread_rwlock_unlock(&pLock);
  }
  if(params.contains("Status") && params["Status"].is_object()) {
    pthread_rwlock_wrlock(&pLock);
    pStatus = params["Status"];
    pthread_rwlock_unlock(&pLock);
  }
  json     Now    = json::array();
  json     Status = json::array();
  uint64_t Time   = Tools::GetTime64() - pTimeout;
  pthread_rwlock_rdlock(&pLock);
  for(auto &Serie : pNowRequest) {
    if(Serie.second > Time) Now.push_back(Serie.first);
  }
  for(auto &Serie : pStatusRequest) {
    if(Serie.second > Time) Status.push_back(Serie.first);
  }
  pthread_rwlock_unlock(&pLock);
  if(Now.size() > 0) response["Now"] = Now;
  if(Status.size() > 0) response["Status"] = Status;
}
void AnalyzerRemoteNow::GetSeriesNow(const json &params, Response &response) {
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t Time = Tools::GetTime64();
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    auto Item = pNowRequest.find(Serie);
    if(Item == pNowRequest.end()) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    pthread_rwlock_wrlock(&pLock);
    Item->second = Time;
    pthread_rwlock_unlock(&pLock);
  }
  pthread_rwlock_rdlock(&pLock);
  json Result = json::object();
  Result["Time"] = pNow["Time"];
  for(auto &Serie : params["Series"]) {
    if(pNow.contains(Serie) == false) {
      pthread_rwlock_unlock(&pLock);
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    Result[(string)Serie] = pNow[(string)Serie];
  }
  pthread_rwlock_unlock(&pLock);
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerRemoteNow::GetStatus(const json &params, Response &response, const json extraProps) {
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t Time = Tools::GetTime64();
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    auto Item = pStatusRequest.find(Serie);
    if(Item == pStatusRequest.end()) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    pthread_rwlock_wrlock(&pLock);
    Item->second = Time;
    pthread_rwlock_unlock(&pLock);
  }
  pthread_rwlock_rdlock(&pLock);
  json Result = extraProps;
  Result["Time"] = pStatus["Time"];
  for(auto &Serie : params["Series"]) {
    if(pStatus.contains(Serie) == false) {
      pthread_rwlock_unlock(&pLock);
      return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    }
    Result[(string)Serie] = pStatus[(string)Serie];
  }
  pthread_rwlock_unlock(&pLock);
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
// ---------------------------------------------------------------------------------------------------------------------