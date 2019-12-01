/* ================================== zMeter ===========================================================================
 * File:     Alarms.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <queue>
#include "muParser/muParser.h"
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
    pthread_t                  pAlarmThread;             //Hilo de ejecucion
    bool                       pTerminate;               //Terminate flag
    WorkerLocks                pWorkerLocks;             //Control access to event detection thread
  public :
    static json  CheckConfig(const json &config);
    static void *Alarms_Thread(void *args);
  public:
    Alarms(AnalyzerSoft *parent, const json &config);
    void Start(AnalyzerSoft::Params_t &aggreg);
    void Wait();
    ~Alarms();
    void TestAlarm(HTTPRequest &req);
    void GetAlarms(HTTPRequest &req);
    void GetAlarm(HTTPRequest &req);
    void SetAlarm(HTTPRequest &req);
    void AddAlarm(HTTPRequest &req);
    void DelAlarm(HTTPRequest &req);
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