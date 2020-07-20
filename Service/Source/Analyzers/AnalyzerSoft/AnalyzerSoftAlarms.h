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
#include <vector>
#include "Tools.h"
#include "AnalyzerSoft.h"
#include "AnalyzerSoftAlarm.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftAlarms final {
  private :
    vector<AnalyzerSoftAlarm>  pAlarms;                  //Alarms control object
    string                     pSerial;
    Worker                    *pWorker;
  private :
    void Alarms_Func(const AnalyzerSoftAggreg *aggreg);
//    void InitAlarmParser(Parser &parser, const string &expr, const AnalyzerSoftAggreg::AggregType type);
    void PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered);

  public :
    AnalyzerSoftAlarms(const string &serial);
    ~AnalyzerSoftAlarms();
    void Run(const AnalyzerSoftAggreg *params);
    void Wait();
    void TestAlarm(const json &params, Response &response);
    void ListAlarms(const json &params, Response &response);
    void GetAlarms(const json &params, Response &response);
    void AddAlarm(const json &params, Response &response);
    void GetAlarm(const json &params, Response &response);
    void SetAlarm(const json &params, Response &response);
    void DelAlarm(const json &params, Response &response);
    void GetAlarmsRange(const json &params, Response &response);
    void DelAlarmsRange(const json &params, Response &response);
};
// ---------------------------------------------------------------------------------------------------------------------