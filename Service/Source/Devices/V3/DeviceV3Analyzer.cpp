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
DeviceV3Analyzer::DeviceV3Analyzer(const string &name, const string &configPath, const uint8_t current) : AnalyzerSoft(name, Analyzer::PHASE1, configPath, 24000.0) {
  pCurrent = current;
  pCalc    = false;
}
DeviceV3Analyzer::DeviceV3Analyzer(const string &name, const string &configPath) : AnalyzerSoft(name, Analyzer::PHASE1, configPath, 24000.0) {
  pCalc    = true;
}
void DeviceV3Analyzer::BufferAppend(const float *values) {
//  float Voltage[3], Current[4];
//  if(pCalc == false) {
//    for(uint8_t Ch = 0; Ch < pVoltageChannels; Ch++) Voltage[Ch] = values[pVoltageMap[Ch]];
//    for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) Current[Ch] = values[pCurrentMap[Ch]] * pInvertMap[Ch];
//  } else {
//    Voltage[0] = values[pVoltageMap[0]];
//    Current[0] = values[3] * pInvertMap[0] + values[4] * pInvertMap[1] + values[5] * pInvertMap[2] + values[6] * pInvertMap[3];
//  }
//  AnalyzerSoft::BufferAppend(Voltage, Current);
}
// ---------------------------------------------------------------------------------------------------------------------