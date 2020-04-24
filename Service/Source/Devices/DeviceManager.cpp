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
#include <string.h>
#include <math.h>
#include "DeviceManager.h"
#include "HTTPServer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
vector<vector<Device*> (*)()> __attribute__((init_priority(200))) DeviceManager::pDrivers;
vector<Device*>  DeviceManager::pDevices;
RWLocks          DeviceManager::pDevicesLock;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  pDevicesLock.WriteLock();
  for(auto Func : pDrivers) {
    for(auto Dev : Func()) pDevices.push_back(Dev);
  }
  pDevicesLock.Unlock();
  HTTPServer::RegisterCallback("getDevices",     GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",      GetDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",      SetDeviceCallback);
  HTTPServer::RegisterCallback("setRemote",      SetRemoteCallback);
  HTTPServer::RegisterCallback("getRemote",      GetRemoteCallback);
  HTTPServer::RegisterCallback("getAnalyzers",   GetAnalyzersCallback);
  HTTPServer::RegisterCallback("getAnalyzer",    GetAnalyzerCallback);
  HTTPServer::RegisterCallback("setAnalyzer",    SetAnalyzerCallback);
  HTTPServer::RegisterCallback("getSeries",      GetSeriesCallback);
  HTTPServer::RegisterCallback("getSeriesNow",   GetSeriesNowCallback);
  HTTPServer::RegisterCallback("getSeriesRange", GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("getEvents",      GetEventsCallback);
  HTTPServer::RegisterCallback("getEvent",       GetEventCallback);
  HTTPServer::RegisterCallback("setEvent",       SetEventCallback);
  HTTPServer::RegisterCallback("delEvent",       DelEventCallback);
  HTTPServer::RegisterCallback("getPrices",      GetPricesCallback);
  HTTPServer::RegisterCallback("testAlarm",      TestAlarmCallback);
  HTTPServer::RegisterCallback("getAlarms",      GetAlarmsCallback);
  HTTPServer::RegisterCallback("getPermissions", GetPermissionsCallback);
  HTTPServer::RegisterCallback("addPermission",  AddPermissionCallback);
  HTTPServer::RegisterCallback("setPermission",  SetPermissionCallback);
  HTTPServer::RegisterCallback("delPermission",  DelPermissionCallback);
  HTTPServer::RegisterCallback("getTariffs",     GetTariffsCallback);
  HTTPServer::RegisterCallback("getTariff",      GetTariffCallback);
  HTTPServer::RegisterCallback("setTariff",      SetTariffCallback);
  HTTPServer::RegisterCallback("delTariff",      DelTariffCallback);
  HTTPServer::RegisterCallback("addTariff",      AddTariffCallback);
  HTTPServer::RegisterCallback("testTariff",     TestTariffCallback);
  HTTPServer::RegisterCallback("testRate",       TestRateCallback);
}
DeviceManager::~DeviceManager() {
  LOGFile::Debug("Stopping DeviceManager\n");
  HTTPServer::UnregisterCallback("getDevices");
  HTTPServer::UnregisterCallback("getDevice");
  HTTPServer::UnregisterCallback("setDevice");
  HTTPServer::UnregisterCallback("setRemote");
  HTTPServer::UnregisterCallback("getRemote");
  HTTPServer::UnregisterCallback("getAnalyzers");
  HTTPServer::UnregisterCallback("getAnalyzer");
  HTTPServer::UnregisterCallback("setAnalyzer");
  HTTPServer::UnregisterCallback("getSeries");
  HTTPServer::UnregisterCallback("getSeriesNow");
  HTTPServer::UnregisterCallback("getSeriesRange");
  HTTPServer::UnregisterCallback("getEvents");
  HTTPServer::UnregisterCallback("getEvent");
  HTTPServer::UnregisterCallback("setEvent");
  HTTPServer::UnregisterCallback("delEvent");
  HTTPServer::UnregisterCallback("getPrices");
  HTTPServer::UnregisterCallback("testAlarm");
  HTTPServer::UnregisterCallback("getAlarms");
  HTTPServer::UnregisterCallback("getPermissions");
  HTTPServer::UnregisterCallback("addPermission");
  HTTPServer::UnregisterCallback("setPermission");
  HTTPServer::UnregisterCallback("delPermission");
  HTTPServer::UnregisterCallback("getTariffs");
  HTTPServer::UnregisterCallback("getTariff");
  HTTPServer::UnregisterCallback("setTariff");
  HTTPServer::UnregisterCallback("delTariff");
  HTTPServer::UnregisterCallback("addTariff");
  HTTPServer::UnregisterCallback("testTariff");
  HTTPServer::UnregisterCallback("testRate");
  LOGFile::Info("Stopping devices...\n");
  pDevicesLock.WriteLock();
  while(pDevices.size() > 0) {
    Device *Dev = pDevices.back();
    pDevices.pop_back();
    delete Dev;
  }
  pDevicesLock.Unlock();
  LOGFile::Debug("DeviceManager stopped\n");
}
void DeviceManager::AddDriver(vector<Device*> (*func)()) {
  pDrivers.push_back(func);
}
void DeviceManager::CallAnalyzerFunction(const json &params, Response &response, bool (Device::*method)(const json &params, Response &response)) {
  if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if((Dev->*method)(params, response) == false) continue;
    pDevicesLock.Unlock();
    return;
  }
  pDevicesLock.Unlock();
  HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
}
void DeviceManager::CallDeviceFunction(const json &params, Response &response, void (Device::*method)(const json &params, Response &response)) {
  if((params.contains("Serial") == false) || (params["Serial"].is_string() == false)) return HTTPServer::FillResponse(response, "'Serial' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Serial"];
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->Serial != Serial) continue;
    (Dev->*method)(params, response);
    pDevicesLock.Unlock();
    return;
  }
  pDevicesLock.Unlock();
  HTTPServer::FillResponse(response, "Device not found", "text/plain", HTTP_BAD_REQUEST);
}
void DeviceManager::GetDevicesCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Response = json::array();
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) Response.push_back(Dev->GetJSONDescriptor());
  pDevicesLock.Unlock();
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void DeviceManager::GetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::GetDevice);
}
void DeviceManager::SetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::SetDevice);
}
void DeviceManager::GetRemoteCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::GetRemote);
}
void DeviceManager::SetRemoteCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::SetRemote);
}
void DeviceManager::GetAnalyzersCallback(const json &params, Response &response) {
  json Response = json::array();
  string UserName = params["UserName"];
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    json Analyzers = Dev->GetJSONAnalyzers(UserName);
    Response.insert(Response.end(), Analyzers.begin(), Analyzers.end());
  }
  pDevicesLock.Unlock();
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void DeviceManager::GetAnalyzerCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetAnalyzer);
}
void DeviceManager::SetAnalyzerCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::SetAnalyzer);
}
void DeviceManager::GetSeriesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetSeries);
}
void DeviceManager::GetSeriesNowCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetSeriesNow);
}
void DeviceManager::GetSeriesRangeCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetSeriesRange);
}
void DeviceManager::GetEventsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetEvents);
}
void DeviceManager::GetEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetEvent);
}
void DeviceManager::SetEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::SetEvent);
}
void DeviceManager::DelEventCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::DelEvent);
}
void DeviceManager::GetPricesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetPrices);
}
void DeviceManager::TestAlarmCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::TestAlarm);
}
void DeviceManager::GetAlarmsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetAlarms);
}
void DeviceManager::GetPermissionsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetPermissions);
}
void DeviceManager::AddPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::AddPermission);
}
void DeviceManager::SetPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::SetPermission);
}
void DeviceManager::DelPermissionCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::DelPermission);
}
void DeviceManager::GetTariffsCallback(const json &params, Response &response) {
  Database DB;
  string   UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('TariffID', tariff, 'Name', name, 'UserName', username))), '[]') as result FROM tariffs" + ((UserName == "admin") ? "" : " WHERE username IN ('admin', " + DB.Bind(UserName) + ")"));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void DeviceManager::GetTariffCallback(const json &params, Response &response) {
  if((params.contains("TariffID") == false) || (params["TariffID"].is_number() == false)) return HTTPServer::FillResponse(response, "'TariffID' param is required", "text/plain", HTTP_BAD_REQUEST);
  Database DB;
  uint32_t TariffID = params["TariffID"];
  string   UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("SELECT json_build_object('TariffID', tariff, 'Name', name, 'UserName', username, 'Template', template) as result FROM tariffs WHERE tariff = " + DB.Bind(TariffID) + ((UserName == "admin") ? "" : " AND username = " + DB.Bind(UserName)));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void DeviceManager::DelTariffCallback(const json &params, Response &response) {
  if((params.contains("TariffID") == false) || (params["TariffID"].is_number() == false)) return HTTPServer::FillResponse(response, "'TariffID' param is required", "text/plain", HTTP_BAD_REQUEST);
  Database DB;
  uint32_t TariffID = params["TariffID"];
  string   UserName = params["UserName"];
  DB.ExecSentenceAsyncResult("DELETE FROM tariffs WHERE tariff = " + DB.Bind(TariffID) + ((UserName == "admin") ? "" : " AND username = " + DB.Bind(UserName)) + " RETURNING tariff");
  return HTTPServer::FillResponse(response, "", "text/plain", (!DB.FecthRow()) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void DeviceManager::SetTariffCallback(const json &params, Response &response) {
  if((params.contains("TariffID") == false) || (params["TariffID"].is_number() == false)) return HTTPServer::FillResponse(response, "'TariffID' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is required", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is required", "text/plain", HTTP_BAD_REQUEST);
  Tariff Tmp(params["Template"]);
  return HTTPServer::FillResponse(response, Tmp.ParseError(), "text/plain", HTTP_OK);
  //Primero verifico, construyo tarifa con los parametros que llegan y evaluo periodo a periodo a ver que sale.

//  uint64_t Start    = params["StartDate"];
//  uint64_t End      = params["EndDate"];
//  string   Name     = params["Name"];
//  string   TimeZone = params["TimeZone"];
//  uint32_t TariffID = params["TariffID"];
//  string   UserName = params["UserName"];
//  Tariff T(Start, End, TimeZone);
//  if(T.SetTemplate(params["Template"]) == false) {
//    response.set_content("Invalid template", "text/plain");
//    response.status = HTTP_BAD_REQUEST;
//    return;
//  }
//  Database DB;
//  bool Result = DB.ExecSentenceNoResult("UPDATE tariffs SET name = " + DB.Bind(Name) + ", time_zone = " + DB.Bind(TimeZone) + ", template = " + DB.Bind(T.GetTemplate().dump()) + ", startdate = " + DB.Bind(Start) + ", enddate = " + DB.Bind(End) + " WHERE tariff = " + DB.Bind(TariffID) + ((UserName != "admin") ? " AND username = " + DB.Bind(UserName) : ""));
//  response.set_content("", "text/plain");
//  response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void DeviceManager::AddTariffCallback(const json &params, Response &response) {
//  if(!params["Template"].is_object() || !params["StartDate"].is_number_unsigned() || !params["EndDate"].is_number_unsigned() || !params["Name"].is_string() || !params["TimeZone"].is_string()) {
//    response.set_content("'Template', StartDate', 'EndDate', 'TimeZone' and 'Name' params required", "text/plain");
//    response.status = HTTP_BAD_REQUEST;
//    return;
//  }
//  string   UserName = params["UserName"];
//  uint64_t Start    = params["StartDate"];
//  uint64_t End      = params["EndDate"];
//  string   Name     = params["Name"];
//  string   TimeZone = params["TimeZone"];
//  Tariff T(Start, End, TimeZone);
//  if(T.SetTemplate(params["Template"]) == false) {
//    response.set_content("Invalid template", "text/plain");
//    response.status = HTTP_BAD_REQUEST;
//  } else {
//    Database DB;
//    bool Result = DB.ExecSentenceNoResult("INSERT into tariffs(name, username, time_zone, template, startdate, enddate) VALUES(" + DB.Bind(Name) + ", " + DB.Bind(UserName) + ", " + DB.Bind(TimeZone) + ", " + DB.Bind(T.GetTemplate().dump()) + ", " + DB.Bind(Start) + ", " + DB.Bind(End) + ")");
//    response.set_content("", "text/plain");
//    response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
//  }
}
void DeviceManager::TestTariffCallback(const json &params, Response &response) {
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPServer::FillResponse(response, "'Template' param is required", "text/plain", HTTP_BAD_REQUEST);
  Tariff Tmp(params["Template"]);
  return HTTPServer::FillResponse(response, Tmp.ParseError(), "text/plain", HTTP_OK);
  //Primero verifico, construyo tarifa con los parametros que llegan y evaluo periodo a periodo a ver que sale.
}
void DeviceManager::TestRateCallback(const json &params, Response &response) {

}