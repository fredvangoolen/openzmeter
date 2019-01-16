/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
vector<DeviceInterface*>        DeviceInterface::pDevices;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceInterface::Config(json_object *config) {
  if(config == NULL) return false;
  for(uint i = 0; i < json_object_array_length(config); i++) {
    json_object *Device = json_object_array_get_idx(config, i);
    char *Name = NULL;
    if(Tools::JSONString(Device, "Driver", &Name) == false) {
      Tools::LogMsg("ERROR", "'Driver' parameter not specified\n");
    } else {
      for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
        if((strcmp(Name, (*it)->Driver()) == 0) && ((*it)->ConfigDev(Device) == true)) break;
      }
    }
  }
  return true;
}
DeviceInterface::DeviceInterface() {
  pTerminate     = false;
  pCaptureThread = 0;
  pState = UNCONFIGURED;
}
//DeviceInterface::~DeviceInterface() {
//  Stop();
//}
void DeviceInterface::AddDevice(DeviceInterface* device) {
  if(pDevices.size() == 0) {
    HTTPServer::RegisterCallback("/getAnalyzers",        DeviceInterface::GetAnalyzersCallback);
    HTTPServer::RegisterCallback("/getSeries",           DeviceInterface::GetSeriesCallback);
    HTTPServer::RegisterCallback("/getSeriesAggreg",     DeviceInterface::GetSeriesAggregCallback);
    HTTPServer::RegisterCallback("/getSeriesNow",        DeviceInterface::GetSeriesNowCallback);
    HTTPServer::RegisterCallback("/getSeriesRange",      DeviceInterface::GetSeriesRangeCallback);
    HTTPServer::RegisterCallback("/getEvents",           DeviceInterface::GetEventsCallback);
    HTTPServer::RegisterCallback("/getPrices",           DeviceInterface::GetPricesCallback);
    HTTPServer::RegisterCallback("/getDevices",          DeviceInterface::GetDevicesCallback);
    HTTPServer::RegisterCallback("/getTariffs",          DeviceInterface::GetTariffsCallback);
  }
  Tools::LogMsg("INFO", "Registering '%s' device driver...\n", device->Driver());
  pDevices.push_back(device);
}
bool DeviceInterface::GetTariffsCallback(char* user, json_object* reader, Database* db, json_object** writer, char** postdata, size_t* resultLen) {
  char *Serial = NULL;
  json_object *Find;
  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
  char QueryText[8192];
  int  QueryLen = 0;
  if(Serial == NULL) {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT array_to_json(array_agg(row_to_json(t1))) FROM (  SELECT     rates.rate as \"RateID\", rates.name as \"Name\", rates.description as \"Description\", rates.country_code as \"Country\", rates.startdate as \"StartDate\", rates.enddate as \"EndDate\",    (select to_json(json_object_agg(q1.name, json_build_object('StarDate', q1.startdate, 'EndDate', q1.enddate, 'DayPeriods', q1.dayperiods))) from (select name, startdate, enddate, dayperiods from rates_seasons where rates.rate = rates_seasons.rate) as q1) as \"Seasons\" from rates) as t1");
  } else {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT array_to_json(array_agg(row_to_json(Result))) FROM (SELECT rate as \"RateID\", startdate as \"StartDate\", name AS \"Name\", module AS \"Module\" FROM analyzers_rates NATURAL JOIN rates WHERE serial = '%s') as Result", Serial);
  }
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool DeviceInterface::GetDevicesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_array();
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    json_object *Device = json_object_new_object();
    json_object_object_add(Device, "Driver", json_object_new_string((*it)->Driver()));
    json_object_object_add(Device, "Name", json_object_new_string((*it)->Name()));
    json_object_object_add(Device, "Serial", json_object_new_string((*it)->Serial()));
    json_object_object_add(Device, "Analyzers", json_object_new_int((*it)->AnalyzersCount()));
    json_object_object_add(Device, "Status", json_object_new_string(((*it)->pState == UNCONFIGURED) ? "UNCONFIGURED" : ((*it)->pState == STOPPED) ? "STOPPED" : "RUNNING"));
    json_object_array_add(*writer, Device);
  }
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return true; 
}
bool DeviceInterface::GetAnalyzersCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  *writer = json_object_new_array();
  db->PrepareSentence("SELECT serial, editable FROM (SELECT serial, false AS editable FROM permisions WHERE username = ? AND read = TRUE UNION SELECT serial, true FROM analyzers WHERE username = ? UNION SELECT serial, true FROM analyzers WHERE 'admin' = ?) AS Result ORDER BY serial ASC, editable DESC");
  db->BindSingleString(user);
  db->BindSingleString(user);
  db->BindSingleString(user);
  db->ExecSentence();
  for(uint32_t i = 0; i < db->ResultRows(); i++) {
    const char *Serial = db->ResultValue(i, "serial");
    AnalyzerInterface *Analyzer = NULL;
    for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
      Analyzer = (*it)->FindAnalyzer(Serial);
      if(Analyzer != NULL) {
        json_object *Device = Analyzer->GetJSONDescriptor();
        json_object_object_add(Device, "Editable", json_object_new_boolean(strcmp("f", db->ResultValue(i, "editable")) == 0));
        json_object_array_add(*writer, Device);
        break;
      }
    }
  }
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return true;    
}
bool DeviceInterface::GetSeriesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
  char    *Serial = NULL, *Aggreg = NULL, *Series = NULL;
  AnalyzerInterface *Analyzer = NULL; 
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
      Serial = (char*)json_object_get_string(JSON_Val);
  }
  if(Serial == NULL) {
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
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    Analyzer = (*it)->FindAnalyzer(Serial);
    if(Analyzer != NULL) break;
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"Analyzer not found";
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
  db->BindSingleString(Serial);
  db->ExecSentence();
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"MaxSamples\", ", (To - From) / PeriodSeconds);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'%s' AS \"Aggregation_Source\", ", Aggreg);
  char *Save;
  char *Serie = Tools::trim(strtok_r(Series, ", ", &Save));
  while(Serie != NULL) {
    int ArrayLenA = 0, ArrayLenB = 0;
    if((strcmp(Serie, "rms_v") == 0) || (strcmp(Serie, "active_power") == 0) || (strcmp(Serie, "active_energy") == 0) || (strcmp(Serie, "reactive_power") == 0) || (strcmp(Serie, "reactive_energy") == 0)) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : 3;
    } else if((strcmp(Aggreg, "1m") == 0) && ((strcmp(Serie, "power_factor") == 0) || (strcmp(Serie, "thd_v") == 0) || (strcmp(Serie, "phi") == 0))) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : 3;
    } else if((strcmp(Serie, "freq") == 0) || (strcmp(Serie, "flag") == 0)) {
    } else if(strcmp(Serie, "rms_i") == 0) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : (Analyzer->GetMode() == AnalyzerInterface::PHASE3) ? 3 : 4;
    } else if((strcmp(Aggreg, "1m") == 0) && (strcmp(Serie, "thd_i") == 0)) { 
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : (Analyzer->GetMode() == AnalyzerInterface::PHASE3) ? 3 : 4;
    } else if((strcmp(Aggreg, "1h") == 0) && (strcmp(Serie, "price") == 0)) {
    } else if((strcmp(Aggreg, "1m") == 0) && (strcmp(Serie, "harmonics_v") == 0)) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : 3;
      ArrayLenB = 50;
    } else if((strcmp(Aggreg, "1m") == 0) && (strcmp(Serie, "harmonics_i") == 0)) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : (Analyzer->GetMode() == AnalyzerInterface::PHASE3) ? 3 : 4;
      ArrayLenB = 50;


    } else if((strcmp(Aggreg, "1m") == 0) && ((strcmp(Serie, "stat_voltage") == 0) || (strcmp(Serie, "stat_frequency") == 0) || (strcmp(Serie, "stat_thd") == 0) || (strcmp(Serie, "stat_harmonics") == 0))) {
      ArrayLenA = (Analyzer->GetMode() == AnalyzerInterface::PHASE1) ? 1 : (Analyzer->GetMode() == AnalyzerInterface::PHASE3) ? 3 : 4;
      ArrayLenB = 50;
    } else if((strcmp(Aggreg, "1h") == 0) && (strcmp(Serie, "price") == 0)) {
    } else {
      *postdata  = (char*)"Invalid serie in selected aggregation";
      *resultLen = strlen(*postdata);
      return false;
    }
    if((ArrayLenA != 0) && (ArrayLenB == 0)) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d) THEN array_fill(NULL::real, array[%d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, ArrayLenA, Serie, Serie);
    } else if((ArrayLenA != 0) && (ArrayLenB != 0)) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d OR array_length(%s, 2) <> %d) THEN array_fill(NULL::real, array[%d, %d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, Serie, ArrayLenB, ArrayLenA, ArrayLenB, Serie, Serie);
    } else {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(%s ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie);
    }
    Serie = Tools::trim(strtok_r(NULL, ", ", &Save));
  }
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\" FROM ");
  if(strcmp(Aggreg, "1m") == 0) {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp");
  } else if(strcmp(Aggreg, "1h") == 0) {
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1h NATURAL LEFT JOIN aggreg_stat_1h NATURAL FULL JOIN prices) AS Tmp");
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
  char *Serial = NULL, *Series = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
      Series = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Serial = (char*)json_object_get_string(JSON_Val);
  }
  if(Serial == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Series == NULL) {
    *postdata  = (char*)"'Series' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    AnalyzerInterface *Analyzer = (*it)->FindAnalyzer(Serial);
    if(Analyzer != NULL) {
      *writer = Analyzer->GetJSONParams(Series);
      *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
      return true;
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
bool DeviceInterface::GetEventsCallback(char* user, json_object* reader, Database* db, json_object** writer, char** postdata, size_t* resultLen) {
  uint64_t LastEvent = 0, From = 0, To = 0, Period = 0;
  char    *Analyzer = NULL;
  if(Tools::JSONString(reader, "Analyzer", &Analyzer, false) == false) {
    *postdata  = (char*)"'Analyzer' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  Tools::JSONUInt64(reader, "LastEvent", &LastEvent);
  Tools::JSONUInt64(reader, "To", &To);
  Tools::JSONUInt64(reader, "Period", &Period);
  if(To == 0) To = Tools::GetTime();
  From = To - Period;
  char QueryText[8192];
  int  QueryLen = 0;
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer);
  db->ExecSentence();
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}') AS \"StartTime\", ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}') AS \"Duration\", ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}') AS \"SampleRate\", ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(type ORDER BY starttime ASC), '{}') AS \"Type\", ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(voltage ORDER BY starttime ASC), '{}') AS \"Voltage\", ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(percent ORDER BY starttime ASC), '{}') AS \"Percentage\" ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM events WHERE starttime > %" PRIu64 " AND starttime >= %" PRIu64 " AND starttime <= %" PRIu64 ") AS Result", LastEvent, From, To);
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
bool DeviceInterface::GetPricesCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
  char    *Analyzer = NULL;
  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
      Period = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
      To = json_object_get_int64(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
      Analyzer = (char*)json_object_get_string(JSON_Val);     
   else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastSample") == 0))
      LastSample = json_object_get_int64(JSON_Val);
  }
  if(Analyzer == NULL) {
    *postdata  = (char*)"'Analyzer' param is requiered";
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
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\",");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(active_energy ORDER BY sampletime ASC), '{}') AS active_energy,");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(price ORDER BY sampletime ASC), '{}') AS price,");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\" ", To);
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM (SELECT sampletime, COALESCE((SELECT SUM(s) FROM UNNEST(active_energy) s), 0) as active_energy, price FROM aggreg_1h NATURAL FULL JOIN prices ");
  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64 ") AS T1) AS Result", LastSample, From, To);    
  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return true;
}
