/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <queue>
#include <sstream>
#include <inttypes.h>
#include <libpq-fe.h>
#include <json-c/json.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class Database {
  private :
    static char *pDatabaseName;
    static char *pHost;
    static int   pPort;
    static char *pUser;
    static char *pPassword;
  private :
    PGconn        *pDBConn;
    string         pSchema;
//    string         pSentence;
//    queue<string> pParams;

//    char       **pParams;

//    int          pParamsLen;
//    int          pParamsAlloc;
    PGresult    *pResult;
  private :
    bool   Connect();
    string BuildQueryString();
  public :
//    void        PrepareSentence(string sentence);
    string        Bind(uint32_t param);
    string        Bind(uint64_t param);
    string        Bind(float param);
    string        Bind(const vector< vector<float> > &params);
    string        Bind(const vector<float> &params);
    string        Bind(const vector<uint16_t> &params);
    string        Bind(bool param);
    string        BindID(string string);
    string        Bind(string string);
//    bool        ExecSentence();
    bool        ExecSentence(string sql);
    //string      ExecSentenceFirstCell();
    bool        ExecResource(string file);

    uint32_t    ResultRows();
    const char *ResultValue(uint32_t row, const char *name);
    
    string      GetSchema();
    Database(string schema = "public");
    ~Database();
  public :
    static bool Config(json_object *config);
};
