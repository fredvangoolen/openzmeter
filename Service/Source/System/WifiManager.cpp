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
#ifdef OPENWRT // ======================================================================================================
#include "SystemManager.h"
#include "LogFile.h"
#include "Status.h"
#include "MessageLoop.h"
#include "ConfigFile.h"
// ---------------------------------------------------------------------------------------------------------------------
string           WifiManager::pInterfaceWIFI = "wlan0";
string           WifiManager::pNetworkWIFI   = "lan";
string           WifiManager::pWirelessWIFI  = "default_radio";
// ---------------------------------------------------------------------------------------------------------------------
WifiManager::WifiManager() {
  LogFile::Info("Starting Wifi manager");
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
  ConfigFile::WriteConfig("/WIFI", Config);
  MessageLoop::RegisterListener("WIFI_PeriodicTasks", PeriodicTasks, 5000);
  HTTPServer::RegisterCallback("scanWIFI", ScanWIFICallback);
  HTTPServer::RegisterCallback("getWIFI",  GetWIFICallback);
  HTTPServer::RegisterCallback("setWIFI",  SetWIFICallback);
}
WifiManager::~WifiManager() {
  LogFile::Info("Stopping Wifi manager...");
  HTTPServer::UnregisterCallback("scanWIFI");
  HTTPServer::UnregisterCallback("getWIFI");
  HTTPServer::UnregisterCallback("setWIFI");
  MessageLoop::UnregisterListener("WIFI_PeriodicTasks");
  LogFile::Info("Stopping Wifi manager...");
}
void WifiManager::PeriodicTasks() {
  json Params = json::object();
  Params["device"] = pInterfaceWIFI;
  json Result = SystemManager::uBusCall("iwinfo", "info", Params);
  Status::SetValue("/WIFI_Network", ((Result == nullptr) || (Result.is_object() == false) || (Result["ssid"].is_string() == false)) ? json() : Result["ssid"]);
  Status::SetValue("/WIFI_Power", ((Result == nullptr) || (Result.is_object() == false) || (Result["quality"].is_number() == false) || (Result["quality_max"].is_number() == false)) ? json() : json(Result["quality"].get<int>() * 100.0 / Result["quality_max"].get<int>()));
}
void WifiManager::ScanWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::array();
  string CommandStr = SystemManager::Exec("ubus call iwinfo scan '{\"device\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if((Tmp.is_discarded() == true) || (Tmp.is_object() == false) || (Tmp["results"].is_array() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
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
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void WifiManager::GetWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  string CommandStr = SystemManager::Exec("ubus call network.device status '{\"name\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object() && Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];
    if(Result["Enabled"] == false) return HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  CommandStr = SystemManager::Exec("ubus call uci get '{\"config\":\"network\", \"section\":\"" + pNetworkWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["values"].is_object()) {
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
      }
    }
  }
  CommandStr = SystemManager::Exec("ubus call uci get '{\"config\":\"wireless\", \"section\":\"" + pWirelessWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["values"].is_object()) {
        if(Tmp["values"]["ssid"].is_string()) Result["ESSID"] = Tmp["values"]["ssid"];
        if(Tmp["values"]["encryption"].is_string()) Result["KeyMode"] = (Tmp["values"]["encryption"] == "none") ? "NONE" : (Tmp["values"]["encryption"] == "wep") ? "WEP" : (Tmp["values"]["encryption"] == "psk") ? "WPA" : "WPA2";
        if(Result["KeyMode"] != "NONE") {
          if(Tmp["values"]["key"].is_string()) Result["Key"] = Tmp["values"]["key"];
        }
      }
    }
  }
  Result["Status"] = json::object();
  CommandStr = SystemManager::Exec("ubus call network.interface." + pNetworkWIFI + " status");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      Result["Status"]["Connected"] = false;
      if(Tmp["up"].is_boolean()) Result["Status"]["Connected"] = Tmp["up"];
      if(Result["Status"]["Connected"] == true) {
        if(Tmp["dns-server"].is_array()) Result["Status"]["DNS"] = Tmp["dns-server"];
        if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["Status"]["IP"] = Tmp["ipv4-address"][0]["address"];
        if(Tmp["proto"].is_string()) Result["Status"]["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
        if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Status"]["Gateway"] = Tmp["route"][0]["nexthop"];
      }
    }
  }
  CommandStr = SystemManager::Exec("ubus call iwinfo info '{\"device\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["ssid"].is_string()) Result["Status"]["ESSID"] = Tmp["ssid"];
      if(Tmp["channel"].is_number()) Result["Status"]["Channel"] = Tmp["channel"];
      if(Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
        float Current = Tmp["quality"], Max = Tmp["quality_max"];
        Result["Status"]["Quality"] = (int) (Current * 100.0 / Max);
      }
    }
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void WifiManager::SetWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  int ErrorCode = 0;
  if(params.contains("Enabled") && params["Enabled"].is_boolean()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"enabled\":\"" + (params["Enabled"] ? "1" : "0") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("IPMode") && params["IPMode"].is_string() && ((params["IPMode"] == "DHCP") || (params["IPMode"] == "STATIC"))) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"proto\":\"" + ((params["IPMode"] == "DHCP") ? "dhcp" : "static") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("IP") && params["IP"].is_string()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"ipaddr\":" + params["IP"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Netmask") && params["Netmask"].is_string()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"netmask\":" + params["Netmask"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Gateway") && params["Gateway"].is_string()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"gateway\":" + params["Gateway"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("DNS") && params["DNS"].is_array()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"dns\":" + params["DNS"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("ESSID") && params["ESSID"].is_string()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"ssid\":" + params["ESSID"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("KeyMode") && params["KeyMode"].is_string() && ((params["KeyMode"] == "NONE") || (params["KeyMode"] == "WEP") || (params["KeyMode"] == "WPA") || (params["KeyMode"] == "WPA2"))) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"encryption\":\"" + ((params["KeyMode"] == "NONE") ? "none" : (params["KeyMode"] == "WEP") ? "wep" : (params["KeyMode"] == "WPA") ? "psk" : "psk2") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Key") && params["Key"].is_string()) {
    SystemManager::Exec("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"key\":" + params["Key"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  SystemManager::Exec("uci commit && ubus call network reload", &ErrorCode);
  return HTTPServer::FillResponse(response, "", "text/plain", (ErrorCode != 0) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
#else // ===============================================================================================================
WifiManager::WifiManager() {}
WifiManager::~WifiManager() {}
#endif // ==============================================================================================================