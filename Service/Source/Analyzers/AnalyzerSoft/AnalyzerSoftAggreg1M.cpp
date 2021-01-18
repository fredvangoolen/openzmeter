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
#include "AnalyzerSoftAggreg1M.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg1M::AnalyzerSoftAggreg1M(const string &serial, const Analyzer::AnalyzerMode mode, DatabaseQueue *db) : AnalyzerSoftAggreg(serial, mode, db) {
  Reset();
}
AnalyzerSoftAggreg1M::~AnalyzerSoftAggreg1M() {
}
void AnalyzerSoftAggreg1M::Save() const {
  pDB->ExecSentenceNoResult("WITH Sample AS (INSERT INTO aggreg_1m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
  pDB->Bind(pTimestamp) + ", " +
  pDB->Bind(pVoltage, pVoltageChannels) + ", " +
  pDB->Bind(pCurrent, pCurrentChannels) + ", " +
  pDB->Bind(pFrequency) + ", " +
  pDB->Bind(pFlag) + ") RETURNING sampletime) INSERT INTO aggreg_ext_1m(sampletime, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance, harmonics_v, harmonics_i, harmonics_p) SELECT sampletime, " +
  pDB->Bind(pActive_Power, pVoltageChannels) + ", " +
  pDB->Bind(pReactive_Power, pVoltageChannels) + ", " +
  pDB->Bind(pPower_Factor, pVoltageChannels) + ", " +
  pDB->Bind(pVoltage_THD, pVoltageChannels) + ", " +
  pDB->Bind(pCurrent_THD, pCurrentChannels) + ", " +
  pDB->Bind(pPhi, pVoltageChannels) + ", " +
  pDB->Bind(pUnbalance) + ", " +
  pDB->Bind(pVoltage_Harmonics, pVoltageChannels, 50) + ", " +
  pDB->Bind(pCurrent_Harmonics, pCurrentChannels, 50) + ", " +
  pDB->Bind(pPower_Harmonics, pVoltageChannels, 50) + " FROM Sample");
}
void AnalyzerSoftAggreg1M::Reset() {
  pSamples = 0;
  AnalyzerSoftAggreg::Reset();
}
bool AnalyzerSoftAggreg1M::Normalize() {
  if(pSamples < 200) return false;
  AnalyzerSoftAggreg::Normalize(pSamples);
  pSamples = 1;
  return true;
}
void AnalyzerSoftAggreg1M::Accumulate(const AnalyzerSoftAggreg200MS *src) {
  pSamples++;
  if(pTimestamp == 0) pTimestamp = src->Timestamp() / 60000 * 60000 + 59999;
  AnalyzerSoftAggreg::Accumulate(src);
}
// ---------------------------------------------------------------------------------------------------------------------