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
  HTTPServer::RegisterCallback("getDevices",          GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",           GetDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",           SetDeviceCallback);
  HTTPServer::RegisterCallback("setRemote",           SetRemoteCallback);
  HTTPServer::RegisterCallback("getRemote",           GetRemoteCallback);
  HTTPServer::RegisterCallback("getAnalyzers",        GetAnalyzersCallback);
  HTTPServer::RegisterCallback("getAnalyzer",         GetAnalyzerCallback);
  HTTPServer::RegisterCallback("setAnalyzer",         SetAnalyzerCallback);
  HTTPServer::RegisterCallback("getSeries",           GetSeriesCallback);
  HTTPServer::RegisterCallback("getSeriesNow",        GetSeriesNowCallback);
  HTTPServer::RegisterCallback("getSeriesRange",      GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("getEvents",           GetEventsCallback);
  HTTPServer::RegisterCallback("getEvent",            GetEventCallback);
  HTTPServer::RegisterCallback("setEvent",            SetEventCallback);
  HTTPServer::RegisterCallback("delEvent",            DelEventCallback);
  HTTPServer::RegisterCallback("getRates",            GetRatesCallback);
  HTTPServer::RegisterCallback("getRate",             GetRateCallback);
  HTTPServer::RegisterCallback("setRate",             SetRateCallback);
  HTTPServer::RegisterCallback("delRate",             DelRateCallback);
  HTTPServer::RegisterCallback("addRate",             AddRateCallback);
  HTTPServer::RegisterCallback("getPrices",           GetPricesCallback);
  HTTPServer::RegisterCallback("getBills",            GetBillsCallback);
  HTTPServer::RegisterCallback("testAlarm",           TestAlarmCallback);
  HTTPServer::RegisterCallback("getAlarms",           GetAlarmsCallback);
  HTTPServer::RegisterCallback("getPermissions",      GetPermissionsCallback);
  HTTPServer::RegisterCallback("addPermission",       AddPermissionCallback);
  HTTPServer::RegisterCallback("setPermission",       SetPermissionCallback);
  HTTPServer::RegisterCallback("delPermission",       DelPermissionCallback);
}
DeviceManager::~DeviceManager() {
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
  HTTPServer::UnregisterCallback("getRates");
  HTTPServer::UnregisterCallback("getRate");
  HTTPServer::UnregisterCallback("setRate");
  HTTPServer::UnregisterCallback("delRate");
  HTTPServer::UnregisterCallback("addRate");
  HTTPServer::UnregisterCallback("getPrices");
  HTTPServer::UnregisterCallback("getBills");
  HTTPServer::UnregisterCallback("testAlarm");
  HTTPServer::UnregisterCallback("getAlarms");
  HTTPServer::UnregisterCallback("getPermissions");
  HTTPServer::UnregisterCallback("addPermission");
  HTTPServer::UnregisterCallback("setPermission");
  HTTPServer::UnregisterCallback("delPermission");
  LOGFile::Info("Stopping devices...\n");
  pDevicesLock.WriteLock();
  while(pDevices.size() > 0) {
    Device *Dev = pDevices.back();
    pDevices.pop_back();
    delete Dev;
  }
  pDevicesLock.Unlock();
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
  //only 'admin' user can get this information
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
void DeviceManager::GetRatesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetRates);
}
void DeviceManager::GetRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetRate);
}
void DeviceManager::SetRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::SetRate);
}
void DeviceManager::DelRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::DelRate);
}
void DeviceManager::AddRateCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::AddRate);
}
void DeviceManager::GetPricesCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetPrices);
}
void DeviceManager::GetBillsCallback(const json &params, Response &response) {
  CallAnalyzerFunction(params, response, &Device::GetBills);
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