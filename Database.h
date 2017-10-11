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
    PGconn *DBConn;
  protected :
    void PrepareSentence(char *sentence);
    void InsertSingleParam();
    void InsertArrayParam();
    bool ExecSentence();
    bool ExecSentence(const char *sql);
  public :
    Database();
    ~Database();
};
