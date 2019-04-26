/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <pthread.h>
#include "AnalyzerInterface.h"
#include "HTTPServer.h"
#include <json.hpp>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class Device {
  public :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  protected :
    pthread_t   pCaptureThread;           //Descriptor del hilo de captura
    bool        pTerminate;               //Marca que deben detenerse los trabajos pendientes
    DeviceState pState;                   //Device state
  public :
    virtual uint32_t           AnalyzersCount() = 0;
    virtual AnalyzerInterface *FindAnalyzer(string serial) = 0;
    virtual string             Serial() = 0;
    virtual string             Driver() = 0;
    virtual string             Name() = 0;
    virtual bool               Config(json config) = 0;
    virtual json               Config() = 0;
    virtual bool               Start() = 0;
    virtual bool               Stop() = 0;
  public :
    Device();
    virtual ~Device();
    DeviceState State();
};
class DeviceManager {
  private :  
    static DeviceManager *pInstance;
    static vector<Device*> pDevices;
  private :
    static void GetDevicesCallback(HTTPRequest &req);
    static void GetDeviceCallback(HTTPRequest &req);
    static void StartDeviceCallback(HTTPRequest &req);
    static void StopDeviceCallback(HTTPRequest &req);
    static void SetDeviceCallback(HTTPRequest &req);
    
    static void GetAnalyzersCallback(HTTPRequest &req);
    static void GetSeriesCallback(HTTPRequest &req);
    static void GetSeriesNowCallback(HTTPRequest &req);
    static void GetSeriesRangeCallback(HTTPRequest &req);
    static void GetEventsCallback(HTTPRequest &req);
    static void GetPricesCallback(HTTPRequest &req);
    static void GetTariffsCallback(HTTPRequest &req);
    static void SetTariffsCallback(HTTPRequest &req);
    static void DelTariffsCallback(HTTPRequest &req);    
  public :
    static void AddDevice(Device *device);
  public :
    DeviceManager(json config);
    ~DeviceManager();    
};