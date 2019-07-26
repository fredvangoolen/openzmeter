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
vector<Device*> __attribute__((init_priority(200))) DeviceManager::pDevices;
RWLocks         __attribute__((init_priority(200))) DeviceManager::pLock;
DeviceManager *DeviceManager::pInstance = NULL;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  pLock.WriteLock();
  if(pInstance != NULL) {
    pLock.WriteUnlock();
    return;
  }
  HTTPServer::RegisterCallback("/getDevices",          GetDevicesCallback);
  HTTPServer::RegisterCallback("/getDevice",           GetDeviceCallback);
  HTTPServer::RegisterCallback("/stopDevice",          StopDeviceCallback);
  HTTPServer::RegisterCallback("/startDevice",         StartDeviceCallback);
  HTTPServer::RegisterCallback("/setDevice",           SetDeviceCallback);
  HTTPServer::RegisterCallback("/getAnalyzers",        DeviceManager::GetAnalyzersCallback);
  HTTPServer::RegisterCallback("/setAnalyzer",         DeviceManager::SetAnalyzerCallback);
  HTTPServer::RegisterCallback("/getSeries",           DeviceManager::GetSeriesCallback);
  HTTPServer::RegisterCallback("/getSeriesNow",        DeviceManager::GetSeriesNowCallback);
  HTTPServer::RegisterCallback("/getSeriesRange",      DeviceManager::GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("/getEvents",           DeviceManager::GetEventsCallback);
  HTTPServer::RegisterCallback("/getEvent",            DeviceManager::GetEventCallback);
  HTTPServer::RegisterCallback("/setEvent",            DeviceManager::SetEventCallback);
  HTTPServer::RegisterCallback("/delEvent",            DeviceManager::DelEventCallback);
  HTTPServer::RegisterCallback("/getPrices",           DeviceManager::GetPricesCallback);
  HTTPServer::RegisterCallback("/getTariffs",          DeviceManager::GetTariffsCallback);
  HTTPServer::RegisterCallback("/getTariff",           DeviceManager::GetTariffCallback);
  HTTPServer::RegisterCallback("/setTariff",           DeviceManager::SetTariffCallback);
  HTTPServer::RegisterCallback("/delTariff",           DeviceManager::DelTariffCallback);
  for(uint i = 0; i < pDevices.size(); i++) pDevices[i]->StartDevice();
  pInstance = this;
  pLock.WriteUnlock();
}
DeviceManager::~DeviceManager() {
  LOGFile::Info("Stopping devices...\n");
  pLock.WriteLock();
  for(uint i = 0; i < pDevices.size(); i++) delete pDevices[i];
  pInstance = NULL;
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
  Device *Device = NULL;
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() == Serial) Device = pDevices[n];
  }
  if(Device != NULL) {
    Device->GetDeviceCallback(req);
  } else {
    req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  }
  pLock.ReadUnlock();
}
void DeviceManager::SetDeviceCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  Device *Device = NULL;
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() == Serial) Device = pDevices[n];
  }
  if(Device != NULL) {
    Device->SetDeviceCallback(req);
  } else {
    req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  }
  pLock.ReadUnlock();
}
void DeviceManager::StartDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  Device *Device = NULL;
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() == Serial) Device = pDevices[n];
  }
  if(Device != NULL) {
    Device->StartDeviceCallback(req);
  } else {
    req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  }
  pLock.ReadUnlock();
}
void DeviceManager::StopDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  Device *Device = NULL;
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() == Serial) Device = pDevices[n];
  }
  if(Device != NULL) {
    Device->StopDeviceCallback(req);
  } else {
    req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  }
  pLock.ReadUnlock();
}
void DeviceManager::GetAnalyzersCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned, for other users only analyzers with any priviledge are returned
  json Params = req.GetParams();
  json Response = json::array();
  if(req.UserName() == "admin") {
    pLock.ReadLock();
    json Result = json::array();
    for(uint n = 0; n != pDevices.size(); n++) {
      json Analyzers = pDevices[n]->GetJSONAllAnalyzerDescriptors();
      for(uint i = 0; i < Analyzers.size(); i++) {
        Analyzers[i]["Read"] = true;        
        Analyzers[i]["Write"] = true;
        Analyzers[i]["Config"] = true;
      }
      Result.insert(Result.end(), Analyzers.begin(), Analyzers.end());
    }
    pLock.ReadUnlock();
    return req.FromJSON(Result);
  } else {
    req.ExecSentenceAsyncResult("SELECT serial, read, write, config FROM permisions WHERE username = " + req.Bind(req.UserName()));
    while(req.FecthRow()) {
      string Serial = req.ResultString("serial");
      pLock.ReadLock();
      for(uint n = 0; n != pDevices.size(); n++) {
        json Analyzer = pDevices[n]->GetJSONAnalyzerDescriptor(Serial);
        if(Analyzer.is_object()) {
          Analyzer["Read"] = (req.ResultString("read") == "t");
          Analyzer["Write"] = (req.ResultString("write") == "t");
          Analyzer["Config"] = (req.ResultString("config") == "t");
          Response.push_back(Analyzer);
          break;
        }
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
    if(pDevices[n]->GetAnalyzerCallback(req) == false) continue;
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
    if(pDevices[n]->SetAnalyzerCallback(req) == false) continue;
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
    if(pDevices[n]->GetSeriesCallback(req) == false) continue;
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
    if(pDevices[n]->GetSeriesNowCallback(req) == false) continue;
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
    if(pDevices[n]->GetSeriesRangeCallback(req) == false) continue;
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
    if(pDevices[n]->GetEventsCallback(req) == false) continue;
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
    if(pDevices[n]->GetEventCallback(req) == false) continue;
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
    if(pDevices[n]->SetEventCallback(req) == false) continue;
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
    if(pDevices[n]->DelEventCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetTariffsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetTariffsCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::GetTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetTariffCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::SetTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetTariffCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::DelTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->DelTariffCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}
void DeviceManager::AddTariffCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  pLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->AddTariffCallback(req) == false) continue;
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
    if(pDevices[n]->GetPricesCallback(req) == false) continue;
    pLock.ReadUnlock();
    return;
  }
  pLock.ReadUnlock();
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
}