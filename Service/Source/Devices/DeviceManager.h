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
#pragma once
#include <vector>
#include <map>
#include "Device.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceManager final {
  private :
    static vector<void (*)()>   pDrivers;
    static map<string, Device*> pDevices;         //Initiated by devices, cleared on destroy
    static pthread_rwlock_t     pDevicesLock;     //Control access to critical sections
  private :
    static void CallDeviceFunction(HTTPRequest *req, void (Device::*method)(HTTPRequest *req));
    static void GetDevicesCallback(HTTPRequest *req);
    static void GetDeviceCallback(HTTPRequest *req);
    static void SetDeviceCallback(HTTPRequest *req);
  public :
    static void AddDriver(void (*func)());
    static void AddDevice(Device *dev);
  public :
    DeviceManager();
    ~DeviceManager();
};
//----------------------------------------------------------------------------------------------------------------------