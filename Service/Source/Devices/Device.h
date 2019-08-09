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
class Device : protected InstanceLocks {
  public :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  protected :
    RWLocks           pAnalyzersLock;     //Analyzer list locks
    vector<Analyzer*> pAnalyzers;         //Analyzers list
    DeviceState       pState;             //Device state
    string            pSerial;            //Device serial
    string            pDriver;            //Driver name
    string            pName;              //Driver description
  public :
    virtual bool StartDevice() = 0;
    virtual bool StopDevice() = 0;
    virtual void GetDevice(HTTPRequest &req) = 0;
    virtual void SetDevice(HTTPRequest &req) = 0;
  protected :
    Device();
  public :
    string Serial();
    json GetJSONDescriptor();
    json GetJSONAnalyzerDescriptor(const string &serial);
    json GetJSONAllAnalyzerDescriptors();
    void StartDevice(HTTPRequest &req);
    void StopDevice(HTTPRequest &req);
    bool GetAnalyzer(HTTPRequest &req);
    bool SetAnalyzer(HTTPRequest &req);
    bool GetSeries(HTTPRequest &req);
    bool GetSeriesNow(HTTPRequest &req);
    bool GetSeriesRange(HTTPRequest &req);
    bool GetEvents(HTTPRequest &req);
    bool GetEvent(HTTPRequest &req);
    bool SetEvent(HTTPRequest &req);
    bool DelEvent(HTTPRequest &req);
    bool GetRates(HTTPRequest &req);
    bool GetRate(HTTPRequest &req);
    bool SetRate(HTTPRequest &req);
    bool DelRate(HTTPRequest &req);
    bool AddRate(HTTPRequest &req);
    bool GetPrices(HTTPRequest &req);
  public :
    virtual ~Device();
};