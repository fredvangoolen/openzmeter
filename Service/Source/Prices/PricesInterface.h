/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class PricesInterface : public Database {
  protected :
    pthread_t  pThread;
    bool       pTerminate;
    uint64_t   pLastPrice;
  protected : 
    void SavePrice(uint64_t hourbegin, float price);
  public :
    PricesInterface(AnalyzerInterface *analyzer, const char *units);
    
    
    
};