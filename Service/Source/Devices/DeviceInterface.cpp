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
    HTTPServer::RegisterCallback("/getSeries",           DeviceInterface::GetSeriesCallback);
    HTTPServer::RegisterCallback("/getSeriesNow",        DeviceInterface::GetSeriesNowCallback);
//    HTTPServer::RegisterCallback("/getSeriesAggreg",     DeviceInterface::GetSeriesAggregCallback);
    HTTPServer::RegisterCallback("/getSeriesRange",      DeviceInterface::GetSeriesRangeCallback);
//    HTTPServer::RegisterCallback("/getEvents",           DeviceInterface::GetEventsCallback);
    HTTPServer::RegisterCallback("/getPrices",           DeviceInterface::GetPricesCallback);
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
      if(Analyzer == NULL) continue;
      Response.emplace_back(Analyzer->GetJSONDescriptor());
      break;
    }
  }
  req.FromJSON(Response);
}
void DeviceInterface::GetSeriesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  AnalyzerInterface *Analyzer = NULL;
  if(!Params["Analyzer"].is_string()) {
    req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
    return;
  }
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    Analyzer = (*it)->FindAnalyzer(Params["Analyzer"]);
    if(Analyzer != NULL) break;
  }
  if(Analyzer == NULL) {
    req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
    return;
  }
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  if(!Params["Series"].is_array()) {
    req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    return;
  }
  uint64_t To     = Params.value("To", 0);
  uint64_t Period = Params.value("Period", 3600000);
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint64_t LastSample =  Params.value("LastSample", 0);
  uint32_t PeriodSeconds = 3000;
  string Aggreg = Params["Aggreg"];
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
    return;
  }  
  req.PrepareSentence("SET search_path TO ?");
  req.BindSingleString(Params.value("Analyzer", "").c_str());
  req.ExecSentence();
  stringstream Query;
  Query << "SELECT row_to_json(Result) AS result FROM ( SELECT ";
  Query << From << " AS \"From\", " << To << " AS \"To\", " << (To - From) / PeriodSeconds << " AS \"MaxSamples\", '" << Aggreg << "' AS \"Aggregation_Source\", ";
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
      return;
    }
    string Serie = Params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query << "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS \"Frequency\", ";
    } else if((Serie == "FLAG") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query << "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS \"Flag\", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query << "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS \"Voltage\", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query << "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS \"Current\", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      Query << "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS \"Active_Power\", ";
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      Query << "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS \"Reactive_Power\", ";
    } else if((Serie == "POWER_FACTOR") && (Aggreg == "1M")) {
      Query << "COALESCE(ARRAY_AGG(power_factor ORDER BY sampletime ASC), '{}') AS \"Power_Factor\", ";
    } else if((Serie == "VOLTAGE_THD") && (Aggreg == "1M")) {
      Query << "COALESCE(ARRAY_AGG(thd_v ORDER BY sampletime ASC), '{}') AS \"Voltage_THD\", ";
    } else if((Serie == "CURRENT_THD") && (Aggreg == "1M")) {
      Query << "COALESCE(ARRAY_AGG(thd_i ORDER BY sampletime ASC), '{}') AS \"Current_THD\", ";
    } else if((Serie == "PHI") && (Aggreg == "1M")) {
      Query << "COALESCE(ARRAY_AGG(phi ORDER BY sampletime ASC), '{}') AS \"Phi\", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      Query << "COALESCE(ARRAY_AGG(harmonics_v ORDER BY sampletime ASC), '{}') AS \"Voltage_Harmonics\", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      Query << "COALESCE(ARRAY_AGG(harmonics_i ORDER BY sampletime ASC), '{}') AS \"Current_Harmonics\", ";
    } else {
     req.FromString("Invalid serie in selected aggregation", MHD_HTTP_BAD_REQUEST);
      return;
    }
