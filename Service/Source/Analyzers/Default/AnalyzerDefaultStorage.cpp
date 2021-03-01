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
#include "LogFile.h"
#include "AnalyzerDefaultStorage.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerDefaultStorage::AnalyzerDefaultStorage(const string &serial) {
  pSerial = serial;
  LogFile::Debug("Creating tables for analyzer '%s'", pSerial.c_str());
  Database DB(serial, true);
  DB.ExecResource("SQL/Database_Analyzer.sql");
}
AnalyzerDefaultStorage::~AnalyzerDefaultStorage() {
}
void AnalyzerDefaultStorage::CleanOldSamples() const {
  Database DB(pSerial);
  uint64_t Now = Tools::GetTime64();
  if(Now > 86400000) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Now - 86400000));   //Delete 1 day old
  if(Now > 3600000)  DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(Now - 3600000));     //Delete 1 hour old
}
void AnalyzerDefaultStorage::GetSeries(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "200MS") && (params["Aggreg"] != "3S") && (params["Aggreg"] != "1M") && (params["Aggreg"] != "10M") && (params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t       To      = params["To"];
  uint64_t       From    = params["From"];
  string         Aggreg  = params["Aggreg"];
  vector<string> Headers = vector<string>({"Time"});
  string         Cols    = "sampletime";
  for(auto Serie : params["Series"]) {
    if((Serie.is_string() == false) || (find(Headers.begin(), Headers.end(), Serie) != Headers.end())) {
      return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    } else if((Serie == "Frequency") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Frequency");
      Cols.append(", freq");
    } else if((Serie == "Unbalance") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Unbalance");
      Cols.append(", unbalance");
    } else if((Serie == "Flag") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Flag");
      Cols.append(", flag");
    } else if((Serie == "Voltage") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Voltage");
      Cols.append(", rms_v");
    } else if((Serie == "Current") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Current");
      Cols.append(", rms_i");
    } else if((Serie == "Active_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Active_Power");
      Cols.append(", active_power");
    } else if((Serie == "Active_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Active_Energy");
      Cols.append((Aggreg == "1H") ? ", active_power" : ", ARRAY(SELECT UNNEST(active_power) / 4.0)");
    } else if((Serie == "Reactive_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Reactive_Power");
      Cols.append(", reactive_power");
    } else if((Serie == "Reactive_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Reactive_Energy");
      Cols.append((Aggreg == "1H") ? ", reactive_power" : ", ARRAY(SELECT UNNEST(reactive_power) / 4.0)");
    } else if((Serie == "Apparent_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Apparent_Power");
      Cols.append(", ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1))");
    } else if((Serie == "Power_Factor") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Power_Factor");
      Cols.append(", power_factor");
    } else if((Serie == "Voltage_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Voltage_THD");
      Cols.append(", thd_v");
    } else if((Serie == "Current_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
      Headers.push_back("Current_THD");
      Cols.append(", thd_i");
    } else if((Serie == "Phi") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Phi");
      Cols.append(", phi");
    } else if((Serie == "Voltage_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Voltage_Harmonics");
      Cols.append((Aggreg == "1M") ? ", harmonics_v" : ", (SELECT array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    } else if((Serie == "Voltage_Harmonics_Complex") && (Aggreg == "200MS")) {
      Headers.push_back("Voltage_Harmonics_Complex");
      Cols.append(", harmonics_v");
    } else if((Serie == "Current_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Current_Harmonics");
      Cols.append((Aggreg == "1M") ? ", harmonics_i" : ", (SELECT array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    } else if((Serie == "Current_Harmonics_Complex") && (Aggreg == "200MS")) {
      Headers.push_back("Current_Harmonics_Complex");
      Cols.append(", harmonics_i");
    } else if((Serie == "Power_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Power_Harmonics");
      Cols.append(", harmonics_p");
    } else if((Serie == "Voltage_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Voltage_Stats");
      Cols.append(", stat_voltage");
    } else if((Serie == "Frequency_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Frequency_Stats");
      Cols.append(", stat_frequency");
    } else if((Serie == "Voltage_THD_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("THD_Stats");
      Cols.append(", stat_thd");
    } else if((Serie == "Voltage_Harmonics_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Harmonics_Stats");
      Cols.append(", stat_harmonics");
    } else if((Serie == "Unbalance_Stats") && (Aggreg == "1H")) {
      Headers.push_back("Unbalance_Stats");
      Cols.append(", stat_unbalance");
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, To, From, Aggreg, Cols, Headers](uint64_t offset, DataSink &sink) -> bool {
    uint64_t Start = From;
    Database DB(pSerial);
    sink.os << "[\n  " + json(Headers).dump();
    while(sink.is_writable()) {
      if(DB.FecthRow() == false) {
        DB.ExecSentenceAsyncResult("SELECT sampletime, json_build_array(" + Cols + ") FROM " + ((Aggreg == "1M") ? "aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m" : "aggreg_" + Aggreg) + " WHERE sampletime >= " + DB.Bind(Start) + " ORDER BY sampletime ASC LIMIT 100");
        if(DB.FecthRow() == false) break;
      }
      Start = DB.ResultUInt64(0);
      if(Start > To) break;
      sink.os << ",\n  " + DB.ResultString(1);
      Start = Start + 1;
    }
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerDefaultStorage::SetSeries(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "200MS") && (params["Aggreg"] != "3S") && (params["Aggreg"] != "1M") && (params["Aggreg"] != "10M") && (params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);


  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if(params["From"] > params["To"]) return HTTPServer::FillResponse(response, "'From' param must be less than 'To' param", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Series") == false) || (params["Series"].is_array() == false)) return HTTPServer::FillResponse(response, "'Series' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To      = params["To"];
  uint64_t From    = params["From"];
  string   Aggreg  = params["Aggreg"];
  json     Headers = json::array({"Time"});
  string   Cols    = "sampletime";
  for(auto Serie : params["Series"]) {
    if(Serie.is_string() == false) {
      return HTTPServer::FillResponse(response, "'Series' must be array of strings", "text/plain", HTTP_BAD_REQUEST);
    } else if((Serie == "Frequency") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Frequency");
      Cols.append(", freq");
    } else if((Serie == "Unbalance") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Unbalance");
      Cols.append(", unbalance");
    } else if((Serie == "Flag") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Flag");
      Cols.append(", flag");
    } else if((Serie == "Voltage") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Voltage");
      Cols.append(", rms_v");
    } else if((Serie == "Current") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "10M") || (Aggreg == "1H"))) {
      Headers.push_back("Current");
      Cols.append(", rms_i");
    } else if((Serie == "Active_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Active_Power");
      Cols.append(", active_power");
    } else if((Serie == "Active_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Active_Energy");
      Cols.append((Aggreg == "1H") ? ", active_power" : ", ARRAY(SELECT UNNEST(active_power) / 4.0)");
    } else if((Serie == "Reactive_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Reactive_Power");
      Cols.append(", reactive_power");
    } else if((Serie == "Reactive_Energy") && ((Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Reactive_Energy");
      Cols.append((Aggreg == "1H") ? ", reactive_power" : ", ARRAY(SELECT UNNEST(reactive_power) / 4.0)");
    } else if((Serie == "Apparent_Power") && ((Aggreg == "200MS") || (Aggreg == "3S") || (Aggreg == "1M") || (Aggreg == "15M") || (Aggreg == "1H"))) {
      Headers.push_back("Apparent_Power");
      Cols.append(", ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1))");
    } else if((Serie == "Power_Factor") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Power_Factor");
      Cols.append(", power_factor");
    } else if((Serie == "Voltage_THD") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Voltage_THD");
      Cols.append(", thd_v");
    } else if((Serie == "Current_THD") && (((Aggreg == "200MS") || Aggreg == "1M"))) {
      Headers.push_back("Current_THD");
      Cols.append(", thd_i");
    } else if((Serie == "Phi") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Phi");
      Cols.append(", phi");
    } else if((Serie == "Voltage_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Voltage_Harmonics");
      Cols.append((Aggreg == "1M") ? ", harmonics_v" : ", (SELECT array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    } else if((Serie == "Voltage_Harmonics_Complex") && (Aggreg == "200MS")) {
      Headers.push_back("Voltage_Harmonics_Complex");
      Cols.append(", harmonics_v");
    } else if((Serie == "Current_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Current_Harmonics");
      Cols.append((Aggreg == "1M") ? ", harmonics_i" : ", (SELECT array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    } else if((Serie == "Current_Harmonics_Complex") && (Aggreg == "200MS")) {
      Headers.push_back("Current_Harmonics_Complex");
      Cols.append(", harmonics_i");
    } else if((Serie == "Power_Harmonics") && ((Aggreg == "200MS") || (Aggreg == "1M"))) {
      Headers.push_back("Power_Harmonics");
      Cols.append(", harmonics_p");
    } else if((Serie == "Voltage_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Voltage_Stats");
      Cols.append(", stat_voltage");
    } else if((Serie == "Frequency_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Frequency_Stats");
      Cols.append(", stat_frequency");
    } else if((Serie == "Voltage_THD_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("THD_Stats");
      Cols.append(", stat_thd");
    } else if((Serie == "Voltage_Harmonics_Stats") && (Aggreg ==  "1H")) {
      Headers.push_back("Harmonics_Stats");
      Cols.append(", stat_harmonics");
    } else if((Serie == "Unbalance_Stats") && (Aggreg == "1H")) {
      Headers.push_back("Unbalance_Stats");
      Cols.append(", stat_unbalance");
    } else {
      return HTTPServer::FillResponse(response, "Invalid serie name", "text/plain", HTTP_BAD_REQUEST);
    }
  }
  auto Func = [&, To, From, Aggreg, Cols, Headers](uint64_t offset, DataSink &sink) -> bool {
    uint64_t Start = From;
    Database DB(pSerial);
    sink.os << "[\n  " + Headers.dump();
    while(sink.is_writable()) {
      if(DB.FecthRow() == false) {
        DB.ExecSentenceAsyncResult("SELECT sampletime, json_build_array(" + Cols + ") FROM " + ((Aggreg == "1M") ? "aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m" : "aggreg_" + Aggreg) + " WHERE sampletime >= " + DB.Bind(Start) + " ORDER BY sampletime ASC LIMIT 100");
        if(DB.FecthRow() == false) break;
      }
      Start = DB.ResultUInt64(0);
      if(Start > To) break;
      sink.os << ",\n  " + DB.ResultString(1);
      Start = Start + 1;
    }
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerDefaultStorage::GetSeriesRange(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(sampletime), 'From', min(sampletime)) AS ranges FROM aggreg_" + Aggreg);
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerDefaultStorage::GetEvents(const json &params, Response &response) const {
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Mode") == false) || (params["Mode"].is_string() == false) || ((params["Mode"] != "ALL") && (params["Mode"] != "DESCRIPTOR_ONLY") && (params["Mode"] != "SAMPLES_RAW") && (params["Mode"] != "SAMPLES_RMS"))) return HTTPServer::FillResponse(response, "'Mode' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t To      = params["To"];
  uint64_t From    = params["From"];
  string   Mode    = params["Mode"];
  Database DB(pSerial);
  if((params.contains("Update") == true) && (params["Update"].is_boolean() == true) && (params["Update"] == true)) {
    string UserName = params["UserName"];
    DB.ExecSentenceNoResult("UPDATE permissions SET eventsread = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
  }
  auto Func = [&, To, From, Mode](uint64_t offset, DataSink &sink) -> bool {
    json Headers = {"StartTime", "Type", "SampleRate", "Reference", "Peak", "Change", "Duration", "Notes"};
    string QuerySamples;
    if((Mode == "ALL") || (Mode == "SAMPLES_RAW")) {
      Headers.push_back("LengthRAW");
      Headers.push_back("SamplesRAW");
      QuerySamples.append(", "
        "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'),"
        " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
        "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RAW_PRESTART') || (SELECT samples FROM ext where sampletype = 'RAW_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RAW_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RAW_POSEND'))");
    }
    if((Mode == "ALL") || (Mode == "SAMPLES_RMS")) {
      Headers.push_back("LengthRMS");
      Headers.push_back("SamplesRMS");
      QuerySamples.append(", "
      "(SELECT array_length(samples, 1) FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART'), "
      " (WITH ext AS (SELECT samples, sampletype FROM events_ext WHERE events_ext.starttime = events.starttime)"
      "  SELECT (SELECT samples FROM ext WHERE sampletype = 'RMS_PRESTART') || (SELECT samples FROM ext where sampletype = 'RMS_POSSTART') || (SELECT samples FROM ext WHERE sampletype = 'RMS_PREEND') || (SELECT samples FROM ext WHERE sampletype = 'RMS_POSEND'))");
    }
    if(Mode == "DESCRIPTOR_ONLY") {
      Headers.push_back("AvaliableRAW");
      Headers.push_back("AvaliableRMS");
      QuerySamples = ", "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RAW_PRESTART'), "
        "(SELECT COALESCE(array_length(samples, 1), 0) > 0 FROM events_ext WHERE events_ext.starttime = events.starttime AND sampletype = 'RMS_PRESTART')";
    }
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(starttime, evttype, samplerate, reference, peak, change, duration, notes" + QuerySamples + ") AS result FROM events WHERE starttime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To) + " ORDER BY starttime ASC");
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerDefaultStorage::GetEventsRange(const json &params, Response &response) const {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(starttime), 'From', min(starttime)) AS ranges FROM events");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
void AnalyzerDefaultStorage::GetEvent(const json &params, Response &response) const {
  if((params.contains("StartTime") == false) || (params["StartTime"].is_number() == false)) return HTTPServer::FillResponse(response, "'StartTime' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_string() == false) || ((params["Samples"] != "RMS") && (params["Samples"] != "RAW"))) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
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
void AnalyzerDefaultStorage::GetAlarms(const json &params, Response &response) const {
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
    json Headers = {"AlarmID", "Trigger", "Release"};
    sink.os << "[\n  " << Headers.dump();
    Database DB(pSerial);
    DB.ExecSentenceAsyncResult("SELECT json_build_array(id, trigger, release) as result FROM alarms WHERE trigger >= " + DB.Bind(From) + " AND trigger <= " + DB.Bind(To));
    while(DB.FecthRow() && sink.is_writable()) sink.os << ",\n  " + DB.ResultString("result");
    sink.os << "\n]";
    sink.done();
    return true;
  };
  response.set_chunked_content_provider("application/json", Func);
}
void AnalyzerDefaultStorage::GetAlarmsRange(const json &params, Response &response) const {
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(trigger), 'From', min(trigger)) AS ranges FROM alarms");
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "application/json", HTTP_OK);
}
// ---------------------------------------------------------------------------------------------------------------------