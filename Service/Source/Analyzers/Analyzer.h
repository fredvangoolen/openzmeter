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
    enum AnalyzerRights { READ, WRITE, CONFIG };
  private :
    RWLocks           pMembersLocks;            //Locks for configurations
    uint64_t          pLastSave;                //Timestamp of last save or communication
    string            pName;                    //Analyzer description
  public :
    const string Serial;                        //Serial number
  protected :
    Analyzer(const string &id);
    string GetName();
    void   SetName(const string &name);
    void   SetOnline();
    bool   IsOnline();
  public :
    virtual void GetAnalyzer(const json &params, Response &response) = 0;
    virtual void SetAnalyzer(const json &params, Response &response) = 0;
    virtual void GetSeries(const json &params, Response &response) = 0;
    virtual void GetSeriesNow(const json &params, Response &response) = 0;
    virtual void GetSeriesRange(const json &params, Response &response) = 0;
    virtual void GetEvents(const json &params, Response &response) = 0;
    virtual void GetEvent(const json &params, Response &response) = 0;
    virtual void SetEvent(const json &params, Response &response) = 0;
    virtual void DelEvent(const json &params, Response &response) = 0;
    virtual void GetRates(const json &params, Response &response) = 0;
    virtual void GetRate(const json &params, Response &response) = 0;
    virtual void SetRate(const json &params, Response &response) = 0;
    virtual void DelRate(const json &params, Response &response) = 0;
    virtual void AddRate(const json &params, Response &response) = 0;
    virtual void GetPrices(const json &params, Response &response) = 0;
    virtual void TestAlarm(const json &params, Response &response) = 0;
    virtual void GetAlarms(const json &params, Response &response) = 0;
    virtual void GetAlarmsLog(const json &params, Response &response) = 0;
    virtual void GetAlarm(const json &params, Response &response) = 0;
    virtual void SetAlarm(const json &params, Response &response) = 0;
    virtual void AddAlarm(const json &params, Response &response) = 0;
    virtual void DelAlarm(const json &params, Response &response) = 0;
  public :
    virtual ~Analyzer();
    json     GetJSONDescriptor();
    bool     CheckRights(const string &user, const AnalyzerRights neededAccess);
    void     GetPermissions(const json &params, Response &response);
    void     AddPermission(const json &params, Response &response);
    void     SetPermission(const json &params, Response &response);
    void     DelPermission(const json &params, Response &response);
};