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
#include "AnalyzerSoftAggreg200MS.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftAggreg200MS::AnalyzerSoftAggreg200MS(const string &serial, const Analyzer::AnalyzerMode mode) : AnalyzerSoftAggreg(serial, mode) {
  Reset();
}
AnalyzerSoftAggreg200MS::~AnalyzerSoftAggreg200MS() {
}
void AnalyzerSoftAggreg200MS::Save() const {
  Database DB(pSerial);
  DB.ExecSentenceNoResult("INSERT INTO aggreg_200ms(sampletime, rms_v, rms_i, freq, flag, active_power, reactive_power, power_factor, thd_v, thd_i, phi, unbalance) VALUES(" +
  DB.Bind(pTimestamp) + ", " +
  DB.Bind(pVoltage, pVoltageChannels) + ", " +
  DB.Bind(pCurrent, pCurrentChannels) + ", " +
  DB.Bind(pFrequency) + ", " +
  DB.Bind(pFlag) + ", " +
  DB.Bind(pActive_Power, pVoltageChannels) + ", " +
  DB.Bind(pReactive_Power, pVoltageChannels) + ", " +
  DB.Bind(pPower_Factor, pVoltageChannels) + ", " +
  DB.Bind(pVoltage_THD, pVoltageChannels) + ", " +
  DB.Bind(pCurrent_THD, pCurrentChannels) + ", " +
  DB.Bind(pPhi, pVoltageChannels) + ", " +
  DB.Bind(pUnbalance) + ")");
}
void AnalyzerSoftAggreg200MS::Reset() {
  AnalyzerSoftAggreg::Reset();
}
void AnalyzerSoftAggreg200MS::setTimestamp(const uint64_t time) {
  pTimestamp = time;
}
void AnalyzerSoftAggreg200MS::setFrequency(const float val) {
  pFrequency = val;
}
void AnalyzerSoftAggreg200MS::setFlag(const bool val) {
  pFlag |= val;
}
void AnalyzerSoftAggreg200MS::setUnbalance(const float val) {
  pUnbalance = val;
}
void  AnalyzerSoftAggreg200MS::setVoltage(const uint8_t channel, const float val) {
  pVoltage[channel] = val;
}
void AnalyzerSoftAggreg200MS::setCurrent(const uint8_t channel, const float val) {
  pCurrent[channel] = val;
}
void AnalyzerSoftAggreg200MS::setActivePower(const uint8_t channel, const float val) {
  pActive_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setApparentPower(const uint8_t channel, const float val) {
  pApparent_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setReactivePower(const uint8_t channel, const float val) {
  pReactive_Power[channel] = val;
}
void AnalyzerSoftAggreg200MS::setActiveEnergy(const uint8_t channel, const float val) {
  pActive_Energy[channel] = val;
}
void AnalyzerSoftAggreg200MS::setReactiveEnergy(const uint8_t channel, const float val) {
  pReactive_Energy[channel] = val;
}
void AnalyzerSoftAggreg200MS::setVoltageHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pVoltage_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setCurrentHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pCurrent_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setPowerHarmonic(const uint8_t channel, const uint8_t harmonic, const float val) {
  pPower_Harmonics[channel * 50 + harmonic] = val;
}
void AnalyzerSoftAggreg200MS::setCurrentTHD(const uint8_t channel, const float val) {
  pCurrent_THD[channel] = val;
}
void AnalyzerSoftAggreg200MS::setVoltageTHD(const uint8_t channel, const float val) {
  pVoltage_THD[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setVoltagePhase(const uint8_t channel, const float val) {
  pVoltage_Phase[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setPhi(const uint8_t channel, const float val) {
  pPhi[channel] = val;
}
void  AnalyzerSoftAggreg200MS::setPowerFactor(const uint8_t channel, const float val) {
  pPower_Factor[channel] = val;
}
// ---------------------------------------------------------------------------------------------------------------------