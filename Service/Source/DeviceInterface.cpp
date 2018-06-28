/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "Drivers/DeviceV1.h"
#include "Drivers/DeviceV2.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
DeviceInterface **DeviceInterface::pDeviceInterfaces      = NULL;
uint8_t           DeviceInterface::pDeviceInterfacesCount = 0;
//----------------------------------------------------------------------------------------------------------------------
void DeviceInterface::Config(json_object *config) {
//  struct json_object *Find;
//  if((json_object_object_get_ex(config, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
//    pNominalVoltage = json_object_get_double(Find);
//  }
//  if((json_object_object_get_ex(config, "NominalFrequency", &Find) == true) && (json_object_get_type(Find) == json_type_double)) {
//    pNominalFrequency = json_object_get_double(Find);
//  }
//  if((json_object_object_get_ex(config, "Devices", &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    for(int i = 0; i < json_object_array_length(config); i++) {
      json_object *Device = json_object_array_get_idx(config, i);
      struct json_object *Driver;
      if((json_object_object_get_ex(Device, "Driver", &Driver) == true) && (json_object_get_type(Driver) == json_type_string)) {
        if(strcmp(json_object_get_string(Driver), "DeviceV1") == 0) {
          AddDeviceInterface(new DeviceV1(Device));
        } else if(strcmp(json_object_get_string(Driver), "DeviceV2") == 0) {
          AddDeviceInterface(new DeviceV2(Device));
        } else if(strcmp(json_object_get_string(Driver), "Virtual") == 0) {
          //AddDeviceInterface(new DeviceV2(Device));
        } else if(strcmp(json_object_get_string(Driver), "Remote") == 0) {
          //AddDeviceInterface(new DeviceV2(Device));
        }
      }
    }
//  }
}
void DeviceInterface::AddDeviceInterface(DeviceInterface* device) {
  pDeviceInterfaces = (DeviceInterface**)realloc(pDeviceInterfaces, sizeof(DeviceInterface*) * (pDeviceInterfacesCount) + 1);
  pDeviceInterfaces[pDeviceInterfacesCount] = device;
  pDeviceInterfacesCount++;
}


