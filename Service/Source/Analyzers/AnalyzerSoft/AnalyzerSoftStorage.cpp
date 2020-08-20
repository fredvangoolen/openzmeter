// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "AnalyzerSoftStorage.h"
#include "Status.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftStorage::AnalyzerSoftStorage(const string &serial, const Analyzer::AnalyzerMode mode) {
  pSerial = serial;
  pVoltageChannels = (mode == Analyzer::PHASE1) ? 1 : 3;
  pCurrentChannels = (mode == Analyzer::PHASE1) ? 1 : (mode == Analyzer::PHASE3) ? 3 : 4;
}
AnalyzerSoftStorage::~AnalyzerSoftStorage() {
}
void AnalyzerSoftStorage::GetStatus(const json &params, Response &response) {
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json Result = json::object();
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    if(Serie == "SYSTEM") {
      Result["System"] = Status::GetJSON();
    } else if(Serie == "UNREAD_EVENTS") {
      string UserName = params["UserName"];
      Result["Unread_Events"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE starttime > eventsread) AS events  FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == false) {
        Result["Unread_Events"]["From"]  = 0;
        Result["Unread_Events"]["Count"] = 0;
      } else {
        Result["Unread_Events"]["From"]  = DB.ResultUInt64("eventsread");
        Result["Unread_Events"]["Count"] = DB.ResultUInt64("events");
      }
    } else if(Serie == "UNREAD_ALARMS") {
      string UserName = params["UserName"];
      Result["Unread_Alarms"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT alarmsread, (SELECT count(*) FROM alarms_ext WHERE trigger > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == false) {
        Result["Unread_Alarms"]["From"]  = 0;
        Result["Unread_Alarms"]["Count"] = 0;
      } else {
        Result["Unread_Alarms"]["From"]  = DB.ResultUInt64("alarmsread");
        Result["Unread_Alarms"]["Count"] = DB.ResultUInt64("alarms");
      }
    } else if(Serie == "PERMISSIONS") {
      Database DB;
      string UserName = params["UserName"];
      DB.ExecSentenceAsyncResult("SELECT json_build_object('Read', read, 'Write', write, 'Configure', config) as access FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == true) Result["Permissions"] = DB.ResultJSON("access");
    } else if(Serie == "LAST_ACCESS") {
      Database DB;
      string UserName = params["UserName"];
      DB.ExecSentenceAsyncResult("SELECT json_build_object('EventsRead', eventsread, 'AlarmsRead', alarmsread) as access FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == true) Result["Last_Access"] = DB.ResultJSON("access");
    } else if(Serie == "STORAGE") {
      Database DB;
      Result["Storage"] = json::array();
      DB.ExecSentenceAsyncResult("WITH sizes AS (SELECT tablename, pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename)) AS size from pg_tables WHERE schemaname = " + DB.Bind(pSerial) + ") "
        "SELECT 'Aggreg_3S' AS name, size FROM sizes WHERE tablename = 'aggreg_3s' UNION "
        "SELECT 'Aggreg_1M' AS name, SUM(size) FROM sizes WHERE tablename IN ('aggreg_1m', 'aggreg_ext_1m') UNION "
        "SELECT 'Aggreg_10M' AS name, size FROM sizes WHERE tablename = 'aggreg_10m' UNION "
        "SELECT 'Aggreg_15M' AS name, size FROM sizes WHERE tablename = 'aggreg_15m' UNION "
        "SELECT 'Aggreg_1H' AS name, size FROM sizes WHERE tablename = 'aggreg_1h' UNION "
        "SELECT 'Events_Stats' AS name, size FROM sizes WHERE tablename = 'events' UNION "
        "SELECT 'Events_Samples' AS name, size FROM sizes WHERE tablename = 'events_ext' UNION "
        "SELECT 'Alarms' AS name, size FROM sizes WHERE tablename = 'alarms'");
      while(DB.FecthRow()) {
        json Row = json::object();
        Row["Table"] = DB.ResultString("name");
        Row["Size"]  = DB.ResultUInt64("size");
        Result["Storage"].push_back(Row);
      }
    }
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void AnalyzerSoftStorage::GetSeries(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "200MS") && (params["Aggreg"] != "3S") && (params["Aggreg"] != "1M") && (params["Aggreg"] != "10M") && (params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To      = params["To"];
  uint64_t From    = params["From"];
  string   Aggreg  = params["Aggreg"];
  uint32_t Series  = AnalyzerSoftStorage::SERIE_TIMESTAMP;
  for(auto Serie : params["Series"]) {
    if(Serie.is_string() == false) {
      return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    } else if((Serie == "FREQUENCY") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_FREQUENCY;
    } else if((Serie == "UNBALANCE") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_UNBALANCE;
    } else if((Serie == "FLAG") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_FLAG;
    } else if((Serie == "VOLTAGE") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE;
    } else if((Serie == "CURRENT") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT;
    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_ACTIVE_POWER;
    } else if((Serie == "ACTIVE_ENERGY") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_ACTIVE_ENERGY;
    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_REACTIVE_POWER;
    } else if((Serie == "REACTIVE_ENERGY") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_REACTIVE_ENERGY;
    } else if((Serie == "APPARENT_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_APPARENT_POWER;
    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_POWER_FACTOR;
    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_THD;
    } else if((Serie == "CURRENT_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT_THD;
    } else if((Serie == "PHI") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_PHI;
    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_HARMONICS;
    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT_HARMONICS;
    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
      Series |= AnalyzerSoftStorage::SERIE_POWER_HARMONICS;
    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_STAT;
    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_FREQUENCY_STAT;
    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_THD_STAT;
    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_HARMONIC_STAT;
    } else if((Serie == "UNBALANCE_STATS") && (Aggreg == "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_UNBALANCE_STAT;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, To, From, Aggreg, Series](uint64_t offset, DataSink &sink) -> bool {
    string Cols    = "sampletime";
    json   Headers = {"Sampletime"};
    if(Series & SERIE_FREQUENCY)         { Headers.push_back("Frequency");         Cols.append(", freq"); }
    if(Series & SERIE_UNBALANCE)         { Headers.push_back("Unbalance");         Cols.append(", unbalance"); }
    if(Series & SERIE_FLAG)              { Headers.push_back("Flag");              Cols.append(", flag"); }
    if(Series & SERIE_VOLTAGE)           { Headers.push_back("Voltage");           Cols.append(", rms_v"); }
    if(Series & SERIE_CURRENT)           { Headers.push_back("Current");           Cols.append(", rms_i"); }
    if(Series & SERIE_ACTIVE_POWER)      { Headers.push_back("Active_Power");      Cols.append(", active_power"); }
    if(Series & SERIE_ACTIVE_ENERGY)     { Headers.push_back("Active_Energy");     Cols.append((Aggreg == "1H") ? ", active_power" : ", ARRAY(SELECT UNNEST(active_power) / 4.0)"); }
    if(Series & SERIE_REACTIVE_POWER)    { Headers.push_back("Reactive_Power");    Cols.append(", reactive_power"); }
    if(Series & SERIE_REACTIVE_ENERGY)   { Headers.push_back("Reactive_Energy");   Cols.append((Aggreg == "1H") ? ", reactive_power" : ", ARRAY(SELECT UNNEST(reactive_power) / 4.0)"); };
    if(Series & SERIE_APPARENT_POWER)    { Headers.push_back("Apparent_Power");    Cols.append(", ARRAY(SELECT v * i FROM UNNEST(rms_v, rms_i) as t(v, i) LIMIT array_length(rms_v, 1))"); }
    if(Series & SERIE_POWER_FACTOR)      { Headers.push_back("Power_Factor");      Cols.append(", power_factor"); }
    if(Series & SERIE_VOLTAGE_THD)       { Headers.push_back("Voltage_THD");       Cols.append(", thd_v"); }
    if(Series & SERIE_CURRENT_THD)       { Headers.push_back("Current_THD");       Cols.append(", thd_i"); }
    if(Series & SERIE_PHI)               { Headers.push_back("Phi");               Cols.append(", phi"); }
    if(Series & SERIE_VOLTAGE_HARMONICS) { Headers.push_back("Voltage_Harmonics"); Cols.append(", harmonics_v"); }
    if(Series & SERIE_CURRENT_HARMONICS) { Headers.push_back("Current_Harmonics"); Cols.append(", harmonics_i"); }
    if(Series & SERIE_POWER_HARMONICS)   { Headers.push_back("Power_Harmonics");   Cols.append(", harmonics_p"); }

//    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_STAT;
//    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_FREQUENCY_STAT;
//    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_THD_STAT;
//    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_HARMONIC_STAT;
//    } else if((Serie == "UNBALANCE_STATS") && (Aggreg == "1H")) {
//      Series |= AnalyzerSoftStorage::SERIE_UNBALANCE_STAT;
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    if(Aggreg == "1M") {
      DB.ExecSentenceAsyncResult("SELECT json_build_array(" + Cols + ") AS result FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + " ORDER BY sampletime ASC");
    } else {
      DB.ExecSentenceAsyncResult("SELECT json_build_array(" + Cols +") AS result FROM " + "aggreg_" + Aggreg + " WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + " ORDER BY sampletime ASC");
    }
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " << DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return false;
  };
  response.set_chunked_content_provider("application/json", Func);




//  if(Aggreg == "3S") PeriodSeconds = 3000;
//  if(Aggreg == "1M") PeriodSeconds = 60000;
//  if(Aggreg == "10M") PeriodSeconds = 600000;
//  if(Aggreg == "15M") PeriodSeconds = 900000;
//  if(Aggreg == "1H") PeriodSeconds = 3600000;
//  if(((To - From) / PeriodSeconds) > 100000) return HTTPServer::FillResponse(response, "Returned period to large for this aggregation level", "text/plain", HTTP_BAD_REQUEST);
//  Database DB(pSerial);
//  string Query = "SELECT ROW_TO_JSON(Result) AS result FROM ( SELECT ";
//  float Default[200];
//  for(uint8_t n = 0; n < 200; n++) Default[n] = 0.0;
//  for(uint i = 0; i < params["Series"].size(); i++) {
//    if(!params["Series"][i].is_string()) return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
//    string Serie = params["Series"][i];
//    if((Serie == "FREQUENCY") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
//      Query += "COALESCE(ARRAY_AGG(freq ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency") + ", ";
//    } else if((Serie == "UNBALANCE") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
//      Query += "COALESCE(ARRAY_AGG(unbalance ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Unbalance") + ", ";
//    } else if((Serie == "FLAG") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
//      Query += "COALESCE(ARRAY_AGG(flag ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Flag") + ", ";
//    } else if((Serie == "VOLTAGE") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
//      Query += "COALESCE(ARRAY_AGG(rms_v ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage") + ", ";
//    } else if((Serie == "CURRENT") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
//      Query += "COALESCE(ARRAY_AGG(rms_i ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current") + ", ";
//    } else if((Serie == "ACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
//      if(Aggreg == "1M") {
//        Query += "COALESCE(ARRAY_AGG(COALESCE(active_power, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
//      } else {
//        Query += "COALESCE(ARRAY_AGG(active_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Active_Power") + ", ";
//      }
//    } else if((Serie == "REACTIVE_POWER") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
//      if(Aggreg == "1M") {
//        Query += "COALESCE(ARRAY_AGG(COALESCE(reactive_power, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
//      } else {
//        Query += "COALESCE(ARRAY_AGG(reactive_power ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Reactive_Power") + ", ";
//      }
//    } else if((Serie == "POWER_FACTOR") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(power_factor, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Factor") + ", ";
//    } else if((Serie == "VOLTAGE_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_v, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD") + ", ";
//    } else if((Serie == "CURRENT_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(thd_i, " + DB.Bind(Default, pCurrentChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_THD") + ", ";
//    } else if((Serie == "PHI") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(phi, " + DB.Bind(Default, pVoltageChannels) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Phi") + ", ";
//    } else if((Serie == "VOLTAGE_HARMONICS") && (Aggreg == "1M")) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_v, " + DB.Bind(Default, pVoltageChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics") + ", ";
//    } else if((Serie == "CURRENT_HARMONICS") && (Aggreg ==  "1M")) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_i, " + DB.Bind(Default, pCurrentChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Current_Harmonics") + ", ";
//    } else if((Serie == "POWER_HARMONICS") && (Aggreg == "1M")) {
//      Query += "COALESCE(ARRAY_AGG(COALESCE(harmonics_p, " + DB.Bind(Default, pVoltageChannels, 50) + ") ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Power_Harmonics") + ", ";
//    } else if((Serie == "VOLTAGE_STATS") && (Aggreg ==  "1H")) {
//      Query += "COALESCE(ARRAY_AGG(stat_voltage ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Stats") + ", ";
//    } else if((Serie == "FREQUENCY_STATS") && (Aggreg ==  "1H")) {
//      Query += "COALESCE(ARRAY_AGG(stat_frequency ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Frequency_Stats") + ", ";
//    } else if((Serie == "VOLTAGE_THD_STATS") && (Aggreg ==  "1H")) {
//      Query += "COALESCE(ARRAY_AGG(stat_thd ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_THD_Stats") + ", ";
//    } else if((Serie == "VOLTAGE_HARMONICS_STATS") && (Aggreg ==  "1H")) {
//      Query += "COALESCE(ARRAY_AGG(stat_harmonics ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Voltage_Harmonics_Stats") + ", ";
//    } else if((Serie == "UNBALANCE_STATS") && (Aggreg == "1H")) {
//      Query += "COALESCE(ARRAY_AGG(stat_unbalance ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Unbalance_Stats") + ", ";
//    } else {
//      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
//    }
//  }
//  Query += "COALESCE(ARRAY_AGG(sampletime ORDER BY sampletime ASC), '{}') AS " + DB.BindID("Sampletime") + " FROM ";
//  if(params["Aggreg"] == "1M") {
//    Query += "(SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS Tmp";
//  } else {
//    Query += "aggreg_" + Aggreg;
//  }
//  Query += " WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To) + ") AS result";
//  DB.ExecSentenceAsyncResult(Query);
//  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
//  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerSoftStorage::GetSeriesRange(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT ranges FROM (SELECT json_build_object('To', max, 'From', min) AS ranges FROM (SELECT max(sampletime) AS max, min(sampletime) AS min FROM aggreg_" + Aggreg + ") AS t1) AS t2 ");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerSoftStorage::DelSeriesRange(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Aggreg = params["Aggreg"];
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime >= " + DB.Bind(From) + " AND sampletime <= " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------