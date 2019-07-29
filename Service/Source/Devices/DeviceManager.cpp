/* ================================== zMeter ===========================================================================
 * File:     DeviceInterface.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <math.h>
#include "DeviceManager.h"
#include "HTTPServer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
vector<Device*> DeviceManager::pDevices;
RWLocks         DeviceManager::pLock;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  HTTPServer::RegisterCallback("/getDevices",          GetDevicesCallback);
  HTTPServer::RegisterCallback("/getDevice",           GetDeviceCallback);
  HTTPServer::RegisterCallback("/stopDevice",          StopDeviceCallback);
  HTTPServer::RegisterCallback("/startDevice",         StartDeviceCallback);
  HTTPServer::RegisterCallback("/setDevice",           SetDeviceCallback);
  HTTPServer::RegisterCallback("/getAnalyzers",        GetAnalyzersCallback);
  HTTPServer::RegisterCallback("/getAnalyzer",         GetAnalyzerCallback);
  HTTPServer::RegisterCallback("/setAnalyzer",         SetAnalyzerCallback);
  HTTPServer::RegisterCallback("/getSeries",           GetSeriesCallback);
  HTTPServer::RegisterCallback("/getSeriesNow",        GetSeriesNowCallback);
  HTTPServer::RegisterCallback("/getSeriesRange",      GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("/getEvents",           GetEventsCallback);
  HTTPServer::RegisterCallback("/getEvent",            GetEventCallback);
  HTTPServer::RegisterCallback("/setEvent",            SetEventCallback);
  HTTPServer::RegisterCallback("/delEvent",            DelEventCallback);
  HTTPServer::RegisterCallback("/getRates",            GetRatesCallback);
  HTTPServer::RegisterCallback("/getRate",             GetRateCallback);
  HTTPServer::RegisterCallback("/setRate",             SetRateCallback);
  HTTPServer::RegisterCallback("/delRate",             DelRateCallback);
  HTTPServer::RegisterCallback("/addRate",             AddRateCallback);
  HTTPServer::RegisterCallback("/getPrices",           GetPricesCallback);
  pLock.ReadLock();
  for(uint i = 0; i < pDevices.size(); i++) pDevices[i]->StartDevice();
  pLock.ReadUnlock();
}
DeviceManager::~DeviceManager() {
  LOGFile::Info("Stopping devices...\n");
  pLock.WriteLock();
  for(uint i = 0; i < pDevices.size(); i++) delete pDevices[i];
  pDevices.resize(0);
  pLock.WriteUnlock();
}
bool DeviceManager::AddDevice(Device *dev) {
  pLock.WriteLock();
  for(uint n = 0; n < pDevices.size(); n++) {
    if(dev->Serial() == pDevices[n]->Serial()) {
      pLock.WriteUnlock();
      return false;
    }
  }
  pDevices.push_back(dev);
  pLock.WriteUnlock();
  return true;
}
void DeviceManager::GetDevicesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  json Response = json::array();
  pLock.ReadLock();
  for(uint i = 0; i < pDevices.size(); i++) Response.push_back(pDevices[i]->GetJSONDescriptor());
  pLock.ReadUnlock();
  req.FromJSON(Response);
}
void DeviceManager::GetDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(Params["Serial"].is_string() == false) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->GetDevice(req);
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetDeviceCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->SetDevice(req);
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::StartDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue; 
    pDevices[n]->StartDevice(req);
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::StopDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->StopDevice(req);
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetAnalyzersCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned, for other users only analyzers with any priviledge are returned
  json Params = req.GetParams();
  json Response = json::array();
  if(req.UserName() == "admin") {
    pLock.ReadLock();
    for(uint n = 0; n != pDevices.size(); n++) {
      json Analyzers = pDevices[n]->GetJSONAllAnalyzerDescriptors();
      for(uint i = 0; i < Analyzers.size(); i++) {
        Analyzers[i]["Read"] = true;        
        Analyzers[i]["Write"] = true;
        Analyzers[i]["Config"] = true;
      }
      Response.insert(Response.end(), Analyzers.begin(), Analyzers.end());
    }
    pLock.ReadUnlock();  
  } else {
    req.ExecSentenceAsyncResult("SELECT serial, read, write, config FROM permisions WHERE username = " + req.Bind(req.UserName()));
    while(req.FecthRow()) {
      string Serial = req.ResultString("serial");
      pLock.ReadUnlock();
      for(uint n = 0; n != pDevices.size(); n++) {
        json Analyzer = pDevices[n]->GetJSONAnalyzerDescriptor(Serial);
        if(!Analyzer.is_object()) continue;
        Analyzer["Read"] = (req.ResultString("read") == "t");
        Analyzer["Write"] = (req.ResultString("write") == "t");
        Analyzer["Config"] = (req.ResultString("config") == "t");
        Response.push_back(Analyzer);
        break;
      }
      pLock.ReadUnlock();
    }
  }
  req.FromJSON(Response);
}
void DeviceManager::GetAnalyzerCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetAnalyzer(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetAnalyzerCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetAnalyzer(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetSeriesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeries(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetSeriesNowCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeriesNow(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeriesRange(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetEvents(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetEvent(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetEvent(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::DelEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->DelEvent(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetRatesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetRates(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetRate(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetRate(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::DelRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->DelRate(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::AddRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->AddRate(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetPricesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetPrices(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}