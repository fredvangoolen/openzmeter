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
#include <stdint.h>
#include <string>
#include <map>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
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
    vector<Rate>::iterator              pRatePtr;
    vector<RateYear>::iterator          pYearPtr;
    vector<uint64_t>::iterator          pBillPtr;
    PriceSource                        *pPriceModule;
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
 // ---------------------------------------------------------------------------------------------------------------------