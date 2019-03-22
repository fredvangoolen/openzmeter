/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
TariffList::TariffList(json array) {
  for(uint32_t n = 0; n < array.size(); n++) {
    if(!array[n]["StartDate"].is_number()) return;
    if(!array[n]["EndDate"].is_number()) return;
    if(!array[n]["Template"].is_object()) return;
    if(!array[n]["Config"].is_object()) return;
    Tariff *Tmp = new Tariff(array[n]["StartDate"], array[n]["EndDate"]);
    if((Tmp->SetTemplate(array[n]["Template"]) == false) || (Tmp->SetConfig(array[n]["Config"]) == false)) {
      delete Tmp;
    } else {
      pList.push_back(Tmp);
    }
  }
}
uint64_t TariffList::BillFrom(uint64_t from) {
  uint64_t BillingFrom = (from / 3600000) * 3600000;
  Tariff *Tariff = Find(BillingFrom);
  if(Tariff != NULL) {
    BillingFrom = Tariff->pStartDate;
    for(vector<uint64_t>::iterator c = Tariff->BillingCycles.begin(); c < Tariff->BillingCycles.end(); c++) {
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
    for(vector<uint64_t>::reverse_iterator c = Tariff->BillingCycles.rbegin(); c < Tariff->BillingCycles.rend(); c++) {
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
bool Tariff::SetTemplate(json t) {
  if(!t["Periods"].is_array()) return false;
  for(uint32_t n = 0; n < t["Periods"].size(); n++) {
    Period_t Period;
    Period.Contracted_Power = 0;
    if(!t["Periods"][n]["PowerMode"].is_string()) return false;
    Period.PowerMode = t["Periods"][n]["PowerMode"];
    if(!t["Periods"][n]["ReactiveMode"].is_string()) return false;
    Period.ReactiveMode = t["Periods"][n]["ReactiveMode"];
    if(!t["Periods"][n]["Color"].is_string()) return false;
    Period.Color = t["Periods"][n]["Color"];
    
    if((Period.PowerMode != "NONE")) return false;
    
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
  //Leo impuestos potencias contratadas, ...
  
//      if(!t["Periods"][n]["Contracted_Power"].is_number()) return false;
//    Period.Contracted_Power = t["Periods"][n]["Contracted_Power"];

  return true;
}
json Tariff::GetConfig() {
  json T = R"(
          {"Taxs": [{"Name": "IVA", "Type": "PERCENT_TOTAL", "Value": "21.0"}, {"Name": "Tasas energia", "Type": "PERCENT_ENERGY_POWER", "Value": " 5.11269632"}, {"Name": "Alquiler contador", "Type": "FIXED_QUANTITY", "Value": "2.30"}, {"Name": "Energy Discount", "Type": "PERCENT_ENERGY", "Value": "5"}, {"Name": "Bono social", "Type": "PERCENT_ENERGY_POWER", "Value": "-10"}], "Periods": [{"Power_Cost": 44, "Energy_Cost": 0.1, "Contracted_Power": 20000}, {"Power_Cost": 44, "Energy_Cost": 0.1, "Contracted_Power": 10000}, {"Power_Cost": 44, "Energy_Cost": 0.1, "Contracted_Power": 8000}], "BillSampletime": [1547420400000, 1550098800000, 1552518000000, 1555192800000, 1557784800000, 1560463200000, 1563055200000, 1565733600000, 1568412000000, 1571004000000, 1573686000000, 1576278000000]}
          
          )"_json;
  return T;
}




//Tariff *Tariff::FromJSON(uint64_t start, uint64_t end, json t) {
//  Tariff *T = new Tariff(start, end);
//  if(T->LoadTemplate(t) == false) {
//    delete T;
//    return NULL;
//  }
//  return T;
//}
//Tariff *Tariff::FromJSON(uint64_t start, uint64_t end, json temp, json conf) {
//  Tariff *T = new Tariff(start, end);
//  if(T->LoadTemplate(temp) == false) {
//    delete T;
//    return NULL;
//  }
//  if(T->LoadConfig(conf) == false) {
//    delete T;
//    return NULL;
//  }
//  return T;
//}



//uint8_t Tariff::FindPeriod(json tariff, uint64_t timestamp) {
//  uint8_t Period = UINT8_MAX;
//  for(uint8_t m = 0; m < tariff["Details"]["Seasons"].size(); m++) {
//    if((tariff["Details"]["Seasons"][m]["StartDate"] >= timestamp) || (tariff["Details"]["Seasons"][m]["EndDate"] < timestamp)) continue;
//    uint8_t Hour = (timestamp / 3600000) % 24;
//    return tariff["Details"]["Seasons"][m]["DayPeriods"][Hour];
//  }
//  return Period;
//}
//bool Tariff::IsBillDate(json tariff, uint64_t timestamp) {
//  return false;
//}
bool Tariff::PushSample(uint64_t time, float active, float reactive) {
  //Si time es el primero de la tarifa, o bien de alguno de los periodos reinicio contadores a 0 de todo
  //añado a donde corresponda
  //
  
  //Si el time + 1 es el de facturacion o time == fin tarifa devuelvo true, si no devuelvo false
  return true;
}
//bool Tariff::PopSample() {
//}
