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
      uint16_t                      Quarters[96];
    } Day_t;
    typedef struct {
      uint16_t                      Year;
      uint16_t                      Days[366];
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
      string                        PriceSource;
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
class PriceModule {
  protected :
    typedef struct {
      PriceModule* (*Constructor)();
      json         (*Info)();
    } Module_t;
  protected :
    static map<string, Module_t> pModules;
  protected :
    PriceModule();
  public :
    static PriceModule *CreateInstance(const string &name);
    static json         GetVarsInfo();
  public :
    virtual ~PriceModule();
    virtual void GetParams(const uint64_t sampletime, map<string, float> &vars) = 0;
    virtual void PrepareRange(const uint64_t from, const uint64_t to) = 0;
};
//----------------------------------------------------------------------------------------------------------------------
class None_PriceModule final : public PriceModule {
  private :
    static bool         pRegistered;
  private :
    static bool         Register();
    static PriceModule *CreateInstance();
    static json         GetVarsInfo();
  public :
    void GetParams(const uint64_t sampletime, map<string, float> &vars) override;
    void PrepareRange(const uint64_t from, const uint64_t to) override;
};
//----------------------------------------------------------------------------------------------------------------------
class RatesEval final {
  public :
    typedef struct {
      uint64_t                          SampleTime;
      float                             ActivePower;
      float                             ActiveEnergy;
      float                             ReactivePower;
      float                             ReactiveEnergy;
      float                             Cost;
      uint16_t                          Period;
      float                             PeriodF;
      string                            Color;
      float                             ContractedPower;
      float                             EnergyCost;
      float                             PowerCost;
    } SampleOut_t;
  private :
    typedef struct {
      Parser             Script;
      map<string, float> Variables;
    } LoadedPeriod_t;
  private :
    Rates                              *pParent;
    uint64_t                            pFrom;
    uint64_t                            pTo;
    uint64_t                            pNowDate;
    uint64_t                            pLoadedPeriodDate;
    uint64_t                            pCurrent;
    vector<Rates::Rate_t>::iterator     pRatePtr;
    vector<Rates::Year_t>::iterator     pYearPtr;
    vector<uint64_t>::iterator          pBillPtr;
    PriceModule                        *pPriceModule;
    SampleOut_t                         pSampleOut;
    Parser                              pEnergyScript;
    Parser                              pPowerScript;
    Parser                              pScript;
    map<string, float>                  pVariables;
    map<string, float>                  pPriceVariables;
    vector<LoadedPeriod_t>              pPeriods;
  private :
    static float *VariableFactory(const char_type *name, void *params);
  private:
    function<void(const SampleOut_t &)> pSampleCallback;
    function<void(const json &)>        pBillCallback;
  private :
    void     LoadRate();
    void     UnloadRate();
    json     BuildBill(bool complete);
    void     HandleSample(const float active, const float reactive);
    uint64_t FindStart(const uint64_t from, bool precise);
    uint64_t FindEnd(const uint64_t to, bool precise);
  public :
    RatesEval(Rates *parent, const uint64_t from, const uint64_t to, const bool precise);
    ~RatesEval();
    void     PushSample(const uint64_t time, const float active, const float reactive);
    void     Finish();
//    void     SetPeriodCallback(function<void(const PeriodOut_t &)> periodOut);
    void     SetSampleCallback(function<void(const SampleOut_t &)> sampleOut);
    void     SetBillCallback(function<void(const json &)> billOut);
    uint64_t StartDate();
    uint64_t EndDate();
 };
//----------------------------------------------------------------------------------------------------------------------