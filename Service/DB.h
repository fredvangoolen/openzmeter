/* ================================== zMeter ===========================================================================
 * File:     DB.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <postgresql/libpq-fe.h>
class DB {
  private :
    static bool Initialized;
  private :
    PGconn *Database;
  private :
    bool Initialize();
  public :
    static DB *getInstance();
    ~DB();
    bool ExecDBSentence(const char *sql);
};
