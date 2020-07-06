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
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
vector<void (*)()> __attribute__((init_priority(200))) DeviceManager::pDrivers;
map<string, Device*>                                   DeviceManager::pDevices;
RWLocks                                                DeviceManager::pDevicesLock;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  for(auto Func : pDrivers) Func();
  HTTPServer::RegisterCallback("getDevices",     GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",      GetDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",      SetDeviceCallback);
}
DeviceManager::~DeviceManager() {
  LogFile::Debug("Stopping DeviceManager\n");
  HTTPServer::UnregisterCallback("getDevices");
  HTTPServer::UnregisterCallback("getDevice");
  HTTPServer::UnregisterCallback("setDevice");
  LogFile::Info("Stopping devices...\n");
  pDevicesLock.WriteLock();
  for(auto &Device : pDevices) delete Device.second;
  pDevices.clear();
  pDevicesLock.Unlock();
  LogFile::Debug("DeviceManager stopped\n");
}
void DeviceManager::AddDevice(Device *dev) {
  pDevicesLock.WriteLock();
  pDevices.insert({ dev->Serial(), dev });
  pDevicesLock.Unlock();
}
void DeviceManager::AddDriver(void (*func)()) {
  pDrivers.push_back(func);
}
void DeviceManager::CallDeviceFunction(const json &params, Response &response, void (Device::*method)(const json &params, Response &response)) {
  if((params.contains("Serial") == false) || (params["Serial"].is_string() == false)) return HTTPServer::FillResponse(response, "'Serial' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Serial"];
  pDevicesLock.ReadLock();
  auto Device = pDevices.find(Serial);
  if(Device == pDevices.end()) {
    HTTPServer::FillResponse(response, "Device not found", "text/plain", HTTP_BAD_REQUEST);
  } else {
    (Device->second->*method)(params, response);
  }
  pDevicesLock.Unlock();
}
void DeviceManager::GetDevicesCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Response = json::array();
  pDevicesLock.ReadLock();
  for(auto &Device : pDevices) Response.push_back(Device.second->GetJSONDescriptor());
  pDevicesLock.Unlock();
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void DeviceManager::GetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::GetDevice);
}
void DeviceManager::SetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::SetDevice);
}
//----------------------------------------------------------------------------------------------------------------------