/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <errno.h>
//#include <cctz/civil_time.h>
#include "Tariff.h"
#include "TariffPlugin.h"
//----------------------------------------------------------------------------------------------------------------------
TariffList::TariffList(const json list, const string timezone) {
  cctz::load_time_zone(timezone, &pTimeZone);
  if(list.is_array() == false) return;
  for(uint32_t n = 0; n < list.size(); n++) PushTariff(list[n]);
}
bool TariffList::PushTariff(json tariff) {
  if(!tariff["StartDate"].is_number()) return false;
  if(!tariff["EndDate"].is_number()) return false;
  if(!tariff["Template"].is_object()) return false;
  if(!tariff["Config"].is_object()) return false;
  Tariff *Tmp = new Tariff(tariff["StartDate"], tariff["EndDate"], pTimeZone.name());
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
      if(*c > from) break;
      BillingFrom = *c + 1;
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
      if(*c < to) break;
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
Tariff::Tariff(uint64_t start, uint64_t end, string tz) {
  cctz::load_time_zone(tz, &pTimeZone);
  pStartDate = start;
  pEndDate = end;
}
Tariff::~Tariff() {
  for(uint32_t n = 0; n < pPeriods.size(); n++) {
    delete pPeriods[n].PowerModule;
    delete pPeriods[n].EnergyModule;
  }
}
uint8_t Tariff::FindPeriod(uint64_t timestamp) {
  uint8_t Period = UINT8_MAX;
  if(pHolidayPeriod != UINT8_MAX) {
    for(vector<Holiday_t>::iterator it = pHolidays.begin(); it != pHolidays.end(); it++) {
      if((timestamp >= it->StartDate) && (timestamp <= it->EndDate)) return pHolidayPeriod;
    }
  }
  for(uint8_t m = 0; m < pSeasons.size(); m++) {
    if((pSeasons[m].StartDate >= timestamp) || (pSeasons[m].EndDate < timestamp)) continue;
    cctz::civil_second Time = cctz::convert(chrono::system_clock::from_time_t(timestamp / 1000), pTimeZone);
    if((pWeekendPeriod != UINT8_MAX) && ((get_weekday(civil_day(Time)) == weekday::saturday) || (get_weekday(civil_day(Time)) == weekday::sunday))) return pWeekendPeriod;
    return pSeasons[m].DayPeriods[Time.hour()];
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
    string Tmp = t["Periods"][n]["PowerMode"];
    if(TariffPowerPlugin::Contains(Tmp) == false) return false;
    Period.PowerModule  = TariffPowerPlugin::Instance(Tmp);        
    if(!t["Periods"][n]["ReactiveMode"].is_string()) return false;
    Tmp = t["Periods"][n]["ReactiveMode"];
    if(TariffEnergyPlugin::Contains(Tmp) == false) return false;
    Period.EnergyModule = TariffEnergyPlugin::Instance(Tmp);
    if(!t["Periods"][n]["Color"].is_string()) return false;
    Period.Color = t["Periods"][n]["Color"];
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
    Period["PowerMode"] = pPeriods[n].PowerModule->Name();
    Period["ReactiveMode"] = pPeriods[n].EnergyModule->Name();
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
    pPeriods[n].PowerModule->Config(pPeriods[n].ContractedPower, pPeriods[n].PowerCost);
    pPeriods[n].EnergyModule->Config(pPeriods[n].EnergyCost);
  }
  if(!t["Billing"].is_array()) return false;
  for(uint32_t n = 0; n < t["Billing"].size(); n++) {
    if(!t["Billing"][n].is_number_unsigned()) return false;
    uint64_t Time = t["Billing"][n];
    if((Time % 900000) != 899999) return false;
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
json Tariff::GetResume() {
  json Result;
  Result["Sampletime"] = pSampletime;
  
  
  return Result;
}
bool Tariff::PushSample(const uint64_t time, const float active, const float reactive) {
  if(pSamples.size() == 0) {
    pSumEnergyCost = 0.0;
    pSumPowerCost  = 0.0;
  }
  Sample_t Sample;
  Sample.Active   = active;
  Sample.Reactive = reactive;
  Sample.Period   = FindPeriod(time);
  pSamples.push(Sample);
  pPeriods[Sample.Period].PowerModule->PushSample(Sample.Active, Sample.Reactive);
  pPeriods[Sample.Period].EnergyModule->PushSample(Sample.Active, Sample.Reactive);
  pSampletime = time;
  if(time == pEndDate) return false;
  for(vector<uint64_t>::iterator it = pBillingCycles.begin(); it != pBillingCycles.end(); it++) {
    if(time == *it) return false;
  }
  return true;
}
bool Tariff::PopSample(uint64_t &time, float &active, float &reactive, string &period, string &color, float &cost, float &contractedPower) {
  if(pSamples.size() == 0) return false;
  Sample_t Sample = pSamples.front();
  pSamples.pop();
  time             = pSampletime - (900000ULL * pSamples.size());
  active           = Sample.Active;
  reactive         = Sample.Reactive;
  period           = "P" + to_string(Sample.Period + 1);
  color            = pPeriods[Sample.Period].Color;
  contractedPower  = pPeriods[Sample.Period].ContractedPower;
  float EnergyCost = pPeriods[Sample.Period].EnergyModule->PopSample(Sample.Active, Sample.Reactive);
  float PowerCost  = pPeriods[Sample.Period].PowerModule->PopSample(Sample.Active, Sample.Reactive);
  float Cost = EnergyCost + PowerCost;
  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
    if(it->Type == "PERCENT_ENERGY") Cost += EnergyCost * (it->Value / 100.0);
    if(it->Type == "PERCENT_POWER") Cost += PowerCost * (it->Value / 100.0);
    if(it->Type == "PERCENT_ENERGY_POWER") Cost += (EnergyCost + PowerCost) * (it->Value / 100.0);
    if(it->Type == "FIXED_QUANTITY") Cost += it->Value / 96.0;
  }
  for(vector<Tax_t>::iterator it = pTaxs.begin(); it != pTaxs.end(); it++) {
    if(it->Type == "PERCENT_TOTAL") Cost = Cost * (it->Value / 100.0);
  }
  cost = Cost;
  return true;
}
