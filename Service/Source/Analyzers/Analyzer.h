/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <complex>
#include <nlohmann/json.hpp>
#include "Tools.h"
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Analyzer {
  public :
    enum AnalyzerMode { PHASE1, PHASE3, PHASE3N };
    enum AnalyzerRights { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
  protected :
    RWLocks     pMembersLocks;            //Locks for configurations
    uint64_t    pLastSave;                //Timestamp of last save or communication
    string      pSerial;                  //Serial number
    string      pName;                    //Analyzer description
  protected :
    bool CheckRights(HTTPRequest &req, const uint neededAccess);
  public :
    virtual void GetAnalyzer(HTTPRequest &req) = 0;
    virtual void SetAnalyzer(HTTPRequest &req) = 0;
    virtual void GetSeries(HTTPRequest &req) = 0;
    virtual void GetSeriesNow(HTTPRequest &req) = 0;
    virtual void GetSeriesRange(HTTPRequest &req) = 0;
    virtual void GetEvents(HTTPRequest &req) = 0;
    virtual void GetEvent(HTTPRequest &req) = 0;
    virtual void SetEvent(HTTPRequest &req) = 0;
    virtual void DelEvent(HTTPRequest &req) = 0;
    virtual void GetRates(HTTPRequest &req) = 0;
    virtual void GetRate(HTTPRequest &req) = 0;
    virtual void SetRate(HTTPRequest &req) = 0;
    virtual void DelRate(HTTPRequest &req) = 0;
    virtual void AddRate(HTTPRequest &req) = 0;
    virtual void GetPrices(HTTPRequest &req) = 0;
    virtual void TestAlarm(HTTPRequest &req) = 0;
    virtual void GetAlarms(HTTPRequest &req) = 0;
    virtual void GetAlarm(HTTPRequest &req) = 0;
    virtual void SetAlarm(HTTPRequest &req) = 0;
    virtual void AddAlarm(HTTPRequest &req) = 0;
    virtual void DelAlarm(HTTPRequest &req) = 0;
  public :
    Analyzer(const string &id);
    virtual ~Analyzer();
    string Serial();
    json   GetJSONShortDescriptor();
};