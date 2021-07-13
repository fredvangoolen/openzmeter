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
#include "DeviceV2Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
DeviceV2Analyzer::DeviceV2Analyzer(const string &name, const Analyzer::AnalyzerMode mode, const string &configPath, const uint8_t *voltage, const uint8_t *current, const bool *invert) : AnalyzerSoft(name, mode, configPath, 24000.0), pVoltageChannels(VoltageChannels(mode)), pCurrentChannels(CurrentChannels(mode)), pCalc(false) {
  for(uint8_t Ch = 0; Ch < pVoltageChannels; Ch++) pVoltageMap[Ch] = voltage[Ch];
  for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) pCurrentMap[Ch] = current[Ch];
  for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) pInvertMap[Ch]  = (invert[Ch] == true) ? -1.0 : 1.0;
  pLastEvent = 0;
}
DeviceV2Analyzer::DeviceV2Analyzer(const string &name, const string &configPath, const uint8_t voltage, const float *current) : AnalyzerSoft(name, Analyzer::PHASE1, configPath, 24000.0), pVoltageChannels(1), pCurrentChannels(4), pCalc(true) {
  pVoltageMap[0]   = voltage;
  for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) pCurrentMap[Ch] = 0;
  for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) pInvertMap[Ch]  = current[Ch];
}
void DeviceV2Analyzer::BufferAppend(const float *values) {
  float Voltage[3], Current[4];
  if(pCalc == false) {
    for(uint8_t Ch = 0; Ch < pVoltageChannels; Ch++) Voltage[Ch] = values[pVoltageMap[Ch]];
    for(uint8_t Ch = 0; Ch < pCurrentChannels; Ch++) Current[Ch] = values[pCurrentMap[Ch]] * pInvertMap[Ch];
  } else {
    Voltage[0] = values[pVoltageMap[0]];
    Current[0] = values[3] * pInvertMap[0] + values[4] * pInvertMap[1] + values[5] * pInvertMap[2] + values[6] * pInvertMap[3];
  }
  AnalyzerSoft::BufferAppend(Voltage, Current);
}
void DeviceV2Analyzer::ResultsReady(AnalyzerSoftAggreg200MS *params) {
  if(params->Flag() == true) pLastEvent = params->Timestamp();
}
uint64_t DeviceV2Analyzer::GetLastEvent() {
  return pLastEvent;
}
// ---------------------------------------------------------------------------------------------------------------------