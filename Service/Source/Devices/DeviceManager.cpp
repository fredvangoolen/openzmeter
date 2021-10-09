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
  LogFile::Debug("Starting DeviceManager...");
  for(auto Func : pDrivers) Func();
  HTTPServer::RegisterCallback("getDevices",     GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",      GetDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",      SetDeviceCallback);
  LogFile::Debug("DeviceManager started");
}
DeviceManager::~DeviceManager() {
  LogFile::Debug("Stopping DeviceManager...");
  HTTPServer::UnregisterCallback("getDevices");
  HTTPServer::UnregisterCallback("getDevice");
  HTTPServer::UnregisterCallback("setDevice");
  LogFile::Info("Stopping devices...");
  if(pthread_rwlock_wrlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  for(auto &Device : pDevices) delete Device.second;
  pDevices.clear();
  if(pthread_rwlock_unlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  LogFile::Debug("DeviceManager stopped");
}
void DeviceManager::AddDevice(Device *dev) {
  if(pthread_rwlock_wrlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pDevices.insert({ dev->Serial(), dev });
  if(pthread_rwlock_unlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void DeviceManager::AddDriver(void (*func)()) {
  pDrivers.push_back(func);
}
void DeviceManager::CallDeviceFunction(HTTPRequest &req, void (Device::*method)(HTTPRequest &req)) {
  if(!req.CheckParamString("Serial")) return;
  string Serial = req.Param("Serial");
  if(pthread_rwlock_rdlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  auto Device = pDevices.find(Serial);
  if(Device == pDevices.end()) {
    req.Error("Device not found", HTTP_BAD_REQUEST);
  } else {
    (Device->second->*method)(req);
  }
  if(pthread_rwlock_unlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void DeviceManager::GetDevicesCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  auto Func = [](function<bool(const json&)> Write) {
    Write({ "Driver", "Name", "Serial", "Enabled", "Analyzers" });
    if(pthread_rwlock_rdlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    for(auto &Tmp : pDevices) {
      json Device = { Tmp.second->Driver(), Tmp.second->Name(), Tmp.second->Serial(), Tmp.second->Enabled(), Tmp.second->AnalyzersCount() };
      if(Write(Device) == false) break;
    }
    if(pthread_rwlock_unlock(&pDevicesLock) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  };
  req.SuccessStreamArray(Func);
}
void DeviceManager::GetDeviceCallback(HTTPRequest &req) {
  CallDeviceFunction(req, &Device::GetDevice);
}
void DeviceManager::SetDeviceCallback(HTTPRequest &req) {
  CallDeviceFunction(req, &Device::SetDevice);
}
//----------------------------------------------------------------------------------------------------------------------