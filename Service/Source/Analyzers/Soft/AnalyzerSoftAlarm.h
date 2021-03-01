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
#include "nlohmann/json.hpp"
#include "muParser/muParser.h"
#include "AnalyzerSoftAggreg.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace mu;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAlarm final {
  public :
    enum Alarm_Result { NONE, TRIGGERED, RELEASED };
  private :
    string            pSerial;
    string            pUID;
    string            pName;
    uint8_t           pPriority;
    uint16_t          pSamples;
    uint16_t          pWindow;
    uint16_t          pResultPos;
    uint16_t          pResultCount;
    string            pAggreg;
    Parser            pEvaluator;
    uint8_t          *pResults;
    bool              pInitialized;
    bool              pEnabled;
    string            pExpression;
    uint64_t          pTriggerTime;
    DatabaseQueue    *pDB;
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftAlarm(const string &serial, float &voltage, float &freq, const json &config, DatabaseQueue *db);
    ~AnalyzerSoftAlarm();
    Alarm_Result PushAggreg(AnalyzerSoftAggreg *data);
    string       Name() const;
    uint8_t      Priority() const;
};
// ---------------------------------------------------------------------------------------------------------------------