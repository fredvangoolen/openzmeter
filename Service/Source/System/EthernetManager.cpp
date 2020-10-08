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
#include "EthernetManager.h"
#ifdef OPENWRT // ======================================================================================================
#include "ConfigFile.h"
#include "LogFile.h"
#include "System.h"
#include "Status.h"
#include "MessageLoop.h"
// ---------------------------------------------------------------------------------------------------------------------
string EthernetManager::pInterface = "";
// ---------------------------------------------------------------------------------------------------------------------
EthernetManager::EthernetManager() {
  LogFile::Info("Starting Ethernet manager");
  json Config = ConfigFile::ReadConfig("/EthernetInterface");
  if(Config.is_string() == true) pInterface = Config;
  ConfigFile::WriteConfig("/EthernetInterface", pInterface);
  if(pInterface.empty() == false) {
    MessageLoop::RegisterListener("Ethernet_PeriodicTasks", PeriodicTasks, 5000);
    HTTPServer::RegisterJSONCallback("getEthernet", GetEthernetCallback);
    HTTPServer::RegisterJSONCallback("setEthernet", SetEthernetCallback);
  }
}
EthernetManager::~EthernetManager() {
  LogFile::Info("Stopping Ethernet manager...");
  HTTPServer::UnregisterJSONCallback("getEthernet");
  HTTPServer::UnregisterJSONCallback("setEthernet");
  MessageLoop::UnregisterListener("Ethernet_PeriodicTasks");
  LogFile::Info("Ethernet manager stoped");
}
void EthernetManager::PeriodicTasks() {
  json Params = json::object();
  json Tmp = System::uBusCall("network.interface." + pInterface, "status");
  Status::SetValue("/Ethernet_Enabled", ((Tmp == nullptr) || (Tmp.is_object() == false) || (Tmp["up"].is_boolean() == false)) ? json() : Tmp["up"]);
}
void EthernetManager::GetEthernetCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  string PhyInterface = "";
  json   Result       = json::object();
  Result["Status"]    = json::object();
  json Tmp = System::uBusCall("network.interface." + pInterface, "status");
  if((Tmp == nullptr) || (Tmp.is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  if(Tmp["up"].is_boolean()) Result["Status"]["Connected"] = Tmp["up"];
  if(Tmp["device"].is_string()) PhyInterface = Tmp["device"];
  if(Tmp["dns-server"].is_array()) Result["Status"]["DNS"] = Tmp["dns-server"];
  if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["Status"]["IP"] = Tmp["ipv4-address"][0]["address"];
  if(Tmp["proto"].is_string()) Result["Status"]["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
  if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Status"]["Gateway"] = Tmp["route"][0]["nexthop"];

  json Params = json::object();
  Params["name"] = PhyInterface;
  Tmp = System::uBusCall("network.device", "status", Params);
  if((Tmp == nullptr) || (Tmp.is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  if(Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];

  Params = json::object();
  Params["config"] = "network";
  Params["section"] = pInterface;
  Tmp = System::uBusCall("uci", "get", Params);
  if((Tmp == nullptr) || (Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  if(Tmp["values"]["proto"].is_string()) Result["IPMode"] = (Tmp["values"]["proto"] == "static") ? "STATIC" : "DHCP";
  if(Tmp["values"]["ipaddr"].is_string()) Result["IP"] = Tmp["values"]["ipaddr"];
  if(Tmp["values"]["netmask"].is_string()) Result["Netmask"] = Tmp["values"]["netmask"];
  if(Tmp["values"]["gateway"].is_string()) Result["Gateway"] = Tmp["values"]["gateway"];
  if(Tmp["values"]["dns"].is_array()) Result["DNS"] = Tmp["values"]["dns"];
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void EthernetManager::SetEthernetCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  int ErrorCode = 0;
  json Params = json::object();
  Params["config"] = "network";
  Params["section"] = pInterface;
  Params["values"] = json::object();
  if(params.contains("Enabled") && params["Enabled"].is_boolean()) Params["values"]["enabled"] = params["Enabled"] ? "1" : "0";
  if(params.contains("IPMode") && params["IPMode"].is_string() && ((params["IPMode"] == "DHCP") || (params["IPMode"] == "STATIC"))) Params["values"]["proto"] = (params["IPMode"] == "DHCP") ? "dhcp" : "static";
  if(params.contains("IP") && params["IP"].is_string()) Params["values"]["ipaddr"] = params["IP"];
  if(params.contains("Netmask") && params["Netmask"].is_string()) Params["values"]["netmask"] = params["Netmask"];
  if(params.contains("Gateway") && params["Gateway"].is_string()) Params["values"]["gateway"] = params["Gateway"];
  if(params.contains("DNS") && params["DNS"].is_array()) Params["values"]["dns"] = params["DNS"];
  System::uBusCall("uci", "set", Params);
  System::Exec("uci commit && ubus call network reload", &ErrorCode);
  return HTTPServer::FillResponse(response, "", "text/plain", (ErrorCode != 0) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
#else // ===============================================================================================================
EthernetManager::EthernetManager() {}
EthernetManager::~EthernetManager() {}
#endif // ==============================================================================================================