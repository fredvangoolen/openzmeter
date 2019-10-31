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
  private :
    typedef struct {
      json      Params;
      string    Method;
      uint64_t  Sequence;
      Analyzer *Parent;
    } RemoteRequest_t;
  protected :
    RWLocks         pConfigLocks;             //Locks for configurations
    InstanceLocks   pInstances;               //Control instance counters
    uint64_t        pLastSave;                //Timestamp of last save or communication
    string          pSerial;                  //Serial number
    string          pName;                    //Analyzer description
  private :
    
    

//  private :
//    void StopSync();
  protected :
    bool CheckRights(HTTPRequest &req, const uint neededAccess) const;
//    bool SetSync(const bool enabled, const string &server, const string &user, const string &pass);
//    json GetJSONSyncConfig();
//  private :
//    static void  *Request_Thread(void *args);
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
    string Serial() const;
    json   GetJSONShortDescriptor();
};