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
#include "muParser/muParser.h"
#include "Rate.h"
#include "PriceSource.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace mu;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class RatesEval final {
  public :
//    enum SeriesMask {
//      SERIE_TIMESTAMP       = 0x0001,
//      SERIE_ACTIVE_POWER    = 0x0002,
//      SERIE_ACTIVE_ENERGY   = 0x0004,
//      SERIE_REACTIVE_POWER  = 0x0008,
//      SERIE_REACTIVE_ENERGY = 0x0010,
//      SERIE_COST            = 0x0020,
//      SERIE_PERIOD          = 0x0040,
//      SERIE_COLOR           = 0x0080,
//      SERIE_CONTRACTED      = 0x0100,
//      SERIE_BILL            = 0x0200,
//      SERIE_ENERGY_COST     = 0x0400,
//      SERIE_POWER_COST      = 0x0800
//    };
  private :
    typedef struct {
      Parser             Script;
      map<string, float> Variables;
    } LoadedPeriod_t;
    typedef function<uint64_t(const uint64_t &currenttime,  float &active, float &reactive)> SampleFunc;
  private :
    bool                          pFinished;          //Evaluation end flag
    time_zone                     pTimeZone;          //Current datazone
    uint64_t                      pSampleTime;        //Current sample time
    float                         pSampleActive;      //Current sample active power
    float                         pSampleReactive;    //Current sample reactive power
    uint64_t                      pTo;                //Requested period end
    vector<string>                pSeries;            //Output series or empty for bill mode
    uint64_t                      pNowDate;           //Current system time
    uint64_t                      pCurrent;           //Current evaluation time
    uint64_t                      pLoadedPeriodDate;  //Active bill period start
    vector<Rate>                  pRates;             //Avaliable rate list
    vector<Rate>::iterator        pRatePtr;           //Current rate ptr (Only active if pPeriods.size > 0)
    vector<TariffYear>::iterator  pYearPtr;           //Active rate year
    vector<uint64_t>::iterator    pBillPtr;           //Active rate bill date
    vector<TariffScript*>         pPeriods;           //Loaded rate period scripts
    TariffScript                 *pScript;            //Loaded rate script
    PriceSource                  *pPriceModule;       //Active price module (if any)
    TariffScript                 *pEnergyScript;      //Energy price calculator script (if any)
    TariffScript                 *pPowerScript;       //Power price calculator script (if any)
    map<string, float>            pPriceVariables;    //List of avaliable variables from price module
    float                         pActivePower;       //Evaluated period active power
    float                         pActiveEnergy;      //Evaluated period active energy
    float                         pReactivePower;     //Evaluated period reactive power
    float                         pReactiveEnergy;    //Evaluated period reactive energy
    float                         pCost;              //Evaluated period total cost
    float                         pPeriod;            //Evaluated period number (base 1)
    string                        pColor;             //Evaluated period decoration color
    float                         pContractedPower;   //Evaluated period contracted power
    float                         pEnergyCost;        //Evaluated period energy cost
    float                         pPowerCost;         //Evaluated period power cost
    SampleFunc                    pGetSample;         //Function to retrieve new samples
  private :
    void LoadPeriod();
    void UnloadPeriod();
    json HandleSample();
    json BuildBill();
    json BuildSample(const bool bill);
  public :
    RatesEval(const time_zone &tz, const uint64_t from, const uint64_t to, const vector<Rate> &rates, SampleFunc getSample, const vector<string> &series = {});
    ~RatesEval();
    json Headers();
    json FetchRow();
};
// ---------------------------------------------------------------------------------------------------------------------