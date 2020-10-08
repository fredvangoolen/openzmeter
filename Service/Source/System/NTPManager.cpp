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
#include "NTPManager.h"
#ifdef OPENWRT // ======================================================================================================
#include <sys/time.h>
#include "System.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
NTPManager::NTPManager() {
  LogFile::Info("Starting NTP manager");
  HTTPServer::RegisterJSONCallback("getNTP",       GetNTPCallback);
  HTTPServer::RegisterJSONCallback("setNTP",       SetNTPCallback);
}
NTPManager::~NTPManager() {
  LogFile::Info("Stopping NTP manager...");
  HTTPServer::UnregisterJSONCallback("getNTP");
  HTTPServer::UnregisterJSONCallback("setNTP");
  LogFile::Info("NTP manager stoped");
}
void NTPManager::GetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  string CommandStr = System::Exec("ubus call uci get '{\"config\":\"system\", \"section\":\"ntp\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["values"].is_object()) {
        if(Tmp["values"]["enabled"].is_string()) {
          string Value = Tmp["values"]["enabled"];
          Result["Enabled"] = (Value == "1");
        }
        if(Tmp["values"]["server"].is_string()) Result["Server"] = Tmp["values"]["server"];
      }
    }
  }
  Result["Time"] = Tools::GetTime64();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void NTPManager::SetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result;
  int ErrorCode = 0;
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Server") == false) || (params["Server"].is_string() == false)) return HTTPServer::FillResponse(response, "'Server' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  bool Enabled  = params["Enabled"];
  string Server = params["Server"];
  System::Exec("ubus call uci set '{\"config\":\"system\", \"section\":\"ntp\", \"values\":{\"enabled\":\"" + string((Enabled == true) ? "1" : "0") + "\", \"server\":\"" + Server + "\"}}'", &ErrorCode);
  if(ErrorCode == 0) System::Exec("uci commit", &ErrorCode);
  if(ErrorCode == 0) System::Exec("reload_config", &ErrorCode);
  if((params["Enabled"] == false) && params.contains("Time") && params["Time"].is_number()) {
    uint64_t Time = params["Time"];
    timeval TimeVal;
    TimeVal.tv_sec  = Time / 1000;
    TimeVal.tv_usec = Time % 1000;
    if(settimeofday(&TimeVal, NULL) != 0) {
      Result["Status"] = false;
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
#else // ===============================================================================================================
NTPManager::NTPManager() {
}
NTPManager::~NTPManager() {
}
#endif // ==============================================================================================================