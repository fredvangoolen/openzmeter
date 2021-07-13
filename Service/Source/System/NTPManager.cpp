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
#include "Tools.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
NTPManager::NTPManager() {
#ifdef OPENWRT
  LogFile::Info("Starting NTPManager...");
  HTTPServer::RegisterCallback("getNTP", GetNTPCallback);
  HTTPServer::RegisterCallback("setNTP", SetNTPCallback);
  LogFile::Info("NTPManager started");
#endif
}
NTPManager::~NTPManager() {
#ifdef OPENWRT
  LogFile::Info("Stopping NTPManager...");
  HTTPServer::UnregisterCallback("getNTP");
  HTTPServer::UnregisterCallback("setNTP");
  LogFile::Info("NTPManager stoped");
#endif
}
#ifdef OPENWRT
void NTPManager::GetNTPCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Full")) return;
  json Result = json::object();
  json Tmp = SystemManager::uBusCall("uci", "get", { {"config", "system"}, {"section", "ntp"} } );
  if((Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  Result["Enabled"] = false;
  if(Tmp["values"]["enabled"].is_string()) Result["Enabled"] = (Tmp["values"]["enabled"] == "1");
  if(req.Param("Full")) {
    Result["Server"] = "";
    if(Tmp["values"]["server"].is_string()) Result["Server"] = Tmp["values"]["server"];
    Result["Time"] = Tools::GetTime64();
  }
  req.Success(Result);
}
void NTPManager::SetNTPCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Enabled") || !req.CheckParamString("Server") || !req.CheckParamNumber("Time")) return;
  bool     Enabled = req.Param("Enabled");
  string   Server  = req.Param("Server");
  uint64_t Time    = req.Param("Time");
  SystemManager::uBusCall("uci", "set", { {"config", "system"}, {"section", "ntp"} , {"values", { {"enabled", (Enabled == true) ? "1" : "0"}, {"server", Server} } } });
  SystemManager::Exec("uci commit && /etc/init.d/network restart");
  if(Enabled == false) {
    timeval TimeVal;
    TimeVal.tv_sec  = Time / 1000;
    TimeVal.tv_usec = Time % 1000;
    if(settimeofday(&TimeVal, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  req.Success(json::object());
}
#endif
// ---------------------------------------------------------------------------------------------------------------------