/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceInterface.h"
#include "HTTPServer.h"
#include "Tariff.h"
#include "Tools.h"
#include <json.hpp>
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
vector<DeviceInterface*> DeviceInterface::pDevices;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceInterface::Config(json_object *config) {
  if(config == NULL) return false;
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    string Serial = (*it)->Serial();
    json_object *Find;
    if((json_object_object_get_ex(config, Serial.c_str(), &Find) != true) || (json_object_get_type(Find) != json_type_object)) json_object_object_add(config, Serial.c_str(), json_object_new_object());
    if((json_object_object_get_ex(config, Serial.c_str(), &Find) != true) || (json_object_get_type(Find) != json_type_object) || ((*it)->ConfigDev(Find) != true)) {
      Tools::LogMsg("WARN", "Invalid configuration for driver with serial '%s'\n", Serial);
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
    HTTPServer::RegisterCallback("/getDevices",          DeviceInterface::GetDevicesCallback);
    HTTPServer::RegisterCallback("/stopDevice",          DeviceInterface::StopDeviceCallback);
    HTTPServer::RegisterCallback("/startDevice",         DeviceInterface::StartDeviceCallback);
//    HTTPServer::RegisterCallback("/getDeviceSettings",   DeviceInterface::StartDeviceCallback);
//    HTTPServer::RegisterCallback("/setDeviceSettings",   DeviceInterface::StartDeviceCallback);

    HTTPServer::RegisterCallback("/getAnalyzers",        DeviceInterface::GetAnalyzersCallback);
    HTTPServer::RegisterCallback("/getSeries",           DeviceInterface::GetSeriesCallback);
    HTTPServer::RegisterCallback("/getSeriesNow",        DeviceInterface::GetSeriesNowCallback);
//    HTTPServer::RegisterCallback("/getSeriesAggreg",     DeviceInterface::GetSeriesAggregCallback);
    HTTPServer::RegisterCallback("/getSeriesRange",      DeviceInterface::GetSeriesRangeCallback);
//    HTTPServer::RegisterCallback("/getEvents",           DeviceInterface::GetEventsCallback);
    HTTPServer::RegisterCallback("/getPrices",           DeviceInterface::GetPricesCallback);
    HTTPServer::RegisterCallback("/getTariffs",          DeviceInterface::GetTariffsCallback);
    HTTPServer::RegisterCallback("/setTariffs",          DeviceInterface::SetTariffsCallback);
  }
  pDevices.push_back(device);
}
void DeviceInterface::GetDevicesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  json Response = json::array();
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    json Device;
    Device["Driver"] = (*it)->Driver();
    Device["Name"] = (*it)->Name();
    Device["Serial"] = (*it)->Serial();
    Device["Analyzers"] = (*it)->AnalyzersCount();
    Device["Status"] = ((*it)->pState == UNCONFIGURED) ? "UNCONFIGURED" : ((*it)->pState == STOPPED) ? "STOPPED" : "RUNNING";
    Response.push_back(Device);
  }
  req.FromJSON(Response);
}
void DeviceInterface::StartDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Device' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) {
      json Response;
      Response["Status"] = (*it)->Start();
      return req.FromJSON(Response); 
    }
  }
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceInterface::StopDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<DeviceInterface*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) {
      json Response;
      Response["Status"] = (*it)->Stop();
      return req.FromJSON(Response); 
    }
  }
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceInterface::GetAnalyzersCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned and can be edited, for other users only own analyzers or authorized ones are returned and only owner can edit. In all cases only present ones are returned
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
  //Only owner or auth users can get access
  ///TODO: Comprobar permisos  
  json Params = req.GetParams();
  //Insert defaults
  if(!Params["To"].is_number())         Params["To"] = 0;
  if(!Params["Period"].is_number())     Params["Period"] = 3600000;
  if(!Params["LastSample"].is_number()) Params["LastSample"] = 0;
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
  uint64_t To = Params["To"];
  uint64_t Period = Params["Period"];
  uint64_t LastSample = Params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
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
  char *Tmp = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  req.FromString(Tmp == NULL ? "Failed query" : Tmp, MHD_HTTP_OK);
}
void DeviceInterface::GetTariffsCallback(HTTPRequest &req) {
  ///TODO: Comprobar permisos
  json Params = req.GetParams();
  stringstream Query;
  if(!Params["Analyzer"].is_string()) {
    if(!Params["RateID"].is_number()) {
      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') from rates";
    } else {
      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  } else {
    if(!Params["RateID"].is_number()) {
      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = '" << Params["Analyzer"].get<string>() << "'";
    } else {
      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template) from analyzers_rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  }
  char *Tmp = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  req.FromString(Tmp == NULL ? "Failed query" : Tmp, MHD_HTTP_OK);
}
void DeviceInterface::SetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  stringstream Query;
  if(!Params["Analyzer"].is_string()) {
    Query << "INSERT INTO rates(rate, name, username, country_code, details, startdate, enddate) VALUES(";
    if(Params["RateID"] == NULL) 
      Query << "null, ";
    else
      Query << Params["RateID"] << ", ";
    Query << "'" << Params["Name"].get<string>() << "', ";
    Query << "'" << Params["Owner"].get<string>() << "', ";
    Query << "'" << Params["CountryCode"].get<string>() << "', ";
    Query << "'" << Params["Details"].dump() << "', ";
    Query << Params["StartDate"].get<uint64_t>() << ", ";
    Query << Params["EndDate"].get<uint64_t>() << ") ON CONFLICT(rate) DO UPDATE SET rate = EXCLUDED.rate, name = EXCLUDED.name, username = EXCLUDED.username, country_code = EXCLUDED.country_code, details = EXCLUDED.details, startdate = EXCLUDED.startdate, enddate = EXCLUDED.enddate";
    

            //    if(!Params["RateID"].is_number()) {
//    Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') from rates";
//    } else {
//      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = " << Params["RateID"].get<uint32_t>();
//    }  
  } else {
//    Query << "INSERT INTO SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = " << Params["Analyzer"].get<string>();
    
    if(Params["RateID"].is_number()) {
//      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = " << Params["Analyzer"].get<string>();
    } else {
//      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  }
  Tools::LogMsg("INFO", "%s\n", Query.str().c_str());
  req.FromString("", (req.ExecSentence(Query.str().c_str()) == true) ? MHD_HTTP_OK : MHD_HTTP_NOT_ACCEPTABLE);

  
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
}
void DeviceInterface::GetPricesCallback(HTTPRequest &req) {
  ///TODO Comprobar permisos
  //Parse, check params and prepare result struct
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
  uint64_t To         = Params.value<uint64_t>("To", 0);
  uint64_t Period     = Params.value<uint64_t>("Period", 3600000);
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  string   Aggreg     = Params["Aggreg"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
    return;
  }  
  json Result;
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Energy"]      = json::array();;
  Result["Reactive_Energy"]    = json::array();;
  Result["Period_Name"]        = json::array();;
  Result["Period_Color"]       = json::array();;
  Result["Contracted_Power"]   = json::array();;
  Result["Expense"]            = json::array();;

  //Load involved tariffs
  stringstream Query;
  Query << "SELECT to_json(array_agg(json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) FROM analyzers_rates WHERE serial = '" << Analyzer << "' AND enddate >= " << From << " AND startdate <= " << To;
  char *JSONText = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  json TariffsJSON = (JSONText == NULL) ? json::array() : json::parse(JSONText);
  TariffList Tariffs(TariffsJSON);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom(From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  Query.str("");
  Query << "SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power  FROM \"" << Analyzer << "\".aggreg_15m WHERE sampletime > " << BillingFrom << " AND sampletime <= " << BillingTo;
  req.ExecSentence(Query.str().c_str());

  uint32_t Row = 0;
  float Active = NAN;
  float Reactive = NAN;  
  for(uint64_t time = BillingFrom + 899999; time < BillingTo; time = time + 900000) {
    Tariff *Tariff = Tariffs.Find(time);
    if((Row < req.ResultRows()) && (strtoull(req.ResultValue(Row, "sampletime"), NULL, 10) == time)) {
      float Tmp = atof(req.ResultValue(Row, "active_power"));
      Active = isnan(Active) ? Tmp / 4.0 : isnan(Tmp) ? Active : Active + Tmp / 4.0;
      Tmp = atof(req.ResultValue(Row, "reactive_power"));
      Reactive = isnan(Reactive) ? Tmp / 4.0 : isnan(Tmp) ? Reactive : Reactive + Tmp / 4.0;
      Row++;
    }
    if(Tariff == NULL) {
      //Hours not under tariff are copied to output directly
      if((Aggreg == "1H") && ((time % 3600000) != 3599999)) continue;
      if((time > LastSample) && (time >= From) && (time <= To)) {
        Result["Sampletime"].push_back(time);
        Result["Active_Energy"].push_back(Active);
        Result["Reactive_Energy"].push_back(Reactive);
      }
    } else {
      //If under a tariff push samples until bill cycle ends
      if(Tariff->PushSample(time, Active, Reactive) == true) {
        
        //        Result["Sampletime"].push_back(time);
        //Result["Active_Energy"].push_back(Active);
        //Result["Reactive_Energy"].push_back(Reactive);
        //Ademas añado estadisticas de perido o lo que sea que devuelve el modulo de tarifas!
        //He alcanzado un ciclo de facturacion, recupero todos los samples y los paso a la salida
      }
    }
    
//  //Iterate over billing time
//  uint32_t Row = 0;
//  float Active = NAN;
//  float Reactive = NAN;  
//  while(BillingFrom < BillingTo) {
//    Tariff *Tariff = Tariffs.Find(BillingFrom);
//    if(Tariff != NULL) {
//      if(Tariff.IsBillDate(BillingFrom) == true) {
////        Bills.push_back(BillingFrom);
//        //Reinicio contadores y llamo a modulesReset
//        //LLamo a modulesProcessBillDate
////      }
//    }
//    if((Row < req.ResultRows()) && (strtoull(req.ResultValue(Row, "sampletime"), NULL, 10) == (BillingFrom + 899999))) {
//      float Tmp = atof(req.ResultValue(Row, "active_power"));
//      Active = isnan(Active) ? Tmp / 4.0 : isnan(Tmp) ? Active : Active + Tmp / 4.0;
//      Tmp = atof(req.ResultValue(Row, "reactive_power"));
//      Reactive = isnan(Reactive) ? Tmp / 4.0 : isnan(Tmp) ? Reactive : Reactive + Tmp / 4.0;
//      Row++;
//    }
//    if(Tariff != NULL) {
//
//    
//    }
//    if(((Aggreg == "15M") || ((BillingFrom % 3600000) == 3599999)) && (BillingFrom > LastSample)) {
//      Result["Sampletime"].push_back(BillingFrom);
//      Result["Active_Energy"].push_back(Active);
//      Result["Reactive_Energy"].push_back(Reactive);
//      Active = NAN;
//      Reactive = NAN;
//      if(Tariff != UINT32_MAX) {
//        uint8_t Period = FindPeriod(Tariffs[Tariff], BillingFrom);
//        if(Period != UINT8_MAX) {
//          Periods.push_back("P" + to_string(Period));
//          Colors.push_back("#EEE");
//          Powers.push_back(Tariffs[Tariff]["Details"]["Periods"][Period - 1]["Contracted_Power"]);
//          Expenses.push_back(0.0);
//        } else {
//          Periods.push_back(NAN);
//          Colors.push_back("#ddd");
//          Powers.push_back(NAN);
//          Expenses.push_back(0.0);
//        }
//      }
//    }
//    BillingFrom += 900000;
    Active = NAN;
    Reactive = NAN;
  }

  req.FromJSON(Result);
  return;  
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
    else return req.FromString("Unknown serie name", MHD_HTTP_BAD_REQUEST);
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