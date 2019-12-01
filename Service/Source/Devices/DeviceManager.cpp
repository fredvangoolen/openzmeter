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
  for(Device *Dev : pDevices) Dev->StartDevice();
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
  LOGFile::Info("Stopping devices...\n");
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
  for(Device *Dev : pDevices) {
    if(dev->Serial() != Dev->Serial()) continue;
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
  for(Device *Dev : pDevices) Response.push_back(Dev->GetJSONDescriptor());
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
  for(Device *Dev : pDevices) {
    if(Dev->Serial() != Serial) continue;
    Dev->GetDevice(req);
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
  for(Device *Dev : pDevices) {
    if(Dev->Serial() != Serial) continue;
    Dev->SetDevice(req);
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
  for(Device *Dev : pDevices) {
    if(Dev->Serial() != Serial) continue; 
    Dev->StartDevice(req);
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
  for(Device *Dev : pDevices) {
    if(Dev->Serial() != Serial) continue;
    Dev->StopDevice(req);
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
    for(Device *Dev : pDevices) {
      json Analyzers = Dev->GetJSONAllAnalyzerDescriptors();
      for(auto Analyzer : Analyzers) {
        Analyzer["Read"] = true;        
        Analyzer["Write"] = true;
        Analyzer["Config"] = true;
      }
      Response.insert(Response.end(), Analyzers.begin(), Analyzers.end());
    }
    pDevicesLock.ReadUnlock();
  } else {
    req.ExecSentenceAsyncResult("SELECT serial, read, write, config FROM permisions WHERE username = " + req.Bind(req.UserName()));
    while(req.FecthRow()) {
      string Serial = req.ResultString("serial");
      pDevicesLock.ReadUnlock();
      for(Device *Dev : pDevices) {
        json Analyzer = Dev->GetJSONAnalyzerDescriptor(Serial);
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
  for(Device *Dev : pDevices) {
    if(Dev->GetAnalyzer(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetAnalyzerCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->SetAnalyzer(req) == true) break;
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
  for(Device *Dev : pDevices) {
    if(Dev->GetSeries(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetSeriesNowCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetSeriesNow(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
  
}
void DeviceManager::GetSeriesRangeCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetSeriesRange(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetEventsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetEvents(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Notes"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Notes' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->SetEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::DelEventCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string()) return req.FromString("'Analyzer', 'StartTime' and 'Type' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->DelEvent(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetRatesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetRates(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["RateID"].is_number()) return req.FromString("'RateID' param is requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["RateID"].is_number() || !Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string() || !Params["Config"].is_object() || !Params["Template"].is_object()) return req.FromString("'RateID', StartDate', 'EndDate', 'Config', 'Template' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->SetRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::DelRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["RateID"].is_number()) return req.FromString("'Analyzer' and 'RateID0 params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->DelRate(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::AddRateCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  if(!Params["StartDate"].is_number_unsigned() || !Params["EndDate"].is_number_unsigned() || !Params["Name"].is_string()) return req.FromString("'StartDate', 'EndDate' and 'Name' params required", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->AddRate(req) == true) break;
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
  for(Device *Dev : pDevices) {
    if(Dev->GetPrices(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::TestAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["Expression"].is_string()) return req.FromString("'Analyzer' and 'Expression' params are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->TestAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetAlarmsCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetAlarms(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::GetAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned()) return req.FromString("'Analyzer' and 'AlarmID' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->GetAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::SetAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned() || !Params["Name"].is_string() || !Params["Priority"].is_number_unsigned() || !Params["Expression"].is_string() || !Params["Aggreg"].is_string() || !Params["Message"].is_string() || !Params["Samples"].is_number_unsigned() || !Params["Window"].is_number_unsigned()) return req.FromString("'Analyzer', 'AlarmID', 'Name', 'Priority', 'Expression', 'Aggreg', 'Message', 'Samples' and 'Window' params are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->SetAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::AddAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["Name"].is_string() || !Params["Priority"].is_number_unsigned() || !Params["Expression"].is_string() || !Params["Aggreg"].is_string() || !Params["Message"].is_string() || !Params["Samples"].is_number_unsigned() || !Params["Window"].is_number_unsigned()) return req.FromString("'Analyzer', 'Name', 'Priority', 'Expression', 'Aggreg', 'Message', 'Samples' and 'Window' params are requiered", MHD_HTTP_BAD_REQUEST);
  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->AddAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}
void DeviceManager::DelAlarmCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned()) return req.FromString("'Analyzer' and 'AlarmID' params are requiered", MHD_HTTP_BAD_REQUEST);
  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
  pDevicesLock.ReadLock();
  for(Device *Dev : pDevices) {
    if(Dev->DelAlarm(req) == true) break;
  }
  pDevicesLock.ReadUnlock();
}