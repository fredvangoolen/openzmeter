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
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg::AnalyzerSoftAggreg(const string &serial, const Analyzer::AnalyzerMode mode, const AggregType type) {
  pType   = type;
  pSerial = serial;
  pVoltageChannels = (mode == Analyzer::PHASE1) ? 1 : 3;
  pCurrentChannels = (mode == Analyzer::PHASE1) ? 1 : (mode == Analyzer::PHASE3) ? 3 : 4;
}
uint64_t AnalyzerSoftAggreg::Timestamp() const {
  return pTimestamp;
}
void AnalyzerSoftAggreg::Timestamp(const uint64_t time) {
  pTimestamp = time;
}
float AnalyzerSoftAggreg::Frequency() const {
  return pFrequency;
}
void AnalyzerSoftAggreg::Frequency(const float val) {
  pFrequency = val;
}
float AnalyzerSoftAggreg::Unbalance() const {
  return pUnbalance;
}
void AnalyzerSoftAggreg::Unbalance(const float val) {
  pUnbalance = val;
}
float AnalyzerSoftAggreg::Voltage(const uint8_t channel) const {
  return pVoltage[channel];
}
void  AnalyzerSoftAggreg::Voltage(const uint8_t channel, const float val) {
  pVoltage[channel] = val;
}
float AnalyzerSoftAggreg::Current(const uint8_t channel) const {
  return pCurrent[channel];
}
void AnalyzerSoftAggreg::Current(const uint8_t channel, const float val) {
  pCurrent[channel] = val;
}
float AnalyzerSoftAggreg::ActivePower(const uint8_t channel) const {
  return pActive_Power[channel];
}
void AnalyzerSoftAggreg::ActivePower(const uint8_t channel, const float val) {
  pActive_Power[channel] = val;
}
float AnalyzerSoftAggreg::ApparentPower(const uint8_t channel) const {
  return pApparent_Power[channel];
}
void AnalyzerSoftAggreg::ApparentPower(const uint8_t channel, const float val) {
  pApparent_Power[channel] = val;
}
float AnalyzerSoftAggreg::ReactivePower(const uint8_t channel) const {
  return pReactive_Power[channel];
}
void AnalyzerSoftAggreg::ReactivePower(const uint8_t channel, const float val) {
  pReactive_Power[channel] = val;
}
float AnalyzerSoftAggreg::ActiveEnergy(const uint8_t channel) const {
  return pActive_Energy[channel];
}
void AnalyzerSoftAggreg::ActiveEnergy(const uint8_t channel, const float val) {
  pActive_Energy[channel] = val;
}
float AnalyzerSoftAggreg::ReactiveEnergy(const uint8_t channel) const {
  return pReactive_Energy[channel];
}
void AnalyzerSoftAggreg::ReactiveEnergy(const uint8_t channel, const float val) {
  pReactive_Energy[channel] = val;
}
float AnalyzerSoftAggreg::VoltageHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pVoltage_Harmonics[channel * 50 + harmonic];
}
void AnalyzerSoftAggreg::VoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pVoltage_Harmonics[channel * 50 + harmonic] = val;
}
float AnalyzerSoftAggreg::CurrentHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pCurrent_Harmonics[channel * 50 + harmonic];
}
void AnalyzerSoftAggreg::CurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pCurrent_Harmonics[channel * 50 + harmonic] = val;
}
float AnalyzerSoftAggreg::PowerHarmonic(const uint8_t channel, const uint8_t harmonic) const {
  return pPower_Harmonics[channel * 50 + harmonic];
}
void AnalyzerSoftAggreg::PowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pPower_Harmonics[channel * 50 + harmonic] = val;
}
float AnalyzerSoftAggreg::CurrentTHD(const uint8_t channel) const {
  return pCurrent_THD[channel];
}
void AnalyzerSoftAggreg::CurrentTHD(const uint8_t channel, const float val) {
  pCurrent_THD[channel] = val;
}
float AnalyzerSoftAggreg::VoltageTHD(const uint8_t channel) const {
  return pVoltage_THD[channel];
}
void AnalyzerSoftAggreg::VoltageTHD(const uint8_t channel, const float val) {
  pVoltage_THD[channel] = val;
}
float AnalyzerSoftAggreg::VoltagePhase(const uint8_t channel) const {
  return pVoltage_Phase[channel];
}
void  AnalyzerSoftAggreg::VoltagePhase(const uint8_t channel, const float val) {
  pVoltage_Phase[channel] = val;
}
float AnalyzerSoftAggreg::Phi(const uint8_t channel) const {
  return pPhi[channel];
}
void  AnalyzerSoftAggreg::Phi(const uint8_t channel, const float val) {
  pPhi[channel] = val;
}
float AnalyzerSoftAggreg::PowerFactor(const uint8_t channel) const {
  return pPower_Factor[channel];
}
void  AnalyzerSoftAggreg::PowerFactor(const uint8_t channel, const float val) {
  pPower_Factor[channel] = val;
}


