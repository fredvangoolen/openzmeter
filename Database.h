/* ================================== zMeter ===========================================================================
 * File:     Database.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <postgresql/libpq-fe.h>
class Database {
  private :
    static bool Initialized;
  private :
    PGconn      *DBConn;
    char       **Params;
    int          ParamsLen;
    int          ParamsAlloc;
    const char  *Sentence;
  protected :
    void PrepareSentence(const char *sentence);
    void BindSingleFloat(float param);
    void BindArrayFloat(float *params, int len);
    bool ExecSentence();
    bool ExecSentence(const char *sql);
  public :
    Database();
    ~Database();
};
