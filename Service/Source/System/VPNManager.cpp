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
#include "HTTPServer.h"
#include "SystemManager.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
VPNManager::VPNManager() {
#ifdef OPENWRT
  LogFile::Info("Starting VPNManager...");
  HTTPServer::RegisterCallback("getVPN", GetVPNCallback);
  HTTPServer::RegisterCallback("setVPN", SetVPNCallback);
  LogFile::Info("VPNManager started");
#endif
}
VPNManager::~VPNManager() {
#ifdef OPENWRT
  LogFile::Info("Stopping VPNManager...");
  HTTPServer::UnregisterCallback("getVPN");
  HTTPServer::UnregisterCallback("setVPN");
  LogFile::Info("VPNManager stoped");
#endif
}
#ifdef OPENWRT
void VPNManager::GetVPNCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Full")) return;
  json Result = json::object();
  json Tmp = SystemManager::uBusCall("uci", "get", { {"config", "openvpn"}, {"section", "custom_config"} });
  if(Tmp.is_object() && Tmp.contains("values") && Tmp["values"].is_object()) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  Result["Enabled"] = true;
  if(Tmp["values"]["enabled"].is_string()) Result["Enabled"] = (Tmp["values"]["enabled"] == "1");
  if(req.Param("Full") == true) {
    Result["ConfigFile"] = pFileName;
    if(Tmp["values"]["config"].is_string()) Result["ConfigFile"] = Tmp["values"]["config"];
    int File = open(pFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(File < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
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
  }
  req.Success(Result);
}
void VPNManager::SetVPNCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Enabled") || !req.CheckParamString("Config")) return;
  int File = open(pFileName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogFile::Error("Can not open '%s' (%s)", pFileName, strerror(errno));
    return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
  bool    Enabled   = req.Param("Enabled");
  string  Write     = req.Param("Config");
  ssize_t Len       = write(File, Write.c_str(), Write.size());
  if(Len == -1) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    close(File);
    return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
  SystemManager::uBusCall("uci", "set", { {"config", "openvpn"}, {"section", "custom_config"}, {"values", { {"enabled", (Enabled == true) ? "1" : "0"}, {"config", pFileName} } } } );
  SystemManager::Exec("uci commit && /etc/init.d/openvpn restart");
  req.Success(json::object());
}
#endif
// ---------------------------------------------------------------------------------------------------------------------