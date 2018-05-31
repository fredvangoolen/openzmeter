/* ================================== zMeter ===========================================================================
 * File:     Devices.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "Devices.h"
#include "DeviceInterface.h"
#include "CaptureV2/DeviceV2.h"
//----------------------------------------------------------------------------------------------------------------------
float             Devices::pNominalVoltage        = 230.0;
float             Devices::pNominalFrequency      = 50.0;
DeviceInterface **Devices::pDeviceInterfaces      = NULL;
uint8_t           Devices::pDeviceInterfacesCount = 0;
//----------------------------------------------------------------------------------------------------------------------
void Devices::Config(json_object *config) {
  struct json_object *Find;
  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    pNominalVoltage = json_object_get_double(Find);
  }
  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    pNominalFrequency = json_object_get_double(Find);
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
  pDeviceInterfaces = (DeviceInterface**)realloc(pDeviceInterfaces, sizeof(DeviceInterface**) * (pDeviceInterfacesCount) + 1);
  pDeviceInterfaces[pDeviceInterfacesCount] = device;
  pDeviceInterfacesCount++;
}
json_object *Devices::GetNodes() {
  json_object *Result = json_object_new_object();
    json_object_object_add(Result, "NominalVoltage", json_object_new_int(pNominalVoltage));
    json_object_object_add(Result, "NominalFreq", json_object_new_int(pNominalFrequency));
    json_object *Devices = json_object_new_array();
    for(uint32_t i = 0; i < pDeviceInterfacesCount; i++) {
      DeviceInterface *Device = pDeviceInterfaces[i];  
      for(uint32_t n = 0; n < Device->GetAnalyzers(); n++) {
        Analyzer *Analyzer = Device->GetAnalyzers(n);
        json_object *Obj = json_object_new_object();
        json_object_object_add(Obj, "ID", json_object_new_int(Analyzer->GetID()));
        json_object_object_add(Obj, "Name", json_object_new_string(Analyzer->GetName()));
        json_object_object_add(Obj, "Type", json_object_new_string(Analyzer->GetTypeString()));
        json_object_object_add(Obj, "SampleFreq", json_object_new_double(Device->GetSamplerate()));
        json_object_object_add(Obj, "MaxCurrent", json_object_new_double(Device->GetMaxCurrent()));
        json_object_array_add(Devices, Obj);
      }
    }
  json_object_object_add(Result, "Devices", Devices);
  return Result;
}
float Devices::GetNominalFrequency() {
  return pNominalFrequency;
}
float Devices::GetNominalVoltage() {
  return pNominalVoltage;
}
uint8_t Devices::GetDevices() {
  return pDeviceInterfacesCount;
}
DeviceInterface *Devices::GetDevices(uint8_t index) {
  if(index < pDeviceInterfacesCount) return pDeviceInterfaces[index];
  return NULL;
}
Analyzer *Devices::FindID(uint32_t id) {
  for(uint32_t i = 0; i < pDeviceInterfacesCount; i++) {
    DeviceInterface *Device = pDeviceInterfaces[i];  
    for(uint32_t n = 0; n < Device->GetAnalyzers(); n++) {
      Analyzer *Analyzer = Device->GetAnalyzers(n);
      //if(Analyzer->GetID() == id) 
        return Analyzer;
    }
  }
  return NULL;
}