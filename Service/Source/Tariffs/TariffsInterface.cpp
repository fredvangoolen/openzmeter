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
  uint64_t To         = Params.value("To", 0);
  uint64_t Period     = Params.value("Period", 3600000);
  uint64_t LastSample = Params.value("LastSample", 0);
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
  Result["To"]     = To;
  Result["From"]   = From;
  json Sampletimes = json::array();
  json Actives     = json::array();
  json Reactives   = json::array();
  json Periods     = json::array();
  json Colors      = json::array();
  json Powers      = json::array();
  json Bills       = json::array();

  stringstream Query;
  Query << "SELECT to_json(array_agg(json_build_object('Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Details', details) ORDER BY startdate ASC)) FROM analyzers_rates WHERE serial = '" << Analyzer << "' AND enddate >= " << From << " AND startdate <= " << To;
  char *JSONText = req.ExecSentenceJSON(Query.str().c_str(), NULL);
  json Tariffs;
  if(JSONText != NULL) json::parse(JSONText);

  uint64_t BillingFrom = BillStart(Tariffs, From);
  Result["BillingFrom"] = BillingFrom;
  uint64_t BillingTo = BillEnd(Tariffs, To);
  Result["BillingTo"] = BillingTo;

  Query.str("");
  Query << "SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power  FROM \"" << Analyzer << "\".aggreg_15m WHERE sampletime > " << LastSample << " AND sampletime >= " << From << " AND sampletime <= " << To;
  req.ExecSentence(Query.str().c_str());
  Tools::LogMsg("INFO", "Calculando desde: %" PRIu64 "\n", From);

  uint32_t Row = 0;
  while(From < To) {
    Sampletimes.push_back(From);
    // Busco la potencia en el periodo que estoy procesando
    float Active = NAN;
    float Reactive = NAN;
    if((Row < req.ResultRows()) && (strtoull(req.ResultValue(Row, "sampletime"), NULL, 10) == From)) {
      Active   = atof(req.ResultValue(Row, "active_power"));
      Reactive = atof(req.ResultValue(Row, "reactive_power"));
      Row++;
      Actives.push_back(Active);
      Reactives.push_back(Reactive);
    } else {
      Actives.push_back(NAN);
      Reactives.push_back(NAN);
    }
    //Busco la tarifa que cubre el rango y el tipo de periodo
    uint32_t Tariff = UINT32_MAX;
    uint8_t Period = UINT8_MAX;
    for(uint32_t n = 0; n < Tariffs.size(); n++) {
      if((Tariffs[n]["StartDate"] >= From) || (Tariffs[n]["EndDate"] < From)) continue;
      Tariff = n;
      for(uint32_t m = 0; m < Tariffs[n]["Details"]["Seasons"].size(); m++) {
        if((Tariffs[n]["Details"]["Seasons"][m]["StartDate"] >= From) || (Tariffs[n]["Details"]["Seasons"][m]["EndDate"] < From)) continue;
        uint8_t Hour   = (From / 3600000) % 24;
        Period = Tariffs[n]["Details"]["Seasons"][m]["DayPeriods"][Hour];
        break;
      }
      break;
    }
    if(Period != UINT8_MAX) {
      string PeriodName = "P" + to_string(Period);
      Periods.push_back(PeriodName);
      Colors.push_back("#ddd");
      Powers.push_back(Tariffs[Tariff]["Details"]["Periods"][Period - 1]["Contracted_Power"]);
    } else {
      Periods.push_back(NAN);
      Colors.push_back("#ddd");
      Powers.push_back(NAN);
    }
////    if(Tariff != UINT32_MAX) Tools::LogMsg("INFO", "Tarifa encontrada para el rango\n");
//    //Busco en las tarifas en cual de ellas cae el periodo en el que estoy    
//    
//    
//    
//      
//    
//    
//    //float Active   = atof(db->ResultValue(i, "active_power"));
////    float Reactive = atof(db->ResultValue(i, "reactive_power"));
////    uint64_t Time  = strtoull(db->ResultValue(i, "sampletime"), NULL, 10);
////    Tools::LogMsg("INFO", "Sampletime: %" PRIu64 ", Activa: %f, Reactiva: %f\n", Time, Active, Reactive);
//
//    
//  //  busco tarifa aplicable en el periodo
//  //  busco periodo del dia en el que estoy
//  //  Es festivo ? es fin de semana?
//  //  calculo coste de energia y otros factores para el bloque de 15 minutos
//  //  Aplico configuracion de modulos adicionales
//  //  Si he pedido agregacion de 15 minutos escupo datos y reinicio contadores, si es la de 1 hora y ha habido cambio de hora pues lo mismo
//  //  
//    
  From += 900000;
  }
  Result["Sampletime"] = Sampletimes;
  Result["Active_Energy"] = Actives;
  Result["Reactive_Energy"] = Reactives;
  Result["Period_Name"] = Periods;
  Result["Period_Color"] = Colors;
  Result["Contracted_Power"] = Powers;
  req.FromJSON(Result);
  return;  
}
uint64_t TariffsInterface::BillStart(json tariffs, uint64_t timestamp) {
  return timestamp;
}
uint64_t TariffsInterface::BillEnd(json tariffs, uint64_t timestamp) {
  return timestamp;
}