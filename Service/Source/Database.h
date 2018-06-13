/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <inttypes.h>
#include <postgresql/libpq-fe.h>
#include <json-c/json.h>
//----------------------------------------------------------------------------------------------------------------------
class Database;
class Database {
  private :
    static bool  pInitialized;
    static char *pDatabaseName;
    static char *pHost;
    static int   pPort;
    static char *pUser;
    static char *pPassword;
  private :
    PGconn      *pDBConn;
    char       **pParams;
    char        *pSchema;
    int          pParamsLen;
    int          pParamsAlloc;
    const char  *pSentence;
    PGresult    *pResult;
  private :
    bool Connect();
  public :
    void  PrepareSentence(const char *sentence);
    void  BindSingleUInt32(uint32_t param);
    void  BindSingleUInt64(uint64_t param);
    void  BindSingleFloat(float param);
    void  BindArrayArrayFloat(float *params, int lenA, int lenB);
    void  BindArrayFloat(float *params, int len);
    void  BindArrayUInt16(uint16_t *params, int len);
    void  BindSingleBool(bool param);
    void  BindSingleString(const char *string);
    bool  ExecSentence();
    bool  ExecSentence(const char *sql);
    char *ExecSentenceJSON(const char* sql, size_t *len);
    Database(char *schema);
    ~Database();
  public :
    static void Config(json_object *config);
};
