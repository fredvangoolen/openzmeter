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
  WriteUnlock();
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  WriteUnlock();
}
string Device::Serial() const {
  return pSerial;
}
json Device::GetJSONDescriptor() {
  json Device;
  ReadLock();
  Device["Driver"] = pDriver;
  Device["Name"] = pName;
  Device["Serial"] = pSerial;
  Device["Analyzers"] = pAnalyzers.size();
  Device["Status"] = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  ReadUnlock();
  return Device;
}
json Device::GetJSONAnalyzerDescriptor(const string &serial) {
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != serial) continue;
    json Response = pAnalyzers[n]->GetJSONDescriptor();
    ReadUnlock();
    return Response;
  }
  ReadUnlock();
  return nullptr;
}
json Device::GetJSONAllAnalyzerDescriptors() {
  json Response = json::array();
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) Response.push_back(pAnalyzers[n]->GetJSONDescriptor());
  ReadUnlock();
  return Response;
}
void Device::GetDeviceCallback(HTTPRequest &req) {
  ReadLock();
  req.FromJSON(ConfigManager::ReadConfig("/Devices/" + pSerial));
  ReadUnlock();
}
void Device::SetDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Settings"].is_object()) return req.FromString("'Settings' param is required", MHD_HTTP_BAD_REQUEST);
  WriteLock();
  DeviceState State = pState;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Params["Settings"]);
  WriteUnlock();
  json Response;
  if(State == Device::RUNNING) {
    StopDevice();
    Response["Result"] = StartDevice();
  } else {
    Response["Result"] = true;
  }
  req.FromJSON(Response);
}
void Device::StartDeviceCallback(HTTPRequest &req) {
  json Response;
  Response["Status"] = StartDevice();
  req.FromJSON(Response); 
}
void Device::StopDeviceCallback(HTTPRequest &req) {
  json Response;
  Response["Status"] = StopDevice();
  req.FromJSON(Response);
}
bool Device::GetAnalyzerCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetAnalyzerCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::SetAnalyzerCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetAnalyzerCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetSeriesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeriesCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetSeriesNowCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeriesNowCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetSeriesRangeCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetEventsCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetEventCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::SetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetEventCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::DelEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->DelEventCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetTariffsCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetTariffCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::SetTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->SetTariffCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::DelTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->DelTariffCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::AddTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->AddTariffCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}
bool Device::GetPricesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  string Serial = Params["Analyzer"];
  ReadLock();
  for(uint n = 0; n != pAnalyzers.size(); n++) {
    if(pAnalyzers[n]->Serial() != Serial) continue;
    pAnalyzers[n]->GetPricesCallback(req);
    ReadUnlock();
    return true;
  }
  ReadUnlock();
  return false;
}