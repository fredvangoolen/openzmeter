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
  private :
    PGconn      *pDBConn;
    string       pSchema;
    PGresult    *pResult;
  private :
    bool   Connect();
    void   ClearResult();
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
    bool        ExecSentenceNoResult(string sql);
    bool        ExecSentenceAsyncResult(string sql);
    bool        ExecResource(string file);
    bool        FecthRow();
    string      ResultString(string name);
    float       ResultFloat(string name);
    uint64_t    ResultUInt64(string name);
    json        ResultJSON(string name);
    string      GetSchema();
    Database(string schema = "public");
    virtual ~Database();
  public :
    static bool Config(json config);
};
