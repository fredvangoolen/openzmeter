/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class TariffsInterface {
  private :
    typedef struct {
    
    } Period_t;
    typedef struct {
    
    } Season_t;
    typedef struct {
    
    } Holiday_t;
    string Owner;
    string Country;
    vector<Period_t>  Periods;
    vector<Season_t>  Seasons;
    vector<Holiday_t> Holidays;
  private :
    static bool Registered;
  private :
    static bool Register();
    static void GetPricesCallback(HTTPRequest &req);
    static void GetTariffsCallback(HTTPRequest &req);
    static void SetTariffsCallback(HTTPRequest &req);
  private :
    static uint64_t BillStart(json tariffs, uint64_t timestamp);
    static uint64_t BillEnd(json tariffs, uint64_t timestamp);
    static uint32_t FindTariff(json tariffs, uint64_t timestamp);
  public :
    static TariffsInterface *FromJSON(json details);
    static TariffsInterface *FromJSON(json details, json config);
  public :
    uint8_t  FindPeriod(json tariff, uint64_t timestamp);    
    bool     IsBillDate(json tariff, uint64_t timestamp);
    
    





//
//  private :
//    typedef PricesInterface* (*ConstructorFunction)(const char *name, json_object *config);
//    typedef void *(*ThreadFunc)(void *);
//    typedef struct {
//      const char          *Name;
//      ConstructorFunction  Constructor;
//    } Type_t;
//  private :
//    static vector<Type_t> pRegisteredTypes;
//  protected :
//    static bool RegisterType(const char *name, ConstructorFunction constructor);
//  private :
//    pthread_t  pThread;
//    bool       pTerminate;
//    uint64_t   pLastPrice;
//  private :
//    static void *Update_Thread(void *args);
//  protected :
//    PricesInterface(const char *id);
//    void SavePrice(uint64_t hourend, float price);
//    void StartThread();
//  protected :
//    virtual void CheckPrices(uint64_t from) = 0;
//  public :
//    static PricesInterface *Config(const char *name, json_object *config);
};