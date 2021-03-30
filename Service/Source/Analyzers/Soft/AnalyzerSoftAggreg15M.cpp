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
#include "AnalyzerSoftAggreg15M.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg15M::AnalyzerSoftAggreg15M(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg15M::~AnalyzerSoftAggreg15M() {
}
void AnalyzerSoftAggreg15M::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_15m(sampletime, active_power, reactive_power) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pActive_Power, pVoltageChannels) + ", " +
  DB.Bind(pReactive_Power, pVoltageChannels) + ")");
}
void AnalyzerSoftAggreg15M::Reset() {
  pSamples = 0;
  AnalyzerSoftAggreg::Reset();
}
bool AnalyzerSoftAggreg15M::Normalize() {
  if(pSamples < 4000) return false;
  AnalyzerSoftAggreg::Normalize(pSamples);
  pSamples = 1;
  return true;
}
void AnalyzerSoftAggreg15M::Accumulate(const AnalyzerSoftAggreg200MS &src) {
  pSamples++;
  if(pTimestamp == 0) pTimestamp = src.Timestamp() / 900000 * 900000 + 899999;
  AnalyzerSoftAggreg::Accumulate(src);
}
// ---------------------------------------------------------------------------------------------------------------------