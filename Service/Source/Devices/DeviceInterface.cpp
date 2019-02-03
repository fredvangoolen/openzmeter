/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "HTTPServer.h"
#include "Tools.h"
#include <json.hpp>
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
vector<DeviceInterface*> DeviceInterface::pDevices;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceInterface::Config(json_object *config) {
  if(config == NULL) return false;
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    const char *Serial = (*it)->Serial();
    json_object *Find;
    if((json_object_object_get_ex(config, Serial, &Find) != true) || (json_object_get_type(Find) != json_type_object)) json_object_object_add(config, Serial, json_object_new_object());
    if((json_object_object_get_ex(config, Serial, &Find) != true) || (json_object_get_type(Find) != json_type_object) || ((*it)->ConfigDev(Find) != true)) {
      Tools::LogMsg("WARN", "invalid configuration for driver with serial '%s'\n", Serial);
    }
  }
  return true;
}
DeviceInterface::DeviceInterface() {
  pTerminate     = false;
  pCaptureThread = 0;
  pState = UNCONFIGURED;
}
void DeviceInterface::AddDevice(DeviceInterface* device) {
  if(pDevices.size() == 0) {
    HTTPServer::RegisterCallback("/getAnalyzers",        DeviceInterface::GetAnalyzersCallback);
//    HTTPServer::RegisterCallback("/getSeries",           DeviceInterface::GetSeriesCallback);
//    HTTPServer::RegisterCallback("/getSeriesAggreg",     DeviceInterface::GetSeriesAggregCallback);
//    HTTPServer::RegisterCallback("/getSeriesNow",        DeviceInterface::GetSeriesNowCallback);
//    HTTPServer::RegisterCallback("/getSeriesRange",      DeviceInterface::GetSeriesRangeCallback);
//    HTTPServer::RegisterCallback("/getEvents",           DeviceInterface::GetEventsCallback);
//    HTTPServer::RegisterCallback("/getPrices",           DeviceInterface::GetPricesCallback);
//    HTTPServer::RegisterCallback("/getDevices",          DeviceInterface::GetDevicesCallback);
//    HTTPServer::RegisterCallback("/getTariffs",          DeviceInterface::GetTariffsCallback);
//    HTTPServer::RegisterCallback("/StopDevice",          DeviceInterface::StopDeviceCallback);
//    HTTPServer::RegisterCallback("/StartDevice",         DeviceInterface::StartDeviceCallback);
  }
  pDevices.push_back(device);
}
string DeviceInterface::GetTariffsCallback(HTTPRequest &req) {
//  char     QueryText[8192];
//  char    *Serial = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if(Serial == NULL) {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int(Find);
//    if(Index == 0) {
//      snprintf(QueryText, sizeof(QueryText), "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') from rates");
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = %u", Index);
//    }  
//  } else {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int64(Find);
//    if(Index == 0) {
//      snprintf(QueryText, sizeof(QueryText), "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = '%s'", Serial);
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = %u", Index);
//    }  
//  }
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::SetTariffsCallback(HTTPRequest &req) {
//  char     QueryText[8192];
//  char    *Serial = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if(Serial == NULL) {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int(Find);
//    if(Index == 0) {
////      snprintf(QueryText, sizeof(QueryText), "insert into rates(\"name\", username, country_code, details, startdate, enddate) select \"Name\", 'admin', \"Country\", \"Details\", \"StartDate\", \"EndDate\" from json_to_record(?) as x(\"Name\" text, \"Country\" text, \"Details\" jsonb, \"StartDate\" bigint, \"EndDate\" bigint)", );
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = %u", Index);
//    }  
//  } else {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int64(Find);
//    if(Index == 0) {
//      snprintf(QueryText, sizeof(QueryText), "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = '%s'", Serial);
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = %u", Index);
//    }  
//  }
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::GetDevicesCallback(HTTPRequest &req) {
//  *writer = json_object_new_array();
//  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    json_object *Device = json_object_new_object();
//    json_object_object_add(Device, "Driver", json_object_new_string((*it)->Driver()));
//    json_object_object_add(Device, "Name", json_object_new_string((*it)->Name()));
//    json_object_object_add(Device, "Serial", json_object_new_string((*it)->Serial()));
//    json_object_object_add(Device, "Analyzers", json_object_new_int((*it)->AnalyzersCount()));
//    json_object_object_add(Device, "Status", json_object_new_string(((*it)->pState == UNCONFIGURED) ? "UNCONFIGURED" : ((*it)->pState == STOPPED) ? "STOPPED" : "RUNNING"));
//    json_object_array_add(*writer, Device);
//  }
//  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return ""; 
}
string DeviceInterface::StartDeviceCallback(HTTPRequest &req) {
//  char        *Serial = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Device", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if(Serial == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    if(strcmp((*it)->Serial(), Serial) == 0) {
//      *writer = json_object_new_object();
//      json_object_object_add(*writer, "Status", json_object_new_boolean((*it)->Start()));
//      *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
//      return true; 
//    }
//  }
//  *postdata  = (char*)"Analyzer not found";
//  *resultLen = strlen(*postdata);
  return "";
}
string DeviceInterface::StopDeviceCallback(HTTPRequest &req) {
//  char        *Serial = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Device", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if(Serial == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    if(strcmp((*it)->Serial(), Serial) == 0) {
//      *writer = json_object_new_object();
//      json_object_object_add(*writer, "Status", json_object_new_boolean((*it)->Stop()));
//      *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
//      return true; 
//    }
//  }
//  *postdata  = (char*)"Analyzer not found";
//  *resultLen = strlen(*postdata);
  return "";
}
void DeviceInterface::GetAnalyzersCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  json Response = json::array();
  req.PrepareSentence("SELECT serial, editable FROM (SELECT serial, false AS editable FROM permisions WHERE username = ? AND read = TRUE UNION SELECT serial, true FROM analyzers WHERE username = ? UNION SELECT serial, true FROM analyzers WHERE 'admin' = ?) AS Result ORDER BY serial ASC, editable DESC");
  req.BindSingleString(req.UserName().c_str());
  req.BindSingleString(req.UserName().c_str());
  req.BindSingleString(req.UserName().c_str());
  req.ExecSentence();
  for(uint32_t i = 0; i < req.ResultRows(); i++) {
    const char *Serial = req.ResultValue(i, "serial");
    AnalyzerInterface *Analyzer = NULL;
    for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
      Analyzer = (*it)->FindAnalyzer(Serial);
      if(Analyzer != NULL) {
        Response.emplace_back(Analyzer->GetJSONDescriptor());
        break;
      }
    }
  }
  req.FromJSON(Response);
}
string DeviceInterface::GetSeriesCallback(HTTPRequest &req) {
//  uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
//  char    *Serial = NULL, *Aggreg = NULL;
//  AnalyzerInterface *Analyzer = NULL;
//  json_object *Series = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "Aggreg", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Aggreg = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "LastSample", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastSample = json_object_get_int64(Find);
//  if((json_object_object_get_ex(reader, "Period", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Period = json_object_get_int64(Find);
//  if((json_object_object_get_ex(reader, "To", &Find) == true) && (json_object_get_type(Find) == json_type_int)) To = json_object_get_int64(Find);
//  if((json_object_object_get_ex(reader, "Series", &Find) == true) && (json_object_get_type(Find) == json_type_array)) Series = Find;
//  if(Serial == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(Series == NULL) {
//    *postdata  = (char*)"Series must be an array of valid serie names";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if((Aggreg == NULL) || ((strcmp(Aggreg, "3S") != 0) && (strcmp(Aggreg, "1M") != 0) && (strcmp(Aggreg, "10M") != 0) && (strcmp(Aggreg, "15M") != 0) && (strcmp(Aggreg, "1H") != 0))) {
//    *postdata  = (char*)"'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    Analyzer = (*it)->FindAnalyzer(Serial);
//    if(Analyzer != NULL) break;
//  }
//  if(Analyzer == NULL) {
//    *postdata  = (char*)"Analyzer not found";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(To == 0) To = Tools::GetTime();
//  From = To - Period;
//  uint32_t PeriodSeconds = 3000;
//  if(strcmp(Aggreg, "1M")  == 0) PeriodSeconds = 60000;
//  if(strcmp(Aggreg, "10M") == 0) PeriodSeconds = 600000;
//  if(strcmp(Aggreg, "15M") == 0) PeriodSeconds = 900000;
//  if(strcmp(Aggreg, "1H")  == 0) PeriodSeconds = 3600000;
//  if((Period / PeriodSeconds) > 100000) {
//    *postdata  = (char*)"Returned period to large for this aggregation level";
//    *resultLen = strlen(*postdata);
//    return false;
//  }  
//  char QueryText[8192];
//  int  QueryLen = 0;
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To);
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Serial);
//  db->ExecSentence();
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"MaxSamples\", ", (To - From) / PeriodSeconds);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'%s' AS \"Aggregation_Source\", ", Aggreg);
//  for(uint i = 0; i < json_object_array_length(Series); i++) {
//    if(json_object_get_type(json_object_array_get_idx(Series, i)) != json_type_string) {
//      *postdata  = (char*)"Series must be an array of valid serie names";
//      *resultLen = strlen(*postdata);
//      return false;
//    }
//    char *Serie = (char*)json_object_get_string(json_object_array_get_idx(Series, i));
//    if((strcmp(Serie, "FREQUENCY") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "10M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS \"Frequency\", ");
//    } else if((strcmp(Serie, "FLAG") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "10M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS \"Flag\", ");
//    } else if((strcmp(Serie, "VOLTAGE") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "10M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS \"Voltage\", ");
//    } else if((strcmp(Serie, "CURRENT") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "10M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS \"Current\", ");
//    } else if((strcmp(Serie, "ACTIVE_POWER") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "15M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS \"Active_Power\", ");
//    } else if((strcmp(Serie, "REACTIVE_POWER") == 0) && ((strcmp(Aggreg, "3S") == 0) || (strcmp(Aggreg, "1M") == 0) || (strcmp(Aggreg, "15M") == 0) || (strcmp(Aggreg, "1H") == 0))) {      
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS \"Reactive_Power\", ");
//    } else if((strcmp(Serie, "POWER_FACTOR") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(power_factor ORDER BY sampletime ASC), '{}') AS \"Power_Factor\", ");
//    } else if((strcmp(Serie, "VOLTAGE_THD") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(thd_v ORDER BY sampletime ASC), '{}') AS \"Voltage_THD\", ");
//    } else if((strcmp(Serie, "CURRENT_THD") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(thd_i ORDER BY sampletime ASC), '{}') AS \"Current_THD\", ");
//    } else if((strcmp(Serie, "PHI") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(phi ORDER BY sampletime ASC), '{}') AS \"Phi\", ");
//    } else if((strcmp(Serie, "VOLTAGE_HARMONICS") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(harmonics_v ORDER BY sampletime ASC), '{}') AS \"Voltage_Harmonics\", ");
//    } else if((strcmp(Serie, "CURRENT_HARMONICS") == 0) && (strcmp(Aggreg, "1M") == 0)) {
//      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(harmonics_i ORDER BY sampletime ASC), '{}') AS \"Current_Harmonics\", ");
//    } else {
//      *postdata  = (char*)"Invalid serie in selected aggregation";
//      *resultLen = strlen(*postdata);
//      return false;
//    }
////    if((ArrayLenA != 0) && (ArrayLenB == 0)) {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d) THEN array_fill(NULL::real, array[%d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, ArrayLenA, Serie, Serie);
////    } else if((ArrayLenA != 0) && (ArrayLenB != 0)) {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d OR array_length(%s, 2) <> %d) THEN array_fill(NULL::real, array[%d, %d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, Serie, ArrayLenB, ArrayLenA, ArrayLenB, Serie, Serie);
////    } else {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(%s ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie);
////    }
//  }
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\" FROM ");
//  if(strcmp(Aggreg, "1M") == 0) {
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp");
////  } else if(strcmp(Aggreg, "1h") == 0) {
////    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1h NATURAL LEFT JOIN aggreg_stat_1h NATURAL FULL JOIN prices) AS Tmp");
//  } else {
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "aggreg_%s", Aggreg);
//  }
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, " WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64 ") AS Result", LastSample, From, To);    
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::GetSeriesAggregCallback(HTTPRequest &req) {
//  uint64_t Period = 0, From = 0, To = 0;
//  char    *Analyzer = NULL, *Aggreg = NULL, *Series = NULL;
//  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
//    enum json_type JSON_Type = json_object_get_type(JSON_Val);
//    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
//      Series = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
//      Aggreg = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
//      Period = json_object_get_int64(JSON_Val);
//    else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
//      To = json_object_get_int64(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
//      Analyzer = (char*)json_object_get_string(JSON_Val);
//  }
//  if(Analyzer == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(Series == NULL) {
//    *postdata  = (char*)"'Series' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0))) {
//    *postdata  = (char*)"'Aggreg' need to be '3s', '1m', '10m' or '1h'";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(To == 0) To = Tools::GetTime();
//  From = To - Period;
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer);
//  db->ExecSentence();
//  char QueryText[8192];
//  int  QueryLen = 0;
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT COUNT(*) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS \"Samples\", ", Aggreg, From, To);
//  char *Save;
//  char *Serie = strtok_r(Series, ", ", &Save);
//  while(Serie != NULL) {
//    char *Oper = NULL, *Tmp1 = strchr(Serie, '('), *Tmp2 = strrchr(Serie, ')');
//    if((Tmp1 != NULL) && (Tmp2 != NULL)) {
//      Oper = Serie;
//      Tmp1[0] = 0;
//      Serie = &Tmp1[1];
//      Tmp2[0] = 0;
//    }
//    if((strcmp(Serie, "rms_v") == 0) || (strcmp(Serie, "rms_i") == 0) || (strcmp(Serie, "active_power") == 0) || (strcmp(Serie, "apparent_power") == 0) || (strcmp(Serie, "power_factor") == 0) || (strcmp(Serie, "active_energy") == 0) || (strcmp(Serie, "thd_v") == 0) || (strcmp(Serie, "thd_i") == 0) || (strcmp(Serie, "phi") == 0) || (strcmp(Serie, "harmonics_i") == 0) || (strcmp(Serie, "harmonics_v") == 0)) {
//      if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
//        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT %s(elem) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " GROUP BY rn ORDER BY rn) as %s_%s,", Oper, Aggreg, Serie, From, To, Oper, Serie);
//      } else if(strcmp(Oper, "rms") == 0) {
//        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT SQRT(SUM(POW(elem, 2.0)) / COUNT(*)) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " GROUP BY rn ORDER BY rn) as %s_%s,", Aggreg, Serie, From, To, Oper, Serie);
//      }
//    } else if(strcmp(Serie, "freq") == 0) {
//      if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
//        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT %s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Oper, Serie, Aggreg, From, To, Oper, Serie);
//      } else if(strcmp(Oper, "rms") == 0) {
//        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT SQRT(SUM(POW(%s, 2.0)) / COUNT(*)) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Serie, Aggreg, From, To, Oper, Serie);
//      }
//    } else if(strcmp(Serie, "flag") == 0) {
//      if((strcmp(Oper, "and") == 0) || (strcmp(Oper, "or") == 0)) {
//        QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT bool_%s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 ") AS %s_%s, ", Oper, Serie, Aggreg, From, To, Oper, Serie);
//      }
//    }
//    Serie = strtok_r(NULL, ", ", &Save);
//  }
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\") AS Result", To); 
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::GetSeriesNowCallback(HTTPRequest &req) {
//  char *Serial = NULL;
//  json_object *Series = NULL;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "Series", &Find) == true) && (json_object_get_type(Find) == json_type_array)) Series = Find;
//  if(Serial == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(Series == NULL) {
//    *postdata  = (char*)"Series must be an array of valid serie names";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
//    AnalyzerInterface *Analyzer = (*it)->FindAnalyzer(Serial);
//    if(Analyzer == NULL) continue;
//    *writer = Analyzer->GetJSONParams(Series);
//    *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
//    return true;
//  }
  return "";
}
string DeviceInterface::GetSeriesRangeCallback(HTTPRequest &req) {
//  char *Analyzer = NULL, *Aggreg = NULL;
//  json_object_object_foreach(reader, JSON_Key, JSON_Val) {
//    enum json_type JSON_Type = json_object_get_type(JSON_Val);
//    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Analyzer") == 0))
//      Analyzer = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
//      Aggreg = (char*)json_object_get_string(JSON_Val);
//  }
//  if(Analyzer == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0))) {
//    *postdata  = (char*)"'Aggreg' need to be '3s', '1m', '10m' or '1h'";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer);
//  db->ExecSentence();
//  char QueryText[8192];
//  snprintf(QueryText, sizeof(QueryText), "SELECT row_to_json(Result) AS result FROM ( SELECT max(sampletime) AS \"To\", min(sampletime) AS \"From\" FROM aggreg_%s) AS Result", Aggreg);
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::GetEventsCallback(HTTPRequest &req) {
//  uint64_t LastEvent = 0, From = 0, To = 0, Period = 0;
//  char    *Analyzer = NULL;
//  if(Tools::JSONString(reader, "Analyzer", &Analyzer, false) == false) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  Tools::JSONUInt64(reader, "LastEvent", &LastEvent);
//  Tools::JSONUInt64(reader, "To", &To);
//  Tools::JSONUInt64(reader, "Period", &Period);
//  if(To == 0) To = Tools::GetTime();
//  From = To - Period;
//  char QueryText[8192];
//  int  QueryLen = 0;
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer);
//  db->ExecSentence();
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}') AS \"StartTime\", ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}') AS \"Duration\", ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}') AS \"SampleRate\", ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(type ORDER BY starttime ASC), '{}') AS \"Type\", ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(voltage ORDER BY starttime ASC), '{}') AS \"Voltage\", ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(percent ORDER BY starttime ASC), '{}') AS \"Percentage\" ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM events WHERE starttime > %" PRIu64 " AND starttime >= %" PRIu64 " AND starttime <= %" PRIu64 ") AS Result", LastEvent, From, To);
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
string DeviceInterface::GetPricesCallback(HTTPRequest &req) {
//  uint64_t     Period = 0, From = 0, To = 0, LastSample  = 0;
//  char        *Aggreg = NULL;
//  char        *Serial = NULL;
//  json_object *Find;
//
//  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "Aggreg", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Aggreg = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "LastSample", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastSample = json_object_get_int64(Find);
//  if((json_object_object_get_ex(reader, "Period", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Period = json_object_get_int64(Find);
//  if((json_object_object_get_ex(reader, "To", &Find) == true) && (json_object_get_type(Find) == json_type_int)) To = json_object_get_int64(Find);
//  if(Serial == NULL) {
//    *postdata  = (char*)"'Analyzer' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if((Aggreg == NULL) || ((strcmp(Aggreg, "15M") != 0) && (strcmp(Aggreg, "1H") != 0))) {
//    *postdata  = (char*)"'Aggreg' need to be '15M' or '1H'";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  *writer = json_object_new_object();
//  if(To == 0) To = Tools::GetTime();
//  From = To - Period;
//  To = (To / 900000) * 900000 - 1;
//  json_object_object_add(*writer, "To", json_object_new_int64(To));
//  From = (From / 900000) * 900000;
//  json_object_object_add(*writer, "From", json_object_new_int64(From));
//  From += 899999;
//  json_object *Sampletimes = json_object_new_array();
//  json_object *Actives = json_object_new_array();
//  json_object *Reactives = json_object_new_array();
//  json_object *Periods = json_object_new_array();
//  json_object *Powers = json_object_new_array();
//  char QueryText[8192];
//  sprintf(QueryText, "SELECT to_json(array_agg(json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Details', details))) FROM analyzers_rates WHERE serial = '%s'", Serial);
//  json Tariffs = json::parse(db->ExecSentenceJSON(QueryText, NULL));
//  sprintf(QueryText, "SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power  FROM \"%s\".aggreg_15m WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64, Serial, LastSample, From, To);  
//  db->ExecSentence(QueryText);
//  Tools::LogMsg("INFO", "Calculando desde: %" PRIu64 "\n", From);
//  uint32_t Row = 0;
//  while(From < To) {
//    json_object_array_add(Sampletimes, json_object_new_int64(From));
//    // Busco la potencia en el periodo que estoy procesando
//    float Active = NAN;
//    float Reactive = NAN;
//    if((Row < db->ResultRows()) && (strtoull(db->ResultValue(Row, "sampletime"), NULL, 10) == From)) {
//      Active   = atof(db->ResultValue(Row, "active_power"));
//      Reactive = atof(db->ResultValue(Row, "reactive_power"));
//      Row++;
//      json_object_array_add(Actives, json_object_new_double(Active));
//      json_object_array_add(Reactives, json_object_new_double(Reactive));
//    } else {
//      json_object_array_add(Actives, NULL);
//      json_object_array_add(Reactives, NULL);
//    }
//    //Busco la tarifa que cubre el rango y el tipo de periodo
//    uint32_t Tariff = UINT32_MAX;
//    uint8_t Period = UINT8_MAX;
//    for(uint32_t n = 0; n < Tariffs.size(); n++) {
//      if((Tariffs[n]["StartDate"] >= From) || (Tariffs[n]["EndDate"] < From)) continue;
//      Tariff = n;
//      for(uint32_t m = 0; m < Tariffs[n]["Details"]["Seasons"].size(); m++) {
//        if((Tariffs[n]["Details"]["Seasons"][m]["StartDate"] >= From) || (Tariffs[n]["Details"]["Seasons"][m]["EndDate"] < From)) continue;
//        uint8_t Hour   = (From / 3600000) % 24;
//        Period = Tariffs[n]["Details"]["Seasons"][m]["DayPeriods"][Hour];
//        break;
//      }
//      break;
//    }
//    if(Period != UINT8_MAX) {
//      char PeriodName[16];
//      snprintf(PeriodName, sizeof(PeriodName), "P%d", Period);
//      json_object_array_add(Periods, json_object_new_string(PeriodName));
//      json_object_array_add(Powers, json_object_new_double(Tariffs[Tariff]["Details"]["Periods"][Period - 1]["Contracted_Power"]));
//    } else {
//      json_object_array_add(Periods, NULL);
//      json_object_array_add(Powers, NULL);
//
//    }
//    
//      
//
////      Tools::LogMsg("INFO", "Hora %d\n", Hour);
//      //esta es la tarifa que esta en el rango
//      //tengo que extraer la hora en localtime!
//
//
//
////    if(Tariff != UINT32_MAX) Tools::LogMsg("INFO", "Tarifa encontrada para el rango\n");
//    //Busco en las tarifas en cual de ellas cae el periodo en el que estoy    
//    
//    
//    
//      
//    
//    
//    //float Active   = atof(db->ResultValue(i, "active_power"));
////    float Reactive = atof(db->ResultValue(i, "reactive_power"));
////    uint64_t Time  = strtoull(db->ResultValue(i, "sampletime"), NULL, 10);
////    Tools::LogMsg("INFO", "Sampletime: %" PRIu64 ", Activa: %f, Reactiva: %f\n", Time, Active, Reactive);
//
//    
//  //  busco tarifa aplicable en el periodo
//  //  busco periodo del dia en el que estoy
//  //  Es festivo ? es fin de semana?
//  //  calculo coste de energia y otros factores para el bloque de 15 minutos
//  //  Aplico configuracion de modulos adicionales
//  //  Si he pedido agregacion de 15 minutos escupo datos y reinicio contadores, si es la de 1 hora y ha habido cambio de hora pues lo mismo
//  //  
//    
//  From += 900000;
//    
//
//
//
//
//
//  }
//  json_object_object_add(*writer, "Sampletime", Sampletimes);
//  json_object_object_add(*writer, "Active_Energy", Actives);
//  json_object_object_add(*writer, "Reactive_Energy", Reactives);
//  json_object_object_add(*writer, "Period_Name", Periods);
//  json_object_object_add(*writer, "Contracted_Power", Powers);
//  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return "";
  
  
  
//  
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer);
//  db->ExecSentence();
//  char QueryText[8192];
//  int  QueryLen = 0;
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\",");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(active_energy ORDER BY sampletime ASC), '{}') AS active_energy,");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(price ORDER BY sampletime ASC), '{}') AS price,");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\" ", To);
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM (SELECT sampletime, COALESCE((SELECT SUM(s) FROM UNNEST(active_energy) s), 0) as active_energy, price FROM aggreg_1h NATURAL FULL JOIN prices ");
//  QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64 ") AS T1) AS Result", LastSample, From, To);    
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
  return "";
}