DeviceInterface::DeviceInterface() {
  pTerminate       = false;
  pAnalyzers       = NULL;
  pAnalyzersCount  = 0;
//  pDriverName      = driver;
//  pVoltageSamples  = NULL;
//  pCurrentSamples  = NULL;
//  pVoltageChannels = 0;
//  pCurrentChannels = 0;
//  pSamplerate      = 20000;
//  pMaxCurrent      = 50;
//  pMinCurrent      = 0.1;
//  pChannelMappings = NULL;
//  pActive          = false;
//  pReadPos         = 0;
//  pWritePos        = 0;
//  pUsed            = 0;
//  pBuffLen         = 0;
//  pLastUpdate      = 0;
}
void DeviceInterface::AddAnalyzer(AnalyzerInterface *analyzer) {
  pAnalyzers = (AnalyzerInterface**)realloc(pAnalyzers, sizeof(AnalyzerInterface*) * (pAnalyzersCount) + 1);
  pAnalyzers[pAnalyzersCount] = analyzer;
  pAnalyzersCount++;
//  AddAnalyzer(analyzer, 0, 0);
}
//void DeviceInterface::AddAnalyzer(AnalyzerSoft *analyzer, uint8_t offsetV, uint8_t offsetC) {
//  pAnalyzers = (AnalyzerSoft**)realloc(pAnalyzers, sizeof(AnalyzerSoft*) * (pAnalyzersCount) + 1);
//  pChannelMappings = (ChannelMap_t*)realloc(pChannelMappings, sizeof(ChannelMap_t*) * (pAnalyzersCount) + 1);
//  pAnalyzers[pAnalyzersCount] = analyzer;
//  pChannelMappings[pAnalyzersCount].OffsetV = offsetV;
//  pChannelMappings[pAnalyzersCount].OffsetC = offsetC;
//  pAnalyzersCount++;
//}
//float DeviceInterface::GetSamplerate() {
//  return pSamplerate;
//}
//float DeviceInterface::GetMaxCurrent() {
//  return pMaxCurrent;
//}
uint32_t DeviceInterface::GetAnalyzers() {
  return pAnalyzersCount;
}
AnalyzerInterface *DeviceInterface::GetAnalyzers(uint32_t idx) {
  if(idx < pAnalyzersCount) return pAnalyzers[idx];
  return NULL;
}
//bool DeviceInterface::BuffersAlloc(uint8_t voltageChannels, uint8_t currentChannels, float samplerate) {
//  pBuffLen = DEVICEINTERFACE_BUFFERSECONDS * samplerate;
//  pVoltageSamples = (float*)malloc(sizeof(float) * pBuffLen * voltageChannels);
//  pCurrentSamples = (float*)malloc(sizeof(float) * pBuffLen * currentChannels);
//  pVoltageChannels = voltageChannels;
//  pCurrentChannels = currentChannels;
//  pSamplerate      = samplerate;
//  return (pVoltageSamples != 0) && (pCurrentSamples != 0);
//}
//void DeviceInterface::BuffersReset() {
//  pUsed     = 0;
//  pWritePos = 0;
//  pReadPos  = 0;
//}
//bool DeviceInterface::BuffersAppend(float *voltageSamples, float *currentSamples) {
//  if(pUsed == pBuffLen) return false;
//  for(int8_t Channel = 0; Channel < pVoltageChannels; Channel++) pVoltageSamples[pWritePos * pVoltageChannels + Channel] = voltageSamples[Channel];
//  for(int8_t Channel = 0; Channel < pCurrentChannels; Channel++) pCurrentSamples[pWritePos * pCurrentChannels + Channel] = currentSamples[Channel];
//  pWritePos = (pWritePos + 1) % pBuffLen;
//  pUsed++;
//  uint64_t Sampletime = Tools::GetTime();
//  if(pLastUpdate < (Sampletime - 500)) {
//    char ThreadName[16];
//    sprintf(ThreadName, "%s_%03d", pDriverName, pUsed * 100 / pBuffLen);
//    pthread_setname_np(pCaptureThread, ThreadName);
//    pLastUpdate = Sampletime;
//  }
//  for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
//    if(pAnalyzers[Analyzer]->IsFinished() == false) return true;
//  }
//  bool Started = false;
//  for(uint8_t Sample = 0; (Sample < 5) && (pUsed > 0) && (Started == false); Sample++) {
//    for(uint8_t Analyzer = 0; Analyzer < pAnalyzersCount; Analyzer++) {
//      if(pAnalyzers[Analyzer]->SampleAppend(&pVoltageSamples[pReadPos * pVoltageChannels + pChannelMappings[Analyzer].OffsetV], &pCurrentSamples[pReadPos * pCurrentChannels + pChannelMappings[Analyzer].OffsetC], Sampletime) == true) Started = true;
//    }
//    pReadPos = (pReadPos + 1) % pBuffLen;
//    pUsed--;
//  }
//  return true;
//}
DeviceInterface::~DeviceInterface() {
  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
  free(pAnalyzers);
//  free(pChannelMappings);
//  if(pVoltageSamples) free(pVoltageSamples);
//  if(pCurrentSamples) free(pCurrentSamples);
}
bool DeviceInterface::GetAnalyzersCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_object();
  json_object *Devices = json_object_new_array();
  for(uint8_t i = 0; i < pDeviceInterfacesCount; i++) {
    DeviceInterface *Device = pDeviceInterfaces[i];
    for(uint32_t n = 0; n < Device->GetAnalyzers(); n++) {
      AnalyzerInterface *Analyzer = Device->GetAnalyzers(n);
      json_object *Obj = json_object_new_object();
      json_object_object_add(Obj, "NominalVoltage", json_object_new_int(Analyzer->GetNominalVoltage()));
      json_object_object_add(Obj, "NominalFreq"   , json_object_new_int(Analyzer->GetNominalFrequency()));
      json_object_object_add(Obj, "ID"            , json_object_new_string(Analyzer->GetID()));
      json_object_object_add(Obj, "Name"          , json_object_new_string(Analyzer->GetName()));
      json_object_object_add(Obj, "Type"          , json_object_new_string(Analyzer->GetTypeString()));
      json_object_object_add(Obj, "SampleFreq"    , json_object_new_double(Analyzer->GetSamplerate()));
      json_object_object_add(Obj, "MaxCurrent"    , json_object_new_double(Analyzer->GetMaxCurrent()));
      json_object_array_add(Devices, Obj);
    }
  }
  json_object_object_add(*writer, "Devices", Devices);
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  return true;
}
bool DeviceInterface::GetSeriesCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
  char    *Node = NULL, *Aggreg = NULL, *Series = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
      Series = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
      Aggreg = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
      Period = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
      To = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastSample") == 0))
      LastSample = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Node") == 0))
      Node = (char*)json_object_get_string(JSON_Val);
  }
  if(Node == NULL) {
    *postdata  = (char*)"'Node' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Series == NULL) {
    *postdata  = (char*)"'Series' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }

  if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0))) {
    *postdata  = (char*)"'Aggreg' need to be '3s', '1m', '10m' or '1h'";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(To == 0) To = Tools::GetTime();
  From = To - Period;
  char QueryText[8192];
  int  QueryLen = 0;
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
  uint32_t MaxPoints = 100000, PeriodSeconds = 3000;
  if(strcmp(Aggreg, "1m")  == 0) PeriodSeconds = 60000;
  if(strcmp(Aggreg, "10m") == 0) PeriodSeconds = 600000;
  if(strcmp(Aggreg, "1h")  == 0) PeriodSeconds = 3600000;
  if((strcmp(Aggreg, "3s") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
    Aggreg = (char*)"1m";
    PeriodSeconds = 60000;
  } 
  if((strcmp(Aggreg, "1m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
    Aggreg = (char*)"10m";
    PeriodSeconds = 600000;
  } 
  if((strcmp(Aggreg, "10m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
    Aggreg = (char*)"1h";
    PeriodSeconds = 3600000;
  } 
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"MaxSamples\", ", (To - From) / PeriodSeconds);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'%s' AS \"Aggregation_Source\", ", Aggreg);
  char *Save;
  char *Serie = strtok_r(Series, ", ", &Save);
  while(Serie != NULL) {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(%s ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie);
    Serie = strtok_r(NULL, ", ", &Save);
  }
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\" FROM ");
  if(strcmp(Aggreg, "1m") == 0) {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1m LEFT JOIN aggreg_ext_1m ON aggreg_1m.sampletime = aggreg_ext_1m.sampletime_ext) AS Tmp");
  } else {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "aggreg_%s", Aggreg);
  }
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, " WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64 ") AS Result", LastSample, From, To);    
  if(*db == NULL) *db = new Database(Node);
  *postdata = (*db)->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool DeviceInterface::GetSeriesAggregCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0;
  char    *Node = NULL, *Aggreg = NULL, *Series = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
      Series = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
      Aggreg = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
      Period = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
      To = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Node") == 0))
      Node = (char*)json_object_get_string(JSON_Val);
  }
  if(Node == NULL) {
    *postdata  = (char*)"'Node' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Series == NULL) {
    *postdata  = (char*)"'Series' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0))) {
    *postdata  = (char*)"'Aggreg' need to be '3s', '1m', '10m' or '1h'";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(To == 0) To = Tools::GetTime();
  From = To - Period;
  char QueryText[8192];
  int  QueryLen = 0;
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT COUNT(*) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS \"Samples\", ", Aggreg, From, To);
  char *Save;
  char *Serie = strtok_r(Series, ", ", &Save);
  while(Serie != NULL) {
    char *Oper = NULL, *Tmp1 = strchr(Serie, '('), *Tmp2 = strrchr(Serie, ')');
    if((Tmp1 != NULL) && (Tmp2 != NULL)) {
      Oper = Serie;
      Tmp1[0] = 0;
      Serie = &Tmp1[1];
      Tmp2[0] = 0;
    }
    if((strcmp(Serie, "rms_v") == 0) || (strcmp(Serie, "rms_i") == 0) || (strcmp(Serie, "active_power") == 0) || (strcmp(Serie, "apparent_power") == 0) || (strcmp(Serie, "power_factor") == 0) || (strcmp(Serie, "active_energy") == 0) || (strcmp(Serie, "fundamenta_active_power") == 0) || (strcmp(Serie, "fundamental_reactive_power") == 0) || (strcmp(Serie, "fundamental_reactive_energy") == 0) || (strcmp(Serie, "fundamental_apparent_power") == 0) || (strcmp(Serie, "fundamental_power_factor") == 0) || (strcmp(Serie, "thd_v") == 0) || (strcmp(Serie, "thd_i") == 0) || (strcmp(Serie, "phi") == 0) || (strcmp(Serie, "harmonics_i") == 0) || (strcmp(Serie, "harmonics_v") == 0)) {
      if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT %s(elem) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " GROUP BY rn ORDER BY rn) as %s_%s,", Oper, Aggreg, Serie, From, To, Oper, Serie);
      } else if(strcmp(Oper, "rms") == 0) {
        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT SQRT(SUM(POW(elem, 2.0)) / COUNT(*)) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " GROUP BY rn ORDER BY rn) as %s_%s,", Aggreg, Serie, From, To, Oper, Serie);
      }
    } else if(strcmp(Serie, "freq") == 0) {
      if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT %s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Oper, Serie, Aggreg, From, To, Oper, Serie);
      } else if(strcmp(Oper, "rms") == 0) {
        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT SQRT(SUM(POW(%s, 2.0)) / COUNT(*)) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Serie, Aggreg, From, To, Oper, Serie);
      }
    } else if(strcmp(Serie, "flag") == 0) {
      if((strcmp(Oper, "and") == 0) || (strcmp(Oper, "or") == 0)) {
        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT bool_%s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Oper, Serie, Aggreg, From, To, Oper, Serie);
      }
    }
    Serie = strtok_r(NULL, ", ", &Save);
  }
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\") AS Result", To); 
  *db = new Database(Node);
  *postdata = (*db)->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool Capture::GetSeriesNowCallback(Database **db, json_object *reader, json_object **writer, char **postdata, size_t *resultLen) {
  char *Node = NULL, *Series = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
      Series = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Node") == 0))
      Node = (char*)json_object_get_string(JSON_Val);
  }
  if(Node == NULL) {
    *postdata  = (char*)"'Node' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Series == NULL) {
    *postdata  = (char*)"'Series' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  for(uint32_t i = 0; i < pDeviceInterfacesCount; i++) {
    DeviceInterface *Device = pDeviceInterfaces[i];
    for(uint32_t n = 0; n < Device->GetAnalyzers(); n++) {
      AnalyzerSoft *AnalyzerSoft = Device->GetAnalyzers(n);
      if(strcmp(AnalyzerSoft->GetID(), Node) == 0) {
        *writer = AnalyzerSoft->GetJSONCopy(Series);
        *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
        return true;      
      }
    }
  }
  return false;
}