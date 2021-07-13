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
RatesEval::RatesEval(const time_zone &tz, const uint64_t from, const uint64_t to, const vector<Rate> &rates, RatesEval::SampleFunc getSample, const vector<string> &series) {
  pTimeZone     = tz;
  pFinished     = false;
  pGetSample    = getSample;
  pPriceModule  = NULL;
  pSeries       = series;
  pRates        = rates;
  pScript       = NULL;
  pEnergyScript = NULL;
  pPowerScript  = NULL;
  pNowDate      = Tools::GetTime64();
  pCurrent      = (from / 3600000) * 3600000 + 899999;
  pTo           = (to / 3600000) * 3600000 + 899999;
  for(pRatePtr = pRates.begin(); (pRatePtr != pRates.end()) && (pRatePtr->pEndDate < pCurrent); pRatePtr++);
  if(pRatePtr == pRates.end()) {
    if(pSeries.empty() == true) {
      pFinished = true;
      return;
    }
  } else {
    pYearPtr          = pRatePtr->pYears.begin();
    if(pRatePtr->pBillingCycles.size() == 0) {
      if(pSeries.empty() == true) pCurrent = pRatePtr->pStartDate + 899999;
    } else {
      uint64_t Time = pRatePtr->pStartDate + 899999;
      for(pBillPtr = pRatePtr->pBillingCycles.begin(); (pBillPtr != pRatePtr->pBillingCycles.end()) && (pCurrent > *pBillPtr); pBillPtr++) Time = *pBillPtr + 900000;
      if(pSeries.empty() == true) pCurrent = Time;
    }
    if(pRatePtr->pStartDate <= pCurrent) LoadPeriod();
  }
  pSampleTime = pGetSample(pCurrent, pSampleActive, pSampleReactive);
}
RatesEval::~RatesEval() {
  UnloadPeriod();
}
void RatesEval::UnloadPeriod() {
  if(pPriceModule != NULL) {
    delete pPriceModule;
    pPriceModule = NULL;
  }
  if(pScript != NULL) {
    delete pScript;
    pScript = NULL;
  }
  if(pPowerScript != NULL) {
    delete pPowerScript;
    pPowerScript = NULL;
  }
  if(pEnergyScript != NULL) {
    delete pEnergyScript;
    pEnergyScript = NULL;
  }
  while(pPeriods.size() > 0) {
    TariffScript *Tmp = pPeriods.back();
    delete Tmp;
    pPeriods.pop_back();
  }
  pPriceVariables.clear();
}
void RatesEval::LoadPeriod() {
  pLoadedPeriodDate = pCurrent - 899999;
  pPriceModule      = PriceSource::Instance(pRatePtr->pPriceSrc, pPriceVariables);
  for(uint16_t P = 0; P < pRatePtr->pPeriods.size(); P++) {
    TariffScript *Tmp = new TariffScript(pRatePtr->pPeriods[P].pScript);
    Tmp->Var("ContractedPower", &pContractedPower);
    Tmp->Var("PowerCost",       &pPowerCost);
    Tmp->Var("EnergyCost",      &pEnergyCost);
    Tmp->Var("ActivePower",     &pActivePower);
    Tmp->Var("ActiveEnergy",    &pActiveEnergy);
    Tmp->Var("ReactivePower",   &pReactivePower);
    Tmp->Var("ReactiveEnergy",  &pReactiveEnergy);
    pPeriods.push_back(Tmp);
  }
  pScript = new TariffScript(pRatePtr->pScript);
  pScript->Var("Period",         &pPeriod);
  pScript->Var("ActivePower",    &pActivePower);
  pScript->Var("ActiveEnergy",   &pActiveEnergy);
  pScript->Var("ReactivePower",  &pReactivePower);
  pScript->Var("ReactiveEnergy", &pReactiveEnergy);
  for(uint16_t P = 0; P < pRatePtr->pPeriods.size(); P++) {
    pScript->Var("ContractedPower_P" + to_string(P + 1), &pRatePtr->pContracted[P]);
    pScript->Var("EnergyCost_P" + to_string(P + 1),      (pRatePtr->pEnergyCost.size() == 0) ? &pEnergyCost : &pRatePtr->pEnergyCost[P]);
    pScript->Var("PowerCost_P" + to_string(P + 1),       (pRatePtr->pPowerCost.size() == 0)  ? &pPowerCost  : &pRatePtr->pPowerCost[P]);
  }
  if(pRatePtr->pEnergyCost.size() == 0) {
    pEnergyScript = new TariffScript(pRatePtr->pEnergyCostScript);
    for(auto &Var : pPriceVariables) pEnergyScript->Var(Var.first, &Var.second);
    pEnergyScript->Var("Period", &pPeriod);
  }
  if(pRatePtr->pPowerCost.size() == 0) {
    pPowerScript = new TariffScript(pRatePtr->pPowerCostScript);
    for(auto &Var : pPriceVariables) pPowerScript->Var(Var.first, &Var.second);
    pPowerScript->Var("Period", &pPeriod);
  }
}
json RatesEval::BuildBill() {
  float EnergyTerm  = 0.0;
  float PowerTerm   = 0.0;
  float GeneralTerm = 0.0;
  json Row          = json::object();
  Row["From"]        = pLoadedPeriodDate;
  Row["To"]          = pCurrent;
  Row["Currency"]    = pRatePtr->pCurrency;
  Row["PowerUnits"]  = pRatePtr->pPowerUnits;
  Row["EnergyUnits"] = pRatePtr->pEnergyUnits;
  Row["Name"]        = pRatePtr->pName;
  Row["Description"] = pRatePtr->pDescription;
  Row["Periods"]     = json::array();
  Row["Complete"]    = (pNowDate >= pCurrent);
  for(uint16_t P = 0; P < pRatePtr->pPeriods.size(); P++) {
    json Period = json::array();
    for(uint16_t C = 0; C < pRatePtr->pPeriods[P].pConcepts.size(); C++) {
      json Concept = json::object();
      Concept["Description"] = pRatePtr->pPeriods[P].pConcepts[C].pDescription;
      if(pRatePtr->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST) {
        float Cost = pPeriods[P]->Var(pRatePtr->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pRatePtr->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P]->Var(pRatePtr->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pRatePtr->pPeriods[P].pConcepts[C].pType == TariffConcept::CONCEPT_COST_POWER) {
        float Cost = pPeriods[P]->Var(pRatePtr->pPeriods[P].pConcepts[C].pVariable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_POWER";
        PowerTerm += Cost;
      } else {
        Concept["Type"] = "INFO";
        Concept["Value"] = pPeriods[P]->Var(pRatePtr->pPeriods[P].pConcepts[C].pVariable);
      }
      Period.push_back(Concept);
    }
    Row["Periods"].push_back(Period);
  }
  Row["General"] = json::array();
  for(uint16_t C = 0; C < pRatePtr->pConcepts.size(); C++) {
    json Concept = json::object();
    Concept["Description"] = pRatePtr->pConcepts[C].pDescription;
    if(pRatePtr->pConcepts[C].pType == TariffConcept::CONCEPT_COST) {
      float Cost = pScript->Var(pRatePtr->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pRatePtr->pConcepts[C].pType == TariffConcept::CONCEPT_COST_ENERGY) {
      float Cost = pScript->Var(pRatePtr->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pRatePtr->pConcepts[C].pType == TariffConcept::CONCEPT_COST_POWER) {
      float Cost = pScript->Var(pRatePtr->pConcepts[C].pVariable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_POWER";
      PowerTerm += Cost;
    } else {
      Concept["Type"] = "INFO";
      Concept["Value"] = pScript->Var(pRatePtr->pConcepts[C].pVariable);
    }
    Row["General"].push_back(Concept);
  }
  float Total = EnergyTerm + PowerTerm + GeneralTerm;
  Row["Taxes"] = json::array();
  for(uint16_t T = 0; T < pRatePtr->pTaxes.size(); T++) {
    json Tax = json::object();
    if(pRatePtr->pTaxes[T].pType == RateTax::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pRatePtr->pTaxes[T].pValue / 100.0);
      Total += EnergyTerm * (pRatePtr->pTaxes[T].pValue / 100.0);
    } else if(pRatePtr->pTaxes[T].pType == RateTax::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pRatePtr->pTaxes[T].pValue / 100.0);
      Total += PowerTerm * (pRatePtr->pTaxes[T].pValue / 100.0);
    } else if(pRatePtr->pTaxes[T].pType == RateTax::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pRatePtr->pTaxes[T].pValue / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pRatePtr->pTaxes[T].pValue / 100.0);
    } else if(pRatePtr->pTaxes[T].pType == RateTax::TAX_FIXED) {
      Tax["Type"] = "FIXED_QUANTITY";
      Tax["Value"] = pRatePtr->pTaxes[T].pValue;
      GeneralTerm += pRatePtr->pTaxes[T].pValue;
      Total += pRatePtr->pTaxes[T].pValue;
    } else {
      continue;
    }
    Tax["Description"] = pRatePtr->pTaxes[T].pDescription;
    Row["Taxes"].push_back(Tax);
  }
  float Tmp = 0.0;
  for(uint16_t T = 0; T < pRatePtr->pTaxes.size(); T++) {
    if(pRatePtr->pTaxes[T].pType == RateTax::TAX_PERCENT) {
      json Tax = json::object();
      Tax["Description"] = pRatePtr->pTaxes[T].pDescription;
      Tax["Type"] = "PERCENT_TOTAL";
      Tax["Value"] = Total * (pRatePtr->pTaxes[T].pValue / 100.0);
      Row["Taxes"].push_back(Tax);
      Tmp += Total * (pRatePtr->pTaxes[T].pValue / 100.0);
    }
  }
  Row["Total"] = Total + Tmp;
  return Row;
}
json RatesEval::BuildSample(const bool bill) {
  json Row = json::array();
  for(auto &Serie : pSeries) {
    if(Serie == "Time")            Row.push_back(pCurrent);
    if(Serie == "Active_Power")    Row.push_back(pActivePower);
    if(Serie == "Active_Energy")   Row.push_back(pActiveEnergy);
    if(Serie == "Reactive_Power")  Row.push_back(pReactivePower);
    if(Serie == "Reactive_Energy") Row.push_back(pReactiveEnergy);
    if(pPeriods.size() == 0) {
      if(Serie == "Cost")          Row.push_back(nullptr);
      if(Serie == "Power_Cost")    Row.push_back(nullptr);
      if(Serie == "Energy_Cost")   Row.push_back(nullptr);
      if(Serie == "Period")        Row.push_back(nullptr);
      if(Serie == "Color")         Row.push_back(nullptr);
      if(Serie == "Contracted")    Row.push_back(nullptr);
    } else {
      if(Serie == "Cost")          Row.push_back(pCost);
      if(Serie == "Power_Cost")    Row.push_back(pPowerCost);
      if(Serie == "Energy_Cost")   Row.push_back(pEnergyCost);
      if(Serie == "Period")        Row.push_back((uint16_t)pPeriod);
      if(Serie == "Color")         Row.push_back(pColor);
      if(Serie == "Contracted")    Row.push_back(pContractedPower);
    }
    if(Serie == "Bill")            Row.push_back(bill);
  }
  return Row;
}
json RatesEval::HandleSample() {
  json     Result = nullptr;
  uint16_t Period = 0;
  if((pPriceModule == NULL) && (pRatePtr != pRates.end())) {
    if((pRatePtr->pStartDate < pCurrent) && (pRatePtr->pEndDate >= pCurrent)) {
      if(pBillPtr != pRatePtr->pBillingCycles.end()) pBillPtr++;
      LoadPeriod();
    } else {
      while(pRatePtr != pRates.end()) {
        if(pRatePtr->pStartDate > pCurrent) break;
        if(pRatePtr->pEndDate >= pCurrent) {
          pBillPtr = pRatePtr->pBillingCycles.begin();
          pYearPtr = pRatePtr->pYears.begin();
          LoadPeriod();
          break;
        }
        pRatePtr++;
      }
    }
  }
  if(pSampleTime == pCurrent) {
    pActivePower   = pSampleActive;
    pReactivePower = pSampleReactive;
    if(pSampleTime != UINT64_MAX) pSampleTime = pGetSample(pCurrent + 900000, pSampleActive, pSampleReactive);
  } else {
    pActivePower   = 0.0;
    pReactivePower = 0.0;
  }
  pActiveEnergy   = pActivePower / 4.0;
  pReactiveEnergy = pReactivePower / 4.0;
  if(pPriceModule != NULL) {
    civil_second Time   = convert(chrono::system_clock::from_time_t(pCurrent / 1000), pTimeZone);
    while(Time.year() > pYearPtr->pYear) pYearPtr++;
    if((pRatePtr->pEnergyCost.size() == 0) || (pRatePtr->pPowerCost.size() == 0)) pPriceModule->GetSample(pCurrent, pPriceVariables);
    Period           = pRatePtr->pDays[pYearPtr->pDays[get_yearday(civil_day(Time)) - 1]].pQuarters[Time.hour() * 4 + Time.minute() / 15];
    pPeriod          = Period + 1.0;
    pColor           = pRatePtr->pPeriods[Period].pColor;
    pContractedPower = pRatePtr->pContracted[Period];
    pEnergyCost      = (pEnergyScript != NULL) ? pEnergyScript->Eval() : pRatePtr->pEnergyCost[Period];
    pPowerCost       = (pPowerScript  != NULL)  ? pPowerScript->Eval() : pRatePtr->pPowerCost[Period];
    pScript->Eval();
    pPeriods[Period]->Eval();
    pCost = pScript->Var("Cost") + pPeriods[Period]->Var("Cost");
    if(pCurrent == ((pBillPtr != pRatePtr->pBillingCycles.end()) ? *pBillPtr : pRatePtr->pEndDate)) {
      Result    = (pSeries.empty() == true) ? BuildBill() : BuildSample(true);
      pCurrent += 900000;
      UnloadPeriod();
      return Result;
    }
  }
  if(pSeries.empty() == false) Result = BuildSample(false);
  pCurrent += 900000;
  return Result;
}
json RatesEval::FetchRow() {
  json Row = nullptr;
  while((Row == nullptr) && (pFinished == false)) {
    Row = HandleSample();
    if((pSeries.empty() == false) && (pCurrent == pTo)) pFinished = true;
    if((pSeries.empty() == true) && (pCurrent > pTo)) {
      if(pRatePtr != pRates.end()) {
        if(pCurrent > ((pBillPtr != pRatePtr->pBillingCycles.end()) ? *pBillPtr : pRatePtr->pEndDate)) pFinished = true;
      } else {
        if(pCurrent > pTo) pFinished = true;
      }
    }
  }
  return Row;
}
// ---------------------------------------------------------------------------------------------------------------------