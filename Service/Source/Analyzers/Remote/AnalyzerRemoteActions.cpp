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
#include "AnalyzerRemoteActions.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemoteActions::AnalyzerRemoteActions(const string &serial, const uint16_t max) {
  pSerial = serial;
  pMaxLen = max;
  pMutex  = PTHREAD_MUTEX_INITIALIZER;
}
AnalyzerRemoteActions::~AnalyzerRemoteActions() {
}
bool AnalyzerRemoteActions::PushAction(const string name, const json &params) {
  pthread_mutex_lock(&pMutex);
  if(pPendingActions.size() == pMaxLen) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pPendingActions.push(json::object());
  pPendingActions.back()["Type"]   = name;
  pPendingActions.back()["Params"] = params;
  if(pPendingActions.back()["Params"].contains("UserName")) pPendingActions.back()["Params"].erase("UserName");
  pthread_mutex_unlock(&pMutex);
  return true;
}
void AnalyzerRemoteActions::DelAlarm(const json &params, Response &response) {
  if((params.contains("AlarmID") == false) || (params["AlarmID"].is_string() == false)) return HTTPServer::FillResponse(response, "'AlarmID' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Trigger") == false) || (params["Trigger"].is_number() == false)) return HTTPServer::FillResponse(response, "'Trigger' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   ID      = params["AlarmID"];
  uint64_t Trigger = params["Trigger"];
  if(PushAction("DelAlarm", params) == false) return HTTPServer::FillResponse(response, "Pending request exceded", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  Database DB(pSerial);
  DB.ExecSentenceNoResult("DELETE FROM alarms WHERE id = " + DB.Bind(ID) + " AND trigger = " + DB.Bind(Trigger));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void AnalyzerRemoteActions::DelAlarmsRange(const json &params, Response &response) {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  if(PushAction("DelAlarmsRange", params) == false) return HTTPServer::FillResponse(response, "Pending request exceded", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  Database DB(pSerial);
  DB.ExecSentenceNoResult("DELETE FROM alarms WHERE trigger BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void AnalyzerRemoteActions::DelSeriesRange(const json &params, Response &response) {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Aggreg = params["Aggreg"];
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  if(PushAction("DelSeriesRange", params) == false) return HTTPServer::FillResponse(response, "Pending request exceded", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  Database DB(pSerial);
  DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void AnalyzerRemoteActions::DelEventsRange(const json &params, Response &response) {
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Mode") == false) || (params["Mode"].is_string() == false) || ((params["Mode"] != "ALL") && (params["Mode"] != "SAMPLES_ONLY") && (params["Mode"] != "SAMPLES_RAW") && (params["Mode"] != "SAMPLES_RMS"))) return HTTPServer::FillResponse(response, "'Mode' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t From = params["From"];
  uint64_t To   = params["To"];
  string   Mode = params["Mode"];
  if(PushAction("DelEventsRange", params) == false) return HTTPServer::FillResponse(response, "Pending request exceded", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  if(Mode == "ALL") {
    DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
  } else if(Mode == "SAMPLES_ONLY") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To));
  } else if(Mode == "SAMPLES_RAW") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')");
  } else if(Mode == "SAMPLES_RMS") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')");
  }
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void AnalyzerRemoteActions::DelEvent(const json &params, Response &response) {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Mode") == false) || (params["Mode"].is_string() == false) || ((params["Mode"] != "ALL") && (params["Mode"] != "SAMPLES_ONLY") && (params["Mode"] != "SAMPLES_RAW") && (params["Mode"] != "SAMPLES_RMS"))) return HTTPServer::FillResponse(response, "'Mode' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  uint64_t StartTime = params["StartTime"];
  string   Mode      = params["Mode"];
  if(PushAction("DelEvent", params) == false) return HTTPServer::FillResponse(response, "Pending request exceded", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  if(Mode == "ALL") {
    DB.ExecSentenceNoResult("DELETE FROM events WHERE starttime = " + DB.Bind(StartTime));
  } else if(Mode == "SAMPLES_ONLY") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime));
  } else if(Mode == "SAMPLES_RAW") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RAW_PRESTART', 'RAW_POSSTART', 'RAW_PREEND', 'RAW_POSEND')");
  } else if(Mode == "SAMPLES_RMS") {
    DB.ExecSentenceNoResult("DELETE FROM events_ext WHERE starttime = " + DB.Bind(StartTime) + " AND sampletype IN ('RMS_PRESTART', 'RMS_POSSTART', 'RMS_PREEND', 'RMS_POSEND')");
  }
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
// ---------------------------------------------------------------------------------------------------------------------