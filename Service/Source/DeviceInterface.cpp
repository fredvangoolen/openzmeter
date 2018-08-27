/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "Drivers/DeviceV1.h"
#include "Drivers/DeviceV2.h"
#include "Drivers/DeviceVirtual.h"
#include "Drivers/DeviceRemote.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
DeviceInterface **DeviceInterface::pDevices      = NULL;
uint8_t           DeviceInterface::pDevicesCount = 0;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceInterface::Config(json_object *config) {
  Database DB(NULL);
  Tools::LogMsg("DEBUG", "Creating device tables\n");
  DB.ExecSentence("CREATE TABLE IF NOT EXISTS permisions("
                  "  name   character varying NOT NULL,"
                  "  serial character varying NOT NULL,"
                  "  read   boolean           NOT NULL,"
                  "  write  boolean           NOT NULL,"
                  "CONSTRAINT permisions_name_pkey FOREIGN KEY (name) REFERENCES users (name) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE"
                  ") WITH (OIDS=FALSE)");
  if(config == NULL) return true;
  for(int i = 0; i < json_object_array_length(config); i++) {
    json_object *Device = json_object_array_get_idx(config, i);
    struct json_object *Driver;
    if((json_object_object_get_ex(Device, "Driver", &Driver) == true) && (json_object_get_type(Driver) == json_type_string)) {
      if(strcmp(json_object_get_string(Driver), "DeviceV1") == 0) {
        AddDeviceInterface(new DeviceV1(Device));
      } else if(strcmp(json_object_get_string(Driver), "DeviceV2") == 0) {
//        AddDeviceInterface(new DeviceV2(Device));
      } else if(strcmp(json_object_get_string(Driver), "Virtual") == 0) {
        AddDeviceInterface(new DeviceVirtual(Device));
      } else if(strcmp(json_object_get_string(Driver), "Remote") == 0) {
        AddDeviceInterface(new DeviceRemote(Device));
      }
    }
  }
  return true;
}
void DeviceInterface::AddDeviceInterface(DeviceInterface* device) {
  pDevices = (DeviceInterface**)realloc(pDevices, sizeof(DeviceInterface*) * (pDevicesCount) + 1);
  pDevices[pDevicesCount] = device;
  pDevicesCount++;
}
void DeviceInterface::AddAnalyzer(AnalyzerInterface* analyzer) {
  pAnalyzers = (AnalyzerInterface**)realloc(pAnalyzers, sizeof(DeviceInterface*) * (pAnalyzersCount) + 1);
  pAnalyzers[pAnalyzersCount] = analyzer;
  pAnalyzersCount++;
}
DeviceInterface::DeviceInterface() {
  pTerminate       = false;
  pAnalyzers       = NULL;
  pAnalyzersCount  = 0;
}
DeviceInterface::~DeviceInterface() {
  for(int i = 0; i < pAnalyzersCount; i++) delete pAnalyzers[i];
  free(pAnalyzers);
}
bool DeviceInterface::GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_array();
  db->PrepareSentence("SELECT admin FROM users WHERE name = ?");
  db->BindSingleString(user);
  db->ExecSentence();
  char *Value = db->ResultValue(0, (char*)"admin");
  if((Value != NULL) && (strcmp("t", Value)) == 0) {
    for(uint8_t j = 0; j < pDevicesCount; j++) {
      for(uint32_t k = 0; k < pDevices[j]->pAnalyzersCount; k++) {
        json_object_array_add(*writer, pDevices[j]->pAnalyzers[k]->GetJSONDescriptor());
      }
    }
  } else {
    db->PrepareSentence("SELECT serial FROM permisions WHERE name = ? AND read = TRUE");
    db->BindSingleString(user);
    db->ExecSentence();
    for(uint32_t i = 0; i < db->ResultRows(); i++) {
      char *Serial = db->ResultValue(i, (char*)"serial");
      for(uint8_t j = 0; j < pDevicesCount; j++) {
        for(uint32_t k = 0; k < pDevices[j]->pAnalyzersCount; k++) {
          if(strcmp(pDevices[j]->pAnalyzers[k]->GetID(), Serial) == 0) json_object_array_add(*writer, pDevices[j]->pAnalyzers[k]->GetJSONDescriptor());
        }
      }
    }
  }
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return true;    
}
bool DeviceInterface::GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
  char    *Analyzer = NULL, *Aggreg = NULL, *Series = NULL;
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
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Analyzer = (char*)json_object_get_string(JSON_Val);
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
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
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer);
  db->ExecSentence();
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
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool DeviceInterface::GetSeriesAggregCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0;
  char    *Analyzer = NULL, *Aggreg = NULL, *Series = NULL;
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
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Analyzer = (char*)json_object_get_string(JSON_Val);
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
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
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer);
  db->ExecSentence();
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
    if((strcmp(Serie, "rms_v") == 0) || (strcmp(Serie, "rms_i") == 0) || (strcmp(Serie, "active_power") == 0) || (strcmp(Serie, "apparent_power") == 0) || (strcmp(Serie, "power_factor") == 0) || (strcmp(Serie, "active_energy") == 0) || (strcmp(Serie, "thd_v") == 0) || (strcmp(Serie, "thd_i") == 0) || (strcmp(Serie, "phi") == 0) || (strcmp(Serie, "harmonics_i") == 0) || (strcmp(Serie, "harmonics_v") == 0)) {
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
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool DeviceInterface::GetSeriesNowCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  char *Analyzer = NULL, *Series = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
      Series = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Analyzer = (char*)json_object_get_string(JSON_Val);
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Series == NULL) {
    *postdata  = (char*)"'Series' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  for(uint32_t i = 0; i < pDevicesCount; i++) {
    DeviceInterface *Device = pDevices[i];
    for(uint32_t n = 0; n < Device->pAnalyzersCount; n++) {
      AnalyzerInterface *Tmp = Device->pAnalyzers[n];
      if(strcmp(Tmp->GetID(), Analyzer) == 0) {
        *writer = Tmp->GetJSONParams(Series);
        *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
        return true;      
      }
    }
  }
  return false;
}
bool DeviceInterface::GetSeriesRangeCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  char *Analyzer = NULL, *Aggreg = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Analyzer = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
      Aggreg = (char*)json_object_get_string(JSON_Val);
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0))) {
    *postdata  = (char*)"'Aggreg' need to be '3s', '1m', '10m' or '1h'";
    *resultLen = strlen(*postdata);
    return false;
  }
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer);
  db->ExecSentence();
  char QueryText[8192];
  snprintf(QueryText, sizeof(QueryText), "SELECT row_to_json(Result) AS result FROM ( SELECT max(sampletime) AS \"To\", min(sampletime) AS \"From\" FROM aggreg_%s) AS Result", Aggreg);
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}