/* ================================== zMeter ===========================================================================
 * File:     Rates.h
 * Author:   Eduardo Viciana (2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tariff.h"
#include "Rates.h"
AnalyzerSoft::Rates::Rates(AnalyzerSoft *parent) {
  pParent = parent;
}
AnalyzerSoft::Rates::~Rates() {
}
void AnalyzerSoft::Rates::GetRates(const json &params, Response &response) {
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') AS result FROM " + DB.BindID(pParent->pSerial) + ".rates");
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("Result"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::Rates::GetRate(const json &params, Response &response) {
  if(!params["RateID"].is_number_unsigned()) {
    response.set_content("'RateID' params required", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  uint32_t RateID = params["RateID"];
  Database DB(pParent->pSerial);
  DB.ExecSentenceAsyncResult("SELECT JSON_BUILD_OBJECT('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) AS result FROM rates WHERE rate = " + DB.Bind(RateID));
  if(!DB.FecthRow()) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("Result"), "application/json");
    response.status = HTTP_OK;
  }
}
void AnalyzerSoft::Rates::SetRate(const json &params, Response &response) {
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  uint32_t RateID   = params["RateID"];
  Tariff T(Start, End, pParent->pTimeZone);
  if(T.SetTemplate(params["Template"]) == false) {
    response.set_content("Invalid template", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  if(T.SetConfig(params["Config"]) == false) {
    response.set_content("Invalid 'Config' format", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  Database DB(pParent->pSerial);
  bool Result = DB.ExecSentenceNoResult("UPDATE rates SET name = " + DB.Bind(Name) + ", template = " + DB.Bind(T.GetTemplate().dump()) + ", config = " + DB.Bind(T.GetConfig().dump()) + ", startdate = " + DB.Bind(Start) + ", enddate = " + DB.Bind(End) + " WHERE rate = " + DB.Bind(RateID));
  response.set_content("", "text/plain");
  response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void AnalyzerSoft::Rates::DelRate(const json &params, Response &response) {
  if(!params["RateID"].is_number_unsigned()) {
    response.set_content("'RateID' params required", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  uint32_t RateID = params["RateID"];
  Database DB(pParent->pSerial);
  bool Result = DB.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + DB.Bind(RateID));
  response.set_content("", "text/plain");
  response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void AnalyzerSoft::Rates::AddRate(const json &params, Response &response) {
  if(!params["Config"].is_object() || !params["Template"].is_object() || !params["StartDate"].is_number_unsigned() || !params["EndDate"].is_number_unsigned() || !params["Name"].is_string()) {
    response.set_content("'StartDate', 'EndDate', 'Config', 'Template' and 'Name' params required", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  uint64_t Start    = params["StartDate"];
  uint64_t End      = params["EndDate"];
  string   Name     = params["Name"];
  Tariff T(Start, End, pParent->pTimeZone);
  if(T.SetTemplate(params["Template"]) == false) {
    response.set_content("Invalid template", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  if(T.SetConfig(params["Config"]) == false) {
    response.set_content("Invalid 'Config' format", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  Database DB(pParent->pSerial);
  bool Result = DB.ExecSentenceNoResult("INSERT INTO rates(startdate, enddate, config, template, name) VALUES (" + DB.Bind(Start) + ", " + DB.Bind(End) + ", " + DB.Bind(T.GetConfig().dump()) + ", " + DB.Bind(T.GetTemplate().dump()) + ", " + DB.Bind(Name) + ")");
  response.set_content("", "text/plain");
  response.status = (Result == true) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR;
}
void AnalyzerSoft::Rates::GetPrices(const json &params, Response &response) {
  //Parse, check params and prepare result struct
  uint64_t To         = params["To"];
  uint64_t Period     = params["Period"];
  uint64_t LastSample = params.value<uint64_t>("LastSample", 0);
  string   Aggreg     = params["Aggreg"];
  uint64_t Now        = Tools::GetTime();
  if(To == 0) To = Now;
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) {
    response.set_content("Returned period to large for this aggregation level", "text/plain");
    response.status = HTTP_BAD_REQUEST;
    return;
  }
  json Result;
  Result["To"]                 = To;
  Result["From"]               = From;
  Result["Aggregation_Source"] = Aggreg;
  Result["Sampletime"]         = json::array();
  Result["Active_Power"]       = json::array();
  Result["Reactive_Power"]     = json::array();
  Result["Period_Name"]        = json::array();
  Result["Period_Color"]       = json::array();
  Result["Contracted_Power"]   = json::array();
  Result["Expense"]            = json::array();
  Result["Bills"]              = json::array();
  //Load involved tariffs, currency and timezone
  Database DB(pParent->pSerial);
  DB.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + DB.Bind(From) + " AND startdate <= " + DB.Bind(To));
  json DBTariffs = json::array();
  if(DB.FecthRow()) {
    Result["Currency"] = "€";//req.ResultString("currency");
    DBTariffs = DB.ResultJSON("rate");
  }
  TariffList Tariffs(DBTariffs, pParent->pTimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  DB.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + DB.Bind(BillingFrom) + " AND sampletime <= " + DB.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = DB.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (DB.ResultUInt64("sampletime") == time)) {
      Active = DB.ResultFloat("active_power");
      Reactive = DB.ResultFloat("reactive_power");
      MoreData = DB.FecthRow();
    }
    Tariff *Tariff = Tariffs.Find(time);
    if((Tariff != NULL) && (Tariff->PushSample(time, Active, Reactive) == true) && ((time + 900000) != BillingTo)) continue;  //If under a tariff push samples until bill cycle ends or no more data!
    //Recover/group values
    do {
      string   Color      = "";
      string   Period     = "";
      uint64_t Time       = time;
      float    Cost       = NAN;
      float    Contracted = NAN;
      if((Tariff != NULL) && (Tariff->PopSample(Time, Active, Reactive, Period, Color, Cost, Contracted) == false)) {
        Result["Bills"].push_back(Tariff->GetResume(Now));
        break;
      }
      if(Time <= Now) AccActive  = isnan(AccActive) ? Active : isnan(Active) ? AccActive : AccActive + Active;
      if(Time <= Now) AccReactive = isnan(AccReactive) ? Reactive : isnan(Reactive) ? AccReactive : AccReactive + Reactive;
      if(Time <= Now) AccCost = isnan(AccCost) ? Cost : isnan(Cost) ? AccCost : AccCost + Cost;
      if(Aggreg == "1H") {
        if((Time % 3600000) != 3599999) {
          if(Tariff != NULL) continue;
          break;
        }
        AccActive = AccActive / 4.0;
        AccReactive = AccReactive / 4.0;
      }
      if((Time > LastSample) && (Time >= From) && (Time <= To)) {
        Result["Sampletime"].push_back(Time);
        Result["Active_Power"].push_back(AccActive);
        Result["Reactive_Power"].push_back(AccReactive);
        Result["Contracted_Power"].push_back(Contracted);
        if(Color.empty() == true)
          Result["Period_Color"].push_back(nullptr);
        else
          Result["Period_Color"].push_back(Color);
        if(Period.empty() == true)
          Result["Period_Name"].push_back(nullptr);
        else
          Result["Period_Name"].push_back(Period);
        Result["Expense"].push_back(AccCost);
      }
      AccActive = NAN;
      AccReactive = NAN;
      AccCost = NAN;
    } while(Tariff != NULL);
  }
  response.set_content(Result.dump(), "application/json");
  response.status = HTTP_OK;
}