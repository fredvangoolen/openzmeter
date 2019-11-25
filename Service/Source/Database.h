/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <queue>
#include <complex>
#include <sstream>
#include <inttypes.h>
#include <libpq-fe.h>
#include <nlohmann/json.hpp>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Database {
  private :
    typedef struct {
      PGconn   *Connection;
      uint64_t  FreeTime;     //Total usage time
      uint64_t  UsageTime;    //Total free time
      uint64_t  LastTime;     //Last time event
    } PoolConnection_t;
  private :
    static string                  pConnectionString;
    static queue<PoolConnection_t> pConnectionPool;
    static pthread_mutex_t         pMutex;
    static bool                    pPoolEnabled;
  private :
    PGconn      *pDBConn;
    string       pSchema;
    PGresult    *pResult;
    uint64_t     pUsageTime;
    uint64_t     pFreeTime;
    uint64_t     pLastTime;
  private :
    static void ReadConfig();
  private :
    void ClearAsync();
  public :
    static void RemoveExpiredSessions();
    static void EnablePool();
    static void DisablePool();
  public :
    string      Bind(const uint32_t param) const;
    string      Bind(const uint64_t param) const;
    string      Bind(const float param) const;
    string      Bind(const vector< vector<float> > &params) const;
    string      Bind(const vector<float> &params) const;
    string      Bind(const vector<uint16_t> &params) const;
    string      Bind(const bool param) const;
    string      Bind(const string &string) const;
    string      BindID(const string &string) const;
    bool        ExecSentenceNoResult(const string &sql);
    bool        ExecSentenceAsyncResult(const string &sql);
    bool        ExecResource(const string &file);
    bool        FecthRow();
    string      ResultString(const string &name) const;
    float       ResultFloat(const string &name) const;
    uint64_t    ResultUInt64(const string &name) const;
    json        ResultJSON(const string &name) const;
    string      GetSchema() const;
    Database(const string &schema = "public");
    ~Database();
};