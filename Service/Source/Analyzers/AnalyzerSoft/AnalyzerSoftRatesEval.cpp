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
#include "AnalyzerSoftRatesEval.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftRatesEval::AnalyzerSoftRatesEval(const string &serial, const time_zone &tz, const uint64_t from, const uint64_t to, const uint8_t group, const uint16_t serieMask) {
  pRateDB       = new Database(serial);
  pDataDB       = new Database(serial);
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
  FindStart(group);
  pCurrent = pFrom;
  pDataDB->ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + pDataDB->Bind(pFrom) + " ORDER BY sampletime ASC LIMIT 1000");
  pDataDB->FecthRow();
}
AnalyzerSoftRatesEval::~AnalyzerSoftRatesEval() {
  UnloadRate();
  delete pLoadedRate;
  delete pRateDB;
  delete pDataDB;
}
json AnalyzerSoftRatesEval::Headers() {
  if(pSeriesMask == 0) return nullptr;
  json Header = json::array();
  if(pSeriesMask & SERIE_TIMESTAMP)       Header.push_back("Sampletime");
  if(pSeriesMask & SERIE_ACTIVE_POWER)    Header.push_back("Active_Power");
  if(pSeriesMask & SERIE_ACTIVE_ENERGY)   Header.push_back("Active_Energy");
  if(pSeriesMask & SERIE_REACTIVE_POWER)  Header.push_back("Reactive_Power");
  if(pSeriesMask & SERIE_REACTIVE_ENERGY) Header.push_back("Reactive_Energy");
  if(pSeriesMask & SERIE_COST)            Header.push_back("Cost");
  if(pSeriesMask & SERIE_POWER_COST)      Header.push_back("Power_Cost");
  if(pSeriesMask & SERIE_ENERGY_COST)     Header.push_back("Energy_Cost");
  if(pSeriesMask & SERIE_PERIOD)          Header.push_back("Period");
  if(pSeriesMask & SERIE_COLOR)           Header.push_back("Color");
  if(pSeriesMask & SERIE_CONTRACTED)      Header.push_back("Power_Contracted");
  if(pSeriesMask & SERIE_BILL)            Header.push_back("Bill");
  return Header;
}
void AnalyzerSoftRatesEval::FindStart(const uint8_t group) {
  pRateDB->ExecSentenceAsyncResult("SELECT jsonb_build_object('Name', name, 'StartDate', to_char(startdate, 'dd/mm/yyyy'), 'EndDate', to_char(enddate, 'dd/mm/yyyy')) || template::jsonb AS result FROM rates WHERE ordering = " + pRateDB->Bind(group) + " AND " +
                                   "enddate >= " + pRateDB->Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(pFrom)), utc_time_zone())) + " AND " +
                                   "startdate <= " + pRateDB->Bind(format("%Y-%m-%d", chrono::system_clock::time_point(chrono::milliseconds(pTo)), utc_time_zone())) + " " +
                                   "ORDER BY startdate ASC");
  if(pRateDB->FecthRow() == true) pLoadedRate = new Rate(pRateDB->ResultJSON("result"), pTimeZone);
  if((pSeriesMask != 0) || (pRateDB->LoadedRow() == false)) return;
  uint64_t Time = pLoadedRate->StartDate + 899999;
  for(uint16_t n = 0; n < pLoadedRate->BillingCycles.size(); n++) {
    if(pFrom < pLoadedRate->BillingCycles[n]) {
      pFrom = Time;
      return;
    }
    Time = pLoadedRate->BillingCycles[n] + 900000;
  }
  pFrom = Time;
}
void AnalyzerSoftRatesEval::LoadRate() {
  if(pPeriods.size() > 0) return;
  pYearPtr          = pLoadedRate->Years.begin();
  pBillPtr          = pLoadedRate->BillingCycles.begin();
  pLoadedPeriodDate = pLoadedRate->StartDate;
  pPriceModule      = PriceSource::CreateInstance(pLoadedRate->PriceSrc);
  pPriceModule->PrepareRange(pCurrent, (pBillPtr != pLoadedRate->BillingCycles.end()) ? *pBillPtr : pLoadedRate->EndDate);
  pPriceModule->GetParams(0, pPriceVariables);
  while((pBillPtr != pLoadedRate->BillingCycles.end()) && (*pBillPtr < pCurrent)) {
    pLoadedPeriodDate = *pBillPtr;
    pBillPtr++;
  }
  for(uint16_t P = 0; P < pLoadedRate->Periods.size(); P++) {
    TariffScript *Script = new TariffScript(pLoadedRate->Periods[P].Script);
    Script->Var("ContractedPower", &pContractedPower);
    Script->Var("PowerCost",       &pPowerCost);
    Script->Var("EnergyCost",      &pEnergyCost);
    Script->Var("ActivePower",     &pActivePower);
    Script->Var("ActiveEnergy",    &pActiveEnergy);
    Script->Var("ReactivePower",   &pReactivePower);
    Script->Var("ReactiveEnergy",  &pReactiveEnergy);
    pPeriods.push_back(Script);
  }
  pScript = new TariffScript(pLoadedRate->Script);
  pScript->Var("Period",         &pPeriod);
  pScript->Var("ActivePower",    &pActivePower);
  pScript->Var("ActiveEnergy",   &pActiveEnergy);
  pScript->Var("ReactivePower",  &pReactivePower);
  pScript->Var("ReactiveEnergy", &pReactiveEnergy);
  for(uint16_t P = 0; P < pLoadedRate->Periods.size(); P++) {
    pScript->Var("ContractedPower_P" + to_string(P + 1), &pLoadedRate->Contracted[P]);
    pScript->Var("EnergyCost_P" + to_string(P + 1),      (pLoadedRate->EnergyCost.size() == 0) ? &pEnergyCost : &pLoadedRate->EnergyCost[P]);
    pScript->Var("PowerCost_P" + to_string(P + 1),       (pLoadedRate->PowerCost.size() == 0)  ? &pPowerCost  : &pLoadedRate->PowerCost[P]);
  }
  if(pLoadedRate->EnergyCost.size() == 0) {
    pEnergyScript = new TariffScript(pLoadedRate->EnergyCostScript);
    for(auto &Var : pPriceVariables) pEnergyScript->Var(Var.first, &Var.second);
    pEnergyScript->Var("Period", &pPeriod);
  }
  if(pLoadedRate->PowerCost.size() == 0) {
    pPowerScript = new TariffScript(pLoadedRate->PowerCostScript);
    for(auto &Var : pPriceVariables) pPowerScript->Var(Var.first, &Var.second);
    pPowerScript->Var("Period", &pPeriod);
  }
}
void AnalyzerSoftRatesEval::UnloadRate() {
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
json AnalyzerSoftRatesEval::HandleSample() {
  while(pLoadedRate != NULL) {
    if(pCurrent < pLoadedRate->StartDate) break;
    if((pCurrent >= pLoadedRate->StartDate) && (pCurrent <= pLoadedRate->EndDate)) {
      LoadRate();
      break;
    }
    delete pLoadedRate;
    pLoadedRate = NULL;
    if(pRateDB->FecthRow()) {
      pLoadedRate = new Rate(pRateDB->ResultJSON("result"), pTimeZone);
    } else {
      if(pSeriesMask == 0) pFinished = true;
      return nullptr;
    }
  }
  json         Result = nullptr;
  civil_second Time   = convert(chrono::system_clock::from_time_t(pCurrent / 1000), pTimeZone);
  if(pPeriods.size() > 0) {
    while(Time.year() > pYearPtr->Year) pYearPtr++;
    uint16_t Period = pLoadedRate->Days[pYearPtr->Days[get_yearday(civil_day(Time)) - 1]].Quarters[Time.hour() * 4 + Time.minute() / 15];
    pPeriod = (Period + 1.0);
    if((pLoadedRate->EnergyCost.size() == 0) || (pLoadedRate->PowerCost.size() == 0)) pPriceModule->GetParams(pCurrent, pPriceVariables);
    pPeriod          = Period + 1.0;
    pColor           = pLoadedRate->Periods[Period].Color;
    pContractedPower = pLoadedRate->Contracted[Period];
    pEnergyCost      = (pLoadedRate->EnergyCost.size() == 0) ? pEnergyScript->Eval() : pLoadedRate->EnergyCost[Period];
    pPowerCost       = (pLoadedRate->PowerCost.size() == 0)  ? pPowerScript->Eval()  : pLoadedRate->PowerCost[Period];
    pScript->Eval();
    pPeriods[Period]->Eval();
    pCost = pScript->Var("Cost") + pPeriods[Period]->Var("Cost");
    if(pCurrent == ((pBillPtr != pLoadedRate->BillingCycles.end()) ? *pBillPtr : pLoadedRate->EndDate)) {
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
json AnalyzerSoftRatesEval::BuildBill() {
  float EnergyTerm  = 0.0;
  float PowerTerm   = 0.0;
  float GeneralTerm = 0.0;
  json Row          = json::object();
  Row["From"]        = pLoadedPeriodDate;
  Row["To"]          = pCurrent;
  Row["Currency"]    = pLoadedRate->Currency;
  Row["PowerUnits"]  = pLoadedRate->PowerUnits;
  Row["EnergyUnits"] = pLoadedRate->EnergyUnits;
  Row["Name"]        = pLoadedRate->Name;
  Row["Description"] = pLoadedRate->Description;
  Row["Periods"]     = json::array();
  Row["Complete"]    = (pNowDate >= pCurrent);
  for(uint16_t P = 0; P < pLoadedRate->Periods.size(); P++) {
    json Period = json::array();
    for(uint16_t C = 0; C < pLoadedRate->Periods[P].Concepts.size(); C++) {
      json Concept = json::object();
      Concept["Description"] = pLoadedRate->Periods[P].Concepts[C].Description;
      if(pLoadedRate->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST) {
        float Cost = pPeriods[P]->Var(pLoadedRate->Periods[P].Concepts[C].Variable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pLoadedRate->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P]->Var(pLoadedRate->Periods[P].Concepts[C].Variable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pLoadedRate->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
        float Cost = pPeriods[P]->Var(pLoadedRate->Periods[P].Concepts[C].Variable);
        Concept["Value"] = Cost;
        Concept["Type"] = "COST_POWER";
        PowerTerm += Cost;
      } else {
        Concept["Type"] = "INFO";
        Concept["Value"] = pPeriods[P]->Var(pLoadedRate->Periods[P].Concepts[C].Variable);
      }
      Period.push_back(Concept);
    }
    Row["Periods"].push_back(Period);
  }
  Row["General"] = json::array();
  for(uint16_t C = 0; C < pLoadedRate->Concepts.size(); C++) {
    json Concept = json::object();
    Concept["Description"] = pLoadedRate->Concepts[C].Description;
    if(pLoadedRate->Concepts[C].Type == TariffConcept::CONCEPT_COST) {
      float Cost = pScript->Var(pLoadedRate->Concepts[C].Variable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pLoadedRate->Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
      float Cost = pScript->Var(pLoadedRate->Concepts[C].Variable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pLoadedRate->Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
      float Cost = pScript->Var(pLoadedRate->Concepts[C].Variable);
      Concept["Value"] = Cost;
      Concept["Type"] = "COST_POWER";
      PowerTerm += Cost;
    } else {
      Concept["Type"] = "INFO";
      Concept["Value"] = pScript->Var(pLoadedRate->Concepts[C].Variable);
    }
    Row["General"].push_back(Concept);
  }
  float Total = EnergyTerm + PowerTerm + GeneralTerm;
  Row["Taxes"] = json::array();
  for(uint16_t T = 0; T < pLoadedRate->Taxs.size(); T++) {
    json Tax = json::object();
    if(pLoadedRate->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pLoadedRate->Taxs[T].Value / 100.0);
      Total += EnergyTerm * (pLoadedRate->Taxs[T].Value / 100.0);
    } else if(pLoadedRate->Taxs[T].Type == RateTax::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pLoadedRate->Taxs[T].Value / 100.0);
      Total += PowerTerm * (pLoadedRate->Taxs[T].Value / 100.0);
    } else if(pLoadedRate->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pLoadedRate->Taxs[T].Value / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pLoadedRate->Taxs[T].Value / 100.0);
    } else if(pLoadedRate->Taxs[T].Type == RateTax::TAX_FIXED) {
      Tax["Type"] = "FIXED_QUANTITY";
      Tax["Value"] = pLoadedRate->Taxs[T].Value;
      GeneralTerm += pLoadedRate->Taxs[T].Value;
      Total += pLoadedRate->Taxs[T].Value;
    } else {
      continue;
    }
    Tax["Description"] = pLoadedRate->Taxs[T].Description;
    Row["Taxes"].push_back(Tax);
  }
  float Tmp = 0.0;
  for(uint16_t T = 0; T < pLoadedRate->Taxs.size(); T++) {
    if(pLoadedRate->Taxs[T].Type == RateTax::TAX_PERCENT) {
      json Tax = json::object();
      Tax["Description"] = pLoadedRate->Taxs[T].Description;
      Tax["Type"] = "PERCENT_TOTAL";
      Tax["Value"] = Total * (pLoadedRate->Taxs[T].Value / 100.0);
      Row["Taxes"].push_back(Tax);
      Tmp += Total * (pLoadedRate->Taxs[T].Value / 100.0);
    }
  }
  Row["Total"] = Total + Tmp;
  return Row;
}
json AnalyzerSoftRatesEval::BuildSample(const bool bill) {
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
json AnalyzerSoftRatesEval::FetchRow() {
  json Row = nullptr;
  while((Row == nullptr) && (pFinished == false)) {
    uint64_t Time = UINT64_MAX;
    if(pDataDB->LoadedRow() == true) Time = (pDataDB->ResultUInt64("sampletime") / 900000) * 900000 + 899999;
    if(Time > pCurrent) {
      pActivePower    = 0.0;
      pActiveEnergy   = 0.0;
      pReactivePower  = 0.0;
      pReactiveEnergy = 0.0;
      Row = HandleSample();
    } else {
      pActivePower    = pDataDB->ResultFloat("active");
      pActiveEnergy   = pActivePower / 4.0;
      pReactivePower  = pDataDB->ResultFloat("reactive");
      pReactiveEnergy = pReactivePower / 4.0;
      Row = HandleSample();
      if(pDataDB->FecthRow() == false) {
        pFrom = pCurrent + 1;
        pDataDB->ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive FROM aggreg_15m WHERE sampletime >= " + pDataDB->Bind(pFrom) + " ORDER BY sampletime ASC LIMIT 1000");
        pDataDB->FecthRow();
      }
    }
    if((pSeriesMask != 0) && (pCurrent == pTo)) pFinished = true;
    if((pSeriesMask == 0) && (pCurrent >= pTo)) {
      if((pLoadedRate != NULL) && (pLoadedRate->EndDate == pCurrent)) pFinished = true;
      if((pLoadedRate != NULL) && (pBillPtr != pLoadedRate->BillingCycles.end()) && (*pBillPtr == pCurrent)) pFinished = true;
    }
  }
  return Row;
}
// ---------------------------------------------------------------------------------------------------------------------