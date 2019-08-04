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
#include "Analyzers.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin;
class TariffEnergyPlugin;
class Tariff {
    friend class TariffList;
  private :
    static void GetTariffsCallback(HTTPRequest &req);
    static void GetTariffCallback(HTTPRequest &req);
    static void SetTariffCallback(HTTPRequest &req);
    static void DelTariffCallback(HTTPRequest &req);
    static void AddTariffCallback(HTTPRequest &req);
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
    uint64_t          pFirstSampleTime;
    uint64_t          pLastSampletime;
    float             pSumPowerCost;
    float             pSumEnergyCost;
  private :
    uint8_t FindPeriod(uint64_t time);
  public :
    Tariff(const uint64_t start, const uint64_t end, const string &tz);
    ~Tariff();
    bool SetTemplate(json t);
    bool SetConfig(json t);
    json GetTemplate();
    json GetConfig();
    json GetResume(uint64_t now);
    uint64_t StartDate();
    bool PushSample(const uint64_t time, const float active, const float reactive);
    bool PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower);
  public :
    
};
//----------------------------------------------------------------------------------------------------------------------
class TariffList {
  private :
    vector<Tariff*> pList;
    time_zone       pTimeZone;
  private :
    bool PushTariff(const json &tariff);
  public :
    TariffList(const json &list, const string &timezone);
    ~TariffList();
    Tariff   *Find(const uint64_t time);
    uint64_t  BillFrom(const uint64_t);
    uint64_t  BillEnd(const uint64_t);
    uint64_t  BillTo(const uint64_t to);
};
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin {
  public :
    typedef struct {
      float    Contracted;
      uint32_t Days;
      float    Cost;
      string   Other;
      float    OtherCosts;
    } CycleStats_t;
  private :
    typedef TariffPowerPlugin* (*Func_t)();
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
  protected :
    static bool Register(const string name, const Func_t instance);
  public :
    static bool Contains(const string name);
    static TariffPowerPlugin *Instance(const string name);
  protected :
    string       pName;
    CycleStats_t pStats;
  public :
    string       Name();
  public :
    virtual ~TariffPowerPlugin();
    virtual CycleStats_t CycleStats() = 0;
    virtual void         Config(const float contracted, const float cost) = 0;
    virtual void         PushSample(const float active, const float reactive) = 0;
    virtual float        PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------
class TariffEnergyPlugin {
  public :
    typedef struct {
      float    Consumed;
      float    Cost;
      string   Other;
      float    OtherCosts;
    } CycleStats_t;
  private :
    typedef TariffEnergyPlugin* (*Func_t)();
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
  protected :
    static bool Register(string name, Func_t instance);
  public :
    static bool Contains(string name);
    static TariffEnergyPlugin *Instance(string name);
  protected :
    string       pName;
    CycleStats_t pStats;
  public :
    string       Name();
  public :
    virtual ~TariffEnergyPlugin();
    virtual CycleStats_t CycleStats() = 0;
    virtual void         Config(const float cost) = 0;
    virtual void         PushSample(const float active, const float reactive) = 0;
    virtual float        PopSample(const float active, const float reactive) = 0;
};
//----------------------------------------------------------------------------------------------------------------------