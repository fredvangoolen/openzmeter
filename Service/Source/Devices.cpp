/* ================================== zMeter ===========================================================================
 * File:     Devices.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "Devices.h"
#include "DeviceInterface.h"
#include "CaptureV2/CaptureV2.h"
//----------------------------------------------------------------------------------------------------------------------
float              Devices::NominalVoltage   = 230.0;
float              Devices::NominalFrequency = 50.0;
DeviceInterface  **Devices::Items            = NULL;
uint32_t           Devices::Len              = 0;
//----------------------------------------------------------------------------------------------------------------------
bool Devices::Init(json_object *config) {
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalVoltage") == 0)) {
      NominalVoltage = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_double) && (strcmp(JSON_Key, "NominalFrequency") == 0)) {
      NominalFrequency = json_object_get_double(JSON_Val);
    } else if((JSON_Type == json_type_array) && (strcmp(JSON_Key, "Inputs") == 0)) {
      int Count = json_object_array_length(JSON_Val);
      Items = (DeviceInterface**)malloc(sizeof(DeviceInterface*) * Count);
      for(int i = 0; i < Count; i++) {
        if(json_object_get_type(json_object_array_get_idx(JSON_Val, i)) != json_type_object) continue;
        Items[Len] = CaptureV2::Init(json_object_array_get_idx(JSON_Val, i));
        if(Items[Len] == NULL) continue;
        Len++;
      }
    }
  }
}
bool Devices::Start() {
  bool Result = true;
  for(int i = 0; i < Len; i++) if(Items[i]->Start() == false) Result = false;
  return Result;
}
