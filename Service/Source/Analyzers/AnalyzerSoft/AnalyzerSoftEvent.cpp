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
#include "AnalyzerSoftEvent.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftEvent::AnalyzerSoftEvent(const Analyzer::AnalyzerMode mode, const float nominalVoltage) {
  pVoltageChannels = (mode == Analyzer::PHASE1) ? 1 : 3;
  pNominalVoltage = nominalVoltage;
}
AnalyzerSoftEvent::~AnalyzerSoftEvent() {

}
float AnalyzerSoftEvent::CalcChange(const float in, const float reference) const {
  return ((in - reference) / reference) * 100;
}
bool AnalyzerSoftEvent::IsRunning(const bool stationary, const float *cycle, const float *avg) const {
  bool IsRunning = false;
  if(pType == EVT_INTERRUPTION) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(cycle[n], pNominalVoltage) > (EVENT_INTERRUPTION - EVENT_INTERRUPTION_HYSTERESIS));
  } else if(pType == EVT_DIP) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (-CalcChange(cycle[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
  } else if(pType == EVT_SWELL) {
    for(uint8_t n = 0; n < pVoltageChannels; n++) IsRunning |= (CalcChange(cycle[n], pNominalVoltage) > (EVENT_DIP_SWELL - EVENT_HYSTERESIS));
  } else if(pType == EVT_RVC) {
    if(stationary == true) {
//      for(uint8_t n = 0; n < pVoltageChannels; n++) pChangeValue[n] = avg[n];
    } else {
      IsRunning = true;
    }
  }
  return IsRunning;

}