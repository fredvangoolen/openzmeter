/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
bool TariffList::PushTariff(json tariff) {
  if(!tariff["StartDate"].is_number()) return false;
  if(!tariff["EndDate"].is_number()) return false;
  if(!tariff["Template"].is_object()) return false;
  if(!tariff["Config"].is_object()) return false;
  Tools::LogMsg("ERROR", "Template: %s\nConfig: %s\n", tariff["Template"].dump().c_str(), tariff["Config"].dump().c_str());
  Tariff *Tmp = new Tariff(tariff["StartDate"], tariff["EndDate"]);
  if((Tmp->SetTemplate(tariff["Template"]) == false) || (Tmp->SetConfig(tariff["Config"]) == false)) {
    delete Tmp;
    return false;
  } 
  pList.push_back(Tmp);
  return true;
}
uint64_t TariffList::BillFrom(uint64_t from) {
  uint64_t BillingFrom = (from / 3600000) * 3600000;
  Tariff *Tariff = Find(BillingFrom);
  if(Tariff != NULL) {
    BillingFrom = Tariff->pStartDate;
    for(vector<uint64_t>::iterator c = Tariff->pBillingCycles.begin(); c < Tariff->pBillingCycles.end(); c++) {
      if(*c > BillingFrom) break;
      BillingFrom = *c;
    }
  }
  return BillingFrom;
}
uint64_t TariffList::BillTo(uint64_t to) {
  uint64_t BillingTo = (to / 3600000) * 3600000 + 3600000 - 1;
  Tariff *Tariff = Find(BillingTo);
  if(Tariff != NULL) {
    BillingTo = Tariff->pEndDate;
    for(vector<uint64_t>::reverse_iterator c = Tariff->pBillingCycles.rbegin(); c < Tariff->pBillingCycles.rend(); c++) {
      if(*c < BillingTo) break;
      BillingTo = *c;
    }
  }
  return BillingTo;
}
Tariff *TariffList::Find(uint64_t time) {
  for(vector<Tariff*>::iterator it = pList.begin(); it != pList.end(); it++) {
    if(((*it)->pStartDate <= time) && ((*it)->pEndDate >= time)) return *it;
  }
  return NULL;
}
TariffList::~TariffList() {
  for(vector<Tariff*>::iterator it = pList.begin(); it < pList.end(); it++) delete *it;
}
//----------------------------------------------------------------------------------------------------------------------
Tariff::Tariff(uint64_t start, uint64_t end) {
  pStartDate = start;
  pEndDate = end;
}
Tariff::~Tariff() {
  for(uint32_t n = 0; n < pPeriods.size(); n++) {
    delete pPeriods[n].ActiveModule;
    delete pPeriods[n].ReactiveModule;
  }
}
uint8_t Tariff::FindPeriod(uint64_t timestamp) {
  uint8_t Period = UINT8_MAX;
  for(uint8_t m = 0; m < pSeasons.size(); m++) {
    if((pSeasons[m].StartDate >= timestamp) || (pSeasons[m].EndDate < timestamp)) continue;
    uint8_t Hour = (timestamp / 3600000) % 24;
    return pSeasons[m].DayPeriods[Hour];
  }
  return Period;
}
bool Tariff::SetTemplate(json t) {
  if((pStartDate % 900000) != 0) return false;
  if((pEndDate % 900000) != 899999) return false;
  if(!t["Periods"].is_array()) return false;
  for(uint32_t n = 0; n < t["Periods"].size(); n++) {
    Period_t Period;
    Period.ContractedPower = 0;
    Period.EnergyCost = 0;
    Period.PowerCost = 0;
    if(!t["Periods"][n]["PowerMode"].is_string()) return false;
    Period.PowerMode = t["Periods"][n]["PowerMode"];
    if(!t["Periods"][n]["ReactiveMode"].is_string()) return false;
    Period.ReactiveMode = t["Periods"][n]["ReactiveMode"];
    if(!t["Periods"][n]["Color"].is_string()) return false;
    Period.Color = t["Periods"][n]["Color"];
    if(TariffPowerPlugin::Contains(Period.PowerMode) == false) return false;
    if(TariffReactivePlugin::Contains(Period.ReactiveMode) == false) return false;
    Period.ActiveModule = TariffPowerPlugin::Instance(Period.PowerMode, Period.ContractedPower, Period.PowerCost);
    Period.ReactiveModule = TariffReactivePlugin::Instance(Period.ReactiveMode);
    pPeriods.push_back(Period);
  }
  if(!t["Seasons"].is_array()) return false;
  for(uint32_t n = 0; n < t["Seasons"].size(); n++) {
    Season_t Season;
    if(!t["Seasons"][n]["Name"].is_string()) return false;
    Season.Name = t["Seasons"][n]["Name"];
    if(Season.Name.size() == 0) return false;
    if(!t["Seasons"][n]["StartDate"].is_number_unsigned()) return false;
    Season.StartDate = t["Seasons"][n]["StartDate"];
    if((pSeasons.size() == 0) && (Season.StartDate > pStartDate)) return false;
    if((pSeasons.size() != 0) && (Season.StartDate != (pSeasons.back().EndDate + 1))) return false;
    if(!t["Seasons"][n]["EndDate"].is_number_unsigned()) return false;
    Season.EndDate = t["Seasons"][n]["EndDate"];
    if((Season.StartDate % 900000) != 0) return false;
    if((Season.EndDate % 900000) != 899999) return false;
    if(Season.EndDate <= Season.StartDate) return false;
    if(!t["Seasons"][n]["DayPeriods"].is_array()) return false;
    if(t["Seasons"][n]["DayPeriods"].size() != 24) return false;
    for(uint8_t p = 0; p < t["Seasons"][n]["DayPeriods"].size(); p++) {
      if(!t["Seasons"][n]["DayPeriods"][p].is_number_integer()) return false;
      uint8_t Hour = t["Seasons"][n]["DayPeriods"][p];
      if((Hour < 0) || (Hour >= pPeriods.size())) return false;
      Season.DayPeriods[p] = Hour;
    }
    pSeasons.push_back(Season);
  }
  if(pSeasons.size() == 0) return false;
  if(pSeasons.back().EndDate < pEndDate) return false;
  if(!t["Holidays"].is_array()) return false;
  for(uint32_t n = 0; n < t["Holidays"].size(); n++) {
    Holiday_t Holiday;
    if(!t["Holidays"][n]["Name"].is_string()) return false;
    Holiday.Name = t["Holidays"][n]["Name"];
    if(!t["Holidays"][n]["StartDate"].is_number_unsigned()) return false;
    Holiday.StartDate = t["Holidays"][n]["StartDate"];
    if(Holiday.StartDate < pStartDate) return false;
    if(!t["Holidays"][n]["EndDate"].is_number_unsigned()) return false;
    Holiday.EndDate = t["Holidays"][n]["EndDate"];
    if((Holiday.StartDate % 900000) != 0) return false;
    if((Holiday.EndDate % 900000) != 899999) return false;
    if((Holiday.EndDate <= Holiday.StartDate) || (Holiday.EndDate > pEndDate)) return false;
    pHolidays.push_back(Holiday);    
  }
  if(!t["Description"].is_string()) return false;
  pDescription = t["Description"];
  if(t["HolidayPeriod"].is_null()) {
    pHolidayPeriod = UINT8_MAX;
  } else if(t["HolidayPeriod"].is_number_unsigned()) {
    pHolidayPeriod = t["HolidayPeriod"];
    if((pHolidayPeriod < 0) || (pHolidayPeriod >= pPeriods.size())) return false;
  } else {
    return false;
  }
  if(t["WeekendPeriod"].is_null()) {
    pWeekendPeriod = UINT8_MAX;
  } else if(t["WeekendPeriod"].is_number_unsigned()) {
    pWeekendPeriod = t["WeekendPeriod"];
    if((pWeekendPeriod < 0) || (pWeekendPeriod >= pPeriods.size())) return false;
  } else {
    return false;
  }
  return true;
}
json Tariff::GetTemplate() {
  json T;
  T["Periods"] = json::array();
  for(uint32_t n = 0; n < pPeriods.size(); n++) {
    json Period;
    Period["PowerMode"] = pPeriods[n].PowerMode;
    Period["ReactiveMode"] = pPeriods[n].ReactiveMode;
    Period["Color"] = pPeriods[n].Color;
    T["Periods"].push_back(Period);
  }
  T["Seasons"] = json::array();
  for(uint32_t n = 0; n < pSeasons.size(); n++) {
    json Season;
    Season["Name"] = pSeasons[n].Name;
    Season["StartDate"] = pSeasons[n].StartDate;
    Season["EndDate"] = pSeasons[n].EndDate;
    Season["DayPeriods"] = json::array();
    for(uint8_t p = 0; p < 24; p++) Season["DayPeriods"].push_back(pSeasons[n].DayPeriods[p]);
    T["Seasons"].push_back(Season);
  }
  T["Holidays"] = json::array();
  for(uint32_t n = 0; n < pHolidays.size(); n++) {
    json Holiday;
    Holiday["Name"] = pHolidays[n].Name;
    Holiday["StartDate"] = pHolidays[n].StartDate;
    Holiday["EndDate"] = pHolidays[n].EndDate;
    T["Holidays"].push_back(Holiday);    
  }
  T["Description"] = pDescription;
  T["HolidayPeriod"] = nullptr;
  if(pHolidayPeriod != UINT8_MAX) T["HolidayPeriod"] = pHolidayPeriod;
  T["WeekendPeriod"] = nullptr;
  if(pWeekendPeriod != UINT8_MAX) T["WeekendPeriod"] = pWeekendPeriod;
  return T;
}
bool Tariff::SetConfig(json t) {
  if(!t["Taxs"].is_array()) return false;
  for(uint32_t n = 0; n < t["Taxs"].size(); n++) {
    Tax_t Tax;
    if(!t["Taxs"][n]["Name"].is_string()) return false;
    Tax.Name = t["Taxs"][n]["Name"];
    if(!t["Taxs"][n]["Value"].is_number()) return false;
    Tax.Value = t["Taxs"][n]["Value"];
    if(!t["Taxs"][n]["Type"].is_string()) return false;
    Tax.Type = t["Taxs"][n]["Type"];
    if((Tax.Type != "PERCENT_TOTAL") && (Tax.Type != "PERCENT_ENERGY") && (Tax.Type != "PERCENT_POWER") && (Tax.Type != "PERCENT_ENERGY_POWER") && (Tax.Type != "FIXED_QUANTITY")) return false;
    pTaxs.push_back(Tax);
  }
  if(!t["Periods"].is_array()) return false;
  //This requiere a previous template is loaded with same periods number
  if(pPeriods.size() != t["Periods"].size()) return false;
  for(uint32_t n = 0; n < t["Periods"].size(); n++) {
    if(!t["Periods"][n]["Power_Cost"].is_number()) return false;
    pPeriods[n].PowerCost = t["Periods"][n]["Power_Cost"];
    if(!t["Periods"][n]["Energy_Cost"].is_number()) return false;
    pPeriods[n].EnergyCost = t["Periods"][n]["Energy_Cost"];
    if(!t["Periods"][n]["Contracted_Power"].is_number()) return false;
    pPeriods[n].ContractedPower = t["Periods"][n]["Contracted_Power"];
  }
  if(!t["Billing"].is_array()) return false;
  for(uint32_t n = 0; n < t["BillSampletime"].size(); n++) {
    if(!t["Billing"][n].is_number_unsigned()) return false;
    uint64_t Time = t["Billing"][n];
    if((Time % 900000) != 0) return false;
    pBillingCycles.push_back(Time);
  }
  return true;
}
json Tariff::GetConfig() {
  json T;
  T["Taxs"] = json::array();
  for(uint32_t n = 0; n < pTaxs.size(); n++) {
    json Tax;
    Tax["Name"] = pTaxs[n].Name;
    Tax["Value"] = pTaxs[n].Value;
    Tax["Type"] = pTaxs[n].Type;
    T["Taxs"].push_back(Tax);
  }
  T["Periods"] = json::array();
  for(uint32_t n = 0; n < pPeriods.size(); n++) {
    json Period;
    Period["Power_Cost"] = pPeriods[n].PowerCost;
    Period["Energy_Cost"] = pPeriods[n].EnergyCost;
    Period["Contracted_Power"] = pPeriods[n].ContractedPower;
    T["Periods"].push_back(Period);
  }
  T["Billing"] = pBillingCycles;
  return T;
}
bool Tariff::PushSample(uint64_t time, float active, float reactive) {
  if(pSamples.size() == 0) {
    for(uint32_t n = 0; n < pPeriods.size(); n++) {
      pPeriods[n].Active = 0.0;
      pPeriods[n].ActiveModule->Reset();
      pPeriods[n].Reactive = 0.0;
      pPeriods[n].ReactiveModule->Reset();
    }
    for(uint32_t n = 0; n < pTaxs.size(); n++) pTaxs[n].Accum = 0;
  }
  Sample_t Sample;
  Sample.Time = time;
  Sample.Active = active;
  Sample.Reactive = reactive;
  pSamples.push(Sample);
  //Search for period
  uint8_t Period = 0;
  pPeriods[Period].ActiveModule->PushSample(active);
  pPeriods[Period].ReactiveModule->PushSample(reactive);
  if(time == pEndDate) return false;
  for(vector<uint64_t>::iterator it = pBillingCycles.begin(); it != pBillingCycles.end(); it++) {
    if(time == *it) return false;
  }
  return true;
}
bool Tariff::PopSample(Sample_t &sample, float &cost) {
  cost = 0.0;
  if(pSamples.size() == 0) return false;
  Sample_t Sample = pSamples.front();
  pSamples.pop();
  //float EnergyCost = Energia del periodo * coste energia
  //float PowerCost = Potencia del periodo * poste potencia
  EnergyCost += PluginActive.popSample();
  PowerCost  += PluginPower.popSample();

  
  //Coste del periodo es Sample.Active / 4 * energy cost;
  // + 
  // + PluginReactive.PopSample
  


  sample = pSamples.front();
  pSamples.pop();
  return true;
}
