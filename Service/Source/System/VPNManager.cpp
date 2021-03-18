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
#include "VPNManager.h"
#include <sys/time.h>
#include "SystemManager.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
VPNManager::VPNManager() {
  LogFile::Info("Starting VPNManager...");
#ifdef OPENWRT
  HTTPServer::RegisterCallback("getVPN", GetVPNCallback);
  HTTPServer::RegisterCallback("setVPN", SetVPNCallback);
#endif
  LogFile::Info("VPNManager started");
}
VPNManager::~VPNManager() {
  LogFile::Info("Stopping VPNManager...");
#ifdef OPENWRT
  HTTPServer::UnregisterCallback("getVPN");
  HTTPServer::UnregisterCallback("setVPN");
#endif
  LogFile::Info("VPNManager stoped");
}
#ifdef OPENWRT
void VPNManager::GetVPNCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  int File = open(pFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);;
  }
  string Read;
  char Buff[256];
  ssize_t Len;
  do {
    Len = read(File, Buff, sizeof(Buff));
    Read += string(Buff, Len);
  } while(Len > 0);
  close(File);
  Result["Config"] = Read;
  json Tmp = SystemManager::uBusCall("uci", "get", { {"config", "openvpn"}, {"section", "custom_config"} });
  if(Result.is_object() && Result.contains("values") && Result["values"].is_object()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  Result["Enabled"] = true;
  if(Tmp["values"]["enabled"].is_string()) Result["Enabled"] = (Tmp["values"]["disabled"] == "0");
  Result["ConfigFile"] = pFileName;
  if(Tmp["values"]["config"].is_string()) Result["ConfigFile"] = Tmp["values"]["config"];
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void VPNManager::SetVPNCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Config") == false) || (params["Config"].is_string() == false)) return HTTPServer::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  int File = open(pFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogFile::Error("Can not open '%s' (%s)", pFileName, strerror(errno));
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);;
  }
  bool    Enabled   = params["Enabled"];
  string  Write     = params["Config"];
  ssize_t Len       = write(File, Write.c_str(), Write.size());
  if(Len == -1) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    close(File);
    return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);;
  }
  SystemManager::uBusCall("uci", "set", { {"config", "openvpn"}, {"section", {"custom_config"}, {"values", { {"enabled", (Enabled == true) ? "1" : "0"}, {"config", pFileName} } } } });
  SystemManager::Exec("uci commit && /etc/init.d/openvpn restart");
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
#endif
// ---------------------------------------------------------------------------------------------------------------------