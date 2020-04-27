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
class Tariff {
  protected :
    enum Concept_Type { CONCEPT_INFO, CONCEPT_COST_ENERGY, CONCEPT_COST_POWER, CONCEPT_COST };
    typedef struct {
      string                        Description;
      string                        Variable;
      Concept_Type                  Type;
    } Concept_t;
    typedef struct {
      float                         Contracted_Power;
      float                         Power_Cost;
      float                         Energy_Cost;
      float                         Active_Power;
      float                         Active_Energy;
      float                         Reactive_Power;
      float                         Reactive_Energy;
    } Eval_Params_t;
    typedef struct {
      string                        Name;
      float                         DefaultPowerCost;
      float                         DefaultEnergyCost;
      string                        Color;
      Parser                        Script;
      map<string, float>            Variables;
      vector<Concept_t>             Concepts;
    } Period_t;
    typedef struct {
      string                        Name;
      string                        Color;
      uint8_t                       Quarters[96];
    } Day_t;
  protected :
    string                          pParseError;
    string                          pDescription;
    string                          pCurrency;
    string                          pPowerUnits;
    string                          pEnergyUnits;
    Eval_Params_t                   pEvalParams;
    Parser                          pScript;
    map<string, float>              pVariables;
    vector<Concept_t>               pConcepts;
    vector<Period_t>                pPeriods;
    vector<Day_t>                   pDays;
    map<uint16_t, vector<uint8_t> > pYears;
  private :
    static float *VariableFactory(const char_type *name, void *params);
  public :
    Tariff(const json &t);
    virtual ~Tariff();
    bool     IsValid();
    string   ParseError();
    json     ToJSON();
};
//----------------------------------------------------------------------------------------------------------------------
class Rate final : private Tariff {
  private :
    enum Tax_Type { TAX_PERCENT, TAX_PERCENT_POWER, TAX_PERCENT_ENERGY, TAX_PERCENT_ENERGY_POWER, FIXED };
    enum Price_Source { PRICES_NONE, PRICES_ESIOS };
    typedef struct {
      Tax_Type  Type;
      float     Value;
      string    Name;
    } Tax_t;
//  typedef struct {
//      Parser                   EnergyCost;
//      Parser                   PowerCost;
//      float                 ContractedPower;
//      float                 PowerCost;
//      float                 EnergyCost;
//      TariffPowerPlugin    *PowerModule;
//      TariffEnergyPlugin   *EnergyModule;
//  } PeriodExt_t;
//    typedef struct {
//      float    Active;
//      float    Reactive;
//      uint8_t  Period;
//    } Sample_t;
  private :
    vector<Tax_t>     pTaxs;
    string            From;
    string            To;
    vector<string>    pBillingCycles;
    bool              pContractedLink;
    vector<float>     pContracted;
    Parser            pPowerCostScript;
    vector<float>     pPowerCosts;
    Parser            pEnergyCostScript;
    vector<float>     pEnergyCosts;




//    queue<Sample_t>   pSamples;

//    uint64_t          pFirstSampleTime;
//    uint64_t          pLastSampletime;
//    float             pSumPowerCost;
//    float             pSumEnergyCost;

//    uint8_t FindPeriod(uint64_t time);
//    json GetResume(uint64_t now);
//    uint64_t StartDate();
//    bool PushSample(const uint64_t time, const float active, const float reactive);
//    bool PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower);
  public :
    Rate(const json &t);
    ~Rate();
    json ToJSON();

};
//----------------------------------------------------------------------------------------------------------------------
class RateList final {
  private :
    list<Rate>      pList;
    time_zone       pTimeZone;
  private :
//    bool PushTariff(const json &tariff);
  public :
    RateList(const json &list, const string &timezone, uint64_t &from, uint64_t &to, const bool &precise);
    ~RateList();
//    void CalcBillRanges(uint64_t &from, uint64_t &to);
    bool EvaluateSample(const uint64_t &sampletime, const float &active, const float &reactive);

//    Tariff   *Find(const uint64_t time);
//    uint64_t  BillFrom(const uint64_t);
//    uint64_t  BillEnd(const uint64_t);
//    uint64_t  BillTo(const uint64_t to);
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