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
#include "WifiManager.h"
#include "SystemManager.h"
#include "LogFile.h"
#include "MessageLoop.h"
#include "ConfigFile.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
WifiManager::WifiStat WifiManager::pWifiStat      = WifiManager::DISABLED;
pthread_mutex_t       WifiManager::pMutex         = PTHREAD_MUTEX_INITIALIZER;
string                WifiManager::pInterfaceWIFI = "wlan0";
string                WifiManager::pNetworkWIFI   = "lan";
string                WifiManager::pWirelessWIFI  = "default_radio";
#endif
// ---------------------------------------------------------------------------------------------------------------------
WifiManager::WifiManager() {
#ifdef OPENWRT
  LogFile::Info("Starting WifiManager...");
  json Config = ConfigFile::ReadConfig("/WIFI");
  if(Config.is_object() == true) {
    if(Config["Interface"].is_string()) pInterfaceWIFI = Config["Interface"];
    if(Config["Network"].is_string()) pNetworkWIFI = Config["Network"];
    if(Config["Config"].is_string()) pWirelessWIFI = Config["Config"];
  }
  Config = json::object();
  Config["Interface"] = pInterfaceWIFI;
  Config["Network"] = pNetworkWIFI;
  Config["Config"] = pWirelessWIFI;
  PeriodicTasks();
  ConfigFile::WriteConfig("/WIFI", Config);
  MessageLoop::RegisterListener("WIFI_PeriodicTasks", PeriodicTasks, 5000);
  HTTPServer::RegisterCallback("scanWIFI", ScanWIFICallback);
  HTTPServer::RegisterCallback("getWIFI",  GetWIFICallback);
  HTTPServer::RegisterCallback("setWIFI",  SetWIFICallback);
  LogFile::Info("WifiManager started");
#endif
}
WifiManager::~WifiManager() {
#ifdef OPENWRT
  LogFile::Info("Stopping WifiManager...");
  HTTPServer::UnregisterCallback("scanWIFI");
  HTTPServer::UnregisterCallback("getWIFI");
  HTTPServer::UnregisterCallback("setWIFI");
  MessageLoop::UnregisterListener("WIFI_PeriodicTasks");
  LogFile::Info("WifiManager stoped");
#endif
}
WifiManager::WifiStat WifiManager::Enabled() {
#ifdef OPENWRT
  pthread_mutex_lock(&pMutex);
  WifiStat Stat = pWifiStat;
  pthread_mutex_unlock(&pMutex);
  return Stat;
#else
  return DISABLED;
#endif
}
#ifdef OPENWRT
void WifiManager::PeriodicTasks() {
  WifiStat Stat = DISABLED;
  json Result = SystemManager::uBusCall("uci", "get", { {"config", "network"}, {"section", pNetworkWIFI} });
  if(Result.is_object() && Result.contains("values") && Result["values"].is_object() && Result["values"].contains("enabled") && Result["values"]["enabled"].is_string()) {
    Stat = (Result["values"]["enabled"] == "1") ? CONFIGURED : DISABLED;
    Result = SystemManager::uBusCall("network.interface", "status", { {"interface", pNetworkWIFI} });
    if(Result.is_object() && Result.contains("up") && Result["up"].is_boolean()) {
      Stat = (Result["up"] == true) ? CONNECTED : CONFIGURED;
      int Code;
      SystemManager::Exec("ping -c 1 8.8.8.8", &Code);
      Stat = (Code == 0) ? INTERNET : CONNECTED;
    }
  }
  pthread_mutex_lock(&pMutex);
  pWifiStat = Stat;
  pthread_mutex_unlock(&pMutex);
}
void WifiManager::ScanWIFICallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Result = json::array();
  json Tmp = SystemManager::uBusCall("iwinfo", "scan", {{"device", pInterfaceWIFI}});
  if((Tmp.is_object() == false) || (Tmp["results"].is_array() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  for(uint Pos = 0; Pos < Tmp["results"].size(); Pos++) {
    json AP = json::object();
    if(Tmp["results"][Pos]["ssid"].is_string()) AP["ESSID"] = Tmp["results"][Pos]["ssid"];
    if(Tmp["results"][Pos]["channel"].is_number()) AP["Channel"] = Tmp["results"][Pos]["channel"];
    if(Tmp["results"][Pos]["quality"].is_number() && Tmp["results"][Pos]["quality_max"].is_number()) {
      float Current = Tmp["results"][Pos]["quality"], Max = Tmp["results"][Pos]["quality_max"];
      AP["Quality"] = (int)(Current * 100.0 / Max);
    }
    Result.push_back(AP);
  }
  req.Success(Result);
}
void WifiManager::GetWIFICallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Full")) return;
  json Result = json::object();
  json Tmp = SystemManager::uBusCall("uci", "get", { {"config", "wireless"}, {"section", pWirelessWIFI } });
  if((Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  Result["Enabled"] = true;
  if(Tmp["values"]["disabled"].is_string()) Result["Enabled"] = (Tmp["values"]["disabled"] == "0");

  if(req.Param("Full")) {
    if(Tmp["values"]["ssid"].is_string()) Result["ESSID"] = Tmp["values"]["ssid"];
    if(Tmp["values"]["encryption"].is_string()) Result["KeyMode"] = (Tmp["values"]["encryption"] == "none") ? "NONE" : (Tmp["values"]["encryption"] == "wep") ? "WEP" : (Tmp["values"]["encryption"] == "psk") ? "WPA" : "WPA2";
    if((Result["KeyMode"] != "NONE") && Tmp["values"]["key"].is_string()) Result["Key"] = Tmp["values"]["key"];

    Tmp = SystemManager::uBusCall("uci", "get", { {"config", "network"}, {"section", pNetworkWIFI } });
    if((Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    Result["IPMode"] = "DHCP";
    if(Tmp["values"]["proto"].is_string()) Result["IPMode"] = (Tmp["values"]["proto"] == "static") ? "STATIC" : "DHCP";
    Result["IP"] = "";
    if(Tmp["values"]["ipaddr"].is_string()) Result["IP"] = Tmp["values"]["ipaddr"];
    Result["Netmask"] = "";
    if(Tmp["values"]["netmask"].is_string()) Result["Netmask"] = Tmp["values"]["netmask"];
    Result["Gateway"] = "";
    if(Tmp["values"]["gateway"].is_string()) Result["Gateway"] = Tmp["values"]["gateway"];
    Result["DNS"] = json::array();
    if(Tmp["values"]["dns"].is_array()) Result["DNS"] = Tmp["values"]["dns"];

    Result["Status"] = json::object();
    Tmp = SystemManager::uBusCall("network.interface." + pNetworkWIFI, "status", {});
    if(Tmp.is_object() == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    Result["Status"]["Connected"] = false;
    if(Tmp["up"].is_boolean()) Result["Status"]["Connected"] = Tmp["up"];
    if(Result["Status"]["Connected"] == true) {
      if(Tmp["dns-server"].is_array()) Result["Status"]["DNS"] = Tmp["dns-server"];
      if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["Status"]["IP"] = Tmp["ipv4-address"][0]["address"];
      if(Tmp["proto"].is_string()) Result["Status"]["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
      if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Status"]["Gateway"] = Tmp["route"][0]["nexthop"];
    }
    Tmp = SystemManager::uBusCall("iwinfo", "info", { {"device", pInterfaceWIFI} });
    if(Tmp.is_object() == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp["ssid"].is_string()) Result["Status"]["ESSID"] = Tmp["ssid"];
    if(Tmp["channel"].is_number()) Result["Status"]["Channel"] = Tmp["channel"];
    if(Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
      float Current = Tmp["quality"], Max = Tmp["quality_max"];
      Result["Status"]["Quality"] = (int) (Current * 100.0 / Max);
    }
  }
  req.Success(Result);
}
void WifiManager::SetWIFICallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamString("ESSID") || !req.CheckParamBool("Enabled") || !req.CheckParamEnum("IPMode", {"DHCP", "STATIC"}) || !req.CheckParamEnum("KeyMode", {"NONE", "WEP", "WPA", "WPA2"})) return;
  if(req.Param("IPMode") == "STATIC") {
    if(!req.CheckParamString("IP") || !req.CheckParamString("Netmask") || !req.CheckParamString("Gateway") || !req.CheckParamArrayString("DNS")) return;
    SystemManager::uBusCall("uci", "set", { {"config", "network"}, {"section", pNetworkWIFI}, {"values", {{"ipaddr", req.Param("IP")}, {"netmask", req.Param("Netmask")}, {"gateway", req.Param("Gateway")}, {"proto", "static"}, {"enabled", req.Param("Enabled") ? "1" : "0"}, {"dns", req.Param("DNS")}}}});
  } else {
    SystemManager::uBusCall("uci", "set", { {"config", "network"}, {"section", pNetworkWIFI}, {"values", {{"proto", "dhcp"}, {"enabled", req.Param("Enabled") ? "1" : "0"}}}});
  }
  if(req.Param("KeyMode") == "NONE") {
    SystemManager::uBusCall("uci", "set", { {"config", "wireless"}, {"section", pWirelessWIFI}, {"values", {{"ssid", req.Param("ESSID")}, {"encryption", "none"}}}});
  } else {
    if(!req.CheckParamString("Key")) return;
    SystemManager::uBusCall("uci", "set", { {"config", "wireless"}, {"section", pWirelessWIFI}, {"values", {{"ssid", req.Param("ESSID")}, {"key", req.Param("Key")}, {"encryption", (req.Param("KeyMode") == "WEP") ? "wep" : (req.Param("KeyMode") == "WPA") ? "psk" : "psk2"}}}});
  }
  SystemManager::Exec("uci commit && /etc/init.d/network restart");
  req.Success(json::object());
}
#endif
// ---------------------------------------------------------------------------------------------------------------------