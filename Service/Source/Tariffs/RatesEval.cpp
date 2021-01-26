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
#include "RatesEval.h"
#include "Tools.h"
// ---------------------------------------------------------------------------------------------------------------------
RatesEval::RatesEval(const time_zone &tz, const uint64_t from, const uint64_t to, RatesEvalProvider &provider, const uint16_t serieMask) {
  pTimeZone     = tz;
  pFinished     = false;
  pLoadedRate   = NULL;
  pPriceModule  = NULL;
  pScript       = NULL;
  pEnergyScript = NULL;
  pPowerScript  = NULL;
  pSeriesMask   = serieMask;
  pNowDate      = Tools::GetTime64();
  pFrom         = (from / 3600000) * 3600000 + 899999;
  pTo           = (to / 3600000) * 3600000 + 899999;
  pProvider     = &provider;
  FindStart();
  pSample       = pProvider->InitSamples(pFrom, pTo);
}
RatesEval::~RatesEval() {
  UnloadRate();
}
json RatesEval::Headers() {
  if(pSeriesMask == 0) return nullptr;
  json Header = json::array();
  if(pSeriesMask & SERIE_TIMESTAMP)       Header.push_back("Time");
  if(pSeriesMask & SERIE_ACTIVE_POWER)    Header.push_back("Active_Power");
  if(pSeriesMask & SERIE_ACTIVE_ENERGY)   Header.push_back("Active_Energy");
  if(pSeriesMask & SERIE_REACTIVE_POWER)  Header.push_back("Reactive_Power");
  if(pSeriesMask & SERIE_REACTIVE_ENERGY) Header.push_back("Reactive_Energy");
  if(pSeriesMask & SERIE_COST)            Header.push_back("Cost");
  if(pSeriesMask & SERIE_POWER_COST)      Header.push_back("Power_Cost");
  if(pSeriesMask & SERIE_ENERGY_COST)     Header.push_back("Energy_Cost");
  if(pSeriesMask & SERIE_PERIOD)          Header.push_back("Period");
  if(pSeriesMask & SERIE_COLOR)           Header.push_back("Color");
  if(pSeriesMask & SERIE_CONTRACTED)      Header.push_back("Contracted");
  if(pSeriesMask & SERIE_BILL)            Header.push_back("Bill");
  return Header;
}
void RatesEval::FindStart() {
  pCurrent = pFrom;
  pLoadedRate = pProvider->InitRates(pFrom, pTo);
  if((pSeriesMask != 0) || (pLoadedRate == NULL)) return;
  uint64_t Time = pLoadedRate->pStartDate + 899999;
  for(uint16_t n = 0; n < pLoadedRate->pBillingCycles.size(); n++) {
    if(pFrom < pLoadedRate->pBillingCycles[n]) {
      pFrom    = Time;
      pCurrent = pFrom;
      return;
    }
    Time = pLoadedRate->pBillingCycles[n] + 900000;
  }
  pFrom    = Time;
  pCurrent = pFrom;
}
void RatesEval::LoadRate() {
  if(pPeriods.size() > 0) return;
  pYearPtr          = pLoadedRate->pYears.begin();
  pBillPtr          = pLoadedRate->pBillingCycles.begin();
  pLoadedPeriodDate = pLoadedRate->pStartDate;
  pPriceModule      = PriceSource::CreateInstance(pLoadedRate->pPriceSrc);
  pPriceModule->PrepareRange(pCurrent, (pBillPtr != pLoadedRate->pBillingCycles.end()) ? *pBillPtr : pLoadedRate->pEndDate);
  pPriceModule->GetParams(0, pPriceVariables);
  while((pBillPtr != pLoadedRate->pBillingCycles.end()) && (*pBillPtr < pCurrent)) {
    pLoadedPeriodDate = *pBillPtr;
    pBillPtr++;
  }
  for(uint16_t P = 0; P < pLoadedRate->pPeriods.size(); P++) {
    TariffScript *Script = new TariffScript(pLoadedRate->pPeriods[P].pScript);
    Script->Var("ContractedPower", &pContractedPower);
    Script->Var("PowerCost",       &pPowerCost);
    Script->Var("EnergyCost",      &pEnergyCost);
    Script->Var("ActivePower",     &pActivePower);
    Script->Var("ActiveEnergy",    &pActiveEnergy);
    Script->Var("ReactivePower",   &pReactivePower);
    Script->Var("ReactiveEnergy",  &pReactiveEnergy);
    pPeriods.push_back(Script);
  }
  pScript = new TariffScript(pLoadedRate->pScript);
  pScript->Var("Period",         &pPeriod);
  pScript->Var("ActivePower",    &pActivePower);
  pScript->Var("ActiveEnergy",   &pActiveEnergy);
  pScript->Var("ReactivePower",  &pReactivePower);
  pScript->Var("ReactiveEnergy", &pReactiveEnergy);
  for(uint16_t P = 0; P < pLoadedRate->pPeriods.size(); P++) {
    pScript->Var("ContractedPower_P" + to_string(P + 1), &pLoadedRate->pContracted[P]);
    pScript->Var("EnergyCost_P" + to_string(P + 1),      (pLoadedRate->pEnergyCost.size() == 0) ? &pEnergyCost : &pLoadedRate->pEnergyCost[P]);
    pScript->Var("PowerCost_P" + to_string(P + 1),       (pLoadedRate->pPowerCost.size() == 0)  ? &pPowerCost  : &pLoadedRate->pPowerCost[P]);
  }
  if(pLoadedRate->pEnergyCost.size() == 0) {
    pEnergyScript = new TariffScript(pLoadedRate->pEnergyCostScript);
    for(auto &Var : pPriceVariables) pEnergyScript->Var(Var.first, &Var.second);
    pEnergyScript->Var("Period", &pPeriod);
  }
  if(pLoadedRate->pPowerCost.size() == 0) {
    pPowerScript = new TariffScript(pLoadedRate->pPowerCostScript);
    for(auto &Var : pPriceVariables) pPowerScript->Var(Var.first, &Var.second);
    pPowerScript->Var("Period", &pPeriod);
  }
}
void RatesEval::UnloadRate() {
  if(pPriceModule != NULL) {
    delete pPriceModule;
    pPriceModule = NULL;
  }
  if(pScript != NULL) {
    delete pScript;
    pScript = NULL;
  }
  if(pEnergyScript != NULL) {
    delete pEnergyScript;
    pEnergyScript = NULL;
  }
  if(pPowerScript != NULL) {
    delete pPowerScript;
    pPowerScript = NULL;
  }
  for(uint16_t n = 0; n < pPeriods.size(); n++) delete pPeriods[n];
  pPeriods.clear();
  pPriceVariables.clear();
}
json RatesEval::HandleSample() {
  while(pLoadedRate != NULL) {
    if(pCurrent < pLoadedRate->pStartDate) break;
    if((pCurrent >= pLoadedRate->pStartDate) && (pCurrent <= pLoadedRate->pEndDate)) {
      LoadRate();
      break;
    }
    pLoadedRate = pProvider->NextRate();
    if(pLoadedRate == NULL) {
      if(pSeriesMask == 0) pFinished = true;
      return nullptr;
    }
  }
  json         Result = nullptr;
  civil_second Time   = convert(chrono::system_clock::from_time_t(pCurrent / 1000), pTimeZone);
  if(pPeriods.size() > 0) {
    while(Time.year() > pYearPtr->pYear) pYearPtr++;
    uint16_t Period = pLoadedRate->pDays[pYearPtr->pDays[get_yearday(civil_day(Time)) - 1]].pQuarters[Time.hour() * 4 + Time.minute() / 15];
    pPeriod = (Period + 1.0);
    if((pLoadedRate->pEnergyCost.size() == 0) || (pLoadedRate->pPowerCost.size() == 0)) pPriceModule->GetParams(pCurrent, pPriceVariables);
    pPeriod          = Period + 1.0;
    pColor           = pLoadedRate->pPeriods[Period].pColor;
    pContractedPower = pLoadedRate->pContracted[Period];
    pEnergyCost      = (pLoadedRate->pEnergyCost.size() == 0) ? pEnergyScript->Eval() : pLoadedRate->pEnergyCost[Period];
    pPowerCost       = (pLoadedRate->pPowerCost.size() == 0)  ? pPowerScript->Eval()  : pLoadedRate->pPowerCost[Period];
    pScript->Eval();
    pPeriods[Period]->Eval();
    pCost = pScript->Var("Cost") + pPeriods[Period]->Var("Cost");
    if(pCurrent == ((pBillPtr != pLoadedRate->pBillingCycles.end()) ? *pBillPtr : pLoadedRate->pEndDate)) {
      Result = (pSeriesMask == 0) ? BuildBill() : BuildSample(true);
      UnloadRate();
    } else {
      if(pSeriesMask != 0) Result = BuildSample(false);
    }
  } else {
    if(pSeriesMask != 0) Result = BuildSample(false);
  }
  pCurrent += 900000;
  return Result;
}
json RatesEval::BuildBill() {
  float EnergyTerm  = 0.0;
  float PowerTerm   = 0.0;
  float GeneralTerm = 0.0;
  json Row          = json::object();
  Row["From"]        = pLoadedPeriodDate;
  Row["To"]          = pCurrent;
  Row["Currency"]    = pLoadedRate->pCurrency;
  Row["PowerUnits"]  = pLoadedRate->pPowerUnits;
  Row["EnergyUnits"] = pLoadedRate->pEnergyUnits;
  Row["Name"]        = pLoadedRate->pName;
  Row["Description"] = pLoadedRate->pDescription;
  Row["Periods"]     = json::array();
  Row["Complete"]    = (pNowDate >= pCurrent);
  for(uint16_t P = 0; P < pLoadedRate->pPeriods.size(); P++) {
    json Period = json::array();
    for(uint16_t C = 0; C < pLoadedRate->pPeriods[P].pConcepts.size(); C++) {
      json Concept = json::object();
      Concept["Description"] = pLoadedRate->pPeriods[P].pConcepts[C].pDescription;
      if(pLoadedRate->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST) {
        float Cost = pPeriods[P]->Var(pLoadedRate->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pLoadedRate->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P]->Var(pLoadedRate->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pLoadedRate->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST_POWER) {
        float Cost = pPeriods[P]->Var(pLoadedRate->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_POWER";
        PowerTerm += Cost;
      } else {
        Concept["Type"] = "INFO";
        Concept["Value"] = pPeriods[P]->Var(pLoadedRate->pPeriods[P].pConcepts[C].pVariable);
      }
      Period.push_back(Concept);
    }
    Row["Periods"].push_back(Period);
  }
  Row["General"] = json::array();
  for(uint16_t C = 0; C < pLoadedRate->pConcepts.size(); C++) {
    json Concept = json::object();
    Concept["Description"] = pLoadedRate->pConcepts[C].pDescription;
    if(pLoadedRate->pConcepts[C].pType == TariffConcept::CONCEPT_COST) {
      float Cost = pScript->Var(pLoadedRate->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pLoadedRate->pConcepts[C].pType == TariffConcept::CONCEPT_COST_ENERGY) {
      float Cost = pScript->Var(pLoadedRate->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pLoadedRate->pConcepts[C].pType == TariffConcept::CONCEPT_COST_POWER) {
      float Cost = pScript->Var(pLoadedRate->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_POWER";
      PowerTerm += Cost;
    } else {
      Concept["Type"] = "INFO";
      Concept["Value"] = pScript->Var(pLoadedRate->pConcepts[C].pVariable);
    }
    Row["General"].push_back(Concept);
  }
  float Total = EnergyTerm + PowerTerm + GeneralTerm;
  Row["Taxes"] = json::array();
  for(uint16_t T = 0; T < pLoadedRate->pTaxes.size(); T++) {
    json Tax = json::object();
    if(pLoadedRate->pTaxes[T].pType == RateTax::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pLoadedRate->pTaxes[T].pValue / 100.0);
      Total += EnergyTerm * (pLoadedRate->pTaxes[T].pValue / 100.0);
    } else if(pLoadedRate->pTaxes[T].pType == RateTax::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pLoadedRate->pTaxes[T].pValue / 100.0);
      Total += PowerTerm * (pLoadedRate->pTaxes[T].pValue / 100.0);
    } else if(pLoadedRate->pTaxes[T].pType == RateTax::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pLoadedRate->pTaxes[T].pValue / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pLoadedRate->pTaxes[T].pValue / 100.0);
    } else if(pLoadedRate->pTaxes[T].pType == RateTax::TAX_FIXED) {
      Tax["Type"] = "FIXED_QUANTITY";
      Tax["Value"] = pLoadedRate->pTaxes[T].pValue;
      GeneralTerm += pLoadedRate->pTaxes[T].pValue;
      Total += pLoadedRate->pTaxes[T].pValue;
    } else {
      continue;
    }
    Tax["Description"] = pLoadedRate->pTaxes[T].pDescription;
    Row["Taxes"].push_back(Tax);
  }
  float Tmp = 0.0;
  for(uint16_t T = 0; T < pLoadedRate->pTaxes.size(); T++) {
    if(pLoadedRate->pTaxes[T].pType == RateTax::TAX_PERCENT) {
      json Tax = json::object();
      Tax["Description"] = pLoadedRate->pTaxes[T].pDescription;
      Tax["Type"] = "PERCENT_TOTAL";
      Tax["Value"] = Total * (pLoadedRate->pTaxes[T].pValue / 100.0);
      Row["Taxes"].push_back(Tax);
      Tmp += Total * (pLoadedRate->pTaxes[T].pValue / 100.0);
    }
  }
  Row["Total"] = Total + Tmp;
  return Row;
}
json RatesEval::BuildSample(const bool bill) {
  json Row = json::array();
  if(pSeriesMask & SERIE_TIMESTAMP)       Row.push_back(pCurrent);
  if(pSeriesMask & SERIE_ACTIVE_POWER)    Row.push_back(pActivePower);
  if(pSeriesMask & SERIE_ACTIVE_ENERGY)   Row.push_back(pActiveEnergy);
  if(pSeriesMask & SERIE_REACTIVE_POWER)  Row.push_back(pReactivePower);
  if(pSeriesMask & SERIE_REACTIVE_ENERGY) Row.push_back(pReactiveEnergy);
  if(pPeriods.size() == 0) {
    if(pSeriesMask & SERIE_COST)          Row.push_back(nullptr);
    if(pSeriesMask & SERIE_POWER_COST)    Row.push_back(nullptr);
    if(pSeriesMask & SERIE_ENERGY_COST)   Row.push_back(nullptr);
    if(pSeriesMask & SERIE_PERIOD)        Row.push_back(nullptr);
    if(pSeriesMask & SERIE_COLOR)         Row.push_back(nullptr);
    if(pSeriesMask & SERIE_CONTRACTED)    Row.push_back(nullptr);
  } else {
    if(pSeriesMask & SERIE_COST)          Row.push_back(pCost);
    if(pSeriesMask & SERIE_POWER_COST)    Row.push_back(pPowerCost);
    if(pSeriesMask & SERIE_ENERGY_COST)   Row.push_back(pEnergyCost);
    if(pSeriesMask & SERIE_PERIOD)        Row.push_back((uint16_t)pPeriod);
    if(pSeriesMask & SERIE_COLOR)         Row.push_back(pColor);
    if(pSeriesMask & SERIE_CONTRACTED)    Row.push_back(pContractedPower);
  }
  if(pSeriesMask & SERIE_BILL)            Row.push_back(bill);
  return Row;
}
json RatesEval::FetchRow() {
  json Row = nullptr;
  while((Row == nullptr) && (pFinished == false)) {
    uint64_t Time = UINT64_MAX;
    if(pSample != NULL) Time = (pSample->Time / 900000) * 900000 + 899999;
    if(Time > pCurrent) {
      pActivePower    = 0.0;
      pActiveEnergy   = 0.0;
      pReactivePower  = 0.0;
      pReactiveEnergy = 0.0;
      Row             = HandleSample();
    } else {
      pActivePower    = pSample->Active;
      pActiveEnergy   = pActivePower / 4.0;
      pReactivePower  = pSample->Reactive;
      pReactiveEnergy = pReactivePower / 4.0;
      Row             = HandleSample();
      pSample         = pProvider->NextSample();
    }
    if((pSeriesMask != 0) && (pCurrent == pTo)) pFinished = true;
    if((pSeriesMask == 0) && (pCurrent >= pTo)) {
      if((pLoadedRate != NULL) && (pLoadedRate->pEndDate == pCurrent)) pFinished = true;
      if((pLoadedRate != NULL) && (pBillPtr != pLoadedRate->pBillingCycles.end()) && (*pBillPtr == pCurrent)) pFinished = true;
      if((pLoadedRate == NULL) && (pCurrent >= pTo)) pFinished = true;
    }
  }
  return Row;
}
// ---------------------------------------------------------------------------------------------------------------------