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
Device::Device() : InstanceLocks() {
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
  Reference();
  string Serial = pSerial;
  Dereference();
  return Serial;
}
json Device::GetJSONDescriptor() {
  Reference();  
  json Device;
  Device["Driver"] = pDriver;
  Device["Name"] = pName;
  Device["Serial"] = pSerial;
  pAnalyzersLock.ReadLock();
  Device["Analyzers"] = pAnalyzers.size();
  pAnalyzersLock.ReadUnlock();
  Device["Status"] = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  Dereference();
  return Device;
}
json Device::GetJSONAnalyzerDescriptor(const string &serial) {
  Reference();
  json Response = nullptr;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    Response = pAnalyzers[n]->GetJSONShortDescriptor();
    break;
  }
  pAnalyzersLock.ReadUnlock();
  Dereference();
  return Response;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  Reference();
  json Response = json::array();
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) Response.push_back(pAnalyzers[n]->GetJSONShortDescriptor());
  pAnalyzersLock.ReadUnlock();
  Dereference();
  return Response;
}
void Device::StartDevice(HTTPRequest &req) {
  Reference();
  json Response = json::object();
  Response["Status"] = StartDevice();
  req.FromJSON(Response);
  Dereference();
}
void Device::StopDevice(HTTPRequest &req) {
  Reference();
  json Response = json::object();
  Response["Status"] = StopDevice();
  req.FromJSON(Response);
  Dereference();
}
bool Device::GetAnalyzer(HTTPRequest& req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetAnalyzer(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::SetAnalyzer(HTTPRequest& req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->SetAnalyzer(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetSeries(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetSeries(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetSeriesNow(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetSeriesNow(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetSeriesRange(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetSeriesRange(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetEvents(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetEvents(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetEvent(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetEvent(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::SetEvent(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->SetEvent(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::DelEvent(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->DelEvent(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetRates(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetRates(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetRate(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetRate(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::SetRate(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->SetRate(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::DelRate(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->DelRate(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::AddRate(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->AddRate(req);
  Dereference();
  return (Analyzer != NULL);
}
bool Device::GetPrices(HTTPRequest &req) {
  Reference();
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  Analyzer *Analyzer = NULL;
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    Analyzer = pAnalyzers[n];
    break;
  }
  pAnalyzersLock.ReadUnlock();
  if(Analyzer) Analyzer->GetPrices(req);
  Dereference();
  return (Analyzer != NULL);
}