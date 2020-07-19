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
#include <string>
#include <inttypes.h>
#include "nlohmann/json.h"
#include "muParser/muParser.h"
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace mu;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAlarm final {
  private :
    string                   Name;
    uint8_t                  Priority;
    uint16_t                 Samples;
    uint16_t                 Window;
    uint16_t                 ResultPos;
    uint16_t                 ResultCount;
//    AnalyzerSoft::AggregType Aggreg;
    Parser                   Evaluator;
    uint8_t                 *Results;
    bool                     Triggered;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftAlarm(const json &config);
};
// ---------------------------------------------------------------------------------------------------------------------