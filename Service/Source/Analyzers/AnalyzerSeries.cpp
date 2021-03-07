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
#include "AnalyzerSeries.h"
#include "HTTPServer.h"
#include "Database.h"
#include "Tools.h"
#include "MessageLoop.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSeries::AnalyzerSeries(const string &serial) : pSerial(serial) {
  MessageLoop::RegisterListener("Analyzer_" + pSerial + "_PeriodicTasks", [&]() { this->CleanOldSamples(); }, 60000);
}
AnalyzerSeries::~AnalyzerSeries() {
  MessageLoop::UnregisterListener("Analyzer_" + pSerial + "_PeriodicTasks");
}
void AnalyzerSeries::CleanOldSamples() const {
  Database DB(pSerial);
  uint64_t Now = Tools::GetTime64();
  if(Now > 86400000) DB.ExecSentenceNoResult("DELETE FROM aggreg_ext_1m WHERE sampletime < " + DB.Bind(Now - 86400000));   //Delete 1 day old
  if(Now > 3600000)  DB.ExecSentenceNoResult("DELETE FROM aggreg_200ms WHERE sampletime < " + DB.Bind(Now - 3600000));     //Delete 1 hour old
}
void AnalyzerSeries::GetSeries(const json &params, Response &response) const {
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
      Cols.append(", NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')");
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
void AnalyzerSeries::SetSeries(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false) || ((params["Aggreg"] != "200MS") && (params["Aggreg"] != "3S") && (params["Aggreg"] != "1M") && (params["Aggreg"] != "10M") && (params["Aggreg"] != "15M") && (params["Aggreg"] != "1H"))) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Samples") == false) || (params["Samples"].is_array() == false) || (params["Samples"].size() == 0)) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Headers") == false) || (params["Headers"].is_array() == false)) return HTTPServer::FillResponse(response, "'Headers' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  set<string> Needed;
  string      Aggreg  = params["Aggreg"];
  if(Aggreg == "200MS") {
    Needed = { "Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Power_Harmonics" };
  } else if(Aggreg == "3S") {
    Needed = { "Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power" };
  } else if(Aggreg == "1M") {
    Needed = { "Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics" };
  } else if(Aggreg == "10M") {
    Needed = { "Time", "Frequency", "Flag", "Voltage", "Current" };
  } else if(Aggreg == "15M") {
    Needed = { "Time", "Active_Power", "Reactive_Power" };
  } else if(Aggreg == "1H") {
    Needed = { "Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats" };
  }
  map<string, uint8_t> ColDefs;
  for(uint8_t n = 0; n < params["Headers"].size(); n++) {
    if(params["Headers"][n].is_string() == false) return HTTPServer::FillResponse(response, "'Headers' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    if(Needed.find(params["Headers"][n]) == Needed.end()) continue;
    ColDefs.insert({params["Headers"][n], n});
    Needed.erase(params["Headers"][n]);
  }
  if(Needed.size() > 0) return HTTPServer::FillResponse(response, "'Headers' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  uint64_t From = UINT64_MAX;
  uint64_t To = 0;
  for(auto &Row : params["Samples"]) {
    json Val = Row[ColDefs.find("Time")->second];
    if((Row.is_array() == false) || (Row.size() != params["Headers"].size()) || (Val.is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'Samples' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    if(Val < From) From = Val;
    if(Val > To)   To = Val;
  }
  Database DB(pSerial);
  if(DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  auto FillColumn = [&](const json &row, const string &col, const Database::DataType type) {
    auto Col = ColDefs.find(col);
    return (Col == ColDefs.end()) ? "NULL" : DB.Bind(row[Col->second], type);
  };
  for(auto &Row : params["Samples"]) {
    string Time                      = FillColumn(Row, "Time", Database::UINT);
    string Voltage                   = FillColumn(Row, "Voltage", Database::ARRAY_FLOAT);
    string Current                   = FillColumn(Row, "Current", Database::ARRAY_FLOAT);
    string Frequency                 = FillColumn(Row, "Frequency", Database::FLOAT);
    string Flag                      = FillColumn(Row, "Flag", Database::BOOL);
    string Active_Power              = FillColumn(Row, "Active_Power", Database::ARRAY_FLOAT);
    string Reactive_Power            = FillColumn(Row, "Reactive_Power", Database::ARRAY_FLOAT);
    string Power_Factor              = FillColumn(Row, "Power_Factor", Database::ARRAY_FLOAT);
    string Voltage_THD               = FillColumn(Row, "Voltage_THD", Database::ARRAY_FLOAT);
    string Current_THD               = FillColumn(Row, "Current_THD", Database::ARRAY_FLOAT);
    string Phi                       = FillColumn(Row, "Phi", Database::ARRAY_FLOAT);
    string Unbalance                 = FillColumn(Row, "Unbalance", Database::FLOAT);
    string Voltage_Harmonics_Complex = FillColumn(Row, "Voltage_Harmonics_Complex", Database::ARRAY_ARRAY_ARRAY_FLOAT);
    string Voltage_Harmonics         = FillColumn(Row, "Voltage_Harmonics", Database::ARRAY_ARRAY_FLOAT);
    string Current_Harmonics_Complex = FillColumn(Row, "Current_Harmonics_Complex", Database::ARRAY_ARRAY_ARRAY_FLOAT);
    string Current_Harmonics         = FillColumn(Row, "Current_Harmonics", Database::ARRAY_ARRAY_FLOAT);
    string Power_Harmonics           = FillColumn(Row, "Power_Harmonics", Database::ARRAY_ARRAY_FLOAT);
    string Voltage_Stats             = FillColumn(Row, "Voltage_Stats", Database::ARRAY_UINT);
    string Frequency_Stats           = FillColumn(Row, "Frequency_Stats", Database::ARRAY_UINT);
    string Voltage_THD_Stats         = FillColumn(Row, "Voltage_THD_Stats", Database::ARRAY_UINT);
    string Voltage_Harmonics_Stats   = FillColumn(Row, "Voltage_Harmonics_Stats", Database::ARRAY_UINT);
    string Unbalance_Stats           = FillColumn(Row, "Unbalance_Stats", Database::ARRAY_UINT);
    if(Aggreg == "200MS") {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ", " + Power_Factor + ", " + Voltage_THD + ", " + Current_THD + ", " + Phi + ", " + Unbalance + ", " + Voltage_Harmonics_Complex + ", " + Current_Harmonics_Complex + ", " + Power_Harmonics + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    } else if(Aggreg == "3S") {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    } else if(Aggreg == "1M") {
      if((Active_Power == "NULL") || (Reactive_Power == "NULL") || (Power_Factor == "NULL") || (Voltage_THD == "NULL") || (Current_THD == "NULL") || (Phi == "NULL") || (Unbalance == "NULL") || (Voltage_Harmonics == "NULL") || (Current_Harmonics == "NULL") || (Power_Harmonics == "NULL")) {
        if(DB.ExecSentenceNoResult("INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
      } else {
        if(DB.ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) SELECT sampletime, " + Active_Power + ", " + Reactive_Power + ", " + Power_Factor + ", " + Voltage_THD + ", " + Current_THD + ", " + Phi + ", " + Unbalance + ", " + Voltage_Harmonics + ", " + Current_Harmonics + ", " + Power_Harmonics + " FROM Sample") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
      }
    } else if(Aggreg == "10M") {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    } else if(Aggreg == "15M") {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" + Time + ", " + Active_Power + ", " + Reactive_Power + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    } else if(Aggreg == "1H") {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ", " + Voltage_Stats + ", " + Frequency_Stats + ", " + Voltage_THD_Stats + ", " + Voltage_Harmonics_Stats + ", " + Unbalance_Stats + ")") == false) HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    }
  }
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "text/plain", HTTP_OK);
}
void AnalyzerSeries::GetSeriesRange(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Aggreg = params["Aggreg"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(sampletime), 'From', min(sampletime)) AS ranges FROM aggreg_" + Aggreg);
  if(!DB.FecthRow()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("ranges"), "text/plain", HTTP_OK);
}
void AnalyzerSeries::DelSeriesRange(const json &params, Response &response) const {
  if((params.contains("Aggreg") == false) || (params["Aggreg"].is_string() == false)) return HTTPServer::FillResponse(response, "'Aggreg' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("From") == false) || (params["From"].is_number() == false)) return HTTPServer::FillResponse(response, "'From' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("To") == false) || (params["To"].is_number() == false)) return HTTPServer::FillResponse(response, "'To' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Aggreg = params["Aggreg"];
  uint64_t From   = params["From"];
  uint64_t To     = params["To"];
  if((Aggreg != "200MS") && (Aggreg != "3S") && (Aggreg != "1M") && (Aggreg != "10M") && (Aggreg != "15M") && (Aggreg != "1H")) return HTTPServer::FillResponse(response, "'Aggreg' param must be '200MS', 3S', '1M', '10M', '15M' or '1H'", "text/plain", HTTP_BAD_REQUEST);
  Database DB(pSerial);
  HTTPServer::FillResponse(response, "", "text/plain", DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
// ---------------------------------------------------------------------------------------------------------------------