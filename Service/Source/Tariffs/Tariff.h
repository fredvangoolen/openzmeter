/* ================================== zMeter ===========================================================================
 * File:     Tariff.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <queue>
#include <cctz/time_zone.h>
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin;
class TariffEnergyPlugin;
class Tariff {
  friend class TariffList;
  public :
    typedef struct {
      float    Active;
      float    Reactive;
      uint8_t  Period;
    } Sample_t;
  private :
    typedef struct {
      float                 ContractedPower;
      float                 PowerCost;
      float                 EnergyCost;
      string                Color;
      TariffPowerPlugin    *PowerModule;
      TariffEnergyPlugin   *EnergyModule;
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
    time_zone         pTimeZone;
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
    uint64_t          pSampletime;
    float             pSumPowerCost;
    float             pSumEnergyCost;
  private :
//    float   CalcTaxs();
    uint8_t FindPeriod(uint64_t time);
  public :
    Tariff(uint64_t start, uint64_t end, string tz);
    ~Tariff();
    bool SetTemplate(json t);
    bool SetConfig(json t);
    json GetTemplate();
    json GetConfig();
    json GetResume();
    bool PushSample(const uint64_t time, const float active, const float reactive);
    bool PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower);
};
class TariffList {
  private :
    vector<Tariff*> pList;
    time_zone       pTimeZone;
  private :
    bool PushTariff(const json tariff);
  public :
    TariffList(const json list, const string timezone);
    ~TariffList();
    Tariff   *Find(const uint64_t time);
    uint64_t  BillFrom(const uint64_t);
    uint64_t  BillEnd(const uint64_t);
    uint64_t  BillTo(const uint64_t to);
};
