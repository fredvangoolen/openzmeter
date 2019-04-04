/* ================================== zMeter ===========================================================================
 * File:     Tariff.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <queue>
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerInterface.h"
#include "TariffPlugin.h"
//----------------------------------------------------------------------------------------------------------------------
class Tariff {
  friend class TariffList;
  public :
    typedef struct {
      uint64_t Time;
      float    Active;
      float    Reactive;
      uint8_t  Period;
      string   Color;
    } Sample_t;
  private :
    typedef struct {
      float                 ContractedPower;
      float                 PowerCost;
      float                 EnergyCost;
      string                PowerMode;
      string                ReactiveMode;
      string                Color;
      float                 Active;
      float                 Reactive;
      TariffPowerPlugin   *PowerModule;
      TariffEnergyPlugin *EnergyModule;
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
      float   Accum;
    } Tax_t;
    typedef struct {
      string Name;
      float  Value;
    } Bill_t;
  private :
    string            pDescription;
    uint8_t           pHolidayPeriod;
    uint8_t           pWeekendPeriod;
    uint64_t          pStartDate;
    uint64_t          pEndDate;
    vector<Period_t>  pPeriods;
    vector<Season_t>  pSeasons;
    vector<Holiday_t> pHolidays;
    vector<Tax_t>     pTaxs;
    queue<Sample_t>   pSamples;
    vector<uint64_t>  pBillingCycles;
    vector<Bill_t>    pBillConcepts;
  private :
    float   CalcTaxs();
    uint8_t FindPeriod(uint64_t time);
  public :
    Tariff(uint64_t start, uint64_t end);
    ~Tariff();
    bool SetTemplate(json t);
    bool SetConfig(json t);
    json GetTemplate();
    json GetConfig();
    bool     PushSample(uint64_t time, float active, float reactive);
    bool     PopSample(Sample_t &sample, float &cost);
    uint32_t Samples();
};
class TariffList {
  private :
    vector<Tariff*> pList;
  public :
    ~TariffList();
    bool      PushTariff(json tariff);
    Tariff   *Find(uint64_t time);
    void      AppendSample(uint64_t time, float active, float reactive);
    uint64_t  BillFrom(uint64_t);
    uint64_t  BillEnd(uint64_t);
    uint64_t  BillTo(uint64_t to);
};