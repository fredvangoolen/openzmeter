/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <pthread.h>
#include "AnalyzerInterface.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class DeviceInterface {
  protected :
    enum DeviceState { UNCONFIGURED, STOPPED, RUNNING };
  private :
    static vector<DeviceInterface*> pDevices;
  protected :
    pthread_t   pCaptureThread;           //Descriptor del hilo de captura
    bool        pTerminate;               //Marca que deben detenerse los trabajos pendientes
    DeviceState pState;                   //Device state
  protected :
    virtual uint32_t           AnalyzersCount() = 0;
    virtual AnalyzerInterface *FindAnalyzer(string serial) = 0;
    virtual const char        *Serial() = 0;
    virtual const char        *Driver() = 0;
    virtual const char        *Name() = 0;
    virtual bool               ConfigDev(json_object *config) = 0;
    virtual bool               Start() = 0;
    virtual bool               Stop() = 0;
  protected :
    DeviceInterface();
  protected :
    static void AddDevice(DeviceInterface *device);
  public :
    static bool Config(json_object *config);
    static void GetSeriesCallback(HTTPRequest &req);
    static string GetSeriesAggregCallback(HTTPRequest &req);
    static void GetAnalyzersCallback(HTTPRequest &req);
    static void GetSeriesNowCallback(HTTPRequest &req);
    static void GetSeriesRangeCallback(HTTPRequest &req);
    static void GetEventsCallback(HTTPRequest &req);
    static void GetPricesCallback(HTTPRequest &req);
    static string GetDevicesCallback(HTTPRequest &req);
    static string GetTariffsCallback(HTTPRequest &req);
    static string SetTariffsCallback(HTTPRequest &req);
    static string StartDeviceCallback(HTTPRequest &req);
    static string StopDeviceCallback(HTTPRequest &req);
};