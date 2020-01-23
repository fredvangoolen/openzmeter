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
Device::Device(const string &serial, const string &driver, const string &name) : Serial(serial), Driver(driver), Name(name) {
  pState  = UNCONFIGURED;
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
json Device::GetJSONDescriptor() {
  json Device = json::object();
  Device["Driver"]    = Driver;
  Device["Name"]      = Name;
  Device["Serial"]    = Serial;
  pMembersLock.ReadLock();
  Device["Analyzers"] = pAnalyzers.size();
  Device["Status"]    = (pState == Device::UNCONFIGURED) ? "UNCONFIGURED" : (pState == Device::STOPPED) ? "STOPPED" : "RUNNING";
  pMembersLock.ReadUnlock();
  return Device;
}
json Device::GetJSONAnalyzers(const string &user) {
  json Result = json::array();
  pMembersLock.ReadLock();
  for(Analyzer *Tmp : pAnalyzers) {
    if(Tmp->CheckRights(user, Analyzer::READ)) Result.push_back(Tmp->GetJSONDescriptor());
  }
  pMembersLock.ReadUnlock();
  return Result;
}
void Device::StartDevice(const json &params, Response &response) {
  json Response = json::object();
  Response["Status"] = StartDevice();
  HTTPCallback::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void Device::StopDevice(const json &params, Response &response) {
  json Response = json::object();
  Response["Status"] = StopDevice();
  HTTPCallback::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
bool Device::CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response)) {
  string Serial = params["Analyzer"];
  pMembersLock.ReadLock();
  for(Analyzer *Tmp : pAnalyzers) {
    if(Tmp->Serial != Serial) continue;
    (Tmp->*method)(params, response);
    pMembersLock.ReadUnlock();
    return true;
  }
  pMembersLock.ReadUnlock();
  return false;
}
bool Device::GetAnalyzer(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetAnalyzer);
}
bool Device::SetAnalyzer(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetAnalyzer);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["MaxCurrent"].is_number() || !Params["MinCurrent"].is_number() || !Params["Name"].is_string() || !Params["NominalFreq"].is_number() || !Params["NominalVoltage"].is_number()) return req.FromString("'MaxCurrent', 'MinCurrent', 'Name', 'NominalFreq' and 'NominalVoltage' param is requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->SetAnalyzer(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetSeries(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeries);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' param are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "10C") && (Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetSeries(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetSeriesNow(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeriesNow);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["Series"].is_array()) return req.FromString("Series must be an array of valid serie names", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetSeriesNow(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetSeriesRange(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetSeriesRange);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '10C', 3S', '1M', '10M', '15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetSeriesRange(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetEvents(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetEvents);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["To"].is_number() || !Params["Period"].is_number() || !Params["LastSample"].is_number()) return req.FromString("'To', 'Period' and 'LastSample' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetEvents(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetEvent);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Samples"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Samples' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetEvent(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::SetEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetEvent);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string() || !Params["Notes"].is_string()) return req.FromString("'Analyzer', 'StartTime', 'Type' and 'Notes' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->SetEvent(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::DelEvent(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelEvent);
  //  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  if(!Params["StartTime"].is_number_unsigned() || !Params["Type"].is_string()) return req.FromString("'Analyzer', 'StartTime' and 'Type' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->DelEvent(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetRates(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetRates);
}
bool Device::GetRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetRate);
}
bool Device::SetRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetRate);
}
bool Device::DelRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelRate);
}
bool Device::AddRate(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::AddRate);
}
bool Device::GetPrices(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetPrices);
}
bool Device::TestAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::TestAlarm);
  //  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["Expression"].is_string()) return req.FromString("'Analyzer' and 'Expression' params are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->TestAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetAlarms(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetAlarms);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetAlarms(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetAlarm);
  //  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned()) return req.FromString("'Analyzer' and 'AlarmID' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::SetAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetAlarm);
//  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned() || !Params["Name"].is_string() || !Params["Priority"].is_number_unsigned() || !Params["Expression"].is_string() || !Params["Aggreg"].is_string() || !Params["Message"].is_string() || !Params["Samples"].is_number_unsigned() || !Params["Window"].is_number_unsigned()) return req.FromString("'Analyzer', 'AlarmID', 'Name', 'Priority', 'Expression', 'Aggreg', 'Message', 'Samples' and 'Window' params are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->SetAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::AddAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::AddAlarm);
 //  json Params = req.GetParams();
//  if(!Params["Analyzer"].is_string() || !Params["Name"].is_string() || !Params["Priority"].is_number_unsigned() || !Params["Expression"].is_string() || !Params["Aggreg"].is_string() || !Params["Message"].is_string() || !Params["Samples"].is_number_unsigned() || !Params["Window"].is_number_unsigned()) return req.FromString("'Analyzer', 'Name', 'Priority', 'Expression', 'Aggreg', 'Message', 'Samples' and 'Window' params are requiered", MHD_HTTP_BAD_REQUEST);
//  if((!Params["Aggreg"].is_string()) || ((Params["Aggreg"] != "3S") && (Params["Aggreg"] != "1M") && (Params["Aggreg"] != "10M") && (Params["Aggreg"] != "15M") && (Params["Aggreg"] != "1H"))) return req.FromString("'Aggreg' need to be '3S', '1M', '10M', 15M' or '1H'", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->AddAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::DelAlarm(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelAlarm);
 //  if(!Params["Analyzer"].is_string() || !Params["AlarmID"].is_number_unsigned()) return req.FromString("'Analyzer' and 'AlarmID' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->DelAlarm(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::GetPermissions(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::GetPermissions);
 //  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->GetPermissions(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::AddPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::AddPermission);
//  if(!Params["Analyzer"].is_string() || !Params["User"].is_string() || !Params["Read"].is_boolean() || !Params["Write"].is_boolean() || !Params["Configure"].is_boolean()) return req.FromString("'Analyzer', 'User', 'Read', 'Write' and 'Configure' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->AddPermission(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::SetPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::SetPermission);
//  if(!params["Analyzer"].is_string() || !params["User"].is_string() || !params["Read"].is_boolean() || !params["Write"].is_boolean() || !params["Configure"].is_boolean()) return req.FromString("'Analyzer', 'User', 'Read', 'Write' and 'Configure' params are requiered", MHD_HTTP_BAD_REQUEST);
//  req.FromString("Analyzer not found", MHD_HTTP_BAD_REQUEST);
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->SetPermission(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
bool Device::DelPermission(const json &params, Response &response) {
  return CallAnalyzerFunction(params, response, &Analyzer::DelPermission);

//  if(!params["Analyzer"].is_string() || !params["User"].is_string()) {
//    response.set_content("'Analyzer' and 'User' params are requiered", "text/plain");
//    response.status = MHD_HTTP_BAD_REQUEST;
//    return;
//  }
//  response.set_content("Analyzer not found", "text/plain");
//  response.status = MHD_HTTP_BAD_REQUEST;
//  pDevicesLock.ReadLock();
//  for(Device *Dev : pDevices) {
//    if(Dev->DelPermission(req) == true) break;
//  }
//  pDevicesLock.ReadUnlock();
}
//480