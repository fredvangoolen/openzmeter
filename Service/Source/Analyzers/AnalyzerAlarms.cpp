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
#include "AnalyzerAlarms.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerAlarms::AnalyzerAlarms(const string &id) : pSerial(id) {
}
AnalyzerAlarms::~AnalyzerAlarms() {
}
void AnalyzerAlarms::GetAlarms(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To   = params["To"];
  uint64_t From = params["From"];
  Database DB(pSerial);
  if((params.contains("Update") == true) && (params["Update"].is_boolean() == true) && (params["Update"] == true)) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET alarmsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  auto Func = [&, To, From](uint64_t offset, DataSink &sink) -> bool {
    json Headers = {"ChangeTime", "AlarmID", "Trigger", "Release"};
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, id, trigger, release) as result FROM alarms WHERE trigger BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " ORDER BY trigger ASC");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerAlarms::GetAlarmsRange(const json &params, Response &response) const {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(trigger), 'From', min(trigger)) AS ranges FROM alarms");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerAlarms::DelAlarm(const json &params, Response &response) const {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Trigger") == false) || (params["Trigger"].is_number() == false)) return HTTPServer::FillResponse(response, "'Trigger' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   ID       = params["AlarmID"];
  uint64_t Trigger = params["Trigger"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "text/plain", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id = " + DB.Bind(ID) + " AND trigger = " + DB.Bind(Trigger)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerAlarms::DelAlarmsRange(const json &params, Response &response) const {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "application/json", DB.ExecSentenceNoResult("DELETE FROM alarms WHERE trigger BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerAlarms::SetAlarm(const json &params, Response &response) const {

}
void AnalyzerAlarms::SetAlarms(const json &params, Response &response) const {

}
// ---------------------------------------------------------------------------------------------------------------------