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
#include "DeviceV3Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
DeviceV3Analyzer::DeviceV3Analyzer(const string &name, const string &configPath, const uint8_t current) : AnalyzerSoft(name, Analyzer::PHASE1, configPath, 24000.0), pCalc(false), pCurrent(current) {
}
DeviceV3Analyzer::DeviceV3Analyzer(const string &name, const string &configPath) : AnalyzerSoft(name, Analyzer::PHASE1, configPath, 24000.0), pCalc(true), pCurrent(0) {
}
void DeviceV3Analyzer::BufferAppend(const float *values) {
  float Voltage = values[0];
  float Current = 0.0;
  if(pCalc == false) {
    Current = values[pCurrent + 1];
  } else {
    for(uint8_t n = 0; n < 8; n++) Current += values[n + 1];
  }
  AnalyzerSoft::BufferAppend(&Voltage, &Current);
}
// ---------------------------------------------------------------------------------------------------------------------