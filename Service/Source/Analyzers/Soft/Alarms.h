/* ================================== zMeter ===========================================================================
 * File:     Alarms.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "muParser/muParser.h"
#include "HTTPServer.h"
#include "AnalyzerSoft.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace mu;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft::Alarms final {
  private :
    typedef struct {
      string Name;
      float  Value;
    } Variable_t;
    typedef struct {
      AggregType         Type;
      vector<Variable_t> Variables;
      Parser             Evaluator;
    } Alarm_t;
  private : 
    AnalyzerSoft              *pParent;                  //Parent analyzer
    vector<Alarm_t>            pAlarms;                  //Alarms list
    RWLocks                    pMemberLocks;             //Synchronization locks
  public:
    Alarms(AnalyzerSoft *parent);
    ~Alarms();
    void TestAlarm(HTTPRequest &req);
};


