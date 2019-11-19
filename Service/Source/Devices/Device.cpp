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
Device::Device(const string &serial, const string &driver, const string &name) {
  pMembersLock.WriteLock();
  pState = UNCONFIGURED;
  pSerial = serial;
  pDriver = driver;
  pName = name;
  pMembersLock.WriteUnlock();
}
Device::~Device() {
  pMembersLock.WriteLock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pMembersLock.WriteUnlock();
}
string Device::Serial() {
  pMembersLock.ReadLock();
  string Serial = pSerial;
  pMembersLock.ReadUnlock();
  return Serial;
}
json Device::GetJSONDescriptor() {
  json Device = json::object();
  pMembersLock.ReadLock();
  Device["Driver"] = pDriver;
  Device["Name"] = pName;
  Device["Serial"] = pSerial;
  Device["Analyzers"] = pAnalyzers.size();
  Device["Status"] = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  pMembersLock.ReadUnlock();
  return Device;
}
json Device::GetJSONAnalyzerDescriptor(const string &serial) {
  json Response = nullptr;
  pMembersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    Response = pAnalyzers[n]->GetJSONShortDescriptor();
    break;
  }
  pMembersLock.ReadUnlock();
  return Response;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  json Response = json::array();
  pMembersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) Response.push_back(pAnalyzers[n]->GetJSONShortDescriptor());
  pMembersLock.ReadUnlock();
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
  Analyzer *Tmp = NULL;
  pMembersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    Tmp = pAnalyzers[n];
    break;
  }
  pMembersLock.ReadUnlock();
  return Tmp;
}
bool Device::GetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAnalyzer(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetAnalyzer(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeries(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetSeries(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesNow(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetSeriesNow(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesRange(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);  
  if(Tmp != NULL) Tmp->GetSeriesRange(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvents(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);  
  if(Tmp != NULL) Tmp->GetEvents(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetEvent(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetEvent(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelEvent(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRates(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetRates(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetRate(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetRate(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelRate(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->AddRate(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetPrices(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetPrices(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::TestAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->TestAlarm(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetAlarms(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAlarms(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAlarm(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetAlarm(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->AddAlarm(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelAlarm(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelAlarm(req);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}