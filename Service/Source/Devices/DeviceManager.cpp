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
pthread_rwlock_t                                       DeviceManager::pDevicesLock = PTHREAD_RWLOCK_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  for(auto Func : pDrivers) Func();
  HTTPServer::RegisterCallback("getDevices",     GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",      GetDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",      SetDeviceCallback);
}
DeviceManager::~DeviceManager() {
  LogFile::Debug("DeviceManager", "Stopping DeviceManager");
  HTTPServer::UnregisterCallback("getDevices");
  HTTPServer::UnregisterCallback("getDevice");
  HTTPServer::UnregisterCallback("setDevice");
  LogFile::Info("DeviceManager", "Stopping devices...");
  pthread_rwlock_wrlock(&pDevicesLock);
  for(auto &Device : pDevices) delete Device.second;
  pDevices.clear();
  pthread_rwlock_unlock(&pDevicesLock);
  LogFile::Debug("DeviceManager", "DeviceManager stopped");
}
void DeviceManager::AddDevice(Device *dev) {
  pthread_rwlock_wrlock(&pDevicesLock);
  pDevices.insert({ dev->Serial(), dev });
  pthread_rwlock_unlock(&pDevicesLock);
}
void DeviceManager::AddDriver(void (*func)()) {
  pDrivers.push_back(func);
}
void DeviceManager::CallDeviceFunction(const json &params, Response &response, void (Device::*method)(const json &params, Response &response)) {
  if((params.contains("Serial") == false) || (params["Serial"].is_string() == false)) return HTTPServer::FillResponse(response, "'Serial' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Serial"];
  pthread_rwlock_rdlock(&pDevicesLock);
  auto Device = pDevices.find(Serial);
  if(Device == pDevices.end()) {
    HTTPServer::FillResponse(response, "Device not found", "text/plain", HTTP_BAD_REQUEST);
  } else {
    (Device->second->*method)(params, response);
  }
  pthread_rwlock_unlock(&pDevicesLock);
}
void DeviceManager::GetDevicesCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Response = json::array();
  pthread_rwlock_rdlock(&pDevicesLock);
  for(auto &Device : pDevices) Response.push_back(Device.second->GetJSONDescriptor());
  pthread_rwlock_unlock(&pDevicesLock);
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void DeviceManager::GetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::GetDevice);
}
void DeviceManager::SetDeviceCallback(const json &params, Response &response) {
  CallDeviceFunction(params, response, &Device::SetDevice);
}
//----------------------------------------------------------------------------------------------------------------------