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
#include "AnalyzerEvents.h"
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerEvents::AnalyzerEvents(const string &id) : pSerial(id) {
}
AnalyzerEvents::~AnalyzerEvents() {
}
void AnalyzerEvents::GetEvents(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false) || ((params["Samples"] != "ALL") && (params["Samples"] != "NONE") && (params["Samples"] != "RAW") && (params["Samples"] != "RMS"))) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Update") == false) || (params["Update"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Update' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("LastChange") == false) || (params["LastChange"].is_number() == false)) return HTTPServer::FillResponse(response, "'LastChange' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To         = params["To"];
  uint64_t From       = params["From"];
  uint64_t LastChange = params["LastChange"];
  string   Samples    = params["Samples"];
  bool     Update     = params["Update"];
  Database DB(pSerial);
  if(Update == true) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET eventsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  auto Func = [&, To, From, Samples, LastChange](uint64_t offset, DataSink &sink) -> bool {
    json Headers = {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes"};
    string QuerySamples;
    if((Samples == "ALL") || (Samples == "RAW")) {
      Headers.push_back("LengthRAW");
      Headers.push_back("SamplesRAW");
      QuerySamples.append(", "
        "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'),"
        " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
        "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RAW_PRESTART') || (SELECT samples FROM ext where sampletype = 'RAW_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RAW_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RAW_POSEND'))");
    }
    if((Samples == "ALL") || (Samples == "RMS")) {
      Headers.push_back("LengthRMS");
      Headers.push_back("SamplesRMS");
      QuerySamples.append(", "
      "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART'), "
      " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
      "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RMS_PRESTART') || (SELECT samples FROM ext where sampletype = 'RMS_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RMS_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RMS_POSEND'))");
    }
    if(Samples == "NONE") {
      Headers.push_back("AvaliableRAW");
      Headers.push_back("AvaliableRMS");
      QuerySamples = ", "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'), "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART')";
    }
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, starttime, evttype, samplerate, reference, peak, change, duration, notes" + QuerySamples + ") AS result FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND changetime > " + DB.Bind(LastChange) + " ORDER BY starttime ASC");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerEvents::GetEventsRange(const json &params, Response &response) const {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(starttime), 'From', min(starttime)) AS ranges FROM events");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerEvents::GetEvent(const json &params, Response &response) const {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false) || ((params["Samples"] != "RAW") && (params["Samples"] != "RMS"))) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  string   Samples   = params["Samples"];
  DB.ExecSentenceAsyncResult("SELECT jsonb_build_object('StartTime', starttime, 'Type', evttype, 'SampleRate', samplerate, 'Reference', reference, 'Peak', peak, 'Change', change, 'Duration', duration, 'Notes', notes, "
                             "'Length', (SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = '" + Samples + "_PRESTART'), "
                             "'Samples', (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime) SELECT (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_PRESTART') || (SELECT samples FROM ext where sampletype = '" + Samples + "_POSSTART') || (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_PREEND') || (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_POSEND')) "
                             ") AS result FROM events WHERE starttime = " + DB.Bind(StartTime));
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void AnalyzerEvents::DelEventsRange(const json &params, Response &response) const {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  bool     Result = false;
  if((params.contains("Samples") == true) && (params["Samples"].is_string() == true) && ((params["Samples"] == "ALL") || (params["Samples"] == "RAW") || (params["Samples"] != "RMS"))) {
    if(params["Samples"] == "ALL") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
    } else if(params["Samples"] == "RAW") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')");
    } else if(params["Samples"] == "RMS") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')");
    }
  } else {
    Result = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
  }
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerEvents::SetEvent(const json &params, Response &response) const {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Notes") == false) || (params["Notes"].is_string() == false)) return HTTPServer::FillResponse(response, "'Notes' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  string   Notes     = params["Notes"];
  bool     Result    = DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(StartTime));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerEvents::DelEvent(const json &params, Response &response) const {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  bool     Result    = false;
  if((params.contains("Samples") == true) && (params["Samples"].is_string() == true) && ((params["Samples"] == "ALL") || (params["Samples"] == "RAW") || (params["Samples"] != "RMS"))) {
    if(params["Samples"] == "ALL") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime));
    } else if(params["Samples"] == "RAW") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')");
    } else if(params["Samples"] == "RMS") {
      Result = DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')");
    }
  } else {
    Result = DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(StartTime));
  }
  HTTPServer::FillResponse(response, "", "text/plain", (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerEvents::SetEvents(const json &params, Response &response) const {

}
// ---------------------------------------------------------------------------------------------------------------------