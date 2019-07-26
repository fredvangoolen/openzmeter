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
Analyzer::Analyzer(const string &id, const Analyzer::AnalyzerMode mode) {
  pSerial = id;
  pMode = mode;
  pNominalVoltage = 230.0;
  pNominalFrequency = 50.0;
  pName = "Unnamed analyzer";
  pMaxCurrent = 35.0;
  pMinCurrent = 0.1;
  pSamplerate = NAN;
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
json Analyzer::CheckConfig(const json& config) {
  json   ConfigOUT = json::object();
//  float  Frequency = 50.0;
//  float  Voltage = 230.0;
//  float  Max_Current = 50.0;
//  float  Min_Current = 0.1;
//  string Country_Code = "UTC";
//  string Contract = "";
//  string Zip_Code = "";
//  string Name = "";
//  float  Geo[2] = {0, 0};
//  if(config.is_object()) {
//    if(config["Frequency"].is_number()) Frequency = config["Frequency"];
//  }
//  ConfigOUT["Frequency"] = Frequency;
//  
  return ConfigOUT;
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
void Analyzer::InitAggreg(Params_t &params, AggregType type) const {
  params.Type = type;
  params.Timestamp = 0;
  params.Frequency = NAN;
  params.Voltage_Inputs = 0;
  params.Frequency_Inputs = 0;
  params.Flag = false;
  params.Current_Inputs = vector<uint16_t>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, 0);
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M) || (params.Type == AGGREG_1H)) {
    params.Voltage = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
  }
  if(params.Type == AGGREG_1H) {
    params.StatsFrequency = vector<uint16_t>(FrequencyRanges.size() - 1, 0);
    params.StatsVoltage = vector<uint16_t>(VoltageRanges.size() - 1, 0);
    params.StatsTHD = vector<uint16_t>(THDRanges.size() - 1, 0);
    params.StatsHarmonics = vector<uint16_t>(2, 0);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_3S) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_15M) || (params.Type == AGGREG_1H)) {
    params.Active_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M)) {
    params.Current_THD = vector<float>((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, NAN);
    params.Phi = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Power_Factor = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Current_Harmonics = vector<vector<complex<float> > >((pMode == PHASE1) ? 1 : (pMode == PHASE3) ? 3 : 4, vector<complex<float> >(50, complex<float>(NAN, 0.0)));
  }
  if((params.Type == AGGREG_10C) || (params.Type == AGGREG_1M) || (params.Type == AGGREG_10M)) {
    params.Voltage_THD.resize((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Harmonics = vector<vector<complex<float> > >((pMode == PHASE1) ? 1 : 3, vector<complex<float> >(50, complex<float>(NAN, 0.0)));
  }
  if(params.Type == AGGREG_10C) {
    params.Active_Energy = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Voltage_Phase  = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Reactive_Energy = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
    params.Apparent_Power = vector<float>((pMode == PHASE1) ? 1 : 3, NAN);
  }
}
void Analyzer::AccumulateAggreg(Params_t &dst, const Params_t &src) const {
  dst.Timestamp = src.Timestamp;
  for(uint i = 0; i < dst.Voltage.size(); i++) dst.Voltage[i] = AccumulateRMS(dst.Voltage[i], src.Voltage[i]);
  dst.Voltage_Inputs += src.Voltage_Inputs;
  dst.Frequency = AccumulateSUM(dst.Frequency, src.Frequency);
  dst.Frequency_Inputs += src.Frequency_Inputs;
  for(uint i = 0; i < dst.Current.size(); i++) dst.Current[i] = AccumulateRMS(dst.Current[i], src.Current[i]);
  for(uint i = 0; i < dst.Current_Inputs.size(); i++) dst.Current_Inputs[i] += src.Current_Inputs[i];
  for(uint i = 0; i < dst.Active_Power.size(); i++) dst.Active_Power[i] = AccumulateSUM(dst.Active_Power[i], src.Active_Power[i]);
  for(uint i = 0; i < dst.Active_Energy.size(); i++) dst.Active_Energy[i] = AccumulateSUM(dst.Active_Energy[i], src.Active_Energy[i]);
  for(uint i = 0; i < dst.Reactive_Power.size(); i++) dst.Reactive_Power[i] = AccumulateSUM(dst.Reactive_Power[i], src.Reactive_Power[i]);
  for(uint i = 0; i < dst.Reactive_Energy.size(); i++) dst.Reactive_Energy[i] = AccumulateSUM(dst.Reactive_Energy[i], src.Reactive_Energy[i]);
  for(uint i = 0; i < dst.Power_Factor.size(); i++) dst.Power_Factor[i] = AccumulateSUM(dst.Power_Factor[i], src.Power_Factor[i]);
  for(uint i = 0; i < dst.Voltage_THD.size(); i++) dst.Voltage_THD[i] = AccumulateRMS(dst.Voltage_THD[i], src.Voltage_THD[i]);
  for(uint i = 0; i < dst.Current_THD.size(); i++) dst.Current_THD[i] = AccumulateRMS(dst.Current_THD[i], src.Current_THD[i]);
  for(uint i = 0; i < dst.Phi.size(); i++) dst.Phi[i] = AccumulateSUM(dst.Phi[i], src.Phi[i]);
  for(uint i = 0; i < dst.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Voltage_Harmonics[i].size(); j++) dst.Voltage_Harmonics[i][j] = AccumulateRMS(dst.Voltage_Harmonics[i][j], src.Voltage_Harmonics[i][j]);
  }
  for(uint i = 0; i < dst.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < dst.Current_Harmonics[i].size(); j++) dst.Current_Harmonics[i][j] = AccumulateRMS(dst.Current_Harmonics[i][j], src.Current_Harmonics[i][j]);
  }
  dst.Flag |= src.Flag;
}
void Analyzer::NormalizeAggreg(Params_t &params) const {
  for(uint i = 0; i < params.Voltage.size(); i++) params.Voltage[i] = sqrt(params.Voltage[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current.size(); i++) params.Current[i] = sqrt(params.Current[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Active_Power.size(); i++) params.Active_Power[i] = params.Active_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Reactive_Power.size(); i++) params.Reactive_Power[i] = params.Reactive_Power[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Power_Factor.size(); i++) params.Power_Factor[i] = params.Power_Factor[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Current_THD.size(); i++) params.Current_THD[i] = sqrt(params.Current_THD[i] / params.Current_Inputs[i]);
  for(uint i = 0; i < params.Phi.size(); i++) params.Phi[i] = params.Phi[i] / params.Current_Inputs[i];
  for(uint i = 0; i < params.Voltage_THD.size(); i++) params.Voltage_THD[i] = sqrt(params.Voltage_THD[i] / params.Voltage_Inputs);
  for(uint i = 0; i < params.Current_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Current_Harmonics[i].size(); j++) params.Current_Harmonics[i][j] = sqrt(params.Current_Harmonics[i][j] / (float)params.Current_Inputs[i]);
  }
  for(uint i = 0; i < params.Voltage_Harmonics.size(); i++) {
    for(uint j = 0; j < params.Voltage_Harmonics[i].size(); j++) params.Voltage_Harmonics[i][j] = sqrt(params.Voltage_Harmonics[i][j] / (float)params.Voltage_Inputs);
  }
  params.Frequency = params.Frequency / params.Frequency_Inputs;
  if(params.Frequency_Inputs > 1) params.Frequency_Inputs = 1;
  if(params.Voltage_Inputs > 1) params.Voltage_Inputs = 1;
  for(uint i = 0; i < params.Current_Inputs.size(); i++) {
    if(params.Current_Inputs[i] > 1) params.Current_Inputs[i] = 1;
  }
}
float Analyzer::AccumulateSUM(const float val1, const float val2) const {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return val2;
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + val2;
}
float Analyzer::AccumulateRMS(const float val1, const float val2) const {
  if(isnan(val1) && isnan(val2))   return NAN;
  if(isnan(val1) && !isnan(val2))  return pow(val2, 2.0);
  if(!isnan(val1) && isnan(val2))  return val1;
  return val1 + pow(val2, 2.0);
}
complex<float> Analyzer::AccumulateRMS(const complex<float> &val1, const complex<float> &val2) const {
  if(isnan(val1.real()) && isnan(val2.real()))   return complex<float>(NAN, 0.0);
  if(isnan(val1.real()) && !isnan(val2.real()))  return pow(val2, 2.0f);
  if(!isnan(val1.real()) && isnan(val2.real()))  return val1;
  return val1 + pow(val2, 2.0f);
}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
void Analyzer::GetAnalyzerCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  ReadLock();
  GetAnalyzer(req);
  ReadUnlock();
}
void Analyzer::SetAnalyzerCallback(HTTPRequest &req) {
  if(CheckRights(req, CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
//Faltan mas parametros por comprobar
  ReadLock();
  SetAnalyzer(req);
  ReadUnlock();
}
void Analyzer::GetSeriesCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' param are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  ReadLock();
  GetSeries(req);
  ReadUnlock();
}
void Analyzer::GetSeriesNowCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  ReadLock();
  GetSeriesNow(req);
  ReadUnlock();
}
void Analyzer::GetSeriesRangeCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  ReadLock();
  GetSeriesRange(req);
  ReadUnlock();
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
void Analyzer::GetTariffsCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  GetTariffs(req);
}
void Analyzer::GetTariffCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number()) return req.FromString("'RateID' param is requiered", MHD_HTTP_BAD_REQUEST);
  GetTariff(req);
}
void Analyzer::SetTariffCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'RateID', StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  SetTariff(req);    
}
void Analyzer::DelTariffCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'RateID', StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  DelTariff(req); 
}
void Analyzer::AddTariffCallback(HTTPRequest &req) {
  if(CheckRights(req, WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["CountryCode"].is_string()) return req.FromString("'StartDate', 'EndDate', 'CountryCode' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  AddTariff(req); 
}
void Analyzer::GetPricesCallback(HTTPRequest &req) {
  if(CheckRights(req, READ | WRITE | CONFIG) == false) return;
  json Params = req.GetParams();
  if(!Params["To"].is_number_unsigned() || !Params["Period"].is_number_unsigned() || !Params["LastSample"].is_number_unsigned()) return req.FromString("'To', 'Period' and 'LastSample' must be a numbers", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  GetPrices(req);
}