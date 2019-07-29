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
class DeviceManager {
  private :  
    static vector<Device*>  pDevices;         //Initiated by devices, cleared on destroy
    static RWLocks          pLock;            //Contro access to critical sections
  public :
    static bool AddDevice(Device *dev);
  private :
    static void GetDevicesCallback(HTTPRequest &req);
    static void GetDeviceCallback(HTTPRequest &req);
    static void SetDeviceCallback(HTTPRequest &req);
    static void StartDeviceCallback(HTTPRequest &req);
    static void StopDeviceCallback(HTTPRequest &req);
    static void GetAnalyzersCallback(HTTPRequest &req);
    static void GetAnalyzerCallback(HTTPRequest &req);
    static void SetAnalyzerCallback(HTTPRequest &req);
    static void GetSeriesCallback(HTTPRequest &req);
    static void GetSeriesNowCallback(HTTPRequest &req);
    static void GetSeriesRangeCallback(HTTPRequest &req);
    static void GetEventsCallback(HTTPRequest &req);
    static void GetEventCallback(HTTPRequest &req);
    static void SetEventCallback(HTTPRequest &req);
    static void DelEventCallback(HTTPRequest &req);
    static void GetRatesCallback(HTTPRequest &req);
    static void GetRateCallback(HTTPRequest &req);
    static void SetRateCallback(HTTPRequest &req);
    static void DelRateCallback(HTTPRequest &req);
    static void AddRateCallback(HTTPRequest &req);
    static void GetPricesCallback(HTTPRequest &req);
  public :
    DeviceManager();
    ~DeviceManager();
};