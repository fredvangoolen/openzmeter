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
RWLocks         DeviceManager::pDevicesLock;
//----------------------------------------------------------------------------------------------------------------------
DeviceManager::DeviceManager() {
  pDevicesLock.ReadLock();
  for(uint i = 0; i < pDevices.size(); i++) pDevices[i]->StartDevice();
  pDevicesLock.ReadUnlock();
  HTTPServer::RegisterCallback("getDevices",          GetDevicesCallback);
  HTTPServer::RegisterCallback("getDevice",           GetDeviceCallback);
  HTTPServer::RegisterCallback("stopDevice",          StopDeviceCallback);
  HTTPServer::RegisterCallback("startDevice",         StartDeviceCallback);
  HTTPServer::RegisterCallback("setDevice",           SetDeviceCallback);
  HTTPServer::RegisterCallback("getAnalyzers",        GetAnalyzersCallback);
  HTTPServer::RegisterCallback("getAnalyzer",         GetAnalyzerCallback);
  HTTPServer::RegisterCallback("setAnalyzer",         SetAnalyzerCallback);
  HTTPServer::RegisterCallback("getSeries",           GetSeriesCallback);
  HTTPServer::RegisterCallback("getSeriesNow",        GetSeriesNowCallback);
  HTTPServer::RegisterCallback("getSeriesRange",      GetSeriesRangeCallback);
  HTTPServer::RegisterCallback("getEvents",           GetEventsCallback);
  HTTPServer::RegisterCallback("getEvent",            GetEventCallback);
  HTTPServer::RegisterCallback("setEvent",            SetEventCallback);
  HTTPServer::RegisterCallback("delEvent",            DelEventCallback);
  HTTPServer::RegisterCallback("getRates",            GetRatesCallback);
  HTTPServer::RegisterCallback("getRate",             GetRateCallback);
  HTTPServer::RegisterCallback("setRate",             SetRateCallback);
  HTTPServer::RegisterCallback("delRate",             DelRateCallback);
  HTTPServer::RegisterCallback("addRate",             AddRateCallback);
  HTTPServer::RegisterCallback("getPrices",           GetPricesCallback);
  HTTPServer::RegisterCallback("testAlarm",           TestAlarmCallback);
  HTTPServer::RegisterCallback("getAlarms",           GetAlarmsCallback);
  HTTPServer::RegisterCallback("getAlarm",            GetAlarmCallback);
  HTTPServer::RegisterCallback("setAlarm",            SetAlarmCallback);
  HTTPServer::RegisterCallback("addAlarm",            AddAlarmCallback);
  HTTPServer::RegisterCallback("delAlarm",            DelAlarmCallback);
}
DeviceManager::~DeviceManager() {
  LOGFile::Info("Stopping devices...\n");
  HTTPServer::UnregisterCallback("getDevices");
  HTTPServer::UnregisterCallback("getDevice");
  HTTPServer::UnregisterCallback("stopDevice");
  HTTPServer::UnregisterCallback("startDevice");
  HTTPServer::UnregisterCallback("setDevice");
  HTTPServer::UnregisterCallback("getAnalyzers");
  HTTPServer::UnregisterCallback("getAnalyzer");
  HTTPServer::UnregisterCallback("setAnalyzer");
  HTTPServer::UnregisterCallback("getSeries");
  HTTPServer::UnregisterCallback("getSeriesNow");
  HTTPServer::UnregisterCallback("getSeriesRange");
  HTTPServer::UnregisterCallback("getEvents");
  HTTPServer::UnregisterCallback("getEvent");
  HTTPServer::UnregisterCallback("setEvent");
  HTTPServer::UnregisterCallback("delEvent");
  HTTPServer::UnregisterCallback("getRates");
  HTTPServer::UnregisterCallback("getRate");
  HTTPServer::UnregisterCallback("setRate");
  HTTPServer::UnregisterCallback("delRate");
  HTTPServer::UnregisterCallback("addRate");
  HTTPServer::UnregisterCallback("getPrices");
  HTTPServer::UnregisterCallback("testAlarm");
  HTTPServer::UnregisterCallback("getAlarms");
  HTTPServer::UnregisterCallback("getAlarm");
  HTTPServer::UnregisterCallback("setAlarm");
  HTTPServer::UnregisterCallback("addAlarm");
  HTTPServer::UnregisterCallback("delAlarm");
  pDevicesLock.WriteLock();
  while(pDevices.size() > 0) {
    Device *Dev = pDevices.back();
    pDevices.pop_back();
    delete Dev;
  }
  pDevicesLock.WriteUnlock();
}
bool DeviceManager::AddDevice(Device *dev) {
  bool Result = true;
  pDevicesLock.WriteLock();
  for(uint n = 0; n < pDevices.size(); n++) {
    if(dev->Serial() != pDevices[n]->Serial()) continue;
    Result = false;
  }
  if(Result == true) pDevices.push_back(dev);
  pDevicesLock.WriteUnlock();
  return Result;
}
void DeviceManager::GetDevicesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_FORBIDDEN);
  json Response = json::array();
  pDevicesLock.ReadLock();
  for(uint i = 0; i < pDevices.size(); i++) Response.push_back(pDevices[i]->GetJSONDescriptor());
  pDevicesLock.ReadUnlock();
  req.FromJSON(Response);
}
void DeviceManager::GetDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_FORBIDDEN);
  if(Params["Serial"].is_string() == false) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->GetDevice(req);
    break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetDeviceCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_FORBIDDEN);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->SetDevice(req);
    break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::StartDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_FORBIDDEN);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue; 
    pDevices[n]->StartDevice(req);
    break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::StopDeviceCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can do this action
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_FORBIDDEN);
  if(!Params["Serial"].is_string()) return req.FromString("'Serial' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Serial"];
  req.FromString("Device not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->Serial() != Serial) continue;
    pDevices[n]->StopDevice(req);
    break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetAnalyzersCallback(HTTPRequest &req) {
  //All users can do this action, for admin all analyzers are returned, for other users only analyzers with any priviledge are returned
  json Params = req.GetParams();
  json Response = json::array();
  if(req.UserName() == "admin") {
    pDevicesLock.ReadLock();
    for(uint n = 0; n != pDevices.size(); n++) {
      json Analyzers = pDevices[n]->GetJSONAllAnalyzerDescriptors();
      for(uint i = 0; i < Analyzers.size(); i++) {
        Analyzers[i]["Read"] = true;        
        Analyzers[i]["Write"] = true;
        Analyzers[i]["Config"] = true;
      }
      Response.insert(Response.end(), Analyzers.begin(), Analyzers.end());
    }
    pDevicesLock.ReadUnlock();  
  } else {
    req.ExecSentenceAsyncResult("SELECT serial, read, write, config FROM permisions WHERE username = " + req.Bind(req.UserName()));
    while(req.FecthRow()) {
      string Serial = req.ResultString("serial");
      pDevicesLock.ReadUnlock();
      for(uint n = 0; n != pDevices.size(); n++) {
        json Analyzer = pDevices[n]->GetJSONAnalyzerDescriptor(Serial);
        if(!Analyzer.is_object()) continue;
        Analyzer["Read"] = (req.ResultString("read") == "t");
        Analyzer["Write"] = (req.ResultString("write") == "t");
        Analyzer["Config"] = (req.ResultString("config") == "t");
        Response.push_back(Analyzer);
        break;
      }
      pDevicesLock.ReadUnlock();
    }
  }
  req.FromJSON(Response);
}
void DeviceManager::GetAnalyzerCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetAnalyzer(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetAnalyzerCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetAnalyzer(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetSeriesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' param are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "10C") && (Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeries(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetSeriesNowCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeriesNow(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
  
}
void DeviceManager::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetSeriesRange(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetEvents(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Notes"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Notes' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::DelEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string()) return req.FromString("'Analyzer', 'StartTime' and 'Type' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->DelEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetRatesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetRates(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["RateID"].is_number()) return req.FromString("'RateID' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["Config"].is_object() || !Params["Template"].is_object()) return req.FromString("'RateID', StartDate', 'EndDate', 'Config', 'Template' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->SetRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::DelRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["RateID"].is_number()) return req.FromString("'Analyzer' and 'RateID0 params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->DelRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::AddRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["TimeZone"].is_string()) return req.FromString("'StartDate', 'EndDate', 'TimeZone' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->AddRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetPricesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["To"].is_number_unsigned() || !Params["Period"].is_number_unsigned() || !Params["LastSample"].is_number_unsigned()) return req.FromString("'To', 'Period' and 'LastSample' must be a numbers", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->GetPrices(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::TestAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["Expression"].is_string()) return req.FromString("'Analyzer' and 'Expression' params are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(uint n = 0; n != pDevices.size(); n++) {
    if(pDevices[n]->TestAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetAlarmsCallback(HTTPRequest &req) {
}
void DeviceManager::GetAlarmCallback(HTTPRequest &req) {
}
void DeviceManager::SetAlarmCallback(HTTPRequest &req) {
}
void DeviceManager::AddAlarmCallback(HTTPRequest &req) {
}
void DeviceManager::DelAlarmCallback(HTTPRequest &req) {
}
