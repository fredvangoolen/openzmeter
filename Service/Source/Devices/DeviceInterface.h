/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <pthread.h>
#include "AnalyzerInterface.h"
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
    virtual AnalyzerInterface *FindAnalyzer(const char *serial) = 0;
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
    static bool GetSeriesCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesAggregCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetAnalyzersCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesNowCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesRangeCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetEventsCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetPricesCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetDevicesCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetTariffsCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool SetTariffsCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool StartDeviceCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool StopDeviceCallback(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
};