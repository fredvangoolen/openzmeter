/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "Device.h"
#include "HTTPServer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
Device::Device() {
  pState = UNCONFIGURED;
}
Device::~Device() {
  pAnalyzersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pAnalyzersLock.WriteUnlock();
}
string Device::Serial() {
  string Serial = pSerial;
  return Serial;
}
json Device::GetJSONDescriptor() {
  json Device;
  Device["Driver"] = pDriver;
  Device["Name"] = pName;
  Device["Serial"] = pSerial;
  pAnalyzersLock.ReadLock();
  Device["Analyzers"] = pAnalyzers.size();
  pAnalyzersLock.ReadUnlock();
  Device["Status"] = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  return Device;
}
json Device::GetJSONAnalyzerDescriptor(const string &serial) {
  json Response = nullptr;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    Response = pAnalyzers[n]->GetJSONShortDescriptor();
    break;
  }
  pAnalyzersLock.ReadUnlock();
  return Response;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  json Response = json::array();
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) Response.push_back(pAnalyzers[n]->GetJSONShortDescriptor());
  pAnalyzersLock.ReadUnlock();
  return Response;
}
void Device::StartDevice(HTTPRequest &req) {
  json Response = json::object();
  Response["Status"] = StartDevice();
  req.FromJSON(Response);
}
void Device::StopDevice(HTTPRequest &req) {
  json Response = json::object();
  Response["Status"] = StopDevice();
  req.FromJSON(Response);
}
Analyzer *Device::FindAnalyzer(const string &serial) {
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    return pAnalyzers[n];
  }
  return NULL;
}
bool Device::GetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetAnalyzer(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->SetAnalyzer(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeries(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetSeries(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesNow(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetSeriesNow(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesRange(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);  
  if(Tmp != NULL) Tmp->GetSeriesRange(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvents(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);  
  if(Tmp != NULL) Tmp->GetEvents(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetEvent(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->SetEvent(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->DelEvent(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRates(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetRates(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetRate(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->SetRate(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->DelRate(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->AddRate(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetPrices(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Params["Analyzer"]);
  if(Tmp != NULL) Tmp->GetPrices(req);
  pAnalyzersLock.ReadUnlock();
  return (Tmp != NULL);
}