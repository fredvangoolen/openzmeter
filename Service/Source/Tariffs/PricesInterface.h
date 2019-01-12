/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class PricesInterface : public Database {
  private :
    typedef PricesInterface* (*ConstructorFunction)(const char *name, json_object *config);
    typedef void *(*ThreadFunc)(void *);
    typedef struct {
      const char          *Name;
      ConstructorFunction  Constructor;
    } Type_t;
  private :
    static vector<Type_t> pRegisteredTypes;
  protected :
    static bool RegisterType(const char *name, ConstructorFunction constructor);
  private :
    pthread_t  pThread;
    bool       pTerminate;
    uint64_t   pLastPrice;
  private :
    static void *Update_Thread(void *args);
  protected :
    PricesInterface(const char *id);
    void SavePrice(uint64_t hourend, float price);
    void StartThread();
  protected :
    virtual void CheckPrices(uint64_t from) = 0;
  public :
    static PricesInterface *Config(const char *name, json_object *config);
};