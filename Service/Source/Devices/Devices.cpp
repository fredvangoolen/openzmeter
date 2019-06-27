/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "Devices.h"
#include "HTTPServer.h"
#include "Tariff.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
vector<Device*> __attribute__((init_priority(200))) DeviceManager::pDevices;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  HTTPServer::RegisterCallback("/getDevices",          GetDevicesCallback);
  HTTPServer::RegisterCallback("/getDevice",           GetDeviceCallback);
  HTTPServer::RegisterCallback("/stopDevice",          StopDeviceCallback);
  HTTPServer::RegisterCallback("/startDevice",         StartDeviceCallback);
  HTTPServer::RegisterCallback("/setDevice",           SetDeviceCallback);
  HTTPServer::RegisterCallback("/getAnalyzers",        DeviceManager::GetAnalyzersCallback);
  HTTPServer::RegisterCallback("/setAnalyzer",         DeviceManager::SetAnalyzerCallback);
  HTTPServer::RegisterCallback("/getSeries",           DeviceManager::GetSeriesCallback);
  HTTPServer::RegisterCallback("/getSeriesNow",        DeviceManager::GetSeriesNowCallback);
  HTTPServer::RegisterCallback("/getSeriesRange",      DeviceManager::GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("/getEvents",           DeviceManager::GetEventsCallback);
  HTTPServer::RegisterCallback("/getPrices",           DeviceManager::GetPricesCallback);
  HTTPServer::RegisterCallback("/getTariffs",          DeviceManager::GetTariffsCallback);
  HTTPServer::RegisterCallback("/setTariffs",          DeviceManager::SetTariffsCallback);
  HTTPServer::RegisterCallback("/delTariffs",          DeviceManager::DelTariffsCallback);
  for(uint32_t i = 0; i < pDevices.size(); i++) pDevices[i]->Start();
}
void DeviceManager::AddDevice(Device *dev) {
  pDevices.push_back(dev);
}
DeviceManager::~DeviceManager() {
  for(uint32_t i = 0; i < pDevices.size(); i++) delete pDevices[i];
}
void DeviceManager::GetDevicesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  json Response = json::array();
  for(uint32_t i = 0; i < pDevices.size(); i++) {
    Device *Dev = pDevices[i];
    json Device;
    Device["Driver"] = Dev->Driver();
    Device["Name"] = Dev->Name();
    Device["Serial"] = Dev->Serial();
    Device["Analyzers"] = Dev->AnalyzersCount();
    Device["Status"] = (Dev->State() == Device::UNCONFIGURED) ? "UNCONFIGURED" : (Dev->State() == Device::STOPPED) ? "STOPPED" : "RUNNING";
    Response.push_back(Device);
  }
  req.FromJSON(Response);
}
void DeviceManager::GetDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(Params["Serial"].is_string() == false) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) return req.FromJSON((*it)->GetConfig());
  }
  return req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::StartDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) {
      json Response;
      Response["Status"] = (*it)->Start();
      return req.FromJSON(Response); 
    }
  }
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::StopDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) {
      json Response;
      Response["Status"] = (*it)->Stop();
      return req.FromJSON(Response); 
    }
  }
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetDeviceCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
    if((*it)->Serial() == Serial) {
      json Response;
//      Response["Status"] = (*it)->SetConfig(Params);
      return req.FromJSON(Response); 
    }
  }
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetAnalyzersCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned and can be edited, for other users only own analyzers or authorized ones are returned and only owner can edit. In all cases only present ones are returned
  json Params = req.GetParams();
  json Response = json::array();
  req.ExecSentenceAsyncResult("SELECT serial, username, timezone, country_code FROM analyzers NATURAL LEFT JOIN country WHERE serial IN (SELECT serial FROM analyzers WHERE username = " + req.Bind(req.UserName()) + " UNION SELECT username FROM permisions WHERE username = " + req.Bind(req.UserName()) + ")");
  while(req.FecthRow()) {
    string Serial = req.ResultString("serial");
    AnalyzerInterface *Analyzer = NULL;
    for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
      Analyzer = (*it)->FindAnalyzer(Serial);
      if(Analyzer == NULL) continue;
      json Row = Analyzer->GetJSONDescriptor();
      Row["Owner"] = req.ResultString("username");
      Row["TimeZone"] = req.ResultString("timezone");
      Response.push_back(Row);
      break;
    }
  }
  req.FromJSON(Response);
}
void DeviceManager::SetAnalyzerCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned and can be edited, for other users only own analyzers or authorized ones are returned and only owner can edit. In all cases only present ones are returned
  json Params = req.GetParams();
  if(!Params["ID"].is_string() || !Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'ID', 'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
  AnalyzerInterface *Analyzer = NULL;
  Device *Dev = NULL;
  string Serial = Params["ID"];
  for(uint n = 0; n < pDevices.size(); n++) {
    Dev = pDevices[n];
    Analyzer = Dev->FindAnalyzer(Serial);
    if(Analyzer != NULL) break;
  }
  if(Analyzer == NULL) return req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  float MaxCurrent = Params["MaxCurrent"];
  float MinCurrent = Params["MinCurrent"];
  string Name = Params["Name"];
  float Freq = Params["NominalFreq"];
  float Voltage = Params["NominalVoltage"];
  json Result = json::object();
  if((MaxCurrent < 0) || (MinCurrent < 0) || (MinCurrent >= MaxCurrent) || (Freq < 0) || (Freq > 100) || (Voltage < 50) || (Voltage > 450)) return req.FromString("'MaxCurrent', 'MinCurrent', 'NominalFreq' or 'NominalVoltage' is out of valid ranges", MHD_HTTP_BAD_REQUEST);
  if(req.ExecSentenceNoResult("UPDATE analyzers SET frequency = " + req.Bind(Freq) + ", voltage = " + req.Bind(Voltage) + ", name = " + req.Bind(Name) + ", max_current = " + req.Bind(MaxCurrent) + ", min_current = " + req.Bind(MinCurrent) + " WHERE serial = " + req.Bind(Serial)) == true) {
    if(Dev->State() == Device::RUNNING) {
      Dev->Stop();
      Result["Result"] = Dev->Start();
    } else {
      Result["Result"] = true;
    }
  } else {
    Result["Result"] = false;
  }
  req.FromJSON(Result);
}
void DeviceManager::GetSeriesCallback(HTTPRequest &req) {
  //Only owner or auth users can get access
  ///TODO: Comprobar permisos
  json Params = req.GetParams();
  //Insert defaults
  if(!Params["To"].is_number())         Params["To"] = 0;
  if(!Params["Period"].is_number())     Params["Period"] = 3600000;
  if(!Params["LastSample"].is_number()) Params["LastSample"] = 0;
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
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
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  string Query = "SELECT row_to_json(Result) AS result FROM ( SELECT " + req.Bind(From) + " AS " + req.BindID("From") + ", " + req.Bind(To) +" AS " + req.BindID("To") + ", ";
  Query += req.Bind((To - From) / PeriodSeconds) + " AS " + req.BindID("MaxSamples") + ", " + req.Bind(Aggreg) + " AS " + req.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < Params["Series"].size(); i++) {
    if(!Params["Series"][i].is_string()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
    string Serie = Params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + req.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Active_Power") + ", ";
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {      
      Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + req.BindID("Reactive_Power") + ", ";
    } else if((Serie == "POWER_FACTOR") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(power_factor ORDER BY sampletime ASC), '{}') AS " + req.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(thd_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(thd_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(phi ORDER BY sampletime ASC), '{}') AS " + req.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      Query += "COALESCE(ARRAY_AGG(harmonics_v ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      Query += "COALESCE(ARRAY_AGG(harmonics_i ORDER BY sampletime ASC), '{}') AS " + req.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + req.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + req.BindID("Voltage_Harmonics_Stats") + ", ";
    } else {
      return req.FromString("Invalid serie in selected aggregation", MHD_HTTP_BAD_REQUEST);
    }
  }
  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + req.BindID("Sampletime") + " FROM ";
  if(Params["Aggreg"] == "1M") {
    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
  } else {
    Query += "aggreg_" + Aggreg;
  }
  Query += " WHERE sampletime > " + req.Bind(LastSample) + " AND sampletime >= " + req.Bind(From) + " AND sampletime <= " + req.Bind(To) + ") AS result";
  req.ExecSentenceAsyncResult(Query);
