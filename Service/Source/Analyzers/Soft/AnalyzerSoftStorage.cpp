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
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftStorage::AnalyzerSoftStorage(const string &serial) {
  pSerial = serial;
}
AnalyzerSoftStorage::~AnalyzerSoftStorage() {
}
void AnalyzerSoftStorage::GetStatus(const json &params, Response &response) const {
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json Result = json::object();
  for(auto &Serie : params["Series"]) {
    if(Serie.is_string() == false) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    uint64_t To = Tools::GetTime64();
    if(Serie == "SYSTEM") {
      Result["System"] = LogFile::ToJSON();
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
      DB.ExecSentenceAsyncResult("SELECT alarmsread, (SELECT count(*) FROM alarms WHERE trigger > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
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
    } else if(Serie == "Storage") {
      Database DB;
      DB.ExecSentenceAsyncResult("WITH sizes AS (SELECT tablename, pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename)) AS size FROM pg_tables WHERE schemaname = " + DB.Bind(pSerial) + ") SELECT json_build_object("
                                 "'Aggreg_200MS', (SELECT size FROM sizes WHERE tablename = 'aggreg_200ms'),"
                                 "'Aggreg_3S', (SELECT size FROM sizes WHERE tablename = 'aggreg_3s'),"
                                 "'Aggreg_1M', (SELECT SUM(size) FROM sizes WHERE tablename IN ('aggreg_1m', 'aggreg_ext_1m')),"
                                 "'Aggreg_10M', (SELECT size FROM sizes WHERE tablename = 'aggreg_10m'),"
                                 "'Aggreg_15M', (SELECT size FROM sizes WHERE tablename = 'aggreg_15m'),"
                                 "'Aggreg_1H', (SELECT size FROM sizes WHERE tablename = 'aggreg_1h'),"
                                 "'Events', (SELECT SUM(size) FROM sizes WHERE tablename IN ('events', 'events_ext')),"
                                 "'Alarms', (SELECT size FROM sizes WHERE tablename = 'alarms')) as tables");
      if(DB.FecthRow() == true) Result["Storage"] = DB.ResultJSON("tables");
    }
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
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
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------