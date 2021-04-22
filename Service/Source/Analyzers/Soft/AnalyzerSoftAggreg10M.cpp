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
#include "AnalyzerSoftAggreg10M.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg10M::AnalyzerSoftAggreg10M(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg10M::~AnalyzerSoftAggreg10M() {
}
void AnalyzerSoftAggreg10M::Accumulate(const AnalyzerSoftAggreg200MS *src) {
  pSamples++;
  if(pTimestamp == 0) pTimestamp = src->Timestamp() / 600000 * 600000 + 599999;
  AnalyzerSoftAggreg::Accumulate(src);
}
void AnalyzerSoftAggreg10M::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_10m(sampletime, rms_v, rms_i, freq, flag) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pVoltage, pVoltageChannels) + ", " +
  DB.Bind(pCurrent, pCurrentChannels) + ", " +
  DB.Bind(pFrequency) + ", " +
  DB.Bind(pFlag) + ")");
}
void AnalyzerSoftAggreg10M::Reset() {
  pSamples = 0;
  AnalyzerSoftAggreg::Reset();
}
bool AnalyzerSoftAggreg10M::Normalize() {
  if(pSamples < 2000) return false;
  AnalyzerSoftAggreg::Normalize(pSamples);
  pSamples = 1;
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------