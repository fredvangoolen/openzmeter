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
DeviceV2Analyzer::DeviceV2Analyzer(const string &name, const Analyzer::AnalyzerMode mode, const string &configPath, const float samplerate, const uint8_t *voltage, const uint8_t *current, const bool *invert) : AnalyzerSoft(name, mode, configPath, samplerate) {
  if(mode == Analyzer::PHASE1) {
    VoltageChannels = 1;
    CurrentChannels = 1;
  } else if(mode == Analyzer::PHASE3) {
    VoltageChannels = 3;
    CurrentChannels = 3;
  } else if(mode == Analyzer::PHASE3N) {
    VoltageChannels = 3;
    CurrentChannels = 4;
  }
  for(uint8_t Ch = 0; Ch < VoltageChannels; Ch++) VoltageMap[Ch] = voltage[Ch];
  for(uint8_t Ch = 0; Ch < CurrentChannels; Ch++) CurrentMap[Ch] = current[Ch];
  for(uint8_t Ch = 0; Ch < CurrentChannels; Ch++) InvertMap[Ch]  = (invert[Ch] == true) ? -1.0 : 1.0;
}
bool DeviceV2Analyzer::BufferAppend(const float *values, const uint32_t buffUsage) {
  float Voltage[3], Current[4];
  for(uint8_t Ch = 0; Ch < VoltageChannels; Ch++) Voltage[Ch] = values[VoltageMap[Ch]];
  for(uint8_t Ch = 0; Ch < CurrentChannels; Ch++) Current[Ch] = values[CurrentMap[Ch]] * InvertMap[Ch];
  return AnalyzerSoft::BufferAppend(Voltage, Current, buffUsage);
}
// ---------------------------------------------------------------------------------------------------------------------