//  printf("%s\n", Query.c_str());
//  fflush(stdout);
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void DeviceManager::GetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) {
    if(!Params["RateID"].is_number_unsigned()) {
      req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') as result FROM rates WHERE username = 'admin' OR username = " + req.Bind(req.UserName()));
    } else {
      uint32_t RateID = Params["RateID"];
      req.ExecSentenceAsyncResult("SELECT json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Template', template) as result FROM rates WHERE (username = 'admin' OR username = " + req.Bind(req.UserName()) + ") AND rate = " + req.Bind(RateID));
    }  
  } else {
    string Analyzer = Params["Analyzer"];
    if(!Params["RateID"].is_number_unsigned()) {
      req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', analyzers_rates.name, 'StartDate', startdate, 'EndDate', enddate))), '[]') as result FROM analyzers_rates LEFT JOIN analyzers USING(serial) WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer));
    } else {
      uint32_t RateID = Params["RateID"];
      req.ExecSentenceAsyncResult("SELECT json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) as result FROM analyzers_rates where rate = " + req.Bind(RateID));
    }
  }
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void DeviceManager::SetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  string   TimeZone = Params["CountryCode"];
  req.ExecSentenceAsyncResult("SELECT timezone FROM country WHERE country_code = " + req.Bind(TimeZone));
  if(req.FecthRow() == false) return req.FromString("Invalid 'CountryCode'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Template"].is_object()) return req.FromString("'Template' param required", MHD_HTTP_BAD_REQUEST);
  Tariff T(Start, End, req.ResultString("timezone"));
  if(T.SetTemplate(Params["Template"]) == false) return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  if(!Params["Analyzer"].is_string()) {
    //System tariff
    if(!Params["RateID"].is_number_unsigned()) {
      req.FromString("", (req.ExecSentenceNoResult("INSERT INTO rates(name, username, country_code, template, startdate, enddate) VALUES (" + req.Bind(Name) + ", " + req.Bind(req.UserName()) + ", 'UTC', " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Start) + ", " + req.Bind(End) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    } else {
      uint32_t RateID = Params["RateID"];
      req.FromString("", (req.ExecSentenceNoResult("UPDATE rates SET name = " + req.Bind(Name) + ", country_code = 'UTC', template = " + req.Bind(T.GetTemplate().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE username = " + req.Bind(req.UserName()) + " AND rate = " + req.Bind(RateID)) == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    }
  } else {
    //Analyzer tariff
    string Analyzer = Params["Analyzer"];
    if(!Params["Config"].is_object()) return req.FromString("'Config' param required", MHD_HTTP_BAD_REQUEST);    
    if(T.SetConfig(Params["Config"]) == false) return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
    if(!Params["RateID"].is_number_unsigned()) {
      req.FromString("", (req.ExecSentenceNoResult("INSERT INTO analyzers_rates(serial, startdate, enddate, config, template, name) VALUES (" + req.Bind(Analyzer) + ", " + req.Bind(Start) + ", " + req.Bind(End) + ", " + req.Bind(T.GetConfig().dump()) + ", " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Name) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    } else {
      uint32_t RateID = Params["RateID"];
      req.FromString("", (req.ExecSentenceNoResult("UPDATE analyzers_rates SET name = " + req.Bind(Name) + ", template = " + req.Bind(T.GetTemplate().dump()) + ", config = " + req.Bind(T.GetConfig().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE rate = " + req.Bind(RateID) + " AND serial IN (SELECT serial FROM analyzers WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
    }
  }
}
void DeviceManager::DelTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  uint32_t RateID = Params["RateID"];
  if(!Params["Analyzer"].is_string()) {
    req.FromString("", (req.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + req.Bind(RateID) + " AND username = " + req.Bind(req.UserName())) == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
  } else {
    string Analyzer = Params["Analyzer"];
    req.FromString("", (req.ExecSentenceNoResult("DELETE FROM analyzers_rates WHERE rate = " + req.Bind(RateID) + " AND serial IN (SELECT serial from analyzers WHERE serial = " + req.Bind(Analyzer) + " AND username = " + req.Bind(req.UserName()) + ")") == true) ? MHD_HTTP_OK : MHD_HTTP_BAD_REQUEST);
  }
}
void DeviceManager::GetPricesCallback(HTTPRequest &req) {
  ///TODO Comprobar permisos
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  if(!Params["To"].is_number_unsigned() || !Params["Period"].is_number_unsigned() || !Params["LastSample"].is_number_unsigned()) return req.FromString("'To', 'Period' and 'LastSample' must be a numbers", MHD_HTTP_BAD_REQUEST);
  uint64_t To         = Params.value<uint64_t>("To", 0);
  uint64_t Period     = Params.value<uint64_t>("Period", 3600000);
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  string   Aggreg     = Params["Aggreg"];
  uint64_t Now        = Tools::GetTime();
  if(To == 0) To = Now;
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  string TimeZone = "Etc/UTC";
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
  json Result;
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Power"]       = json::array();
  Result["Reactive_Power"]     = json::array();
  Result["Period_Name"]        = json::array();
  Result["Period_Color"]       = json::array();
  Result["Contracted_Power"]   = json::array();
  Result["Expense"]            = json::array();
  Result["Bills"]              = json::array();
  //Load involved tariffs, currency and timezone
  req.ExecSentenceAsyncResult("SELECT currency, timezone, to_json(array_agg(json_build_object('Name', analyzers_rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as tariff " 
                   "FROM analyzers, analyzers_rates, country WHERE country.country_code = analyzers.country_code AND analyzers_rates.serial = analyzers.serial AND analyzers.serial = " + req.Bind(Analyzer) + " AND enddate >= " + req.Bind(From) + " AND startdate <= " + req.Bind(To) + 
                   " AND analyzers.serial IN (SELECT serial FROM analyzers WHERE username = " + req.Bind(req.UserName()) + " UNION SELECT serial FROM permisions WHERE username = " + req.Bind(req.UserName()) + ") GROUP BY currency, timezone");
  json DBTariffs = json::array();
  if(req.FecthRow()) {
    Result["Currency"] = req.ResultString("currency");
    TimeZone = req.ResultString("timezone");
    DBTariffs = req.ResultJSON("tariff");
  }
  TariffList Tariffs(DBTariffs, TimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  req.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + req.Bind(BillingFrom) + " AND sampletime <= " + req.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = req.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (req.ResultUInt64("sampletime") == time)) {
      Active = req.ResultFloat("active_power");
      Reactive = req.ResultFloat("reactive_power");
      MoreData = req.FecthRow();
    }
    Tariff *Tariff = Tariffs.Find(time);
    if((Tariff != NULL) && (Tariff->PushSample(time, Active, Reactive) == true) && ((time + 900000) != BillingTo)) continue;  //If under a tariff push samples until bill cycle ends or no more data!
    //Recover/group values
    do {
      string   Color      = "";
      string   Period     = "";
      uint64_t Time       = time;
      float    Cost       = NAN;
      float    Contracted = NAN;
      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
        Result["Bills"].push_back(Tariff->GetResume(Now));
        break;
      }
      if(Time <= Now) AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
      if(Time <= Now) AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
      if(Time <= Now) AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
      if(Aggreg == "1H") {
        if((Time % 3600000) != 3599999) {
          if(Tariff != NULL) continue;
          break;
        }
        AccActive = AccActive / 4.0;
        AccReactive = AccReactive / 4.0;
      }
      if((Time > LastSample) && (Time >= From) && (Time <= To)) {
        Result["Sampletime"].push_back(Time);
        Result["Active_Power"].push_back(AccActive);
        Result["Reactive_Power"].push_back(AccReactive);
        Result["Contracted_Power"].push_back(Contracted);
        if(Color.empty() == true)
          Result["Period_Color"].push_back(nullptr);
        else 
          Result["Period_Color"].push_back(Color);
        if(Period.empty() == true)
          Result["Period_Name"].push_back(nullptr);
        else 
          Result["Period_Name"].push_back(Period);
        Result["Expense"].push_back(AccCost);
      }
      AccActive = NAN;
      AccReactive = NAN;
      AccCost = NAN;
    } while(Tariff != NULL);
  }
  req.FromJSON(Result);
  return;  
}
void DeviceManager::GetSeriesNowCallback(HTTPRequest &req) {
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
    else if(Serie == "SYS_STATS")         Copy = (AnalyzerInterface::ParamsType)(Copy | AnalyzerInterface::SYS_STATS);
    else return req.FromString("Unknown serie name", MHD_HTTP_BAD_REQUEST);
  }
  for(vector<Device*>::iterator it = pDevices.begin() ; it != pDevices.end(); ++it) {
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
    if(Copy & AnalyzerInterface::SYS_STATS) Result["Sys_Stats"] = Aggreg.Sys_Stats;
    Result["Sampletime"] = Aggreg.Timestamp;
    req.FromJSON(Result);
    return;
  }
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  return;
}
void DeviceManager::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  string Aggreg = Params["Aggreg"];
  string Analyzer = Params["Analyzer"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer) + ", 'public'");
  req.ExecSentenceAsyncResult("SELECT ranges FROM analyzers, (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 "
                              "WHERE username = " + req.Bind(req.UserName()) + " OR serial IN (SELECT serial from permisions WHERE username = " + req.Bind(req.UserName()) + " AND serial = " + req.Bind(Analyzer) + ") LIMIT 1"  
  );
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("ranges"), MHD_HTTP_OK);
}
void DeviceManager::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Analyzer = Params["Analyzer"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Analyzer));
  if(Params["ID"].is_number_unsigned()) {
    if(!Params["Type"].is_string()) return req.FromString("'Type' param is requiered", MHD_HTTP_BAD_REQUEST);
    uint64_t ID = Params["ID"];
    string Type = Params["Type"];
    req.ExecSentenceAsyncResult("SELECT json_build_object('StartTime', starttime, 'Duration', duration, 'SampleRate', samplerate, 'Finished', finished, 'Type', evttype, 'Voltage', voltage, 'Peak', peak, 'RAWPre', samples1, 'RMSPre', rms1, 'RAW', samples2, 'RMS', rms2, 'RAWPost', samples3, 'RMSPost', rms3) AS evt "
                                "FROM events WHERE starttime = " + req.Bind(ID) + " AND evttype = " + req.Bind(Type));
  } else {
    if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
    uint64_t To = Params["To"];
    uint64_t Period = Params["Period"];
    uint64_t LastSample = Params["LastSample"];
    if(To == 0) To = Tools::GetTime();
    uint64_t From = To - Period;
    req.ExecSentenceAsyncResult("SELECT json_build_object('From', " + req.Bind(From) + ", 'To', " + req.Bind(To) + ", 'StartTime', COALESCE(ARRAY_AGG(starttime ORDER BY starttime ASC), '{}'), 'Change', COALESCE(ARRAY_AGG(change ORDER BY starttime ASC), '{}'), "
                                "'Duration', COALESCE(ARRAY_AGG(duration ORDER BY starttime ASC), '{}'), 'SampleRate', COALESCE(ARRAY_AGG(samplerate ORDER BY starttime ASC), '{}'), 'Finished', COALESCE(ARRAY_AGG(finished ORDER BY starttime ASC), '{}'), "
                                "'Type', COALESCE(ARRAY_AGG(evttype ORDER BY starttime ASC), '{}'), 'Reference', COALESCE(ARRAY_AGG(reference ORDER BY starttime ASC), '{}'), 'Peak', COALESCE(ARRAY_AGG(peak ORDER BY starttime ASC), '{}')) AS evt "
                                "FROM events WHERE starttime > " + req.Bind(LastSample) + " AND starttime >= " + req.Bind(From) + " AND starttime <= " + req.Bind(To));
  }
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("evt"), MHD_HTTP_OK);
}
//----------------------------------------------------------------------------------------------------------------------
Device::Device() {
  pState = UNCONFIGURED;
}
Device::DeviceState Device::State() {
  return pState;
}
Device::~Device() {
}
json Device::GetConfig() {
  json Config = ConfigManager::ReadConfig("/Devices/" + Serial());
  if(!Config.is_object()) Config = json::object();
  return Config;
}
string Device::Serial() {
  return pSerial;
}
string Device::Name() {
  return pName;
}
string Device::Driver() {
  return pDriver;
}