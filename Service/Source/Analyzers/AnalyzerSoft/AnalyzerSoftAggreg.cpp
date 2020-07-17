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
void AnalyzerSoft::InitAggreg(Params_t &params, AggregType type) {
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
}
// ---------------------------------------------------------------------------------------------------------------------