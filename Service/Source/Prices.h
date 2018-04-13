/* ================================== zMeter ===========================================================================
 * File:     Prices.cpp
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#pragma once
#include "Database.h"
class Prices : private Database {
  private :
    bool      pTerminate;
    pthread_t pThread;
    uint64_t  pLastCheck;
    uint64_t  pLastPrice;
  private : 
    static void *Update_Thread(void *args);
  public :
    Prices(uint32_t interval);
    ~Prices();
};