/*
void AnalyzerSoftAggreg::InitAggreg(Params_t &params, AggregType type) {
  memset(&params, 0, sizeof(params));
  params.Type = type;
}
void AnalyzerSoft::AccumulateAggreg(Params_t &dst, const Params_t &src) {
  if(src.AggregSamples == 0) return;
  if(dst.Timestamp == 0) dst.Timestamp = src.Timestamp;
  dst.AggregSamples += src.AggregSamples;
  dst.Frequency += src.Frequency;
  dst.Unbalance += src.Unbalance;
  dst.Flag |= src.Flag;
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage[n] += src.Voltage[n] * src.Voltage[n];
  for(uint8_t n = 0; n < pCurrentChannels; n++) dst.Current[n] += src.Current[n] * src.Current[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Active_Power[n] += src.Active_Power[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Reactive_Power[n] += src.Reactive_Power[n];
  for(uint8_t n = 0; n < pCurrentChannels; n++) dst.Current_THD[n] += src.Current_THD[n] * src.Current_THD[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Phi[n] += src.Phi[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Power_Factor[n] += src.Power_Factor[n];
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) dst.Current_Harmonics[n] += src.Current_Harmonics[n] * src.Current_Harmonics[n];
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) dst.Power_Harmonics[n] += src.Power_Harmonics[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage_THD[n] += src.Voltage_THD[n] * src.Voltage_THD[n];
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) dst.Voltage_Harmonics[n] += src.Voltage_Harmonics[n] * src.Voltage_Harmonics[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Active_Energy[n] += src.Active_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Voltage_Phase[n] += src.Voltage_Phase[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Reactive_Energy[n] += src.Reactive_Energy[n];
  for(uint8_t n = 0; n < pVoltageChannels; n++) dst.Apparent_Power[n] += src.Apparent_Power[n];
}
void AnalyzerSoft::NormalizeAggreg(Params_t &params) {
  if(params.AggregSamples == 0) return;
  params.Frequency = params.Frequency / params.AggregSamples;
  params.Unbalance = params.Unbalance / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage[n] = sqrt(params.Voltage[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current[n] = sqrt(params.Current[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Active_Power[n] = params.Active_Power[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Reactive_Power[n] = params.Reactive_Power[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pCurrentChannels; n++) params.Current_THD[n] = sqrt(params.Current_THD[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Phi[n] = params.Phi[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Power_Factor[n] = params.Power_Factor[n] / params.AggregSamples;
  for(uint8_t n = 0; n < (50 * pCurrentChannels); n++) params.Current_Harmonics[n] = sqrt(params.Current_Harmonics[n] / params.AggregSamples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Power_Harmonics[n] = params.Power_Harmonics[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_THD[n] = sqrt(params.Voltage_THD[n] / params.AggregSamples);
  for(uint8_t n = 0; n < (50 * pVoltageChannels); n++) params.Voltage_Harmonics[n] = sqrt(params.Voltage_Harmonics[n] / params.AggregSamples);
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Voltage_Phase[n] = params.Voltage_Phase[n] / params.AggregSamples;
  for(uint8_t n = 0; n < pVoltageChannels; n++) params.Apparent_Power[n] = params.Apparent_Power[n] / params.AggregSamples;
  params.AggregSamples = 1;
}
void AnalyzerSoft::SaveAggreg(Params_t &params) {
  if(params.AggregSamples == 0) return;
  SetOnline();
  Database DB(pSerial);
  if(params.Type == AGGREG_200MS) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Power_Factor, pVoltageChannels) + ", " +
    DB.Bind(params.Voltage_THD, pVoltageChannels) + ", " +
    DB.Bind(params.Current_THD, pCurrentChannels) + ", " +
    DB.Bind(params.Phi, pVoltageChannels) + ", " +
    DB.Bind(params.Unbalance) + ")");
  } else if(params.Type == AGGREG_3S) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  } else if(params.Type == AGGREG_1M) {
    DB.ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) SELECT sampletime, " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Power_Factor, pVoltageChannels) + ", " +
    DB.Bind(params.Voltage_THD, pVoltageChannels) + ", " +
    DB.Bind(params.Current_THD, pCurrentChannels) + ", " +
    DB.Bind(params.Phi, pVoltageChannels) + ", " +
    DB.Bind(params.Unbalance) + ", " +
    DB.Bind(params.Voltage_Harmonics, pVoltageChannels, 50) + ", " +
    DB.Bind(params.Current_Harmonics, pCurrentChannels, 50) + ", " +
    DB.Bind(params.Power_Harmonics, pVoltageChannels, 50) + " FROM Sample");
  } else if(params.Type == AGGREG_10M) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ")");
  } else if(params.Type == AGGREG_15M) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ")");
  } else if(params.Type == AGGREG_1H) {
    DB.ExecSentenceNoResult("INSERT INTO aggreg_1h(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, stat_voltage, stat_frequency, stat_thd, stat_harmonics, stat_unbalance) VALUES(" +
    DB.Bind(params.Timestamp) + ", " +
    DB.Bind(params.Voltage, pVoltageChannels) + ", " +
    DB.Bind(params.Current, pCurrentChannels) + ", " +
    DB.Bind(params.Frequency) + ", " +
    DB.Bind(params.Flag) + ", " +
    DB.Bind(params.Active_Power, pVoltageChannels) + ", " +
    DB.Bind(params.Reactive_Power, pVoltageChannels) + ", " +
    DB.Bind(params.StatsVoltage, VoltageRangesLen) + ", " +
    DB.Bind(params.StatsFrequency, FrequencyRangesLen) + ", " +
    DB.Bind(params.StatsTHD, THDRangesLen) + ", " +
    DB.Bind(params.StatsHarmonics, 2) + ", " +
    DB.Bind(params.StatsUnbalance, UnbalanceRangesLen) + ")");
  }
}*/
bool AnalyzerSoftAggreg::AppendToJSON(const string &serie, json &result) {
//  Result["Sampletime"] = pAggreg200MS.Timestamp;
  if(serie == "FREQUENCY") {
    result["Frequency"] = pFrequency;
  } else if(serie == "FLAG") {
    result["Flag"] = pFlag;
  } else if(serie == "UNBALANCE") {
    result["Unbalance"] = pUnbalance;
  } else if(serie == "VOLTAGE") {
    json Tmp = json::array();
    result["Voltage"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pVoltage[n]);
    result["Voltage"] = Tmp;
  } else if(serie == "CURRENT") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) Tmp.push_back(pCurrent[n]);
    result["Current"] = Tmp;
  } else if(serie == "ACTIVE_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pActive_Power[n]);
    result["Active_Power"] = Tmp;
  } else if(serie == "REACTIVE_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pReactive_Power[n]);
    result["Reactive_Power"] = Tmp;
  } else if(serie == "ACTIVE_ENERGY") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pActive_Energy[n]);
    result["Active_Energy"] = Tmp;
  } else if(serie == "REACTIVE_ENERGY") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pReactive_Energy[n]);
    result["Rective_Energy"] = Tmp;
  } else if(serie == "APPARENT_POWER") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pApparent_Power[n]);
    result["Apparent_Power"] = Tmp;
  } else if(serie == "POWER_FACTOR") {
    json Tmp = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) Tmp.push_back(pPower_Factor[n]);

    result["Power_Factor"] = Tmp;
  } else if(serie == "VOLTAGE_THD") {
    result["Voltage_THD"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Voltage_THD"].push_back(pVoltage_THD[n]);
  } else if(serie == "CURRENT_THD") {
    result["Current_THD"] = json::array();
    for(uint8_t n = 0; n < pCurrentChannels; n++) result["Current_THD"].push_back(pCurrent_THD[n]);
  } else if(serie == "PHI") {
    result["Phi"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Phi"].push_back(pPhi[n]);
  } else if(serie == "VOLTAGE_PHASE") {
    result["Voltage_Phase"] = json::array();
    for(uint8_t n = 0; n < pVoltageChannels; n++) result["Voltage_Phase"].push_back(pVoltage_Phase[n]);
//  } else if(serie == "VOLTAGE_HARMONICS") {
//    result["Voltage_Harmonics"] = json::array();
//    for(uint8_t n = 0; n < pVoltageChannels; n++) {
//      result["Voltage_Harmonics"].push_back(json::array());
//      for(uint8_t i = 0; i < 50; i++) {
//        result["Voltage_Harmonics"][n].push_back(json::array());
//        AnalyzerSoftFFT::Complex_t Complex = pVoltageFFT[n]->Harmonic(i);
//        Result["Voltage_Harmonics"][n][i].push_back(Complex.Re);
//        Result["Voltage_Harmonics"][n][i].push_back(Complex.Im);
//      }
//    }
//  } else if(Serie == "CURRENT_HARMONICS") {
//      Result["Current_Harmonics"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_Harmonics"].push_back(json::array());
//        for(uint8_t i = 0; i < 50; i++) {
//          Result["Current_Harmonics"][n].push_back(json::array());
//          AnalyzerSoftFFT::Complex_t Complex = pCurrentFFT[n]->Harmonic(i);
//          Result["Current_Harmonics"][n][i].push_back(Complex.Re);
//          Result["Current_Harmonics"][n][i].push_back(Complex.Im);
//        }
//      }
//    } else if(Serie == "POWER_HARMONICS") {
//      Result["Power_Harmonics"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Power_Harmonics"].push_back(json::array());
//        for(uint8_t i = 0; i < 50; i++) Result["Power_Harmonics"][n].push_back(pAggreg200MS.Power_Harmonics[n * 50 + i]);
//      }
//    } else if(Serie == "VOLTAGE_FFT") {
//      Result["Voltage_FFT"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Voltage_FFT"].push_back(json::array());
//        uint16_t BlockSize = pVoltageFFT[n]->OutLen();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Voltage_FFT"][n].push_back(pVoltageFFT[n]->BinPower(i));
//      }
//    } else if(Serie == "CURRENT_FFT") {
//      Result["Current_FFT"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_FFT"].push_back(json::array());
//        uint16_t BlockSize = pCurrentFFT[n]->OutLen();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Current_FFT"][n].push_back(pCurrentFFT[n]->BinPower(i));
//      }
//    } else if(Serie == "VOLTAGE_SAMPLES") {
//      Result["Voltage_Samples"] = json::array();
//      for(uint8_t n = 0; n < pVoltageChannels; n++) {
//        Result["Voltage_Samples"].push_back(json::array());
//        uint16_t BlockSize = pInputBlock->BlockSamples();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Voltage_Samples"][n].push_back(pInputBlock->VoltageSample(n, i));
//      }
//    } else if(Serie == "CURRENT_SAMPLES") {
//      Result["Current_Samples"] = json::array();
//      for(uint8_t n = 0; n < pCurrentChannels; n++) {
//        Result["Current_Samples"].push_back(json::array());
//        uint16_t BlockSize = pInputBlock->BlockSamples();
//        for(uint16_t i = 0; i < BlockSize; i++) Result["Current_Samples"][n].push_back(pInputBlock->CurrentSample(n, i));
//      }
//    } else if(Serie == "GA_POWER") {
//      Result["GA_Power"] = CalcGA();
//    } else if(Serie == "SYS_STATS") {
//      Result["Sys_Stats"] = SysSupport::GetSystemStatusJSON();
//    } else if(Serie == "UNREAD") {
//      string UserName = params["UserName"];
//      Result["Unread"] = json::object();
//      Database DB(pSerial);
//      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE starttime > eventsread) AS events, alarmsread, (SELECT count(*) FROM alarms WHERE alarmtime > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(UserName) + " AND serial = " + DB.Bind(pSerial));
//      if(DB.FecthRow() == false) {
//        Result["Unread"]["EventsFrom"] = 0;
//        Result["Unread"]["Events"] = 0;
//        Result["Unread"]["AlarmsFrom"] = 0;
//        Result["Unread"]["Alarms"] = 0;
//      } else {
//        Result["Unread"]["EventsFrom"] = DB.ResultUInt64("eventsread");
//        Result["Unread"]["Events"] = DB.ResultUInt64("events");
//        Result["Unread"]["AlarmsFrom"] = DB.ResultUInt64("alarmsread");
//        Result["Unread"]["Alarms"] = DB.ResultUInt64("alarms");
//      }
  } else {
    return false;
  }
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------