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
#include "ConfigFile.h"
#include "LogFile.h"
#include "SystemManager.h"
#include "MessageLoop.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
string EthernetManager::pInterface = "";
#endif
// ---------------------------------------------------------------------------------------------------------------------
EthernetManager::EthernetManager() {
#ifdef OPENWRT
  LogFile::Info("Starting EthernetManager...");
  json Config = ConfigFile::ReadConfig("/EthernetInterface");
  if(Config.is_string() == true) pInterface = Config;
  ConfigFile::WriteConfig("/EthernetInterface", pInterface);
  if(pInterface.empty() == false) {
    MessageLoop::RegisterListener("Ethernet_PeriodicTasks", PeriodicTasks, 5000);
    HTTPServer::RegisterCallback("getEthernet", GetEthernetCallback);
    HTTPServer::RegisterCallback("setEthernet", SetEthernetCallback);
  }
  LogFile::Info("EthernetManager started");
#endif
}
EthernetManager::~EthernetManager() {
#ifdef OPENWRT
  LogFile::Info("Stopping EthernetManager...");
  HTTPServer::UnregisterCallback("getEthernet");
  HTTPServer::UnregisterCallback("setEthernet");
  MessageLoop::UnregisterListener("Ethernet_PeriodicTasks");
  LogFile::Info("EthernetManager stoped");
#endif
}
#ifdef OPENWRT
void EthernetManager::PeriodicTasks() {
//  json Params = json::object();
//  json Tmp = SystemManager::uBusCall("network.interface." + pInterface, "status");
//LogFile::SetValue("Ethernet", "/Ethernet_Enabled", ((Tmp == nullptr) || (Tmp.is_object() == false) || (Tmp["up"].is_boolean() == false)) ? json() : Tmp["up"]);
}
void EthernetManager::GetEthernetCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
//  string PhyInterface = "";
//  json   Result       = json::object();
//  Result["Status"]    = json::object();
//  json Tmp = SystemManager::uBusCall("network.interface." + pInterface, "status");
//  if((Tmp == nullptr) || (Tmp.is_object() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
//  if(Tmp["up"].is_boolean()) Result["Status"]["Connected"] = Tmp["up"];
//  if(Tmp["device"].is_string()) PhyInterface = Tmp["device"];
//  if(Tmp["dns-server"].is_array()) Result["Status"]["DNS"] = Tmp["dns-server"];
//  if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["Status"]["IP"] = Tmp["ipv4-address"][0]["address"];
//  if(Tmp["proto"].is_string()) Result["Status"]["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
//  if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Status"]["Gateway"] = Tmp["route"][0]["nexthop"];

//  json Params = json::object();
//  Params["name"] = PhyInterface;
//  Tmp = SystemManager::uBusCall("network.device", "status", Params);
//  if((Tmp == nullptr) || (Tmp.is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  if(Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];

//  Params = json::object();
//  Params["config"] = "network";
//  Params["section"] = pInterface;
//  Tmp = SystemManager::uBusCall("uci", "get", Params);
//  if((Tmp == nullptr) || (Tmp.is_object() == false) || (Tmp["values"].is_object() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  if(Tmp["values"]["proto"].is_string()) Result["IPMode"] = (Tmp["values"]["proto"] == "static") ? "STATIC" : "DHCP";
//  if(Tmp["values"]["ipaddr"].is_string()) Result["IP"] = Tmp["values"]["ipaddr"];
//  if(Tmp["values"]["netmask"].is_string()) Result["Netmask"] = Tmp["values"]["netmask"];
//  if(Tmp["values"]["gateway"].is_string()) Result["Gateway"] = Tmp["values"]["gateway"];
//  if(Tmp["values"]["dns"].is_array()) Result["DNS"] = Tmp["values"]["dns"];
//  req.Success(Result);
}
void EthernetManager::SetEthernetCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Enabled") || !req.CheckParamEnum("IPMode", {"DHCP", "STATIC"})) return;
  if(req.Param("IPMode") == "STATIC") {
    if(!req.CheckParamString("IP") || !req.CheckParamString("Netmask") || !req.CheckParamString("Gateway") || !req.CheckParamArrayString("DNS")) return;
    SystemManager::uBusCall("uci", "set", { {"config", "network"}, {"section", pInterface}, {"values", {{"ipaddr", req.Param("IP")}, {"netmask", req.Param("Netmask")}, {"gateway", req.Param("Gateway")}, {"proto", "static"}, {"enabled", req.Param("Enabled") ? "1" : "0"}, {"dns", req.Param("DNS")}}}});
  } else {
    SystemManager::uBusCall("uci", "set", { {"config", "network"}, {"section", pInterface}, {"values", {{"proto", "dhcp"}, {"enabled", req.Param("Enabled") ? "1" : "0"}}}});
  }
  SystemManager::Exec("uci commit && /etc/init.d/network restart");
  req.Success(json::object());
}
#endif
// ---------------------------------------------------------------------------------------------------------------------