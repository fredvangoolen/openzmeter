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
  public :
    static float             NominalVoltage;
    static float             NominalFrequency;
    static DeviceInterface **Items;
    static uint32_t          Len;
  public :
    static bool Init(json_object *config);
    static bool Start();
};
//----------------------------------------------------------------------------------------------------------------------