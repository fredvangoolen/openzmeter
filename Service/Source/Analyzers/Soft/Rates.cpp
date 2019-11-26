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
void AnalyzerSoft::Rates::GetRates(HTTPRequest &req) {
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(json_build_object('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate))), '[]') AS result FROM rates");
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void AnalyzerSoft::Rates::GetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  uint32_t RateID = Params["RateID"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT JSON_BUILD_OBJECT('RateID', rate, 'Name', name, 'StartDate', startdate, 'EndDate', enddate, 'Template', template, 'Config', config) AS result FROM rates WHERE rate = " + req.Bind(RateID));
  if(!req.FecthRow()) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  req.FromString(req.ResultString("Result"), MHD_HTTP_OK);
}
void AnalyzerSoft::Rates::SetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  uint32_t RateID   = Params["RateID"];
  Tariff T(Start, End, pParent->pTimeZone);
  if(T.SetTemplate(Params["Template"]) == false) return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  if(T.SetConfig(Params["Config"]) == false) return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  bool Result = req.ExecSentenceNoResult("UPDATE rates SET name = " + req.Bind(Name) + ", template = " + req.Bind(T.GetTemplate().dump()) + ", config = " + req.Bind(T.GetConfig().dump()) + ", startdate = " + req.Bind(Start) + ", enddate = " + req.Bind(End) + " WHERE rate = " + req.Bind(RateID));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::Rates::DelRate(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["RateID"].is_number_unsigned()) return req.FromString("'RateID' params required", MHD_HTTP_BAD_REQUEST);
  uint32_t RateID = Params["RateID"];
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  bool Result = req.ExecSentenceNoResult("DELETE FROM rates WHERE rate = " + req.Bind(RateID));
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::Rates::AddRate(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Config"].is_object() || !Params["Template"].is_object() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string()) return req.FromString("'StartDate', 'EndDate', 'Config', 'Template' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  uint64_t Start    = Params["StartDate"];
  uint64_t End      = Params["EndDate"];
  string   Name     = Params["Name"];
  Tariff T(Start, End, pParent->pTimeZone);
  if(T.SetTemplate(Params["Template"]) == false) return req.FromString("Invalid 'Template' format", MHD_HTTP_BAD_REQUEST);
  if(T.SetConfig(Params["Config"]) == false) return req.FromString("Invalid 'Config' format", MHD_HTTP_BAD_REQUEST);
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  bool Result = req.ExecSentenceNoResult("INSERT INTO rates(startdate, enddate, config, template, name) VALUES (" + req.Bind(Start) + ", " + req.Bind(End) + ", " + req.Bind(T.GetConfig().dump()) + ", " + req.Bind(T.GetTemplate().dump()) + ", " + req.Bind(Name) + ")");
  req.FromString((Result == true) ? "{}" : "", (Result == true) ? MHD_HTTP_OK : MHD_HTTP_INTERNAL_SERVER_ERROR);
}
void AnalyzerSoft::Rates::GetPrices(HTTPRequest &req) {
  //Parse, check params and prepare result struct
  json Params = req.GetParams();
  uint64_t To         = Params["To"];
  uint64_t Period     = Params["Period"];
  uint64_t LastSample = Params.value<uint64_t>("LastSample", 0);
  string   Aggreg     = Params["Aggreg"];
  uint64_t Now        = Tools::GetTime();
  if(To == 0) To = Now;
  uint64_t From = To - Period;
  uint32_t PeriodSeconds = 900000;
  if(Aggreg == "1H") PeriodSeconds = 3600000;
  if((Period / PeriodSeconds) > 100000) return req.FromString("Returned period to large for this aggregation level", MHD_HTTP_BAD_REQUEST);
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
  req.ExecSentenceNoResult("SET search_path TO " + req.Bind(pParent->pSerial));
  req.ExecSentenceAsyncResult("SELECT to_json(array_agg(json_build_object('Name', rates.name, 'StartDate', startdate, 'EndDate', enddate, 'Config', config, 'Template', template) ORDER BY startdate ASC)) as rate FROM rates WHERE enddate >= " + req.Bind(From) + " AND startdate <= " + req.Bind(To));
  json DBTariffs = json::array();
  if(req.FecthRow()) {
    Result["Currency"] = "€";//req.ResultString("currency");
    DBTariffs = req.ResultJSON("rate");
  }
  TariffList Tariffs(DBTariffs, pParent->pTimeZone);
  //Find start and end date to retrieve data
  uint64_t BillingFrom = Tariffs.BillFrom((LastSample > From) ? LastSample : From);
  uint64_t BillingTo = Tariffs.BillTo(To);
  //Retrieve active and reactive samples
  req.ExecSentenceAsyncResult("SELECT sampletime, (SELECT SUM(s) FROM UNNEST(active_power) s) as active_power, (SELECT SUM(s) FROM UNNEST(reactive_power) s) as reactive_power FROM aggreg_15m WHERE sampletime > " + req.Bind(BillingFrom) + " AND sampletime <= " + req.Bind(BillingTo) + " ORDER BY sampletime ASC");
  bool MoreData = req.FecthRow();
  //Traverse all samples
  float AccActive = NAN;
  float AccReactive = NAN;
  float AccCost = NAN;
  for(uint64_t time = BillingFrom + 899999; time != BillingTo; time = time + 900000) {
    float Active = NAN, Reactive = NAN;
    if((MoreData == true) && (req.ResultUInt64("sampletime") == time)) {
      Active = req.ResultFloat("active_power");
      Reactive = req.ResultFloat("reactive_power");
      MoreData = req.FecthRow();
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
  req.FromJSON(Result);
}