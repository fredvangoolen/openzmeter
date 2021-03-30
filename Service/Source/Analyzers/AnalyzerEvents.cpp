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
void AnalyzerEvents::GetEvents(HTTPRequest &req) const {
  if(!req.CheckParamNumber("To") || !req.CheckParamNumber("From") || !req.CheckParamNumber("LastChange") || !req.CheckParamBool("Update") || !req.CheckParamEnum("Samples", {"All", "None", "RAW", "RMS"})) return;
  Database DB(pSerial);
  uint64_t To         = req.Param("To");
  uint64_t From       = req.Param("From");
  uint64_t LastChange = req.Param("LastChange");
  string   Samples    = req.Param("Samples");
  bool     Update     = req.Param("Update");
  if(Update == true) DB.ExecSentenceNoResult("UPDATE permissions SET eventsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(req.UserName()) + " AND serial = " + DB.Bind(pSerial));
  auto Func = [&, To, From, Samples, LastChange](function<bool(const json &content)> Write) {
    json Headers = {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes"};
    string QuerySamples;
    if((Samples == "All") || (Samples == "RAW")) {
      Headers.push_back("LengthRAW");
      Headers.push_back("SamplesRAW");
      QuerySamples.append(", "
        "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'),"
        " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
        "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RAW_PRESTART') || (SELECT samples FROM ext where sampletype = 'RAW_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RAW_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RAW_POSEND'))");
    }
    if((Samples == "All") || (Samples == "RMS")) {
      Headers.push_back("LengthRMS");
      Headers.push_back("SamplesRMS");
      QuerySamples.append(", "
      "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART'), "
      " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
      "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RMS_PRESTART') || (SELECT samples FROM ext where sampletype = 'RMS_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RMS_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RMS_POSEND'))");
    }
    if(Samples == "None") {
      Headers.push_back("AvaliableRAW");
      Headers.push_back("AvaliableRMS");
      QuerySamples = ", "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'), "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART')";
    }
    Write(Headers);
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, starttime, evttype, samplerate, reference, peak, change, duration, notes" + QuerySamples + ") FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND changetime > " + DB.Bind(LastChange) + " ORDER BY starttime ASC");
    while(DB.FecthRow()) {
      if(Write(DB.ResultJSON(0)) == false) return;
    }
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerEvents::GetEventsRange(HTTPRequest &req) const {
  Database DB(pSerial);
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(starttime), 'From', min(starttime)) FROM events") == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void AnalyzerEvents::GetEvent(HTTPRequest &req) const {
  if(!req.CheckParamNumber("StartTime") || !req.CheckParamEnum("Samples", {"RAW", "RMS"})) return;
  Database DB(pSerial);
  uint64_t StartTime = req.Param("StartTime");
  string   Samples   = req.Param("Samples");
  if((DB.ExecSentenceAsyncResult("SELECT jsonb_build_object('StartTime', starttime, 'Type', evttype, 'SampleRate', samplerate, 'Reference', reference, 'Peak', peak, 'Change', change, 'Duration', duration, 'Notes', notes, "
                                 "'Length', (SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = '" + Samples + "_PRESTART'), "
                                 "'Samples', (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime) SELECT (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_PRESTART') || (SELECT samples FROM ext where sampletype = '" + Samples + "_POSSTART') || (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_PREEND') || (SELECT samples FROM ext WHERE sampletype = '" + Samples + "_POSEND')) "
                                 ") FROM events WHERE starttime = " + DB.Bind(StartTime)) == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void AnalyzerEvents::DelEventsRange(HTTPRequest &req) const {
  if(!req.CheckParamNumber("From") || !req.CheckParamNumber("To") || !req.CheckParamEnum("Mode", {"All", "Samples", "RMS", "RAW"})) return;
  Database DB(pSerial);
  uint64_t From   = req.Param("From");
  uint64_t To     = req.Param("To");
  if(req.Param("Mode") == "All") {
    if(DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "Samples") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RAW") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RMS") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
  req.Success(json::object());
}
void AnalyzerEvents::SetEvent(HTTPRequest &req) const {
  if(!req.CheckParamNumber("StartTime") || !req.CheckParamString("Notes")) return;
  Database DB(pSerial);
  uint64_t StartTime = req.Param("StartTime");
  string   Notes     = req.Param("Notes");
  if(DB.ExecSentenceNoResult("UPDATE events SET changetime = " + DB.Bind(Tools::GetTime64()) + ", Notes = " + DB.Bind(Notes) + " WHERE starttime = " + DB.Bind(StartTime)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void AnalyzerEvents::DelEvent(HTTPRequest &req) const {
  if(!req.CheckParamNumber("StartTime") || !req.CheckParamEnum("Mode", {"All", "Samples", "RMS", "RAW"})) return;
  Database DB(pSerial);
  uint64_t StartTime = req.Param("StartTime");
  if(req.Param("Mode") == "All") {
    if(DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(StartTime)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "Samples") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RAW") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RMS") {
    if(DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
  req.Success(json::object());
}
void AnalyzerEvents::SetEvents(HTTPRequest &req) const {
  //TODO
}
// ---------------------------------------------------------------------------------------------------------------------