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
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Device : protected RWLocks {
  public :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  protected :
    vector<Analyzer*> pAnalyzers;         //Analyzers list
    DeviceState       pState;             //Device state
    string            pSerial;            //Device serial
    string            pDriver;            //Driver name
    string            pName;              //Driver description
  public :
    virtual bool StartDevice() = 0;
    virtual bool StopDevice() = 0;
  public :
    Device();
    virtual ~Device();
    json   GetJSONDescriptor();
    json   GetJSONAnalyzerDescriptor(const string &serial);
    json   GetJSONAllAnalyzerDescriptors();
    string Serial() const;
    void   GetDeviceCallback(HTTPRequest &req);
    void   SetDeviceCallback(HTTPRequest &req);
    void   StartDeviceCallback(HTTPRequest &req);
    void   StopDeviceCallback(HTTPRequest &req);
    bool   GetAnalyzerCallback(HTTPRequest &req);
    bool   SetAnalyzerCallback(HTTPRequest &req);
    bool   GetSeriesCallback(HTTPRequest &req);
    bool   GetSeriesNowCallback(HTTPRequest &req);
    bool   GetSeriesRangeCallback(HTTPRequest &req);
    bool   GetEventsCallback(HTTPRequest &req);
    bool   GetEventCallback(HTTPRequest &req);
    bool   SetEventCallback(HTTPRequest &req);
    bool   DelEventCallback(HTTPRequest &req);
    bool   GetRatesCallback(HTTPRequest &req);
    bool   GetRateCallback(HTTPRequest &req);
    bool   SetRateCallback(HTTPRequest &req);
    bool   DelRateCallback(HTTPRequest &req);
    bool   AddRateCallback(HTTPRequest &req);
    bool   GetPricesCallback(HTTPRequest &req);
};