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
#include <sys/time.h>
#include "SystemManager.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
NTPManager::NTPManager() {
  LogFile::Info("Starting NTPManager...");
#ifdef OPENWRT
  HTTPServer::RegisterCallback("getNTP", GetNTPCallback);
  HTTPServer::RegisterCallback("setNTP", SetNTPCallback);
#endif
  LogFile::Info("NTPManager started");
}
NTPManager::~NTPManager() {
  LogFile::Info("Stopping NTPManager...");
#ifdef OPENWRT
  HTTPServer::UnregisterCallback("getNTP");
  HTTPServer::UnregisterCallback("setNTP");
#endif
  LogFile::Info("NTPManager stoped");
}
#ifdef OPENWRT
void NTPManager::GetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  json Tmp = SystemManager::uBusCall("uci", "get", { {"config", "system"}, {"section", "ntp"} } );
  if((Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  Result["Enabled"] = false;
  if(Tmp["values"]["enabled"].is_string()) Result["Enabled"] = (Tmp["values"]["enabled"] == "1");
  Result["Server"] = "";
  if(Tmp["values"]["server"].is_string()) Result["Server"] = Tmp["values"]["server"];
  Result["Time"] = Tools::GetTime64();
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void NTPManager::SetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Server") == false) || (params["Server"].is_string() == false)) return HTTPServer::FillResponse(response, "'Server' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Time") == false) || (params["Time"].is_number() == false)) return HTTPServer::FillResponse(response, "'Time' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  bool     Enabled = params["Enabled"];
  string   Server  = params["Server"];
  uint64_t Time    = params["Time"];
  SystemManager::uBusCall("uci", "set", { {"config", "system"}, {"section", "ntp"} , {"values", { {"enabled", (Enabled == true) ? "1" : "0"}, {"server", Server} } } });
  SystemManager::Exec("uci commit && /etc/init.d/network restart");
  if(Enabled == false) {
    timeval TimeVal;
    TimeVal.tv_sec  = Time / 1000;
    TimeVal.tv_usec = Time % 1000;
    if(settimeofday(&TimeVal, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
#endif
// ---------------------------------------------------------------------------------------------------------------------