////    if((ArrayLenA != 0) && (ArrayLenB == 0)) {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d) THEN array_fill(NULL::real, array[%d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, ArrayLenA, Serie, Serie);
////    } else if((ArrayLenA != 0) && (ArrayLenB != 0)) {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(CASE WHEN (%s IS NULL OR array_length(%s, 1) <> %d OR array_length(%s, 2) <> %d) THEN array_fill(NULL::real, array[%d, %d]) ELSE %s END ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie, ArrayLenA, Serie, ArrayLenB, ArrayLenA, ArrayLenB, Serie, Serie);
////    } else {
////      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "COALESCE(ARRAY_AGG(%s ORDER BY sampletime ASC), '{}') AS %s, ", Serie, Serie);
////    }
  }
  Query << "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS \"Sampletime\" FROM ";
  if(Params["Aggreg"] == "1M") {
    Query << "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
////  } else if(strcmp(Aggreg, "1h") == 0) {
////    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT * FROM aggreg_1h NATURAL LEFT JOIN aggreg_stat_1h NATURAL FULL JOIN prices) AS Tmp");
  } else {
    Query << "aggreg_" << Aggreg;
  }
  Query << " WHERE sampletime > " << LastSample << " AND sampletime >= " << From << " AND sampletime <= " << To << ") AS Result";
  size_t resultLen;
  char *Tmp = req.ExecSentenceJSON(Query.str().c_str(), &resultLen);
  req.FromString(Tmp == NULL ? "Failed query" : Tmp, MHD_HTTP_OK);
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
void DeviceInterface::GetSeriesNowCallback(HTTPRequest &req) {
  ///TODO : CHECK PERMISIONS
  json                          Params = req.GetParams();
  AnalyzerInterface::Params_t   Aggreg = { AnalyzerInterface::UNINITIALIZED };
  AnalyzerInterface::ParamsType Copy   = (AnalyzerInterface::ParamsType)0;
  if(!Params["Analyzer"].is_string()) {
    req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
    return;
  }
  if(!Params["Series"].is_array()) {
    req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    return;
  }
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) {
      req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
      return;
    }
    string Serie = Params["Series"][i];
    if(Serie == "FREQUENCY")              Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::FREQUENCY);
    else if(Serie == "FLAG")              Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::FLAG);
    else if(Serie == "VOLTAGE")           Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE);
    else if(Serie == "CURRENT")           Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::CURRENT);
    else if(Serie == "ACTIVE_POWER")      Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::ACTIVE_POWER);
    else if(Serie == "REACTIVE_POWER")    Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::REACTIVE_POWER);
    else if(Serie == "ACTIVE_ENERGY")     Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::ACTIVE_ENERGY);
    else if(Serie == "REACTIVE_ENERGY")   Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::REACTIVE_ENERGY);
    else if(Serie == "APPARENT_POWER")    Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::APPARENT_POWER);
    else if(Serie == "POWER_FACTOR")      Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::POWER_FACTOR);
    else if(Serie == "VOLTAGE_THD")       Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE_THD);
    else if(Serie == "CURRENT_THD")       Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::CURRENT_THD);
    else if(Serie == "PHI")               Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::PHI);
    else if(Serie == "VOLTAGE_PHASE")     Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE_PHASE);
    else if(Serie == "VOLTAGE_HARMONICS") Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE_HARMONICS);
    else if(Serie == "CURRENT_HARMONICS") Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::CURRENT_HARMONICS);
    else if(Serie == "VOLTAGE_FFT")       Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE_FFT);
    else if(Serie == "CURRENT_FFT")       Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::CURRENT_FFT);
    else if(Serie == "VOLTAGE_SAMPLES")   Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::VOLTAGE_SAMPLES);
    else if(Serie == "CURRENT_SAMPLES")   Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::CURRENT_SAMPLES);
  }
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    AnalyzerInterface *Analyzer = (*it)->FindAnalyzer(Params["Analyzer"]);
    if(Analyzer == NULL) continue;
    Analyzer->GetParams(Aggreg, Copy);
    json Result;
    if(Copy & AnalyzerInterface::FREQUENCY) Result["Frequency"] = Aggreg.Frequency;
    if(Copy & AnalyzerInterface::FLAG) Result["Flag"] = Aggreg.Flag;
    if(Copy & AnalyzerInterface::VOLTAGE) Result["Voltage"] = Aggreg.Voltage;
    if(Copy & AnalyzerInterface::CURRENT) Result["Current"] = Aggreg.Current;
    if(Copy & AnalyzerInterface::ACTIVE_POWER) Result["Active_Power"] = Aggreg.Active_Power;
    if(Copy & AnalyzerInterface::REACTIVE_POWER) Result["Reactive_Power"] = Aggreg.Reactive_Power;
    if(Copy & AnalyzerInterface::ACTIVE_ENERGY)  Result["Active_Energy"] = Aggreg.Active_Energy;
    if(Copy & AnalyzerInterface::REACTIVE_ENERGY) Result["Reactive_Energy"] = Aggreg.Reactive_Energy;
    if(Copy & AnalyzerInterface::APPARENT_POWER) Result["Apparent_Power"] = Aggreg.Apparent_Power;
    if(Copy & AnalyzerInterface::POWER_FACTOR) Result["Power_Factor"] = Aggreg.Power_Factor;
    if(Copy & AnalyzerInterface::VOLTAGE_THD) Result["Voltage_THD"] = Aggreg.Voltage_THD;
    if(Copy & AnalyzerInterface::CURRENT_THD) Result["Current_THD"] = Aggreg.Current_THD;
    if(Copy & AnalyzerInterface::PHI) Result["Phi"] = Aggreg.Phi;
    if(Copy & AnalyzerInterface::VOLTAGE_PHASE) Result["Voltage_Phase"] = Aggreg.Voltage_Phase;
    if(Copy & AnalyzerInterface::VOLTAGE_HARMONICS) Result["Voltage_Harmonics"] = Aggreg.Voltage_Harmonics;
    if(Copy & AnalyzerInterface::CURRENT_HARMONICS) Result["Current_Harmonics"] = Aggreg.Current_Harmonics;
    if(Copy & AnalyzerInterface::VOLTAGE_SAMPLES) Result["Voltage_Samples"] = Aggreg.Voltage_Samples;
    if(Copy & AnalyzerInterface::CURRENT_SAMPLES) Result["Current_Samples"] = Aggreg.Current_Samples;
    if(Copy & AnalyzerInterface::VOLTAGE_FFT) Result["Voltage_FFT"] = Aggreg.Voltage_FFT;
    if(Copy & AnalyzerInterface::CURRENT_FFT) Result["Current_FFT"] = Aggreg.Current_FFT;
    Result["Sampletime"] = Aggreg.Timestamp;
    req.FromJSON(Result);
    return;
  }
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  return;
}
void DeviceInterface::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  AnalyzerInterface *Analyzer = NULL;
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    Analyzer = (*it)->FindAnalyzer(Params["Analyzer"]);
    if(Analyzer != NULL) break;
  }
  if(Analyzer == NULL) return req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  string Aggreg = Params["Aggreg"];
  req.PrepareSentence("SET search_path TO ?");
  req.BindSingleString(Analyzer->GetID().c_str());
  req.ExecSentence();
  string Query = "SELECT row_to_json(Result) AS result FROM ( SELECT max(sampletime) AS \"To\", min(sampletime) AS \"From\" FROM aggreg_" + Aggreg + ") AS Result";
  return req.FromString(req.ExecSentenceJSON(Query.c_str(), NULL), MHD_HTTP_OK);
}
void DeviceInterface::GetEventsCallback(HTTPRequest &req) {
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
  return;
}
void DeviceInterface::GetPricesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) {
    req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
    return;
  }
  string Analyzer = Params["Analyzer"];
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  uint64_t To     = Params.value("To", 0);
  uint64_t Period = Params.value("Period", 3600000);
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint64_t LastSample =  Params.value("LastSample", 0);
  uint32_t PeriodSeconds = 900000;
  string Aggreg = Params["Aggreg"];
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
    return;
  }  
  json Result;
  To = (To / 900000) * 900000 - 1;
  Result["To"] = To;
  From = (From / 900000) * 900000;
  Result["From"] = From;
  From += 899999;
  json Sampletimes = json::array();
  json Actives     = json::array();
  json Reactives   = json::array();
  json Periods     = json::array();
  json Powers      = json::array();
  stringstream Query;
  Query << "SELECT to_json(array_agg(json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Details', details))) FROM analyzers_rates WHERE serial = '" << Analyzer << "'";
  json Tariffs = json::parse(req.ExecSentenceJSON(Query.str().c_str(), NULL));
  //CALCULO PERIODOS DE FACTURACION!
  uint64_t BillingFrom = From;
  Result["BillingFrom"] = BillingFrom;
  uint64_t BillingTo = To;
  Result["BillingTo"] = BillingTo;
  Query.str("");
  Query << "SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power  FROM \"" << Analyzer << "\".aggreg_15m WHERE sampletime > " << LastSample << " AND sampletime >= " << From << " AND sampletime <= " << To;
  req.ExecSentence(Query.str().c_str());
  Tools::LogMsg("INFO", "Calculando desde: %" PRIu64 "\n", From);
  uint32_t Row = 0;
  while(From < To) {
    Sampletimes.push_back(From);
    // Busco la potencia en el periodo que estoy procesando
    float Active = NAN;
    float Reactive = NAN;
    if((Row < req.ResultRows()) && (strtoull(req.ResultValue(Row, "sampletime"), NULL, 10) == From)) {
      Active   = atof(req.ResultValue(Row, "active_power"));
      Reactive = atof(req.ResultValue(Row, "reactive_power"));
      Row++;
      Actives.push_back(Active);
      Reactives.push_back(Reactive);
    } else {
      Actives.push_back(NAN);
      Reactives.push_back(NAN);
    }
    //Busco la tarifa que cubre el rango y el tipo de periodo
    uint32_t Tariff = UINT32_MAX;
    uint8_t Period = UINT8_MAX;
    for(uint32_t n = 0; n < Tariffs.size(); n++) {
      if((Tariffs[n]["StartDate"] >= From) || (Tariffs[n]["EndDate"] < From)) continue;
      Tariff = n;
      for(uint32_t m = 0; m < Tariffs[n]["Details"]["Seasons"].size(); m++) {
        if((Tariffs[n]["Details"]["Seasons"][m]["StartDate"] >= From) || (Tariffs[n]["Details"]["Seasons"][m]["EndDate"] < From)) continue;
        uint8_t Hour   = (From / 3600000) % 24;
        Period = Tariffs[n]["Details"]["Seasons"][m]["DayPeriods"][Hour];
        break;
      }
      break;
    }
    if(Period != UINT8_MAX) {
      string PeriodName = "P" + to_string(Period);
      Periods.push_back(PeriodName);
      Powers.push_back(Tariffs[Tariff]["Details"]["Periods"][Period - 1]["Contracted_Power"]);
    } else {
      Periods.push_back(NAN);
      Powers.push_back(NAN);
    }
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
  From += 900000;
  }
  Result["Sampletime"] = Sampletimes;
  Result["Active_Energy"] = Actives;
  Result["Reactive_Energy"] = Reactives;
  Result["Period_Name"] = Periods;
  Result["Contracted_Power"] = Powers;
  req.FromJSON(Result);
  return;  
}
