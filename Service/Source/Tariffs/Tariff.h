/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.h
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <queue>
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "AnalyzerInterface.h"
//----------------------------------------------------------------------------------------------------------------------
class TariffPowerPlugin {
  private :
    typedef TariffPowerPlugin* (*Func_t)();
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
    static bool           pRegistered;
  private :
    static TariffPowerPlugin *Instance();
  public :
    static bool Register(string name, Func_t instance);
    static bool Contains(string name);
    static TariffPowerPlugin *Instance(string name);
  public :
    virtual ~TariffPowerPlugin();
    virtual void Reset();
    virtual void PushSample(float);
};
class TariffReactivePlugin {
  private :
    typedef TariffReactivePlugin* (*Func_t)();
    typedef struct {
      string Name;
      Func_t Instance;
    } Type_t;
  private :
    static vector<Type_t> pPlugins;
    static bool           pRegistered;
  private :
    static TariffReactivePlugin *Instance();
  public :
    static bool Register(string name, Func_t instance);
    static bool Contains(string name);
    static TariffReactivePlugin *Instance(string name);
  public :
    virtual ~TariffReactivePlugin();
    virtual void Reset();
    virtual void PushSample(float);
};
class TariffPowerPluginNONE {
  private :
    static TariffPowerPlugin *Instance();
  public :
    void Reset();
};
class Tariff {
  friend class TariffList;
  public :
    typedef struct {
      uint64_t Time;
      float    Active;
      float    Reactive;
    } Sample_t;
  private :
    typedef struct {
      float                 ContractedPower;
      float                 PowerCost;
      float                 EnergyCost;
      string                PowerMode;
      string                ReactiveMode;
      string                Color;
      TariffPowerPlugin    *PowerModule;
      TariffReactivePlugin *ReactiveModule;
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
    vector<uint64_t>  pBillingCycle;
    queue<Sample_t>   pSamples;
    string            pDescription;
    uint8_t           pHolidayPeriod;
    uint8_t           pWeekendPeriod;
    vector<uint64_t>  BillingCycles;
  private :
    static uint64_t BillStart(json tariffs, uint64_t timestamp);
    static uint64_t BillEnd(json tariffs, uint64_t timestamp);
    static uint32_t FindTariff(json tariffs, uint64_t timestamp);
  public :
    Tariff(uint64_t start, uint64_t end);
    ~Tariff();
    bool SetTemplate(json t);
    bool SetConfig(json t);
    json GetTemplate();
    json GetConfig();
    bool     PushSample(uint64_t time, float active, float reactive);
    bool     PopSample(Sample_t &sample);
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