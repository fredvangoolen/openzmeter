/* ================================== zMeter ===========================================================================
 * File:     Devices.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "Capture.h"
#include "DeviceInterface.h"
#include "CaptureV2/DeviceV2.h"
//----------------------------------------------------------------------------------------------------------------------
float             Capture::pNominalVoltage        = 230.0;
float             Capture::pNominalFrequency      = 50.0;
DeviceInterface **Capture::pDeviceInterfaces      = NULL;
uint8_t           Capture::pDeviceInterfacesCount = 0;
//----------------------------------------------------------------------------------------------------------------------
void Capture::Config(json_object *config) {
  struct json_object *Find;
  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    pNominalVoltage = json_object_get_double(Find);
  }
  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
    pNominalFrequency = json_object_get_double(Find);
  }
  if((json_object_object_get_ex(config, "Devices", &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    for(int i = 0; i < json_object_array_length(Find); i++) {
      json_object *Device = json_object_array_get_idx(Find, i);
      struct json_object *Driver;
      if((json_object_object_get_ex(Device, "Driver", &Driver) == true) && (json_object_get_type(Driver) == json_type_string)) {
        if(strcmp(json_object_get_string(Driver), "DeviceV1") == 0) {
          //AddDevice(new DeviceV1(JSON_Val));
        } else if(strcmp(json_object_get_string(Driver), "DeviceV2") == 0) {
          AddDevice(new DeviceV2(Device));
        }
      }
    }
  }
}
void Capture::AddDevice(DeviceInterface* device) {
  pDeviceInterfaces = (DeviceInterface**)realloc(pDeviceInterfaces, sizeof(DeviceInterface**) * (pDeviceInterfacesCount) + 1);
  pDeviceInterfaces[pDeviceInterfacesCount] = device;
  pDeviceInterfacesCount++;
}
json_object *Capture::GetNodes() {
  json_object *Result = json_object_new_object();
    json_object_object_add(Result, "NominalVoltage", json_object_new_int(pNominalVoltage));
    json_object_object_add(Result, "NominalFreq", json_object_new_int(pNominalFrequency));
    json_object *Devices = json_object_new_array();
    for(uint32_t i = 0; i < pDeviceInterfacesCount; i++) {
      DeviceInterface *Device = pDeviceInterfaces[i];  
      for(uint32_t n = 0; n < Device->GetAnalyzers(); n++) {
        Analyzer *Analyzer = Device->GetAnalyzers(n);
        json_object *Obj = json_object_new_object();
        json_object_object_add(Obj, "ID", json_object_new_string(Analyzer->GetID()));
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
float Capture::GetNominalFrequency() {
  return pNominalFrequency;
}
float Capture::GetNominalVoltage() {
  return pNominalVoltage;
}
uint8_t Capture::GetDevices() {
  return pDeviceInterfacesCount;
}
DeviceInterface *Capture::GetDevices(uint8_t index) {
  if(index < pDeviceInterfacesCount) return pDeviceInterfaces[index];
  return NULL;
}
Analyzer *Capture::FindID(uint32_t id) {
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