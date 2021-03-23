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
#include "AnalyzerSoftAggreg3S.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg3S::AnalyzerSoftAggreg3S(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg3S::~AnalyzerSoftAggreg3S() {
}
void AnalyzerSoftAggreg3S::Accumulate(const AnalyzerSoftAggreg200MS &src) {
  pSamples++;
  pTimestamp = src.Timestamp();
  AnalyzerSoftAggreg::Accumulate(src);
}
void AnalyzerSoftAggreg3S::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_3s(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pVoltage, pVoltageChannels) + ", " +
  DB.Bind(pCurrent, pCurrentChannels) + ", " +
  DB.Bind(pFrequency) + ", " +
  DB.Bind(pFlag) + ", " +
  DB.Bind(pActive_Power, pVoltageChannels) + ", " +
  DB.Bind(pReactive_Power, pVoltageChannels) + ")");
}
void AnalyzerSoftAggreg3S::Reset() {
  pSamples = 0;
  AnalyzerSoftAggreg::Reset();
}
bool AnalyzerSoftAggreg3S::Normalize() {
  if(pSamples < 10) return false;
  AnalyzerSoftAggreg::Normalize(pSamples);
  pSamples = 1;
  return true;
}
// ---------------------------------------------------------------------------------------------------------------------