/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
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
    PGconn      *pDBConn;
    char       **pParams;
    const char  *pSchema;
    int          pParamsLen;
    int          pParamsAlloc;
    const char  *pSentence;
    PGresult    *pResult;
  private :
    bool Connect();
  public :
    void        PrepareSentence(const char *sentence);
    void        BindSingleUInt32(uint32_t param);
    void        BindSingleUInt64(uint64_t param);
    void        BindSingleFloat(float param);
    void        BindArrayArrayFloat(const vector< vector<float> > &params);
    void        BindArrayFloat(const vector<float> &params);
    void        BindArrayUInt16(const vector<uint16_t> &params);
    void        BindSingleBool(bool param);
    void        BindSingleString(const char *string, bool identifier = false);
    bool        ExecSentence();
    bool        ExecResource(const char *file);
    uint32_t    ResultRows();
    const char *ResultValue(uint32_t row, const char *name);
    bool        ExecSentence(const char *sql);
    char       *ExecSentenceJSON(const char* sql, size_t *len);
    const char *GetSchema();
    Database(const char *schema);
    ~Database();
  public :
    static bool Config(json_object *config);
};
