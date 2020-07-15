#include "RatesEval.h"
RatesEval::RatesEval(const json &t, const time_zone &timezone, const uint64_t from, const uint64_t to, const bool precise) {
  if(t.is_array() == false)
  for(uint16_t R = 0; R < t.size(); R++) pRates.emplace_back(t[R], timezone);
  pTimeZone    = timezone;
  pFrom        = FindStart(from, precise);
  pTo          = FindEnd(to, precise);
  pCurrent     = pFrom;
  pRatePtr     = pRates.begin();
  pPriceModule = NULL;
  pNowDate     = Tools::GetTime64();
}
RatesEval::~RatesEval() {
  UnloadRate();
}
json RatesEval::CheckConfig(const json &r) {
  json ConfigOUT = json::array();
  if(r.is_array() == false) return "Rate list is not an array";
  for(uint16_t R = 0; R < r.size(); R++) {
    json Tmp = Rate::CheckConfig(r[R], true);
    if(Tmp.is_string() == true) return Tmp;
    ConfigOUT.push_back(Tmp);
  }
  return ConfigOUT;
}



uint64_t RatesEval::StartDate() {
  return pFrom;
}
uint64_t RatesEval::EndDate() {
  return pTo;
}
uint64_t RatesEval::FindStart(const uint64_t from, bool precise) {
  uint64_t Time = (from / 3600000) * 3600000 + 899999;
  if(precise == false) return Time;
  for(uint16_t R = 0; R < pRates.size(); R++) {
    if((Time < pRates[R].StartDate) || (Time > pRates[R].EndDate)) continue;
    Time = pRates[R].StartDate + 899999;
    for(uint16_t B = 0; B < pRates[R].BillingCycles.size(); B++) {
      if(pRates[R].BillingCycles[B] > Time) return Time;
      Time = pRates[R].BillingCycles[B] + 900000;
    }
  }
  return Time;
}
uint64_t RatesEval::FindEnd(const uint64_t to, bool precise) {
  uint64_t Time = (to / 3600000) * 3600000 + 899999;
  if(precise == false) return Time;
  for(uint16_t R = 0; R < pRates.size(); R++) {
    if((Time < pRates[R].StartDate) || (Time > pRates[R].EndDate)) continue;
    Time = pRates[R].EndDate;
    for(uint16_t B = pRates[R].BillingCycles.size(); B > 0; B--) {
      if(pRates[R].BillingCycles[B - 1] < Time) return Time;
      Time = pRates[R].BillingCycles[B - 1];
    }
  }
  return Time;
}
float *RatesEval::VariableFactory(const char_type *name, void *params) {
  map<string, float> *Tmp = (map<string, float>*)params;
  return &Tmp->emplace(name, 0.0).first->second;
};
void RatesEval::LoadRate() {
  if(pPeriods.size() > 0) return;
  pYearPtr = pRatePtr->Years.begin();
  pBillPtr = pRatePtr->BillingCycles.begin();
  pLoadedPeriodDate = pRatePtr->StartDate;
  while((pBillPtr != pRatePtr->BillingCycles.end()) && (*pBillPtr < pCurrent)) {
    pLoadedPeriodDate = *pBillPtr;
    pBillPtr++;
  }
  pPeriods.resize(pRatePtr->Periods.size());
  pScript.SetVarFactory(VariableFactory, &pVariables);
  for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) {
    pPeriods[P].Script.SetVarFactory(VariableFactory, &pPeriods[P].Variables);
    pPeriods[P].Script.DefineVar("ContractedPower",   &pSampleOut.ContractedPower);
    pPeriods[P].Script.DefineVar("PowerCost",         &pSampleOut.PowerCost);
    pPeriods[P].Script.DefineVar("EnergyCost",        &pSampleOut.EnergyCost);
    pPeriods[P].Script.DefineVar("ActivePower",       &pSampleOut.ActivePower);
    pPeriods[P].Script.DefineVar("ActiveEnergy",      &pSampleOut.ActiveEnergy);
    pPeriods[P].Script.DefineVar("ReactivePower",     &pSampleOut.ReactivePower);
    pPeriods[P].Script.DefineVar("ReactiveEnergy",    &pSampleOut.ReactiveEnergy);
    pPeriods[P].Script.SetExpr(pRatePtr->Periods[P].Script);
    pScript.DefineVar("ContractedPower_P" + to_string(P + 1),  &pRatePtr->Contracted[P]);
    pScript.DefineVar("EnergyCost_P" + to_string(P + 1),       (pRatePtr->EnergyCost.size() == 0) ? &pSampleOut.EnergyCost : &pRatePtr->EnergyCost[P]);
    pScript.DefineVar("PowerCost_P" + to_string(P + 1),        (pRatePtr->PowerCost.size() == 0)  ? &pSampleOut.PowerCost  : &pRatePtr->PowerCost[P]);
  }
  pScript.DefineVar("Period",         &pSampleOut.PeriodF);
  pScript.DefineVar("ActivePower",    &pSampleOut.ActivePower);
  pScript.DefineVar("ActiveEnergy",   &pSampleOut.ActiveEnergy);
  pScript.DefineVar("ReactivePower",  &pSampleOut.ReactivePower);
  pScript.DefineVar("ReactiveEnergy", &pSampleOut.ReactiveEnergy);
  pScript.SetExpr(pRatePtr->Script);
  pPriceModule = PriceSource::CreateInstance(pRatePtr->pPriceSource);
  pPriceModule->PrepareRange(pCurrent, (pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate);
  pPriceModule->GetParams(0, pPriceVariables);
  if(pRatePtr->EnergyCost.size() == 0) {
    for(auto &Var : pPriceVariables) pEnergyScript.DefineVar(Var.first, &Var.second);
    pEnergyScript.DefineVar("Period", &pSampleOut.PeriodF);
    pEnergyScript.SetExpr(pRatePtr->EnergyCostScript);
  }
  if(pRatePtr->PowerCost.size() == 0) {
    for(auto &Var : pPriceVariables) pPowerScript.DefineVar(Var.first, &Var.second);
    pPowerScript.DefineVar("Period", &pSampleOut.PeriodF);
    pPowerScript.SetExpr(pRatePtr->PowerCostScript);
  }
}
void RatesEval::UnloadRate() {
  if(pPeriods.size() == 0) return;
  if(pPriceModule != NULL) {
    delete pPriceModule;
    pPriceModule = NULL;
  }
  pScript = Parser();
  pVariables.clear();
  pPeriods.clear();
}
void RatesEval::HandleSample(const float active, const float reactive) {
  while(pRatePtr != pRates.end()) {
    if(pCurrent < pRatePtr->StartDate) break;
    if((pCurrent >= pRatePtr->StartDate) && (pCurrent <= pRatePtr->EndDate)) {
      LoadRate();
      break;
    }
    pRatePtr++;
  }
  civil_second Time = convert(chrono::system_clock::from_time_t(pCurrent / 1000), pTimeZone);
  if(pPeriods.size() > 0) {
    while(Time.year() > pYearPtr->Year) pYearPtr++;
    pSampleOut.Period  = pRatePtr->Days[pYearPtr->Days[get_yearday(civil_day(Time)) - 1]].Quarters[Time.hour() * 4 + Time.minute() / 15];
    pSampleOut.PeriodF = (pSampleOut.Period + 1.0);
    if((pRatePtr->EnergyCost.size() == 0) || (pRatePtr->PowerCost.size() == 0)) pPriceModule->GetParams(pCurrent, pPriceVariables);
    pSampleOut.SampleTime      = pCurrent;
    pSampleOut.ActivePower     = active;
    pSampleOut.ActiveEnergy    = active / 4.0;
    pSampleOut.ReactivePower   = reactive;
    pSampleOut.ReactiveEnergy  = reactive / 4.0;
    pSampleOut.Color           = pRatePtr->Periods[pSampleOut.Period].Color;
    pSampleOut.ContractedPower = pRatePtr->Contracted[pSampleOut.Period];
    pSampleOut.EnergyCost      = (pRatePtr->EnergyCost.size() == 0) ? pEnergyScript.Eval() : pRatePtr->EnergyCost[pSampleOut.Period];
    pSampleOut.PowerCost       = (pRatePtr->PowerCost.size() == 0)  ? pPowerScript.Eval()  : pRatePtr->PowerCost[pSampleOut.Period];
    pScript.Eval();
    pPeriods[pSampleOut.Period].Script.Eval();
    pSampleOut.Cost = pVariables["Cost"] + pPeriods[pSampleOut.Period].Variables["Cost"];
    if(pSampleCallback) pSampleCallback(pSampleOut);
    if(pCurrent == ((pBillPtr != pRatePtr->BillingCycles.end()) ? *pBillPtr : pRatePtr->EndDate)) {
      if(pBillCallback) pBillCallback(BuildBill(true));
      UnloadRate();
    }
  } else if(pSampleCallback) {
    pSampleOut.Period = UINT16_MAX;
    pSampleOut.PeriodF = NAN;
    pSampleOut.SampleTime = pCurrent;
    pSampleOut.ActivePower = active;
    pSampleOut.ActiveEnergy = active / 4.0;
    pSampleOut.ReactivePower = reactive;
    pSampleOut.ReactiveEnergy = reactive / 4.0;
    pSampleOut.Color = "";
    pSampleOut.ContractedPower = NAN;
    pSampleOut.EnergyCost = NAN;
    pSampleOut.PowerCost = NAN;
    pSampleOut.Cost = NAN;
    pSampleCallback(pSampleOut);
  }
  pCurrent += 900000;
}
json RatesEval::BuildBill(bool complete) {
  float EnergyTerm      = 0.0;
  float PowerTerm       = 0.0;
  float GeneralTerm     = 0.0;
  json Result           = json::object();
  Result["From"]        = pLoadedPeriodDate;
  Result["To"]          = pCurrent;
  Result["Currency"]    = pRatePtr->Currency;
  Result["PowerUnits"]  = pRatePtr->PowerUnits;
  Result["EnergyUnits"] = pRatePtr->EnergyUnits;
  Result["Name"]        = pRatePtr->Name;
  Result["Description"] = pRatePtr->Description;
  Result["Periods"]     = json::array();
  Result["Complete"]    = (pNowDate > pCurrent) && complete;
  for(uint16_t P = 0; P < pRatePtr->Periods.size(); P++) {
    Result["Periods"].push_back(json::array());
    for(uint16_t C = 0; C < pRatePtr->Periods[P].Concepts.size(); C++) {
      Result["Periods"][P].push_back(json::object());
      Result["Periods"][P][C]["Description"] = pRatePtr->Periods[P].Concepts[C].Description;
      if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST";
        GeneralTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST_ENERGY";
        EnergyTerm += Cost;
      } else if(pRatePtr->Periods[P].Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
        float Cost = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
        Result["Periods"][P][C]["Value"] = Cost;
        Result["Periods"][P][C]["Type"] = "COST_POWER";
        PowerTerm += Cost;
      } else {
        Result["Periods"][P][C]["Type"] = "INFO";
        Result["Periods"][P][C]["Value"] = pPeriods[P].Variables[pRatePtr->Periods[P].Concepts[C].Variable];
      }
    }
  }
  Result["General"] = json::array();
  for(uint16_t C = 0; C < pRatePtr->Concepts.size(); C++) {
    Result["General"].push_back(json::object());
    Result["General"][C]["Description"] = pRatePtr->Concepts[C].Description;
    if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST";
      GeneralTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST_ENERGY) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST_ENERGY";
      EnergyTerm += Cost;
    } else if(pRatePtr->Concepts[C].Type == TariffConcept::CONCEPT_COST_POWER) {
      float Cost = pVariables[pRatePtr->Concepts[C].Variable];
      Result["General"][C]["Value"] = Cost;
      Result["General"][C]["Type"] = "COST_POWER";
      PowerTerm += Cost;
    } else {
      Result["General"][C]["Type"] = "INFO";
      Result["General"][C]["Value"] = pVariables[pRatePtr->Concepts[C].Variable];
    }
  }
  float Total = EnergyTerm + PowerTerm + GeneralTerm;
  Result["Taxes"] = json::array();
  for(uint16_t T = 0; T < pRatePtr->Taxs.size(); T++) {
    json Tax = json::object();
    if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY) {
      Tax["Type"] = "PERCENT_ENERGY";
      Tax["Value"] = EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += EnergyTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_POWER) {
      Tax["Type"] = "PERCENT_POWER";
      Tax["Value"] = PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
      Total += PowerTerm * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT_ENERGY_POWER) {
      Tax["Type"] = "PERCENT_ENERGY_POWER";
      Tax["Value"] = (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
      Total += (EnergyTerm + PowerTerm) * (pRatePtr->Taxs[T].Value / 100.0);
    } else if(pRatePtr->Taxs[T].Type == RateTax::TAX_FIXED) {
      Tax["Type"] = "FIXED_QUANTITY";
      Tax["Value"] = pRatePtr->Taxs[T].Value;
      GeneralTerm += pRatePtr->Taxs[T].Value;
      Total += pRatePtr->Taxs[T].Value;
    } else {
      continue;
    }
    Tax["Description"] = pRatePtr->Taxs[T].Description;
    Result["Taxes"].push_back(Tax);
  }
  float Tmp = 0.0;
  for(uint16_t T = 0; T < pRatePtr->Taxs.size(); T++) {
    if(pRatePtr->Taxs[T].Type == RateTax::TAX_PERCENT) {
      json Tax = json::object();
      Tax["Description"] = pRatePtr->Taxs[T].Description;
      Tax["Type"] = "PERCENT_TOTAL";
      Tax["Value"] = Total * (pRatePtr->Taxs[T].Value / 100.0);
      Result["Taxes"].push_back(Tax);
      Tmp += Total * (pRatePtr->Taxs[T].Value / 100.0);
    }
  }
  Result["Total"] = Total + Tmp;
  return Result;
}
void RatesEval::PushSample(const uint64_t time, const float active, const float reactive) {
  uint64_t Time = (time / 900000) * 900000 + 899999;
  while(pCurrent < Time) HandleSample(0.0, 0.0);
  HandleSample(active, reactive);
}
void RatesEval::Finish() {
  while(pCurrent < pTo) HandleSample(0.0, 0.0);
  if(pPeriods.size() > 0) {
    if(pBillCallback) pBillCallback(BuildBill(true));
    UnloadRate();
  }
}
//void RatesEval::SetPeriodCallback(function<void(const PeriodOut_t &)> periodOut) {
//  pPeriodCallback = periodOut;
//}
void RatesEval::SetSampleCallback(function<void(const SampleOut_t &)> sampleOut) {
  pSampleCallback = sampleOut;
}
void RatesEval::SetBillCallback(function<void(const json &)> billOut) {
  pBillCallback = billOut;
}
