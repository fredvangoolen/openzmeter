// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <queue>
#include "cctz/time_zone.h"
#include "muParser/muParser.h"
#include "Database.h"
#include "HTTPServer.h"
#include "Tools.h"
#include "Analyzer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace cctz;
using namespace mu;
//----------------------------------------------------------------------------------------------------------------------
class Tariff final {
  public :
    enum Period_Concept_Type { CONCEPT_INFO, CONCEPT_POWER, CONCEPT_ENERGY, CONCEPT_GENERAL };
    typedef struct {
      string                     Description;
      string                     Variable;
      Period_Concept_Type        Type;
    } Period_Concept_t;
    typedef struct {
      float                      Contracted_Power;
      float                      Power_Cost;
      float                      Energy_Cost;

      float                      Active_Power;
      float                      Active_Energy;
      float                      Reactive_Power;
      float                      Reactive_Energy;
    } Period_Eval_Params_t;
    typedef struct {
      string                     Name;
      float                      DefaultPowerCost;
      float                      DefaultEnergyCost;
      string                     Color;
      Parser                     Script;
      map<string, float>         Variables;
      vector<Period_Concept_t>   Concepts;
//      Parser                   EnergyCost;
//      Parser                   PowerCost;
//      float                 ContractedPower;
//      float                 PowerCost;
//      float                 EnergyCost;
//      TariffPowerPlugin    *PowerModule;
//      TariffEnergyPlugin   *EnergyModule;
    } Period_t;
    typedef struct {
      string                     Name;
      string                     Color;
      uint8_t                    Quarters[96];
    } Day_t;


//    typedef struct {
//      float    Active;
//      float    Reactive;
//      uint8_t  Period;
//    } Sample_t;
//  private :

//    typedef struct {
//      string   Name;
//      uint64_t StartDate;
//      uint64_t EndDate;
//    } Holiday_t;
//    typedef struct {
//      string  Type;
//      float   Value;
//      string  Name;
//    } Tax_t;
  private :
    string                          pParseError;
    string                          pDescription;
    Period_Eval_Params_t            pPeriodEvalParams;
    vector<Period_t>                pPeriods;
    vector<Day_t>                   pDays;
    map<uint16_t, vector<uint8_t> > pYears;


//    bool              pValidTimeZone;
//    time_zone         pTimeZone;
//    string            pDescription;
//    uint8_t           pHolidayPeriod;
//    uint8_t           pWeekendPeriod;
//    uint64_t          pStartDate;
//    uint64_t          pEndDate;

//    vector<Season_t>  pSeasons;
//    vector<Holiday_t> pHolidays;
//    vector<Tax_t>     pTaxs;
//    queue<Sample_t>   pSamples;
//    vector<uint64_t>  pBillingCycles;
//    uint64_t          pFirstSampleTime;
//    uint64_t          pLastSampletime;
//    float             pSumPowerCost;
//    float             pSumEnergyCost;
  private :
    static float *VariableFactory(const char_type *name, void *params);
  private :
      uint8_t FindPeriod(uint64_t time);
  public :
    Tariff(const json &t);
    ~Tariff();
    bool   IsValid();
    string ParseError();

    json   GetTemplate();
    json   GetRate();


    json GetResume(uint64_t now);
    uint64_t StartDate();
    bool PushSample(const uint64_t time, const float active, const float reactive);
    bool PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower);
};
//----------------------------------------------------------------------------------------------------------------------
class TariffList final {
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
//class TariffPowerPlugin {
//  public :
//    typedef struct {
//      float    Contracted;
//      uint32_t Days;
//      float    Cost;
//      string   Other;
//      float    OtherCosts;
//    } CycleStats_t;
//  private :
//    typedef TariffPowerPlugin* (*Func_t)();
//    typedef struct {
//      string Name;
//      Func_t Instance;
//    } Type_t;
//  private :
//    static vector<Type_t> pPlugins;
//  protected :
//    static bool Register(const string name, const Func_t instance);
//  public :
//    static bool Contains(const string name);
//    static TariffPowerPlugin *Instance(const string name);
//  protected :
//    string       pName;
//    CycleStats_t pStats;
//  public :
//    string       Name();
//  public :
//    virtual ~TariffPowerPlugin();
//    virtual CycleStats_t CycleStats() = 0;
//    virtual void         Config(const float contracted, const float cost) = 0;
//    virtual void         PushSample(const float active, const float reactive) = 0;
//    virtual float        PopSample(const float active, const float reactive) = 0;
//};
//----------------------------------------------------------------------------------------------------------------------
//class TariffEnergyPlugin {
//  public :
//    typedef struct {
//      float    Consumed;
//      float    Cost;
//      string   Other;
//      float    OtherCosts;
//    } CycleStats_t;
//  private :
//    typedef TariffEnergyPlugin* (*Func_t)();
//    typedef struct {
//      string Name;
//      Func_t Instance;
//    } Type_t;
//  private :
//    static vector<Type_t> pPlugins;
//  protected :
//    static bool Register(string name, Func_t instance);
//  public :
//    static bool Contains(string name);
//    static TariffEnergyPlugin *Instance(string name);
//  protected :
//    string       pName;
//    CycleStats_t pStats;
//  public :
//    string       Name();
//  public :
//    virtual ~TariffEnergyPlugin();
//    virtual CycleStats_t CycleStats() = 0;
//    virtual void         Config(const float cost) = 0;
//    virtual void         PushSample(const float active, const float reactive) = 0;
//    virtual float        PopSample(const float active, const float reactive) = 0;
//};
//----------------------------------------------------------------------------------------------------------------------