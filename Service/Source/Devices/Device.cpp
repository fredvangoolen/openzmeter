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
string Device::Serial() const {
  return pSerial;
}
json Device::GetJSONDescriptor() {
  json Device;
  Device["Driver"] = pDriver;
  Device["Name"] = pName;
  Device["Serial"] = pSerial;
  Device["Analyzers"] = pAnalyzers.size();
  Device["Status"] = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  return Device;
}
json Device::GetJSONAnalyzerDescriptor(const string &serial) {
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    json Response = pAnalyzers[n]->GetJSONShortDescriptor();
    pAnalyzersLock.ReadUnlock();
    return Response;
  }
  pAnalyzersLock.ReadUnlock();
  return nullptr;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  json Response = json::array();
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) Response.push_back(pAnalyzers[n]->GetJSONShortDescriptor());
  pAnalyzersLock.ReadUnlock();
  return Response;
}
void Device::StartDevice(HTTPRequest &req) {
  json Response;
  Response["Status"] = StartDevice();
  req.FromJSON(Response); 
}
void Device::StopDevice(HTTPRequest &req) {
  json Response;
  Response["Status"] = StopDevice();
  req.FromJSON(Response);
}
bool Device::GetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    Analyzer *Analyzer = pAnalyzers[n];
    if(Analyzer->Serial() != Serial) continue;
    Analyzer->GetAnalyzer(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::SetAnalyzer(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetAnalyzer(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetSeries(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeries(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetSeriesNow(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeriesNow(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetSeriesRange(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeriesRange(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetEvents(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetEvents(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetEvent(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::SetEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetEvent(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::DelEvent(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->DelEvent(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetRates(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetRates(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetRate(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::SetRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetRate(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::DelRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->DelRate(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::AddRate(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->AddRate(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}
bool Device::GetPrices(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetPrices(req);
    pAnalyzersLock.ReadUnlock();
    return true;
  }
  pAnalyzersLock.ReadUnlock();
  return false;
}