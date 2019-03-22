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
class Tariff {
  friend class TariffList;
  private :
    typedef struct {
      float  Contracted_Power;
      string PowerMode;
      string ReactiveMode;
      string Color;
    } Period_t;
    typedef struct {
      string   Name;
      uint64_t StartDate;
      uint64_t EndDate;
      uint8_t  DayPeriods[24];
    } Season_t;
    typedef struct {
      string   Name;
      uint64_t StartDate;
      uint64_t EndDate;    
    } Holiday_t;
    typedef struct {
      string  Type;
      float   Value;
      string  Name;
    } Tax_t;
  private :
    uint64_t          pStartDate;
    uint64_t          pEndDate;
    vector<Period_t>  pPeriods;
    vector<Season_t>  pSeasons;
    vector<Holiday_t> pHolidays;
    vector<Tax_t>     pTaxs;
    string            pDescription;
    uint8_t           pHolidayPeriod;
    uint8_t           pWeekendPeriod;
    
    vector<uint64_t>  BillingCycles;
  private :
    static uint64_t BillStart(json tariffs, uint64_t timestamp);
    static uint64_t BillEnd(json tariffs, uint64_t timestamp);
    static uint32_t FindTariff(json tariffs, uint64_t timestamp);
  private :
//    bool  LoadTemplate(json t);
//    bool  LoadConfig(json t);    
  public :
//    static Tariff *FromJSON(uint64_t start, uint64_t end, json t);
//    static Tariff *FromJSON(uint64_t start, uint64_t end, json temp, json conf);
  public :
    Tariff(uint64_t start, uint64_t end);
    bool SetTemplate(json t);
    bool SetConfig(json t);
    json GetTemplate();
    json GetConfig();
//    static Tariff *FromJSON(uint64_t start, uint64_t end, json temp, json conf);

    //    uint8_t  FindPeriod(json tariff, uint64_t timestamp);    
//    bool     IsBillDate(json tariff, uint64_t timestamp);
    bool     PushSample(uint64_t time, float active, float reactive);
    
    






};
class TariffList {
  private :
    vector<Tariff*> pList;
  public :
    TariffList(json array);
    ~TariffList();
    Tariff   *Find(uint64_t time);
    void      AppendSample(uint64_t time, float active, float reactive);
    uint64_t  BillFrom(uint64_t);
    uint64_t  BillEnd(uint64_t);
    uint64_t  BillTo(uint64_t to);
};