/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "Analyzers.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
vector<float> Analyzer::FrequencyRanges = {-INFINITY, -5, -4, -3, -2, -1, -0.9, -0.8, -0.7, -0.6, -0.5, -0.45, -0.40, -0.35, -0.30, -0.25, -0.20, -0.15, -0.10, -0.05, 0.00, 0.05, 0.10, 0.15, 0.20, 0.25, 0.30, 0.35, 0.40, 0.45, 0.5, 0.6, 0.7, 0.8, 0.9, 1, 2, 3, 4, 5, INFINITY};
vector<float> Analyzer::VoltageRanges = { -INFINITY, -20, -15, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 15, 20, INFINITY };
vector<float> Analyzer::THDRanges = { 0, 0.5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5, 7, 7.5, 8, 8.5, 9, 9.5, 10, 11, 12, 13, 14, 15, INFINITY };
vector<float> Analyzer::HarmonicsAllowed = {2, 5, 1, 6, 0.5, 5, 0.5, 1.5, 0.5, 3.5, 0.5, 3, 0.5, 0.5, 0.5, 2, 0.5, 1.5, 0.5, 0.5, 0.5, 1.5, 0.5, 1.5};
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) {
  pSerial = id;
  pName = "Unnamed analyzer";
  pLastSave = 0;
 
//  ExecSentenceNoResult("INSERT INTO analyzers(username, serial, country_code, contract, zipcode, frequency, voltage, name, geo, max_current, min_current) "
//                       "VALUES('admin', " + Bind(id) + ", 'UTC', '', '', 50.0, 230.0, 'Unnamed input analyzer', POINT(0, 0), 50.0, 0.1) ON CONFLICT(serial) DO NOTHING");
//  ExecSentenceAsyncResult("SELECT frequency, voltage, name, max_current, min_current FROM analyzers WHERE serial = " + Bind(id));
//  if(FecthRow()) {
//    pNominalVoltage = ResultFloat("voltage");
//    pNominalFrequency = ResultFloat("frequency");
//    pName = ResultString("name");
//    pMaxCurrent = ResultFloat("max_current");
//    pMinCurrent =  ResultFloat("min_current");
//  } else {
//    LOGFile::Error("Can not create or read analyzer configuration\n");
//  }
}
Analyzer::~Analyzer() {
}
//float Analyzer::GetMaxCurrent() const {
//  return pMaxCurrent;
//}
string Analyzer::Serial() const {
  return pSerial;
}
//float Analyzer::GetNominalVoltage() const {
//  return pNominalVoltage;
//}
//float Analyzer::GetNominalFrequency() const {
//  return pNominalFrequency;
//}
//Analyzer::AnalyzerMode Analyzer::Mode() const {
//  return pMode;
//}
json Analyzer::GetJSONShortDescriptor() const {
  json Response;
  Response["ID"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  return Response;
}
//complex<float> Analyzer::AccumulateRMS(const complex<float> &val1, const complex<float> &val2) const {
//  if(isnan(val1.real()) && isnan(val2.real()))   return complex<float>(NAN, 0.0);
//  if(isnan(val1.real()) && !isnan(val2.real()))  return pow(val2, 2.0f);
//  if(!isnan(val1.real()) && isnan(val2.real()))  return val1;
//  return val1 + pow(val2, 2.0f);
//}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
void Analyzer::GetSeriesCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' param are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  GetSeries(req);
}
void Analyzer::GetSeriesNowCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  GetSeriesNow(req);
}
void Analyzer::GetSeriesRangeCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  GetSeriesRange(req);
}
void Analyzer::GetEventsCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
  GetEvents(req);
}
void Analyzer::GetEventCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
  GetEvent(req);
}
void Analyzer::DelEventCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string()) return req.FromString("'Analyzer', 'StartTime' and 'Type' params are requiered", MHD_HTTP_BAD_REQUEST);
  DelEvent(req);
}
void Analyzer::SetEventCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Notes"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Notes' params are requiered", MHD_HTTP_BAD_REQUEST);
  SetEvent(req);
}
void Analyzer::GetRatesCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  GetRates(req);
}
void Analyzer::GetRateCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number()) return req.FromString("'RateID' param is requiered", MHD_HTTP_BAD_REQUEST);
  GetRate(req);
}
void Analyzer::SetRateCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'RateID', StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  SetRate(req);    
}
void Analyzer::DelRateCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'RateID', StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  DelRate(req); 
}
void Analyzer::AddRateCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  AddRate(req); 
}
void Analyzer::GetPricesCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["To"].is_number_unsigned() || !Params["Period"].is_number_unsigned() || !Params["LastSample"].is_number_unsigned()) return req.FromString("'To', 'Period' and 'LastSample' must be a numbers", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  GetPrices(req);
}