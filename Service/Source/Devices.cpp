/* ================================== zMeter ===========================================================================
 * File:     Devices.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "Devices.h"
#include "DeviceInterface.h"
#include "CaptureV2/DeviceV2.h"
//----------------------------------------------------------------------------------------------------------------------
float             Devices::NominalVoltage        = 230.0;
float             Devices::NominalFrequency      = 50.0;
DeviceInterface **Devices::DeviceInterfaces      = NULL;
uint8_t           Devices::DeviceInterfacesCount = 0;
//----------------------------------------------------------------------------------------------------------------------
void Devices::Config(json_object *config) {
  struct json_object *Find;
  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    NominalVoltage = json_object_get_double(Find);
  }
  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    NominalFrequency = json_object_get_double(Find);
  }
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "DeviceV1") == 0)) {
      //AddDevice(new DeviceV1(JSON_Val));
    } else if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "DeviceV2") == 0)) {
      AddDevice(new DeviceV2(JSON_Val));
    }
  }
}
void Devices::AddDevice(DeviceInterface* device) {
  DeviceInterfaces = (DeviceInterface**)realloc(DeviceInterfaces, sizeof(DeviceInterface**) * (DeviceInterfacesCount) + 1);
  DeviceInterfaces[DeviceInterfacesCount] = device;
  DeviceInterfacesCount++;
}
json_object *Devices::GetNodes() {
  json_object *Result = json_object_new_object();
    json_object_object_add(Result, "NominalVoltage", json_object_new_int(NominalVoltage));
    json_object_object_add(Result, "NominalFreq", json_object_new_int(NominalFrequency));
    json_object *Devices = json_object_new_array();
    for(uint32_t i = 0; i < Devices::DeviceInterfacesCount; i++) {
      DeviceInterface *Device = Devices::DeviceInterfaces[i];
      for(uint32_t n = 0; i < Device->AnalyzersCount; i++) {
        json_object *Obj = json_object_new_object();
        json_object_object_add(Obj, "ID", json_object_new_int(Device->Analyzers[i]->ID));
        json_object_object_add(Obj, "Name", json_object_new_string(Device->Analyzers[i]->Name));
        json_object_object_add(Obj, "Type", json_object_new_string(Device->Analyzers[i]->pTriphase ? "3Phase" : "1Phase"));
        json_object_object_add(Obj, "SampleFreq", json_object_new_double(Device->Analyzers[i]->pSamplerate));
        json_object_object_add(Obj, "MaxCurrent", json_object_new_double(20));
        json_object_array_add(Devices, Obj);
      }
    }
  json_object_object_add(Result, "Devices", Devices);
  return Result;
}