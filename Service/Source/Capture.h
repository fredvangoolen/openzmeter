/* ================================== zMeter ===========================================================================
 * File:     Devices.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include "DeviceInterface.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
class Capture {
  public :
    static float             pNominalVoltage;
    static float             pNominalFrequency;
    static DeviceInterface **pDeviceInterfaces;
    static uint8_t           pDeviceInterfacesCount;
  private :  
    static void AddDevice(DeviceInterface *device);
  public :
    static float GetNominalVoltage();
    static float GetNominalFrequency();
    static void  Config(json_object *config);
    static bool  GetSeriesCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen);
    static bool  GetSeriesAggregCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen);
    static bool  GetNodesCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen);
    static bool  GetSeriesNowCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen);
    
};
//----------------------------------------------------------------------------------------------------------------------