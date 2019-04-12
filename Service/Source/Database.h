/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <sstream>
#include <inttypes.h>
#include <libpq-fe.h>
#include <json.hpp>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Database {
  private :
    static string pConnectionString;
//    static char *pDatabaseName;
//    static char *pHost;
//    static int   pPort;
//    static char *pUser;
//    static char *pPassword;
  private :
    PGconn      *pDBConn;
    string       pSchema;
    PGresult    *pResult;
  private :
    bool   Connect();
    void   ClearResult();
//    string BuildQueryString();
  public :
    string      Bind(uint32_t param);
    string      Bind(uint64_t param);
    string      Bind(float param);
    string      Bind(const vector< vector<float> > &params);
    string      Bind(const vector<float> &params);
    string      Bind(const vector<uint16_t> &params);
    string      Bind(bool param);
    string      BindID(string string);
    string      Bind(string string);
    bool        ExecSentence(string sql);
    bool        ExecResource(string file);
    uint32_t    ResultRows();
    string      ResultString(uint32_t row, string name);
    float       ResultFloat(uint32_t row, string name);
    uint64_t    ResultUInt64(uint32_t row, string name);
    json        ResultJSON(uint32_t row, string name);
    string      GetSchema();
    Database(string schema = "public");
    virtual ~Database();
  public :
    static bool Config(json config);
};
