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
#include <map>
#include <pthread.h>
#include "Tools.h"
#include "AnalyzerSoft.h"
#include "AnalyzerSoftAlarm.h"
#include "AnalyzerSoftAggreg200MS.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftMQTT;
class AnalyzerSoftAlarms final {
  private :
    float                           pNominalVoltage;
    float                           pNominalFrequency;
    pthread_mutex_t                 pConfigMutex;
    pthread_mutex_t                 pAlarmsMutex;
    map<string, AnalyzerSoftAlarm>  pAlarms;
    string                          pSerial;
    Worker                         *pWorker;
    string                          pRunningAggreg;
    AnalyzerSoftMQTT               *pMQTT;
  private :
    void Alarms_Func(AnalyzerSoftAggreg *aggreg);
    void PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered);
  public :
    AnalyzerSoftAlarms(AnalyzerSoftMQTT *mqtt, const string &serial);
    ~AnalyzerSoftAlarms();
    void Config(const bool enabled, const float nominalVoltage, const float nominalFreq);
    void Run(const string &name, const AnalyzerSoftAggreg *params);
    void Wait();
    void GetAlarmsDef(const json &params, Response &response);
    void TestAlarm(const json &params, Response &response, AnalyzerSoftAggreg200MS *aggreg);
    void AddAlarmDef(const json &params, Response &response);
    void GetAlarmDef(const json &params, Response &response);
    void SetAlarmDef(const json &params, Response &response);
    void DelAlarmDef(const json &params, Response &response);
    void GetAlarms(const json &params, Response &response);
    void GetAlarmsRange(const json &params, Response &response);
    void DelAlarm(const json &params, Response &response);
    void DelAlarmsRange(const json &params, Response &response);
};
// ---------------------------------------------------------------------------------------------------------------------