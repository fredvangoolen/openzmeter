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
#include "cctz/time_zone.h"
#include "AnalyzerSoftAlarm.h"
#include "AnalyzerSoftAggreg200MS.h"
#include "AnalyzerSoftMQTT.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
// ---------------------------------------------------------------------------------------------------------------------
//class AnalyzerSoftMQTT;
class AnalyzerSoftAlarms final {
  private :
    float                                     pNominalVoltage;
    float                                     pNominalFrequency;
    pthread_rwlock_t                          pLock;
    map<string, vector<AnalyzerSoftAlarm*> >  pAlarms;
    string                                    pSerial;
    Worker                                   *pWorker;
    AnalyzerSoftMQTT                         *pMQTT;
    DatabaseQueue                            *pDB;
    time_zone                                 pTimeZone;
  private :
    void Alarms_Func(const string &name, AnalyzerSoftAggreg *params);
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftAlarms(const string &serial, AnalyzerSoftMQTT *mqtt, DatabaseQueue *db);
    ~AnalyzerSoftAlarms();
    void Config(const bool enabled, const string &tz, const json &alarms);
    void ConfigNominals(const float nominalVoltage, const float nominalFreq);
    void Run(const string &name, AnalyzerSoftAggreg *params);
    void Wait();
    void TestAlarm(const json &params, Response &response, AnalyzerSoftAggreg200MS *aggreg) const;
    void DelAlarm(const json &params, Response &response) const;
    void DelAlarmsRange(const json &params, Response &response) const;
};
// ---------------------------------------------------------------------------------------------------------------------