/* ================================== zMeter ===========================================================================
 * File:     PricesInterface.cpp
 * Author:   Eduardo Viciana (septiembre 2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "TariffsInterface.h"
//----------------------------------------------------------------------------------------------------------------------
bool TariffsInterface::Registered = TariffsInterface::Register();
//----------------------------------------------------------------------------------------------------------------------
bool TariffsInterface::Register() {
  HTTPServer::RegisterCallback("/getPrices",  TariffsInterface::GetPricesCallback);
  HTTPServer::RegisterCallback("/getTariffs", TariffsInterface::GetTariffsCallback);
  HTTPServer::RegisterCallback("/setTariffs", TariffsInterface::SetTariffsCallback);
  //getTariffsModules //Devuelve los modulos compilados para periodos
  return true;
}
void TariffsInterface::GetTariffsCallback(HTTPRequest &req) {
  ///TODO: Comprobar permisos
  json Params = req.GetParams();
  stringstream Query;
  if(!Params["Analyzer"].is_string()) {
    if(!Params["RateID"].is_number()) {
      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') from rates";
    } else {
      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  } else {
    if(Params["RateID"].is_number()) {
      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = " << Params["Analyzer"].get<string>();
    } else {
      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  }
  char *Tmp = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  req.FromString(Tmp == NULL ? "Failed query" : Tmp, MHD_HTTP_OK);
}
void TariffsInterface::SetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  stringstream Query;
  if(!Params["Analyzer"].is_string()) {
    Query << "INSERT INTO rates(rate, name, username, country_code, details, startdate, enddate) VALUES(";
    if(Params["RateID"] == NULL) 
      Query << "null, ";
    else
      Query << Params["RateID"] << ", ";
    Query << "'" << Params["Name"].get<string>() << "', ";
    Query << "'" << Params["Owner"].get<string>() << "', ";
    Query << "'" << Params["CountryCode"].get<string>() << "', ";
    Query << "'" << Params["Details"].dump() << "', ";
    Query << Params["StartDate"].get<uint64_t>() << ", ";
    Query << Params["EndDate"].get<uint64_t>() << ") ON CONFLICT(rate) DO UPDATE SET rate = EXCLUDED.rate, name = EXCLUDED.name, username = EXCLUDED.username, country_code = EXCLUDED.country_code, details = EXCLUDED.details, startdate = EXCLUDED.startdate, enddate = EXCLUDED.enddate";
    

            //    if(!Params["RateID"].is_number()) {
//    Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username))), '[]') from rates";
//    } else {
//      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = " << Params["RateID"].get<uint32_t>();
//    }  
  } else {
//    Query << "INSERT INTO SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = " << Params["Analyzer"].get<string>();
    
    if(Params["RateID"].is_number()) {
//      Query << "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = " << Params["Analyzer"].get<string>();
    } else {
//      Query << "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = " << Params["RateID"].get<uint32_t>();
    }  
  }
  Tools::LogMsg("INFO", "%s\n", Query.str().c_str());
  req.FromString("", (req.ExecSentence(Query.str().c_str()) == true) ? MHD_HTTP_OK : MHD_HTTP_NOT_ACCEPTABLE);

  
  //  if((json_object_object_get_ex(reader, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if(Serial == NULL) {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int(Find);
//    if(Index == 0) {
////      snprintf(QueryText, sizeof(QueryText), "insert into rates(\"name\", username, country_code, details, startdate, enddate) select \"Name\", 'admin', \"Country\", \"Details\", \"StartDate\", \"EndDate\" from json_to_record(?) as x(\"Name\" text, \"Country\" text, \"Details\" jsonb, \"StartDate\" bigint, \"EndDate\" bigint)", );
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'CountryCode', country_code, 'Owner', username, 'Details', details) from rates where rate = %u", Index);
//    }  
//  } else {
//    uint32_t Index = 0;
//    if((json_object_object_get_ex(reader, "RateID", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Index = json_object_get_int64(Find);
//    if(Index == 0) {
//      snprintf(QueryText, sizeof(QueryText), "SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') from analyzers_rates WHERE serial = '%s'", Serial);
//    } else {
//      snprintf(QueryText, sizeof(QueryText), "SELECT json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Details', details) from analyzers_rates where rate = %u", Index);
//    }  
//  }
//  *postdata = db->ExecSentenceJSON(QueryText, resultLen);
}
void TariffsInterface::GetPricesCallback(HTTPRequest &req) {
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) {
    req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
    return;
  }
  string Analyzer = Params["Analyzer"];
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) {
    req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
    return;
  }
  uint64_t To         = Params.value<uint64_t>("To", 0);
  uint64_t Period     = Params.value<uint64_t>("Period", 3600000);
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  string   Aggreg     = Params["Aggreg"];
  if(To == 0) To = Tools::GetTime();
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
    return;
  }  
  json Result;
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Energy"]      = json::array();;
  Result["Reactive_Energy"]    = json::array();;
  Result["Period_Name"]        = json::array();;
  Result["Period_Color"]       = json::array();;
  Result["Contracted_Power"]   = json::array();;
  Result["Expense"]            = json::array();;

  //Load involved tariffs
  stringstream Query;
  Query << "SELECT to_json(array_agg(json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Details', details) ORDER BY startdate ASC)) FROM analyzers_rates WHERE serial = '" << Analyzer << "' AND enddate >= " << From << " AND startdate <= " << To;
  char *JSONText = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  json TariffsJSON = (JSONText == NULL) ? json::array() : json::parse(JSONText);
  vector<TariffsInterface*> Tariffs;
  for(uint32_t n = 0; n < TariffsJSON.size(); n++) {
    TariffsInterface *Interface = FromJSON(TariffsJSON[n]);
    if(Interface != NULL) Tariffs.push_back(Interface);
  }
  //Find start and end date to retrieve data (Billing date or requested if no tariff asigned)
  uint64_t BillingFrom = (From / PeriodSeconds) * PeriodSeconds;
  for(vector<TariffsInterface*>::iterator it = Tariffs.begin(); it != Tariffs.end(); it++) {
    if(((*it)->StartDate >= BillingFrom) || ((*it)->StartDate < BillingFrom)) continue;
    BillingFrom = (*it)->StartDate;
    for(vector<uint64_t>::iterator c = (*it)->BillingCycles.begin(); c < (*it)->BillingCycles.end(); c++) {
      if(*c > BillingFrom) break;
      BillingFrom = *c;
    }
    break;
  }
  uint64_t BillingTo = (To / PeriodSeconds) * PeriodSeconds + PeriodSeconds - 1;
  for(vector<TariffsInterface*>::iterator it = Tariffs.begin(); it != Tariffs.end(); it++) {
    if(((*it)->StartDate >= BillingFrom) || ((*it)->StartDate < BillingFrom)) continue;
    BillingTo = (*it)->EndDate;
    for(vector<uint64_t>::reverse_iterator c = (*it)->BillingCycles.rbegin(); c < (*it)->BillingCycles.rend(); c++) {
      if(*c < BillingTo) break;
      BillingTo = *c;
    }
    break;
  }
  //Retrieve active and reactive samples
  Query.str("");
  Query << "SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power  FROM \"" << Analyzer << "\".aggreg_15m WHERE sampletime > " << BillingFrom << " AND sampletime <= " << BillingTo;
  req.ExecSentence(Query.str().c_str());

  
  uint32_t Row = 0;
  float Active = NAN;
  float Reactive = NAN;  
  while(BillingFrom < BillingTo) {
    uint32_t Tariff = FindTariff(Tariffs, BillingFrom);
    if(Tariff != UINT32_MAX) {
      if(IsBillDate(Tariffs[Tariff], BillingFrom) == true) {
        Bills.push_back(BillingFrom);
        //Reinicio contadores y llamo a modulesReset
        //LLamo a modulesProcessBillDate
      }
    }
    if((Row < req.ResultRows()) && (strtoull(req.ResultValue(Row, "sampletime"), NULL, 10) == (BillingFrom + 899999))) {
      float Tmp = atof(req.ResultValue(Row, "active_power"));
      Active = isnan(Active) ? Tmp / 4.0 : isnan(Tmp) ? Active : Active + Tmp / 4.0;
      Tmp = atof(req.ResultValue(Row, "reactive_power"));
      Reactive = isnan(Reactive) ? Tmp / 4.0 : isnan(Tmp) ? Reactive : Reactive + Tmp / 4.0;
      Row++;
    }
    if(Tariff != UINT32_MAX) {

    
    }
    if(((Aggreg == "15M") || ((BillingFrom % 3600000) == 3599999)) && (BillingFrom > LastSample)) {
      Sampletimes.push_back(BillingFrom);
      Actives.push_back(Active);
      Reactives.push_back(Reactive);
      Active = NAN;
      Reactive = NAN;
      if(Tariff != UINT32_MAX) {
        uint8_t Period = FindPeriod(Tariffs[Tariff], BillingFrom);
        if(Period != UINT8_MAX) {
          Periods.push_back("P" + to_string(Period));
          Colors.push_back("#EEE");
          Powers.push_back(Tariffs[Tariff]["Details"]["Periods"][Period - 1]["Contracted_Power"]);
          Expenses.push_back(0.0);
        } else {
          Periods.push_back(NAN);
          Colors.push_back("#ddd");
          Powers.push_back(NAN);
          Expenses.push_back(0.0);
        }
      }
    }
    BillingFrom += 900000;
  }

  req.FromJSON(Result);
  return;  
}
uint8_t TariffsInterface::FindPeriod(json tariff, uint64_t timestamp) {
  uint8_t Period = UINT8_MAX;
  for(uint8_t m = 0; m < tariff["Details"]["Seasons"].size(); m++) {
    if((tariff["Details"]["Seasons"][m]["StartDate"] >= timestamp) || (tariff["Details"]["Seasons"][m]["EndDate"] < timestamp)) continue;
    uint8_t Hour = (timestamp / 3600000) % 24;
    return tariff["Details"]["Seasons"][m]["DayPeriods"][Hour];
  }
  return Period;
}
bool TariffsInterface::IsBillDate(json tariff, uint64_t timestamp) {
  return false;
}