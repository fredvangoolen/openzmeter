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
#include "Device.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceManager final {
  private :
    static vector<vector<Device*> (*)()> pDrivers;
    static vector<Device*>               pDevices;         //Initiated by devices, cleared on destroy
    static RWLocks                       pDevicesLock;     //Control access to critical sections
  private :
    static void CallAnalyzerFunction(const json &params, Response &response, bool (Device::*method)(const json &params, Response &response));
    static void CallDeviceFunction(const json &params, Response &response, void (Device::*method)(const json &params, Response &response));
    static void GetDevicesCallback(const json &params, Response &response);
    static void SetDevicesCallback(const json &params, Response &response);
    static void GetDeviceCallback(const json &params, Response &response);
    static void SetDeviceCallback(const json &params, Response &response);
    static void GetAnalyzersCallback(const json &params, Response &response);
    static void GetAnalyzerCallback(const json &params, Response &response);
    static void SetAnalyzerCallback(const json &params, Response &response);
    static void GetSeriesCallback(const json &params, Response &response);
    static void GetSeriesNowCallback(const json &params, Response &response);
    static void GetSeriesRangeCallback(const json &params, Response &response);
    static void GetEventsCallback(const json &params, Response &response);
    static void GetEventCallback(const json &params, Response &response);
    static void SetEventCallback(const json &params, Response &response);
    static void DelEventCallback(const json &params, Response &response);
    static void GetRatesCallback(const json &params, Response &response);
    static void GetRateCallback(const json &params, Response &response);
    static void SetRateCallback(const json &params, Response &response);
    static void DelRateCallback(const json &params, Response &response);
    static void AddRateCallback(const json &params, Response &response);
    static void GetPricesCallback(const json &params, Response &response);
    static void TestAlarmCallback(const json &params, Response &response);
    static void GetAlarmsCallback(const json &params, Response &response);
    static void GetPermissionsCallback(const json &params, Response &response);
    static void AddPermissionCallback(const json &params, Response &response);
    static void SetPermissionCallback(const json &params, Response &response);
    static void DelPermissionCallback(const json &params, Response &response);
  public :
    static void AddDriver(vector<Device*> (*func)());
  public :
    DeviceManager();
    ~DeviceManager();
};