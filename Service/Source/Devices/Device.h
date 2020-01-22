/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <pthread.h>
#include <nlohmann/json.hpp>
#include "Analyzer.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Device {
  public :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  protected :
    RWLocks           pMembersLock;       //Lock to properties
    vector<Analyzer*> pAnalyzers;         //Analyzers list
    DeviceState       pState;             //Device state
    string            pSerial;            //Device serial
    string            pDriver;            //Driver name
    string            pName;              //Driver description
  public :
    virtual bool StartDevice() = 0;
    virtual bool StopDevice() = 0;
    virtual void GetDevice(const json &params, Response &response) = 0;
    virtual void SetDevice(const json &params, Response &response) = 0;
  private :
    Analyzer *FindAnalyzer(const string &serial);
  protected :
    Device(const string &serial, const string &driver, const string &name);
  public :
    string Serial();
    json GetJSONDescriptor();
    json GetJSONAnalyzerDescriptor(const string &serial);
    json GetJSONAllAnalyzerDescriptors();
    void StartDevice(const json &params, Response &response);
    void StopDevice(const json &params, Response &response);
    bool GetAnalyzer(const json &params, Response &response);
    bool SetAnalyzer(const json &params, Response &response);
    bool GetSeries(const json &params, Response &response);
    bool GetSeriesNow(const json &params, Response &response);
    bool GetSeriesRange(const json &params, Response &response);
    bool GetEvents(const json &params, Response &response);
    bool GetEvent(const json &params, Response &response);
    bool SetEvent(const json &params, Response &response);
    bool DelEvent(const json &params, Response &response);
    bool GetRates(const json &params, Response &response);
    bool GetRate(const json &params, Response &response);
    bool SetRate(const json &params, Response &response);
    bool DelRate(const json &params, Response &response);
    bool AddRate(const json &params, Response &response);
    bool GetPrices(const json &params, Response &response);
    bool TestAlarm(const json &params, Response &response);
    bool GetAlarms(const json &params, Response &response);
    bool GetAlarm(const json &params, Response &response);
    bool SetAlarm(const json &params, Response &response);
    bool AddAlarm(const json &params, Response &response);
    bool DelAlarm(const json &params, Response &response);
    bool GetPermissions(const json &params, Response &response);
    bool AddPermission(const json &params, Response &response);
    bool SetPermission(const json &params, Response &response);
    bool DelPermission(const json &params, Response &response);
  public :
    virtual ~Device();
};