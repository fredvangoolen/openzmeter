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
    enum DeviceState {UNCONFIGURED, STOPPED, RUNNING};
  private :
    static vector<DeviceInterface*> pDevices;
  protected :
    pthread_t   pCaptureThread;           //Descriptor del hilo de captura
    bool        pTerminate;               //Marca que deben detenerse los trabajos pendientes
    DeviceState pState;                   //Device state
  protected :
    virtual AnalyzerInterface *FindAnalyzer(const char *serial) = 0;
    virtual const char        *Serial() = 0;
    virtual const char        *Driver() = 0;
    virtual const char        *Name() = 0;
    virtual uint32_t           AnalyzersCount() = 0;
    virtual bool               ConfigDev(json_object *config) = 0;
    virtual bool               Start() = 0;
    virtual bool               Stop() = 0;
  protected :
    DeviceInterface();
  protected :
    static void AddDevice(DeviceInterface *device);
  public :
    virtual ~DeviceInterface();
  public :
    static bool Config(json_object *config);
    static bool GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesAggregCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesNowCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesRangeCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetEventsCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetPricesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetDevicesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetTariffsCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
};