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
#pragma once
#include <AnalyzerSoft.h>
// ---------------------------------------------------------------------------------------------------------------------
class DeviceV2Analyzer final : public AnalyzerSoft {
  protected :
    uint8_t                 pVoltageMap[3];
    uint8_t                 pCurrentMap[4];
    float                   pInvertMap[4];
    uint8_t                 pVoltageChannels;
    uint8_t                 pCurrentChannels;
    bool                    pCalc;
    uint64_t                pLastEvent;
  private :
    void ResultsReady(AnalyzerSoftAggreg200MS &params) override;
  public :
    DeviceV2Analyzer(const string &name, const Analyzer::AnalyzerMode mode, const string &configPath, const uint8_t *voltage, const uint8_t *current, const bool *invert);
    DeviceV2Analyzer(const string &name, const string &configPath, const uint8_t voltage, const float *current);
    void     BufferAppend(const float *values);
    uint64_t GetLastEvent();
};
// ---------------------------------------------------------------------------------------------------------------------