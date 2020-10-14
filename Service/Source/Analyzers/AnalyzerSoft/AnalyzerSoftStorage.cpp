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
    uint64_t To = Tools::GetTime64();
    if(Serie == "SYSTEM") {
      Result["System"] = Status::GetJSON();
    } else if(Serie == "UNREAD_EVENTS") {
      string UserName = params["UserName"];
      Result["Unread_Events"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE starttime > eventsread) AS events  FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
      Result["Unread_Events"]["To"] = To;
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
      Result["Unread_Alarms"]["To"]    = To;
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
        "SELECT 'Aggreg_200MS' AS name, size FROM sizes WHERE tablename = 'aggreg_200ms' UNION "
        "SELECT 'Aggreg_3S' AS name, size FROM sizes WHERE tablename = 'aggreg_3s' UNION "
        "SELECT 'Aggreg_1M' AS name, SUM(size) FROM sizes WHERE tablename IN ('aggreg_1m', 'aggreg_ext_1m') UNION "
        "SELECT 'Aggreg_10M' AS name, size FROM sizes WHERE tablename = 'aggreg_10m' UNION "
        "SELECT 'Aggreg_15M' AS name, size FROM sizes WHERE tablename = 'aggreg_15m' UNION "
        "SELECT 'Aggreg_1H' AS name, size FROM sizes WHERE tablename = 'aggreg_1h' UNION "
        "SELECT 'Events' AS name, SUM(size) FROM sizes WHERE tablename IN ('events', 'events_ext') UNION "
        "SELECT 'Alarms' AS name, size FROM sizes WHERE tablename = 'alarms_ext' UNION "
        "SELECT 'Settings' AS name, SUM(size) FROM sizes WHERE tablename IN ('alarms', 'rates')");
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
    } else if((Serie == "Frequency") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_FREQUENCY;
    } else if((Serie == "Unbalance") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_UNBALANCE;
    } else if((Serie == "Flag") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_FLAG;
    } else if((Serie == "Voltage") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE;
    } else if((Serie == "Current") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT;
    } else if((Serie == "Active_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_ACTIVE_POWER;
    } else if((Serie == "Active_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_ACTIVE_ENERGY;
    } else if((Serie == "Reactive_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_REACTIVE_POWER;
    } else if((Serie == "Reactive_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_REACTIVE_ENERGY;
    } else if((Serie == "Apparent_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Series |= AnalyzerSoftStorage::SERIE_APPARENT_POWER;
    } else if((Serie == "Power_Factor") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_POWER_FACTOR;
    } else if((Serie == "Voltage_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_THD;
    } else if((Serie == "Current_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT_THD;
    } else if((Serie == "Phi") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_PHI;
    } else if((Serie == "Voltage_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_HARMONICS;
    } else if((Serie == "Voltage_Harmonics_Complex") && (Aggreg == "200MS")) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_HARMONICS_COMPLEX;
    } else if((Serie == "Current_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT_HARMONICS;
    } else if((Serie == "Current_Harmonics_Complex") && (Aggreg == "200MS")) {
      Series |= AnalyzerSoftStorage::SERIE_CURRENT_HARMONICS_COMPLEX;
    } else if((Serie == "Power_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Series |= AnalyzerSoftStorage::SERIE_POWER_HARMONICS;
    } else if((Serie == "Voltage_Stats") && (Aggreg ==  "1H")) {
      Series |= AnalyzerSoftStorage::SERIE_VOLTAGE_STAT;
    } else if((Serie == "Frequency_Stats") && (Aggreg ==  "1H")) {
      Series |= AnalyzerSoftStorage::SERIE_FREQUENCY_STAT;
    } else if((Serie == "Voltage_THD_Stats") && (Aggreg ==  "1H")) {
      Series |= AnalyzerSoftStorage::SERIE_THD_STAT;
    } else if((Serie == "Voltage_Harmonics_Stats") && (Aggreg ==  "1H")) {
      Series |= AnalyzerSoftStorage::SERIE_HARMONIC_STAT;
    } else if((Serie == "Unbalance_Stats") && (Aggreg == "1H")) {
      Series |= AnalyzerSoftStorage::SERIE_UNBALANCE_STAT;
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, To, From, Aggreg, Series](uint64_t offset, DataSink &sink) -> bool {
    string   Cols    = "sampletime";
    json     Headers = {"Time"};
    uint64_t Start   = From;
    if(Series & SERIE_FREQUENCY)                 { Headers.push_back("Frequency");         Cols.append(", freq"); }
    if(Series & SERIE_UNBALANCE)                 { Headers.push_back("Unbalance");         Cols.append(", unbalance"); }
    if(Series & SERIE_FLAG)                      { Headers.push_back("Flag");              Cols.append(", flag"); }
    if(Series & SERIE_VOLTAGE)                   { Headers.push_back("Voltage");           Cols.append(", rms_v"); }
    if(Series & SERIE_CURRENT)                   { Headers.push_back("Current");           Cols.append(", rms_i"); }
    if(Series & SERIE_ACTIVE_POWER)              { Headers.push_back("Active_Power");      Cols.append(", active_power"); }
    if(Series & SERIE_ACTIVE_ENERGY)             { Headers.push_back("Active_Energy");     Cols.append((Aggreg == "1H") ? ", active_power" : ", ARRAY(SELECT UNNEST(active_power) / 4.0)"); }
    if(Series & SERIE_REACTIVE_POWER)            { Headers.push_back("Reactive_Power");    Cols.append(", reactive_power"); }
    if(Series & SERIE_REACTIVE_ENERGY)           { Headers.push_back("Reactive_Energy");   Cols.append((Aggreg == "1H") ? ", reactive_power" : ", ARRAY(SELECT UNNEST(reactive_power) / 4.0)"); };
    if(Series & SERIE_APPARENT_POWER)            { Headers.push_back("Apparent_Power");    Cols.append(", ARRAY(SELECT v * i FROM UNNEST(rms_v, rms_i) as t(v, i) LIMIT array_length(rms_v, 1))"); }
    if(Series & SERIE_POWER_FACTOR)              { Headers.push_back("Power_Factor");      Cols.append(", power_factor"); }
    if(Series & SERIE_VOLTAGE_THD)               { Headers.push_back("Voltage_THD");       Cols.append(", thd_v"); }
    if(Series & SERIE_CURRENT_THD)               { Headers.push_back("Current_THD");       Cols.append(", thd_i"); }
    if(Series & SERIE_PHI)                       { Headers.push_back("Phi");               Cols.append(", phi"); }
    if(Series & SERIE_VOLTAGE_HARMONICS)         { Headers.push_back("Voltage_Harmonics"); Cols.append((Aggreg == "1M") ? ", harmonics_v" : "(select array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)"); }
    if(Series & SERIE_VOLTAGE_HARMONICS_COMPLEX) { Headers.push_back("Voltage_Harmonics_Complex"); Cols.append(", harmonics_v"); }
    if(Series & SERIE_CURRENT_HARMONICS)         { Headers.push_back("Current_Harmonics"); Cols.append((Aggreg == "1M") ? ", harmonics_v" : "(select array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)"); }
    if(Series & SERIE_CURRENT_HARMONICS_COMPLEX) { Headers.push_back("Current_Harmonics_Complex"); Cols.append(", harmonics_i"); }
    if(Series & SERIE_POWER_HARMONICS)           { Headers.push_back("Power_Harmonics");   Cols.append(", harmonics_p"); }
    if(Series & SERIE_VOLTAGE_STAT)              { Headers.push_back("Voltage_Stats");     Cols.append(", stat_voltage"); }
    if(Series & SERIE_FREQUENCY_STAT)            { Headers.push_back("Frequency_Stats");   Cols.append(", stat_frequency"); }
    if(Series & SERIE_THD_STAT)                  { Headers.push_back("THD_Stats");         Cols.append(", stat_thd"); }
    if(Series & SERIE_HARMONIC_STAT)             { Headers.push_back("Harmonics_Stats");   Cols.append(", stat_harmonics"); }
    if(Series & SERIE_UNBALANCE_STAT)            { Headers.push_back("Unbalance_Stats");   Cols.append(", stat_unbalance"); }
    sink.os << "[\n  " + Headers.dump();
    Database DB(pSerial);
    do {
      if(Aggreg == "1M") {
        DB.ExecSentenceAsyncResult("SELECT sampletime, json_build_array(" + Cols + ") AS result FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m WHERE sampletime >= " + DB.Bind(Start) + " ORDER BY sampletime ASC LIMIT 1000");
      } else {
        DB.ExecSentenceAsyncResult("SELECT sampletime, json_build_array(" + Cols + ") AS result FROM " + "aggreg_" + Aggreg + " WHERE sampletime >= " + DB.Bind(Start) + " ORDER BY sampletime ASC LIMIT 1000");
      }
      while(DB.FecthRow() && sink.is_writable()) {
        Start = DB.ResultUInt64("sampletime") + 1;
        if(Start > To) break;
        sink.os << ",\n  " + DB.ResultString("result");
      }
    } while((DB.RowIndex() != 0) && (Start < To));
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
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