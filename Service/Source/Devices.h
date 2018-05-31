/* ================================== zMeter ===========================================================================
 * File:     Devices.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include "DeviceInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class Devices {
  public :
    static float             NominalVoltage;
    static float             NominalFrequency;
    static DeviceInterface **DeviceInterfaces;
    static uint8_t           DeviceInterfacesCount;
  private :  
    static void AddDevice(DeviceInterface *device);
  public :
    static void         Config(json_object *config);
    static json_object *GetNodes();
};
//----------------------------------------------------------------------------------------------------------------------