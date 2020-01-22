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
  for(Analyzer *Tmp : pAnalyzers) {
    if(Tmp->Serial() != serial) continue;
    Response = Tmp->GetJSONShortDescriptor();
    break;
  }
  pMembersLock.ReadUnlock();
  return Response;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  json Response = json::array();
  pMembersLock.ReadLock();
  for(Analyzer *Tmp : pAnalyzers) Response.push_back(Tmp->GetJSONShortDescriptor());
  pMembersLock.ReadUnlock();
  return Response;
}
void Device::StartDevice(const json &params, Response &response) {
  json Response = json::object();
  Response["Status"] = StartDevice();
  response.set_content(Response.dump(), "application/json");
  response.status = HTTP_OK;
}
void Device::StopDevice(const json &params, Response &response) {
  json Response = json::object();
  Response["Status"] = StopDevice();
  response.set_content(Response.dump(), "application/json");
  response.status = HTTP_OK;
}
Analyzer *Device::FindAnalyzer(const string &serial) {
  pMembersLock.ReadLock();
  for(Analyzer *Tmp : pAnalyzers) {
    if(Tmp->Serial() != serial) continue;
    pMembersLock.ReadUnlock();
    return Tmp;
  }
  pMembersLock.ReadUnlock();
  return NULL;
}
bool Device::GetAnalyzer(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAnalyzer(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetAnalyzer(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetAnalyzer(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeries(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetSeries(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesNow(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetSeriesNow(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetSeriesRange(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetSeriesRange(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvents(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetEvents(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetEvent(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetEvent(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetEvent(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetEvent(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelEvent(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelEvent(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRates(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetRates(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetRate(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetRate(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetRate(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetRate(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelRate(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelRate(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddRate(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->AddRate(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetPrices(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetPrices(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::TestAlarm(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->TestAlarm(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetAlarms(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAlarms(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetAlarm(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetAlarm(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetAlarm(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetAlarm(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddAlarm(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->AddAlarm(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelAlarm(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelAlarm(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::GetPermissions(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->GetPermissions(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::AddPermission(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->AddPermission(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::SetPermission(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->SetPermission(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
bool Device::DelPermission(const json &params, Response &response) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  Analyzer *Tmp = FindAnalyzer(Serial);
  if(Tmp != NULL) Tmp->DelPermission(params, response);
  pMembersLock.ReadUnlock();
  return (Tmp != NULL);
}
