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
AnalyzerSeries::AnalyzerSeries(const string &serial, const Analyzer::AnalyzerMode mode) : pSerial(serial), pVoltageChannels(Analyzer::VoltageChannels(mode)), pCurrentChannels(Analyzer::CurrentChannels(mode)) {
  MessageLoop::RegisterListener("Analyzer_" + pSerial + "_PeriodicTasks", [&]() { this->CleanOldSamples(); }, 600000);
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
void AnalyzerSeries::GetSeries(HTTPRequest &req) const {
  if(!req.CheckParamEnum("Aggreg", {"200MS", "3S", "1M", "10M", "15M", "1H"}) || !req.CheckParamNumber("From") || !req.CheckParamNumber("To")) return;
  if((req.Param("Aggreg") == "200MS") && !req.CheckParamSet("Series", {"Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Voltage_Harmonics_Complex", "Current_Harmonics", "Current_Harmonics_Complex", "Power_Harmonics"})) return;
  else if((req.Param("Aggreg") == "3S") && !req.CheckParamSet("Series", {"Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power"})) return;
  else if((req.Param("Aggreg") == "1M") && !req.CheckParamSet("Series", {"Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Apparent_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics"})) return;
  else if((req.Param("Aggreg") == "10M") && !req.CheckParamSet("Series", {"Frequency", "Flag", "Voltage", "Current"})) return;
  else if((req.Param("Aggreg") == "15M") && !req.CheckParamSet("Series", {"Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power"})) return;
  else if((req.Param("Aggreg") == "1H") && !req.CheckParamSet("Series", {"Frequency", "Flag", "Voltage", "Current", "Active_Power", "Active_Energy", "Reactive_Power", "Reactive_Energy", "Apparent_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats"})) return;
  uint64_t       To      = req.Param("To");
  uint64_t       From    = req.Param("From");
  string         Aggreg  = req.Param("Aggreg");
  vector<string> Headers = {"Time"};
  string         Cols    = "sampletime";
  for(auto &Serie : req.Param("Series")) Headers.push_back(Serie);
  for(auto Serie : req.Param("Series")) {
    if(Serie == "Frequency")                      Cols.append(", freq");
    else if(Serie == "Unbalance")                 Cols.append(", unbalance");
    else if(Serie == "Flag")                      Cols.append(", flag");
    else if(Serie == "Voltage")                   Cols.append(", rms_v");
    else if(Serie == "Current")                   Cols.append(", rms_i");
    else if(Serie == "Active_Power")              Cols.append(", active_power");
    else if(Serie == "Active_Energy")             Cols.append((Aggreg == "1H") ? ", active_power" : ", ARRAY(SELECT UNNEST(active_power) / 4.0)");
    else if(Serie == "Reactive_Power")            Cols.append(", reactive_power");
    else if(Serie == "Reactive_Energy")           Cols.append((Aggreg == "1H") ? ", reactive_power" : ", ARRAY(SELECT UNNEST(reactive_power) / 4.0)");
    else if(Serie == "Apparent_Power")            Cols.append(", NULLIF(ARRAY(SELECT sqrt(pow(a, 2.0) + pow(r, 2.0)) FROM UNNEST(active_power, reactive_power) as t(a, r) LIMIT array_length(active_power, 1)), '{}')");
    else if(Serie == "Power_Factor")              Cols.append(", power_factor");
    else if(Serie == "Voltage_THD")               Cols.append(", thd_v");
    else if(Serie == "Current_THD")               Cols.append(", thd_i");
    else if(Serie == "Phi")                       Cols.append(", phi");
    else if(Serie == "Voltage_Harmonics")         Cols.append((Aggreg == "1M") ? ", harmonics_v" : ", (SELECT array_agg(harmonics_v) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_v[ch][h][1],2.0) + pow(harmonics_v[ch][h][2], 2.0))) harmonics_v FROM (SELECT generate_series(1, array_length(harmonics_v, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    else if(Serie == "Voltage_Harmonics_Complex") Cols.append(", harmonics_v");
    else if(Serie == "Current_Harmonics")         Cols.append((Aggreg == "1M") ? ", harmonics_i" : ", (SELECT array_agg(harmonics_i) FROM (SELECT ch, array_agg(sqrt(pow(harmonics_i[ch][h][1],2.0) + pow(harmonics_i[ch][h][2], 2.0))) harmonics_i FROM (SELECT generate_series(1, array_length(harmonics_i, 1)) as ch) tmp1, (SELECT generate_series(1, 50) as h) tmp2 GROUP BY ch) tmp3)");
    else if(Serie == "Current_Harmonics_Complex") Cols.append(", harmonics_i");
    else if(Serie == "Power_Harmonics")           Cols.append(", harmonics_p");
    else if(Serie == "Voltage_Stats")             Cols.append(", stat_voltage");
    else if(Serie == "Frequency_Stats")           Cols.append(", stat_frequency");
    else if(Serie == "Voltage_THD_Stats")         Cols.append(", stat_thd");
    else if(Serie == "Voltage_Harmonics_Stats")   Cols.append(", stat_harmonics");
    else if(Serie == "Unbalance_Stats")           Cols.append(", stat_unbalance");
  }
  auto Func = [&, To, From, Aggreg, Cols, Headers](function<bool(const json &content)> Write) {
    uint64_t Start = From;
    Database DB(pSerial);
    Write(Headers);
    while(true) {
      if(DB.FecthRow() == false) {
        DB.ExecSentenceAsyncResult("SELECT sampletime, json_build_array(" + Cols + ") FROM " + ((Aggreg == "1M") ? "aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m" : "aggreg_" + Aggreg) + " WHERE sampletime >= " + DB.Bind(Start) + " ORDER BY sampletime ASC LIMIT 100");
        if(DB.FecthRow() == false) return;
      }
      Start = DB.ResultUInt64(0);
      if((Start > To) || (Write(DB.ResultJSON(1)) == false)) return;
      Start = Start + 1;
    }
  };
  req.SuccessStreamArray(Func);
}
void AnalyzerSeries::SetSeries(HTTPRequest &req) const {
  if(!req.CheckParamEnum("Aggreg", {"200MS", "3S", "1M", "10M", "15M", "1H"}) || !req.CheckParamArray("Samples")) return;
  if((req.Param("Aggreg") == "200MS") && !req.CheckParamSet("Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics_Complex", "Current_Harmonics_Complex", "Power_Harmonics"}, true)) return;
  else if((req.Param("Aggreg") == "3S") && !req.CheckParamSet("Headers", {"Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power"}, true)) return;
  else if((req.Param("Aggreg") == "1M") && !req.CheckParamSet("Headers", {"Time", "Frequency", "Unbalance", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Power_Factor", "Voltage_THD", "Current_THD", "Phi", "Voltage_Harmonics", "Current_Harmonics", "Power_Harmonics"}, true)) return;
  else if((req.Param("Aggreg") == "10M") && !req.CheckParamSet("Headers", {"Time", "Frequency", "Flag", "Voltage", "Current"}, true)) return;
  else if((req.Param("Aggreg") == "15M") && !req.CheckParamSet("Headers", {"Time", "Active_Power", "Reactive_Power"}, true)) return;
  else if((req.Param("Aggreg") == "1H") && !req.CheckParamSet("Headers", {"Time", "Frequency", "Flag", "Voltage", "Current", "Active_Power", "Reactive_Power", "Voltage_Stats", "Frequency_Stats", "Voltage_THD_Stats", "Voltage_Harmonics_Stats", "Unbalance_Stats"}, true)) return;
  string         Aggreg  = req.Param("Aggreg");
  vector<string> Series  = req.Param("Headers");
  Database DB(pSerial);
  for(auto &Row : req.Param("Samples")) {
    if((Row.is_array() == false) || (Row.size() != Series.size())) return req.Error("Invalid 'Samples' param", HTTP_BAD_REQUEST);
    string Time, Voltage, Current, Frequency, Flag, Active_Power, Reactive_Power, Power_Factor, Voltage_THD, Current_THD, Phi, Unbalance, Voltage_Harmonics_Complex, Voltage_Harmonics, Current_Harmonics_Complex, Current_Harmonics, Power_Harmonics, Voltage_Stats, Frequency_Stats, Voltage_THD_Stats, Voltage_Harmonics_Stats, Unbalance_Stats;
    auto RowPtr = Row.begin();
    for(auto &Col : Series) {
      if(Col == "Time")                           Time                      = DB.BindUint(*RowPtr);
      else if(Col == "Voltage")                   Voltage                   = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Current")                   Current                   = DB.BindArrayFloat(*RowPtr, pCurrentChannels);
      else if(Col == "Frequency")                 Frequency                 = DB.BindFloat(*RowPtr);
      else if(Col == "Flag")                      Flag                      = DB.BindBool(*RowPtr);
      else if(Col == "Active_Power")              Active_Power              = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Reactive_Power")            Reactive_Power            = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Power_Factor")              Power_Factor              = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Voltage_THD")               Voltage_THD               = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Current_THD")               Current_THD               = DB.BindArrayFloat(*RowPtr, pCurrentChannels);
      else if(Col == "Phi")                       Phi                       = DB.BindArrayFloat(*RowPtr, pVoltageChannels);
      else if(Col == "Unbalance")                 Unbalance                 = DB.BindFloat(*RowPtr);
      else if(Col == "Voltage_Harmonics_Complex") Voltage_Harmonics_Complex = DB.BindArrayArrayArrayFloat(*RowPtr, pVoltageChannels, 50, 2);
      else if(Col == "Voltage_Harmonics")         Voltage_Harmonics         = DB.BindArrayArrayFloat(*RowPtr, pVoltageChannels, 50);
      else if(Col == "Current_Harmonics_Complex") Current_Harmonics_Complex = DB.BindArrayArrayArrayFloat(*RowPtr, pCurrentChannels, 50, 2);
      else if(Col == "Current_Harmonics")         Current_Harmonics         = DB.BindArrayArrayFloat(*RowPtr, pCurrentChannels, 50);
      else if(Col == "Power_Harmonics")           Power_Harmonics           = DB.BindArrayArrayFloat(*RowPtr, pVoltageChannels, 50);
      else if(Col == "Voltage_Stats")             Voltage_Stats             = DB.BindArrayUint(*RowPtr, Analyzer::VoltageRangesLen);
      else if(Col == "Frequency_Stats")           Frequency_Stats           = DB.BindArrayUint(*RowPtr, Analyzer::FrequencyRangesLen);
      else if(Col == "Voltage_THD_Stats")         Voltage_THD_Stats         = DB.BindArrayUint(*RowPtr, Analyzer::THDRangesLen);
      else if(Col == "Voltage_Harmonics_Stats")   Voltage_Harmonics_Stats   = DB.BindArrayUint(*RowPtr, 2);
      else if(Col == "Unbalance_Stats")           Unbalance_Stats           = DB.BindArrayUint(*RowPtr, Analyzer::UnbalanceRangesLen);
      RowPtr++;
    }
    if((Aggreg == "200MS") && !Time.empty() && !Frequency.empty() && !Unbalance.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty() && !Active_Power.empty() && !Reactive_Power.empty() && !Power_Factor.empty() && !Voltage_THD.empty() && !Current_THD.empty() && !Phi.empty() && !Voltage_Harmonics_Complex.empty() && !Current_Harmonics_Complex.empty() && !Power_Harmonics.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) "
                                 "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ", " + Power_Factor + ", " + Voltage_THD + ", " + Current_THD + ", " + Phi + ", " + Unbalance + ", " + Voltage_Harmonics_Complex + ", " + Current_Harmonics_Complex + ", " + Power_Harmonics + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag, active_power = EXCLUDED.active_power, reactive_power = EXCLUDED.reactive_power, power_factor = EXCLUDED.power_factor, thd_v = EXCLUDED.thd_v, thd_i = EXCLUDED.thd_i, phi = EXCLUDED.phi, unbalance = EXCLUDED.unbalance, harmonics_v = EXCLUDED.harmonics_v, harmonics_i = EXCLUDED.harmonics_i, harmonics_p = EXCLUDED.harmonics.p") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "3S") && !Time.empty() && !Frequency.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty() && !Active_Power.empty() && !Reactive_Power.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) "
                                 "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag, active_power = EXCLUDED.active_power, reactive_power = EXCLUDED.reactive_power") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "1M") && !Time.empty() && !Frequency.empty() && !Unbalance.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty() && !Active_Power.empty() && !Reactive_Power.empty() && !Power_Factor.empty() && !Voltage_THD.empty() && !Current_THD.empty() && !Phi.empty() && !Voltage_Harmonics.empty() && !Current_Harmonics.empty() && !Power_Harmonics.empty()) {
      if(DB.ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) "
                                 "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag RETURNING sampletime) "
                                 "INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) "
                                 "SELECT sampletime, " + Active_Power + ", " + Reactive_Power + ", " + Power_Factor + ", " + Voltage_THD + ", " + Current_THD + ", " + Phi + ", " + Unbalance + ", " + Voltage_Harmonics + ", " + Current_Harmonics + ", " + Power_Harmonics + " FROM Sample "
                                 "ON CONFLICT(sampletime) DO UPDATE SET active_power = EXCLUDED.active_power, reactive_power = EXCLUDED.reactive_power, power_factor = EXCLUDED.power_factor, thd_v = EXCLUDED.thd_v, thd_i = EXCLUDED.thd_i, phi = EXCLUDED.phi, unbalance = EXCLUDED.unbalance, harmonics_v = EXCLUDED.harmonics_v, harmonics_i = EXCLUDED.harmonics_i, harmonics_p = EXCLUDED.harmonics_p") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "1M") && !Time.empty() && !Frequency.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) "
                                 "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "10M") && !Time.empty() && !Frequency.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) "
                                 "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "15M") && !Time.empty() && !Active_Power.empty() && !Reactive_Power.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) "
                                 "VALUES(" + Time + ", " + Active_Power + ", " + Reactive_Power + ") "
                                 "ON CONFLICT(sampletime) DO UPDATE SET active_power = EXCLUDED.active_power, reactive_power = EXCLUDED.reactive_power") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else if((Aggreg == "1H") && !Time.empty() && !Frequency.empty() && !Flag.empty() && !Voltage.empty() && !Current.empty() && !Active_Power.empty() && !Reactive_Power.empty() && !Voltage_Stats.empty() && !Frequency_Stats.empty() && !Voltage_THD_Stats.empty() && !Voltage_Harmonics_Stats.empty() && !Unbalance_Stats.empty()) {
      if(DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) "
                                "VALUES(" + Time + ", " + Voltage + ", " + Current + ", " + Frequency + ", " + Flag + ", " + Active_Power + ", " + Reactive_Power + ", " + Voltage_Stats + ", " + Frequency_Stats + ", " + Voltage_THD_Stats + ", " + Voltage_Harmonics_Stats + ", " + Unbalance_Stats + ") "
                                "ON CONFLICT(sampletime) DO UPDATE SET rms_v = EXCLUDED.rms_v, rms_i = EXCLUDED.rms_i, freq = EXCLUDED.freq, flag = EXCLUDED.flag, active_power = EXCLUDED.active_power, reactive_power = EXCLUDED.reactive_power, stat_voltage = EXCLUDED.stat_voltage, stat_frequency = EXCLUDED.stat_frequency, stat_thd = EXCLUDED.stat_thd, stat_harmonics = EXCLUDED.stat_harmonics, stat_unbalance = EXCLUDED.stat_unbalance") == false) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
    } else {
      return req.Error("Invalid 'Samples' param", HTTP_BAD_REQUEST);
    }
  }
  req.Success(json::object());
}
void AnalyzerSeries::GetSeriesRange(HTTPRequest &req) const {
  if(!req.CheckParamEnum("Aggreg", {"200MS", "3S", "1M", "10M", "15M", "1H"})) return;
  string Aggreg = req.Param("Aggreg");
  Database DB(pSerial);
  if((DB.ExecSentenceAsyncResult("SELECT json_build_object('To', max(sampletime), 'From', min(sampletime)) FROM aggreg_" + Aggreg) == false) || (DB.FecthRow() == false)) return req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(DB.ResultJSON(0));
}
void AnalyzerSeries::DelSeriesRange(HTTPRequest &req) const {
  if(!req.CheckParamEnum("Aggreg", {"200MS", "3S", "1M", "10M", "15M", "1H"}) || !req.CheckParamNumber("From") || !req.CheckParamNumber("To")) return;
  string   Aggreg = req.Param("Aggreg");
  uint64_t From   = req.Param("From");
  uint64_t To     = req.Param("To");
  Database DB(pSerial);
  if(DB.ExecSentenceNoResult("DELETE FROM aggreg_" + Aggreg + " WHERE sampletime BETWEEN " + DB.Bind(From) + " AND " + DB.Bind(To)) == false) req.Error("", HTTP_INTERNAL_SERVER_ERROR);
  req.Success(json::object());
}
// ---------------------------------------------------------------------------------------------------------------------