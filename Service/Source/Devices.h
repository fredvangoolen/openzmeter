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
    static float             pNominalVoltage;
    static float             pNominalFrequency;
    static DeviceInterface **pDeviceInterfaces;
    static uint8_t           pDeviceInterfacesCount;
  private :  
    static void AddDevice(DeviceInterface *device);
  public :
    static float            GetNominalVoltage();
    static float            GetNominalFrequency();
    static uint8_t          GetDevices();
    static DeviceInterface *GetDevices(uint8_t index);
    static void             Config(json_object *config);
    static json_object     *GetNodes();
    static Analyzer        *FindID(uint32_t id);
};
//----------------------------------------------------------------------------------------------------------------------