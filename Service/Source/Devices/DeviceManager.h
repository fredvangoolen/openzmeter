/* ================================== zMeter ===========================================================================
 * File:     DeviceManager.h
 * Author:   Eduardo Viciana
 * -------------------------------------------------------------------------------------------------------------------*/
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
    static vector<Device*>  pDevices;         //Initiated by devices, cleared on destroy
    static RWLocks          pDevicesLock;     //Control access to critical sections
  public :
    static bool AddDevice(Device *dev);
  private :
    static void CallDeviceFunction(const json &params, Response &response, bool (Device::*method)(const json &params, Response &response));
    static void GetDevicesCallback(const json &params, Response &response);
    static void GetDeviceCallback(const json &params, Response &response);
    static void SetDeviceCallback(const json &params, Response &response);
    static void StartDeviceCallback(const json &params, Response &response);
    static void StopDeviceCallback(const json &params, Response &response);
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
    static void GetAlarmCallback(const json &params, Response &response);
    static void SetAlarmCallback(const json &params, Response &response);
    static void AddAlarmCallback(const json &params, Response &response);
    static void DelAlarmCallback(const json &params, Response &response);
    static void GetPermissionsCallback(const json &params, Response &response);
    static void AddPermissionCallback(const json &params, Response &response);
    static void SetPermissionCallback(const json &params, Response &response);
    static void DelPermissionCallback(const json &params, Response &response);
  public :
    DeviceManager();
    ~DeviceManager();
};