/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <inttypes.h>
#include <postgresql/libpq-fe.h>
class Database {
  private :
    PGconn      *DBConn;
    char       **Params;
    int          ParamsLen;
    int          ParamsAlloc;
    const char  *Sentence;
    PGresult    *Result;
  public :
    static bool PrepareDB();
  public :
    void  PrepareSentence(const char *sentence);
    void  BindSingleUInt32(uint32_t param);
    void  BindSingleUInt64(uint64_t param);
    void  BindSingleFloat(float param);
    void  BindArrayFloat(float *params, int len);
    void  BindSingleBool(bool param);
    void  BindSingleString(const char *string);
    bool  ExecSentence();
    bool  ExecSentence(const char *sql);
    char *ExecSentenceJSON(const char* sql, int *len);
    Database();
    ~Database();
};
