#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::GetAnalyzer(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  json Response = CheckConfig(ConfigManager::ReadConfig(pConfigPath));
  pMembersLocks.ReadLock();
  Response["Analyzer"] = Serial;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  Response["Mode"] = (pMode == PHASE1) ? "1PHASE" : (pMode == PHASE3) ? "3PHASE" : "3PHASE+N";
  Response["SampleRate"] = pSamplerate;
  Response.erase("Alarms");
  if(pSync != NULL) Response["RemoteLogged"] = pSync->Logged();
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT SUM(pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename))) AS size from pg_tables WHERE schemaname = " + DB.Bind(Serial));
  if(DB.FecthRow()) Response["Size"] = DB.ResultUInt64("size");
  pMembersLocks.ReadUnlock();
  response.set_content(Response.dump(), "application/json");
  response.status = HTTP_OK;
}
void AnalyzerSoft::SetAnalyzer(const json &params, Response &response) {
//  if(CheckRights(params, response, CONFIG) == false) return;
  json Config = CheckConfig(params);
  Config.erase("Alarms");
  ConfigManager::WriteConfig(pConfigPath, Config);
  pMembersLocks.WriteLock();
  Configure(ConfigManager::ReadConfig(pConfigPath));
  pMembersLocks.WriteUnlock();
  response.set_content("", "text/plain");
  response.status = HTTP_OK;
}
void AnalyzerSoft::GetSeries(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  //Insert defaults
  uint64_t To = params["To"];
  uint64_t Period = params["Period"];
  uint64_t LastSample = params["LastSample"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 200;
  string Aggreg = params["Aggreg"];
  if(Aggreg == "3S") PeriodSeconds = 3000;
  if(Aggreg == "1M") PeriodSeconds = 60000;
  if(Aggreg == "10M") PeriodSeconds = 600000;
  if(Aggreg == "15M") PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    response.set_content("Returned period to large for this aggregation level", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  Database DB(Serial);
//  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(Serial));
  string Query = "SELECT ROW_TO_JSON(Result) AS result FROM ( SELECT " + DB.Bind(From) + " AS " + DB.BindID("From") + ", " + DB.Bind(To) +" AS " + DB.BindID("To") + ", ";
  Query += DB.Bind((To - From) / PeriodSeconds) + " AS " + DB.BindID("MaxSamples") + ", " + DB.Bind(Aggreg) + " AS " + DB.BindID("Aggregation_Source") + ", ";
  for(uint i = 0; i < params["Series"].size(); i++) {
    if(!params["Series"][i].is_string()) {
      response.set_content("Series must be an array of valid serie names", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
    string Serie = params["Series"][i];
    if((Serie == "FREQUENCY") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency") + ", ";
    } else if((Serie == "FLAG") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Flag") + ", ";
    } else if((Serie == "VOLTAGE") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage") + ", ";
    } else if((Serie == "CURRENT") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current") + ", ";
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
      }
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "10C") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      if(Aggreg == "1M") {
        vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      } else {
        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
      }
    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Factor") + ", ";
    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD") + ", ";
    } else if((Serie == "CURRENT_THD") && (((Aggreg == "10C") || Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_THD") + ", ";
    } else if((Serie == "PHI") && ((Aggreg == "10C") || (Aggreg == "1M"))) {
      vector<float> Default = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Phi") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics") + ", ";
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_Harmonics") + ", ";
    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
      vector<vector<float> > Default = vector<vector<float> >((pMode == PHASE1) ? 1 : 3, vector<float>(50, NAN));
      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_p, " + DB.Bind(Default) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Harmonics") + ", ";
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Stats") + ", ";
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency_Stats") + ", ";
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD_Stats") + ", ";
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics_Stats") + ", ";
    } else {
      response.set_content("Invalid serie in selected aggregation", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
  }
  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Sampletime") + " FROM ";
  if(params["Aggreg"] == "1M") {
    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
  } else {
    Query += "aggreg_" + Aggreg;
  }
  Query += " WHERE sampletime > " + DB.Bind(LastSample) + " AND sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + ") AS result";
  DB.ExecSentenceAsyncResult(Query);
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("Result"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::GetSeriesNow(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  if(pNowDataLock.ConsumerWait(5000) == false) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  }
  json Result = json::object();
  Result["Sampletime"] = pAggreg10C.Timestamp;
  for(uint i = 0; i < params["Series"].size(); i++) {
    if(!params["Series"][i].is_string()) {
      pNowDataLock.ConsumerEnds();
      response.set_content("Series must be an array of valid serie names", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
    string Serie = params["Series"][i];
    if(Serie == "FREQUENCY") {
      Result["Frequency"] = pAggreg10C.Frequency;
    } else if(Serie == "FLAG") {
      Result["Flag"] = pAggreg10C.Flag;
    } else if(Serie == "VOLTAGE") {
      Result["Voltage"] = pAggreg10C.Voltage;
    } else if(Serie == "CURRENT") {
      Result["Current"] = pAggreg10C.Current;
    } else if(Serie == "ACTIVE_POWER") {
      Result["Active_Power"] = pAggreg10C.Active_Power;
    } else if(Serie == "REACTIVE_POWER") {
      Result["Reactive_Power"] = pAggreg10C.Reactive_Power;
    } else if(Serie == "ACTIVE_ENERGY") {
      Result["Active_Energy"] = pAggreg10C.Active_Energy;
    } else if(Serie == "REACTIVE_ENERGY") {
      Result["Reactive_Energy"] = pAggreg10C.Reactive_Energy;
    } else if(Serie == "APPARENT_POWER") {
      Result["Apparent_Power"] = pAggreg10C.Apparent_Power;
    } else if(Serie == "POWER_FACTOR") {
      Result["Power_Factor"] = pAggreg10C.Power_Factor;
    } else if(Serie == "VOLTAGE_THD") {
      Result["Voltage_THD"] = pAggreg10C.Voltage_THD;
    } else if(Serie == "CURRENT_THD") {
      Result["Current_THD"] = pAggreg10C.Current_THD;
    } else if(Serie == "PHI") {
      Result["Phi"] = pAggreg10C.Phi;
    } else if(Serie == "VOLTAGE_PHASE") {
       Result["Voltage_Phase"] = pAggreg10C.Voltage_Phase;
    } else if(Serie == "VOLTAGE_HARMONICS") {
      Result["Voltage_Harmonics"] = json::array();
      for(unsigned n = 0; n < pAggreg10C.Voltage_Harmonics.size(); n++) {
        json Ch = json::array();
        for(unsigned i = 0; i < pAggreg10C.Voltage_Harmonics[n].size(); i++) {
          json Row = json::array();
          complex<float> Val = pVoltageFFT[n]->Output[10 + i * 10] * (float)M_SQRT2;
          Row.push_back(Val.real());
          Row.push_back(Val.imag());
          Ch.push_back(Row);
        }
        Result["Voltage_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "CURRENT_HARMONICS") {
      Result["Current_Harmonics"] = json::array();
      for(unsigned n = 0; n < pAggreg10C.Current_Harmonics.size(); n++) {
        json Ch = json::array();
        for(unsigned i = 0; i < pAggreg10C.Current_Harmonics[n].size(); i++) {
          json Row = json::array();
          complex<float> Val = pCurrentFFT[n]->Output[10 + i * 10] * (float)M_SQRT2;
          Row.push_back(Val.real());
          Row.push_back(Val.imag());
          Ch.push_back(Row);
        }
        Result["Current_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "POWER_HARMONICS") {
      Result["Power_Harmonics"] = json::array();
      for(uint n = 0; n < pAggreg10C.Power_Harmonics.size(); n++) {
        json Ch = json::array();
        for(uint i = 0; i < pAggreg10C.Power_Harmonics[n].size(); i++) Ch.push_back(pAggreg10C.Power_Harmonics[n][i]);
        Result["Power_Harmonics"].push_back(Ch);
      }
    } else if(Serie == "VOLTAGE_FFT") {
      Result["Voltage_FFT"] = json::array();
      for(uint i = 0; i < pVoltageChannels; i++) {
        json Ch = json::array();
        for(uint n = 0; n < pVoltageFFT[i]->Output.size(); n++) Ch.push_back(pVoltageFFT[i]->BinPower(n));
        Result["Voltage_FFT"].push_back(Ch);
      }
    } else if(Serie == "CURRENT_FFT") {
      Result["Current_FFT"] = json::array();
      for(uint i = 0; i < pCurrentChannels; i++) {
        json Ch = json::array();
        for(uint n = 0; n < pCurrentFFT[i]->Output.size(); n++) Ch.push_back(pCurrentFFT[i]->BinPower(n));
        Result["Current_FFT"].push_back(Ch);
      }
    } else if(Serie == "VOLTAGE_SAMPLES") {
      Result["Voltage_Samples"] = pVoltageSamples;
    } else if(Serie == "CURRENT_SAMPLES") {
      Result["Current_Samples"] = pCurrentSamples;
    } else if(Serie == "SYS_STATS") {
      Result["Sys_Stats"] = SysSupport::GetSystemStatusJSON();
    } else {
      pNowDataLock.ConsumerEnds();
      response.set_content("Unknown serie name", "text/plain");
      response.status = HTTP_BAD_REQUEST;
      return;
    }
  }
  pNowDataLock.ConsumerEnds();
  response.set_content(Result.dump(), "application/json");
  response.status = HTTP_OK;
}
void AnalyzerSoft::GetSeriesRange(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  string Aggreg = params["Aggreg"];
  Database DB;
  DB.ExecSentenceNoResult("SET search_path TO " + DB.Bind(Serial) + ", 'public'");
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("ranges"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::GetEvents(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->GetEvents(params, response);
  } else {
    response.set_content("Events detection disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetEvent(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->GetEvent(params, response);
  } else {
    response.set_content("Events detection disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::SetEvent(const json &params, Response &response) {
//  if(CheckRights(params, response, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->SetEvent(params, response);
  } else {
    response.set_content("Events detection disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::DelEvent(const json &params, Response &response) {
//  if(CheckRights(params, response, WRITE | CONFIG) == false) return;
  pMembersLocks.ReadLock();
  if(pEventsDetector != NULL) {
    pEventsDetector->DelEvent(params, response);
  } else {
    response.set_content("Events detection disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetRates(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') AS result FROM rates");
  if(DB.FecthRow() == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPCallback::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::GetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID = params["RateID"];
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT JSON_BUILD_OBJECT('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) AS result FROM rates WHERE rate = " + DB.Bind(RateID));
  if(DB.FecthRow() == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPCallback::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoft::SetRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("StartDate") == false) || (params["StartDate"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'StartDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("EndDate") == false) || (params["EndDate"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'EndDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPCallback::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Config") == false) || (params["Config"].is_object() == false)) return HTTPCallback::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPCallback::FillResponse(response, "'Template' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID   = params["RateID"];
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  Tariff T(Start, End, pTimeZone);  //THREAD SAFE ISSUE!
  if(T.SetTemplate(params["Template"]) == false) return HTTPCallback::FillResponse(response, "Invalid template", "text/plain", HTTP_BAD_REQUEST);
  if(T.SetConfig(params["Config"]) == false) return HTTPCallback::FillResponse(response, "Invalid 'Config' format", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("UPDATE rates SET name = " + DB.Bind(Name) + ", template = " + DB.Bind(T.GetTemplate().dump()) + ", config = " + DB.Bind(T.GetConfig().dump()) + ", startdate = " + DB.Bind(Start) + ", enddate = " + DB.Bind(End) + " WHERE rate = " + DB.Bind(RateID));
  HTTPCallback::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::DelRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("RateID") == false) || (params["RateID"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'RateID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint32_t RateID = params["RateID"];
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + DB.Bind(RateID));
  HTTPCallback::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::AddRate(const json &params, Response &response) {
  if(CheckRights(params["UserName"], CONFIG) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("StartDate") == false) || (params["StartDate"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'StartDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("EndDate") == false) || (params["EndDate"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'EndDate' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPCallback::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Config") == false) || (params["Config"].is_object() == false)) return HTTPCallback::FillResponse(response, "'Config' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Template") == false) || (params["Template"].is_object() == false)) return HTTPCallback::FillResponse(response, "'Template' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  Tariff T(Start, End, pTimeZone);  //THREAD SAFE ISSUE!
  if(T.SetTemplate(params["Template"]) == false) return HTTPCallback::FillResponse(response, "Invalid template", "text/plain", HTTP_BAD_REQUEST);
  if(T.SetConfig(params["Config"]) == false) return HTTPCallback::FillResponse(response, "Invalid 'Config' format", "text/plain", HTTP_BAD_REQUEST);
  Database DB(Serial);
  bool Result = DB.ExecSentenceNoResult("INSERT INTO rates(startdate, enddate, config, template, name) VALUES (" + DB.Bind(Start) + ", " + DB.Bind(End) + ", " + DB.Bind(T.GetConfig().dump()) + ", " + DB.Bind(T.GetTemplate().dump()) + ", " + DB.Bind(Name) + ")");
  HTTPCallback::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::GetPrices(const json &params, Response &response) {
  if(CheckRights(params["UserName"], READ) == false) return HTTPCallback::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("To") == false) || (params["To"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Period") == false) || (params["Period"].is_number_unsigned() == false)) return HTTPCallback::FillResponse(response, "'Period' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return HTTPCallback::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To         = params["To"];
  uint64_t Period     = params["Period"];
  string   Aggreg     = params["Aggreg"];
  json     Result     = json::object();
  if((Period / ((Aggreg == "1H") ? 3600000 : 900000)) > 100000) return HTTPCallback::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
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
  Database DB(Serial);
  DB.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + DB.Bind(From) + " AND startdate <= " + DB.Bind(To));
  json DBTariffs = json::array();
  if(DB.FecthRow()) {
    Result["Currency"] = "€";//req.ResultString("currency");
    DBTariffs = DB.ResultJSON("rate");
  }
  TariffList Tariffs(DBTariffs, pTimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + DB.Bind(BillingFrom) + " AND sampletime <= " + DB.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = DB.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (DB.ResultUInt64("sampletime") == time)) {
      Active = DB.ResultFloat("active_power");
      Reactive = DB.ResultFloat("reactive_power");
      MoreData = DB.FecthRow();
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
  response.set_content(Result.dump(), "application/json");
  response.status = HTTP_OK;
}
void AnalyzerSoft::TestAlarm(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->TestAlarm(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetAlarms(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->GetAlarms(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::GetAlarm(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->GetAlarm(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::SetAlarm(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->SetAlarm(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::AddAlarm(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->AddAlarm(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
void AnalyzerSoft::DelAlarm(const json &params, Response &response) {
//  if(CheckRights(params, response, READ) == false) return;
  pMembersLocks.ReadLock();
  if(pAlarms != NULL) {
    pAlarms->DelAlarm(params, response);
  } else {
    response.set_content("Alarms disabled", "text/plain");
    response.status = HTTP_SERVICE_UNAVAILABLE;
  }
  pMembersLocks.ReadUnlock();
}
