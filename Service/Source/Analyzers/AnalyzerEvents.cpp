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
AnalyzerEvents::AnalyzerEvents(const string &id, const Analyzer::AnalyzerMode mode) : pSerial(id), pVoltageChannels((mode == Analyzer::PHASE1) ? 1 : 3) {
}
AnalyzerEvents::~AnalyzerEvents() {
}
void AnalyzerEvents::GetEvents(HTTPRequest &req) const {
  if(!req.CheckParamNumber("To") || !req.CheckParamNumber("From") || !req.CheckParamNumber("ChangeTime") || !req.CheckParamBool("Update") || !req.CheckParamEnum("Samples", {"All", "None", "RAW", "RMS"})) return;
  Database DB(pSerial);
  uint64_t To         = req.Param("To");
  uint64_t From       = req.Param("From");
  uint64_t ChangeTime = req.Param("ChangeTime");
  string   Samples    = req.Param("Samples");
  bool     Update     = req.Param("Update");
  if(Update == true) DB.ExecSentenceNoResult("UPDATE permissions SET eventsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(req.UserName()) + " AND serial = " + DB.Bind(pSerial));
  auto Func = [&, To, From, Samples, ChangeTime](function<bool(const json &content)> Write) {
    json Headers = {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes"};
    string QuerySamples;
    if((Samples == "All") || (Samples == "RAW")) {
      Headers.push_back("LengthRAW");
      Headers.push_back("SamplesRAW");
      QuerySamples.append(", size_raw, samples_raw");
    }
    if((Samples == "All") || (Samples == "RMS")) {
      Headers.push_back("LengthRMS");
      Headers.push_back("SamplesRMS");
      QuerySamples.append(", size_rms, samples_rms");
    }
    if(Samples == "None") {
      Headers.push_back("AvaliableRAW");
      Headers.push_back("AvaliableRMS");
      QuerySamples.append(", samples_raw IS NOT NULL, samples_rms IS NOT NULL");
    }
    Write(Headers);
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, starttime, evttype, samplerate, reference, peak, change, duration, notes" + QuerySamples + ") FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND changetime > " + DB.Bind(ChangeTime) + " ORDER BY starttime ASC");
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
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('StartTime', starttime, 'Type', evttype, 'SampleRate', samplerate, 'Reference', reference, 'Peak', peak, 'Change', change, 'Duration', duration, 'Notes', notes, "
                                 "'Length', " + string((Samples == "RMS") ? "size_rms" : "size_raw") + ", 'Samples', " + string((Samples == "RMS") ? "samples_rms" : "samples_raw") +
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
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_raw = NULL, samples_rms = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RAW") {
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_raw = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RMS") {
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_rms = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
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
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_raw = NULL, samples_rms = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime = " + DB.Bind(StartTime)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RAW") {
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_raw = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  } else if(req.Param("Mode") == "RMS") {
    if(DB.ExecSentenceNoResult("UPDATE events SET samples_rms = NULL, changetime = " + DB.Bind(Tools::GetTime64()) + " WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  }
  req.Success(json::object());
}
void AnalyzerEvents::SetEvents(HTTPRequest &req) const {
  if(!req.CheckParamSet("Headers", {"ChangeTime", "StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes", "LengthRAW", "SamplesRAW", "LengthRMS", "SamplesRMS"}, true) || !req.CheckParamArray("Alarms")) return;
  vector<string> Series  = req.Param("Headers");
  Database DB(pSerial);
  for(auto &Row : req.Param("Alarms")) {
    if((Row.is_array() == false) || (Row.size() != Series.size())) return req.Error("Invalid 'Samples' param", HTTP_BAD_REQUEST);
    string   ChangeTime, StartTime, Type, SampleRate, Reference, Peak, Change, Duration, Notes, LengthRAW, LengthRMS, SamplesRAW, SamplesRMS;
    auto     RowPtr = Row.begin();
    for(auto &Col : Series) {
      if(Col == "ChangeTime")      ChangeTime  = DB.BindUint(*RowPtr);
      else if(Col == "StartTime")  StartTime  = DB.BindUint(*RowPtr);
      else if(Col == "Type")       Type       = DB.BindEnum(*RowPtr, {"RVC", "DIP", "SWELL", "INTERRUPTION"});
      else if(Col == "SampleRate") SampleRate = DB.BindFloat(*RowPtr);
      else if(Col == "Reference")  Reference  = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Peak")       Peak       = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Change")     Change     = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Duration")   Duration   = DB.BindUint(*RowPtr);
      else if(Col == "Notes")      Notes      = DB.BindString(*RowPtr);
      else if(Col == "LengthRAW")  LengthRAW  = DB.BindUint(*RowPtr);
      else if(Col == "SamplesRAW") SamplesRAW = DB.BindArrayArrayFloat(*RowPtr, 0, pVoltageChannels);
      else if(Col == "LengthRMS")  LengthRMS  = DB.BindUint(*RowPtr);
      else if(Col == "SamplesRMS") SamplesRMS = DB.BindArrayArrayFloat(*RowPtr, 0, pVoltageChannels);
      RowPtr++;
    }
    if(SamplesRAW.empty()) SamplesRAW = "NULL";
    if(SamplesRMS.empty()) SamplesRMS = "NULL";
    if(!ChangeTime.empty() && !StartTime.empty() && !Type.empty() && !SampleRate.empty() && !Reference.empty() && !Peak.empty() && !Change.empty() && !Duration.empty() && !Notes.empty() && !LengthRAW.empty() && !LengthRMS.empty() && !SamplesRAW.empty() && !SamplesRMS.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO events(changetime, starttime, evttype, samplerate, reference, peak, change, notes, duration, size_raw, size_rms, samples_raw, samples_rms) "
                                 "VALUES (" + ChangeTime + ", " + StartTime + ", " + Type + ", " + SampleRate + ", " + Reference + ", " + Peak + ", " + Change + ", " + Notes + ", " + Duration + ", " + LengthRAW + ", " + SamplesRAW + ", " + LengthRMS + ", " + SamplesRMS + ") "
                                 "ON CONFLICT(starttime) DO UPDATE SET changetime = EXCLUDED.changetime, starttime = EXCLUDED.starttime, evttype = EXCLUDED.evttype, samplerate = EXCLUDED.samplerate, reference = EXCLUDED.reference, peak = EXCLUDED.peak, change = EXCLUDED.change, notes = EXCLUDED.notes, duration = EXCLUDED.duration, size_raw = EXCLUDED.size_raw, size_rms = EXCLUDED.size_rms, samples_raw = EXCLUDED.samples_raw, samples_rms = EXCLUDED.samples_rms") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else {
      return req.Error("Invalid 'Events' param", HTTP_BAD_REQUEST);
    }
  }
  req.Success(json::object());
}
// ---------------------------------------------------------------------------------------------------------------------