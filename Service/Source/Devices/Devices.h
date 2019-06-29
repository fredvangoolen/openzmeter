/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <pthread.h>
#include <nlohmann/json.hpp>
#include "Analyzers.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Device {
  public :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  protected :
    DeviceState pState;                   //Device state
    string      pSerial;                  //Device serial
    string      pDriver;                  //Driver name
    string      pName;                    //Driver description
  public :
    string Serial();
    string Driver();
    string Name();
  public :
    virtual uint32_t           AnalyzersCount() = 0;
    virtual AnalyzerInterface *FindAnalyzer(string serial) = 0;
    virtual bool               Start() = 0;
    virtual bool               Stop() = 0;
  public :
    Device();
    virtual ~Device();
    DeviceState State();
    json        GetConfig();
};
class DeviceManager {
  private :  
    static vector<Device*>  pDevices;
  public :
    static void AddDevice(Device *dev);
  private :
    static void GetDevicesCallback(HTTPRequest &req);
    static void GetDeviceCallback(HTTPRequest &req);
    static void StartDeviceCallback(HTTPRequest &req);
    static void StopDeviceCallback(HTTPRequest &req);
    static void SetDeviceCallback(HTTPRequest &req);
    static void GetAnalyzersCallback(HTTPRequest &req);
    static void SetAnalyzerCallback(HTTPRequest &req);
    static void GetSeriesCallback(HTTPRequest &req);
    static void GetSeriesNowCallback(HTTPRequest &req);
    static void GetSeriesRangeCallback(HTTPRequest &req);
    static void GetEventsCallback(HTTPRequest &req);
    static void GetEventCallback(HTTPRequest &req);
    static void GetPricesCallback(HTTPRequest &req);
    static void GetTariffsCallback(HTTPRequest &req);
    static void SetTariffsCallback(HTTPRequest &req);
    static void DelTariffsCallback(HTTPRequest &req);    
  public :
    DeviceManager();
    ~DeviceManager();    
};