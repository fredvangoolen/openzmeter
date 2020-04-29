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
#include <set>
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
class Rates final {
  friend class RatesEval;
  private :
    enum Concept_Type { CONCEPT_INFO, CONCEPT_COST_ENERGY, CONCEPT_COST_POWER, CONCEPT_COST };
    enum Tax_Type { TAX_PERCENT, TAX_PERCENT_POWER, TAX_PERCENT_ENERGY, TAX_PERCENT_ENERGY_POWER, TAX_FIXED };
    enum Price_Source { PRICES_NONE, PRICES_ESIOS };
    typedef struct {
      string                        Description;
      string                        Variable;
      Concept_Type                  Type;
    } Concept_t;
    typedef struct {
      string                        Name;
      float                         DefaultPowerCost;
      float                         DefaultEnergyCost;
      string                        Color;
      string                        Script;
      vector<Concept_t>             Concepts;
    } Period_t;
    typedef struct {
      string                        Name;
      string                        Color;
      uint8_t                       Quarters[96];
    } Day_t;
    typedef struct {
      uint16_t                      Year;
      uint8_t                       Days[366];
    } Year_t;
    typedef struct {
      Tax_Type                      Type;
      float                         Value;
      string                        Description;
    } Tax_t;
    typedef struct {
      string                        Name;
      string                        Description;
      string                        Currency;
      string                        PowerUnits;
      string                        EnergyUnits;
      string                        Script;
      vector<Tax_t>                 Taxs;
      vector<Concept_t>             Concepts;
      vector<Period_t>              Periods;
      vector<Day_t>                 Days;
      vector<Year_t>                Years;
      vector<float>                 Contracted;
      string                        PowerCostScript;
      vector<float>                 PowerCost;
      string                        EnergyCostScript;
      vector<float>                 EnergyCost;
      uint64_t                      StartDate;
      Price_Source                  PriceSource;
      uint64_t                      EndDate;
      vector<uint64_t>              BillingCycles;
    } Rate_t;

  private :
    vector<Rate_t>                  pRates;
    time_zone const                *pTimeZone;
  private :
    static float                    pTmpVar;
  public :
    static json CheckConfig(const json &t);
    static json CheckTariffConfig(const json &t);
    static json CheckRateConfig(const json &t);
  public :
    Rates(const json &t, const time_zone &timezone);
};
//----------------------------------------------------------------------------------------------------------------------
class RatesEval final {
  public :
    typedef struct {
      uint64_t        SampleTime;
      float           ActivePower;
      float           ReactivePower;
      float           Cost;
    } SampleOut_t;
    typedef struct {
      uint64_t        SampleTime;
      uint8_t         Period;
      string          Color;
      float           ContractedPower;
    } PeriodOut_t;
  private :
    const Rates      *pParent;
    uint64_t          pFrom;
    uint64_t          pTo;
    uint64_t          pCurrent;
  private:
    function<void(const PeriodOut_t &)> pPeriodCallback;
    function<void(const SampleOut_t &)> pSampleCallback;
    function<void(const json &)>        pBillCallback;
  private :
    uint64_t FindStart(const uint64_t from);
    uint64_t FindEnd(const uint64_t to);
  public :
    RatesEval(const Rates *parent, const uint64_t from, const uint64_t to, const bool precise);
    void     PushSample(const uint64_t time, const float active, const float reactive);
    void     Finish();
    void     SetPeriodCallback(function<void(const PeriodOut_t &)> periodOut);
    void     SetSampleCallback(function<void(const SampleOut_t &)> sampleOut);
    void     SetBillCallback(function<void(const json &)> billOut);
    uint64_t StartDate();
    uint64_t EndDate();
 };
//----------------------------------------------------------------------------------------------------------------------
class PriceModule {
  public :
    static PriceModule *CreateInstance(const string &name);
  public :
    PriceModule(const uint64_t from, const uint64_t to);
    virtual ~PriceModule();
    virtual void GetParams(const uint64_t sampletime, map<string, float> &vars);
};
//----------------------------------------------------------------------------------------------------------------------







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



//----------------------------------------------------------------------------------------------------------------------