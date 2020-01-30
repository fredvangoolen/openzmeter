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
#include <queue>
#include "HTTPServer.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft::Alarms final {
  private :
    class Alarm;
  private :
    uint32_t                   pLastAlarmID;             //Last added alarm serial
    AnalyzerSoft              *pParent;                  //Parent analyzer
    vector<Alarm>              pAlarms;                  //Alarms list
    RWLocks                    pMemberLocks;             //Synchronization locks
    AnalyzerSoft::Params_t    *pParams;                  //Params to evaluate alarms
    bool                       pTerminate;               //Terminate flag
    pthread_t                  pAlarmThread;             //Hilo de ejecucion
    WorkerLocks                pWorkerLocks;             //Control access to event detection thread
  public :
    static json  CheckConfig(const json &config);
    static void *Alarms_Thread(void *args);
  public:
    Alarms(AnalyzerSoft *parent, const json &config);
    void Start(AnalyzerSoft::Params_t &aggreg);
    void Wait();
    ~Alarms();
    void TestAlarm(const json &params, Response &response);
    void GetAlarms(const json &params, Response &response);
    void GetAlarm(const json &params, Response &response);
    void SetAlarm(const json &params, Response &response);
    void AddAlarm(const json &params, Response &response);
    void DelAlarm(const json &params, Response &response);
};
class AnalyzerSoft::Alarms::Alarm final {
  private :
    RWLocks            pMemberLocks;             //Locks access to members
    uint32_t           pAlarmID;
    string             pName;
    uint8_t            pPriority;
    string             pMessage;
    uint32_t           pSamples;
    uint32_t           pWindow;
    AggregType         pType;
    Parser             pEvaluator;
    queue<bool>        pResults;
  public :
    static json CheckConfig(const json &config);
  public :
    Alarm(const uint32_t id, const json &config);
    void     Apend(const AnalyzerSoft::Params_t &params);
    void     Configure(const json &config);
    json     GetShortDescriptor();
    json     GetFullDescriptor();
    uint64_t GetID();
};