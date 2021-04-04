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
void AnalyzerAlarms::GetAlarms(HTTPRequest &req) const {
  if(!req.CheckParamNumber("To") || !req.CheckParamNumber("From") || !req.CheckParamBool("Update") || !req.CheckParamNumber("ChangeTime")) return;
  uint64_t To         = req.Param("To");
  uint64_t From       = req.Param("From");
  uint64_t ChangeTime = req.Param("ChangeTime");
  bool     Update     = req.Param("Update");
  Database DB(pSerial);
  if(Update == true) DB.ExecSentenceNoResult("UPDATE permissions SET alarmsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(req.UserName()) + " AND serial = " + DB.Bind(pSerial));
  auto Func = [&, To, From, ChangeTime](function<bool(const json &content)> Write) {
    Write({"ChangeTime", "AlarmID", "Trigger", "Release", "Notes"});
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(changetime, id, trigger, release, notes) FROM alarms WHERE trigger BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND changetime > " + DB.Bind(ChangeTime) +" ORDER BY trigger ASC");
    while(DB.FecthRow()) {
      if(Write(DB.ResultJSON(0)) == false) break;
    }
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerAlarms::GetAlarmsRange(HTTPRequest &req) const {
  Database DB(pSerial);
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(trigger), 'From', min(trigger)) FROM alarms") == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void AnalyzerAlarms::DelAlarm(HTTPRequest &req) const {
  if(!req.CheckParamString("AlarmID") || !req.CheckParamNumber("Trigger")) return;
  string   ID      = req.Param("AlarmID");
  uint64_t Trigger = req.Param("Trigger");
  Database DB(pSerial);
  if(DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id = " + DB.Bind(ID) + " AND trigger = " + DB.Bind(Trigger)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void AnalyzerAlarms::DelAlarmsRange(HTTPRequest &req) const {
  if(!req.CheckParamNumber("From") || !req.CheckParamNumber("To")) return;
  uint64_t From = req.Param("From");
  uint64_t To   = req.Param("To");
  Database DB(pSerial);
  if(DB.ExecSentenceNoResult("DELETE FROM alarms WHERE trigger BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void AnalyzerAlarms::SetAlarm(HTTPRequest &req) const {
  if(!req.CheckParamString("AlarmID") || !req.CheckParamNumber("Trigger") || !req.CheckParamString("Notes")) return;
  Database DB(pSerial);
  string   ID        = req.Param("AlarmID");
  uint64_t Trigger   = req.Param("Trigger");
  string   Notes     = req.Param("Notes");
  if(DB.ExecSentenceNoResult("UPDATE alarms SET changetime = " + DB.Bind(Tools::GetTime64()) + ", Notes = " + DB.Bind(Notes) + " WHERE trigger = " + DB.Bind(Trigger) + " AND id = " + DB.Bind(ID)) == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
void AnalyzerAlarms::SetAlarms(HTTPRequest &req) const {
  if(!req.CheckParamSet("Headers", {"ChangeTime", "AlarmID", "Trigger", "Release", "Notes"}, true) || !req.CheckParamArray("Alarms")) return;
  vector<string> Series  = req.Param("Headers");
  Database DB(pSerial);
  for(auto &Row : req.Param("Alarms")) {
    if((Row.is_array() == false) || (Row.size() != Series.size())) return req.Error("Invalid 'Samples' param", HTTP_BAD_REQUEST);
    string ChangeTime, Trigger, Release, AlarmID, Notes;
    auto   RowPtr = Row.begin();
    for(auto &Col : Series) {
      if(Col == "ChangeTime")      ChangeTime = DB.BindUint(*RowPtr);
      else if(Col == "Trigger")    Trigger    = DB.BindUint(*RowPtr);
      else if(Col == "Release")    Release    = DB.BindUint(*RowPtr);
      else if(Col == "AlarmID")    AlarmID    = DB.BindString(*RowPtr);
      else if(Col == "Notes")      Notes      = DB.BindString(*RowPtr);
      RowPtr++;
    }
    if(Release.empty()) Release = "NULL";
    if(!ChangeTime.empty() && !Trigger.empty() && !Release.empty() && !AlarmID.empty() && !Notes.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO alarms(changetime, trigger, release, id, notes) VALUES(" + ChangeTime + ", " + Trigger + ", " + Release + ", " + AlarmID + ", " + Notes + ") "
                                 "ON CONFLICT(trigger, id) DO UPDATE SET changetime = EXCLUDED.changetime, release = EXCLUDED.release, notes = EXCLUDED.notes") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else {
      return req.Error("Invalid 'Alarms' param", HTTP_BAD_REQUEST);
    }
  }
  req.Success(json::object());
}
// ---------------------------------------------------------------------------------------------------------------------