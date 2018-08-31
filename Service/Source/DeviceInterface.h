/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class DeviceInterface {
  protected : 
//    AnalyzerInterface **pAnalyzers;               //Analizadores disponibles
//    uint8_t             pAnalyzersCount;          //Numero de analizadores
    pthread_t           pCaptureThread;           //Descriptor del hilo de captura 
    bool                pTerminate;               //Marca que deben detenerse los trabajos pendientes
  protected :
    virtual AnalyzerInterface *FindAnalyzer(char *serial) = 0;
    DeviceInterface();
  public :
    virtual ~DeviceInterface();
  private :
    static DeviceInterface **pDevices;
    static uint8_t           pDevicesCount;
  private :
    static void AddDeviceInterface(DeviceInterface *device);
  public :
    static bool Config(json_object *config);
    static bool GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesAggregCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesNowCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static bool GetSeriesRangeCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
};