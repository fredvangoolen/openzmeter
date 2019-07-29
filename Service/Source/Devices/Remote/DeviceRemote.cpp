/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "DeviceRemote.h"
#include "DeviceManager.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::pRegistered = DeviceRemote::GetInstances();
DeviceRemote *DeviceRemote::pInstance = NULL;
// Funciones ===========================================================================================================
bool DeviceRemote::GetInstances() {
  LOGFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'\n");
  pInstance = new DeviceRemote();
  DeviceManager::AddDevice(pInstance);
  return true;
}
DeviceRemote::DeviceRemote() {
  pSerial = "REMOTE";
  pDriver = "Remote";
  pName   = "Allow repplication of remote openZmeter devices in local machine";
}
DeviceRemote::~DeviceRemote() {
  StopDevice();
}
bool DeviceRemote::StartDevice() {
  pAnalyzersLock.WriteLock();
  if(pState == RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Loading 'Remote' driver configuration\n");
  bool AutoStart = false;
  json ConfigIN  = ConfigManager::ReadConfig("/Devices/" + pSerial);
  json ConfigOUT = json::object();
  ConfigOUT["Analyzers"] = json::array();
  if(ConfigIN["AutoStart"].is_boolean()) AutoStart = ConfigIN["AutoStart"];
  if(ConfigIN.is_object() && ConfigIN["Analyzers"].is_array()) {
    for(uint n = 0; n < ConfigIN["Analyzers"].size(); n++) {
      if(ConfigIN["Analyzers"][n].is_string()) ConfigOUT["Analyzers"].push_back(ConfigIN["Analyzers"][n]);
    }
  }
  ConfigOUT["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, ConfigOUT);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pAnalyzersLock.WriteUnlock();
      return true;
    }
  }
  LOGFile::Info("Starting 'Remote' driver\n");
  HTTPServer::RegisterCallback("/setRemote",  SetRemoteCallback);
  HTTPServer::RegisterCallback("/getRemote",  GetRemoteCallback);
  for(uint n = 0; n < ConfigOUT["Analyzers"].size(); n++) pAnalyzers.push_back(new AnalyzerRemote(ConfigOUT["Analyzers"][n]));
  pState = RUNNING;
  pAnalyzersLock.WriteUnlock();
  return true;
}
bool DeviceRemote::StopDevice() {
  pAnalyzersLock.WriteLock();
  if(pState != RUNNING) {
    pAnalyzersLock.WriteUnlock();
    return false;
  }
  LOGFile::Info("Stopping 'Remote' driver...\n");
  while(pAnalyzers.size() > 0) {
    Analyzer *Analyzer = pAnalyzers.back();
    pAnalyzers.pop_back();
    delete Analyzer;
  }
  pState = STOPPED;  
  pAnalyzersLock.WriteUnlock();
  return true;
}
void DeviceRemote::GetDevice(HTTPRequest &req) {}
void DeviceRemote::SetDevice(HTTPRequest &req) {
}
void DeviceRemote::GetRemoteCallback(HTTPRequest &req) {}
void DeviceRemote::SetRemoteCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  if(!Params["Analyzer"].is_string()) return req.FromString("'Analyzer' param is requiered", MHD_HTTP_BAD_REQUEST);
  string Serial = Params["Analyzer"];
  pInstance->pAnalyzersLock.ReadLock();
  for(uint n = 0; n != pInstance->pAnalyzers.size(); n++) {
    AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->pAnalyzers[n];
    if(Analyzer->Serial() != Serial) continue;
    Analyzer->SetRemote(req);
    pInstance->pAnalyzersLock.ReadUnlock();
    return;
  }
  pInstance->pAnalyzersLock.ReadUnlock();
}


//bool DeviceRemote::SetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
//  char *Serial = NULL, *Name = NULL, *Mode = NULL;
//  float NominalVoltage = NAN, NominalFreq = NAN, MaxCurrent = NAN, MinCurrent = NAN, SampleFreq = NAN;
//  bool Online = false;
//  json_object *Find;
//  if((json_object_object_get_ex(reader, "ID", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalVoltage = json_object_get_double(Find);
//  if((json_object_object_get_ex(reader, "NominalFreq", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalFreq = json_object_get_double(Find);
//  if((json_object_object_get_ex(reader, "Name", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Name = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "MaxCurrent", &Find) == true) && (json_object_get_type(Find) == json_type_double)) MaxCurrent = json_object_get_double(Find);
//  if((json_object_object_get_ex(reader, "MinCurrent", &Find) == true) && (json_object_get_type(Find) == json_type_double)) MinCurrent = json_object_get_double(Find);
//  if((json_object_object_get_ex(reader, "Mode", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Mode = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(reader, "SampleFreq", &Find) == true) && (json_object_get_type(Find) == json_type_double)) SampleFreq = json_object_get_double(Find);
//  if((json_object_object_get_ex(reader, "Online", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Online = json_object_get_boolean(Find);
//  AnalyzerInterface::AnalyzerMode ModeB;
//  if(strcmp(Mode, "1PHASE") == 0) {
//    ModeB = AnalyzerInterface::PHASE1;
//  } else if(strcmp(Mode, "3PHASE") == 0) {
//    ModeB = AnalyzerInterface::PHASE3;
//  } else if(strcmp(Mode, "3PHASEN") == 0) {
//    ModeB = AnalyzerInterface::PHASE3N;
//  } else {
//    *postdata  = (char*)"'Mode' param must be '1PHASE', '3PHASE' or '3PHASEN'";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  if(Serial == NULL) {
//    *postdata  = (char*)"'ID' param is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->FindAnalyzer(Serial);
//  if(Analyzer == NULL) {
//    pthread_mutex_lock(&pListMutex);
//    pAnalyzers = (Analyzer_t*)realloc(pAnalyzers, sizeof(Analyzer_t) * (pAnalyzersCount + 1));
//    pAnalyzers[pAnalyzersCount].ID   = strdup(Serial);
//    pAnalyzers[pAnalyzersCount].Name = strdup(Name);
//    Analyzer = new AnalyzerRemote(pAnalyzers[pAnalyzersCount].ID, NominalVoltage, NominalFreq, pAnalyzers[pAnalyzersCount].Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);
//    pAnalyzers[pAnalyzersCount].Analyzer = Analyzer;
//    pAnalyzersCount++;
//    pthread_mutex_unlock(&pListMutex);
//  } else {
//    Analyzer->UpdateConfig(NominalVoltage, NominalFreq, Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);
//  }
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer->GetID());
//  db->ExecSentence();
//  *postdata = db->ExecSentenceJSON("SELECT row_to_json(Result) FROM (SELECT (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_3s) AS aggreg_3s, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_1m) AS aggreg_1m, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_10m) AS aggreg_10m, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_1h) AS aggreg_1h, (SELECT COALESCE(MAX(eventtime), 0) FROM events) AS events) AS Result", resultLen);
//  if(*postdata == NULL) {
//    *postdata  = (char*)"Config failed";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  return true;
//}
//bool DeviceRemote::SetRemoteDataCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
//  char *Serial = NULL;
//  Tools::JSONString(reader, (char*)"ID", &Serial, false);
//  if(Serial == NULL) {
//    *postdata  = (char*)"'ID' params is requiered";
//    *resultLen = strlen(*postdata);
//    return false;
//  }
//  AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->FindAnalyzer(Serial);
//  if(Analyzer == NULL) {
//    *postdata  = (char*)"Analyzer not configured";
//    *resultLen = strlen(*postdata);
//    return false;  
//  }
//  db->PrepareSentence("SET search_path TO ?");
//  db->BindSingleString(Analyzer->GetID());
//  db->ExecSentence();
//  if(Analyzer->SaveJSON(reader, db) == false) {
//    *postdata  = (char*)"Invalid or imcomplete data to save";
//    *resultLen = strlen(*postdata);
//    return false;
//  } else {
//    *writer = json_object_new_object();
//    json_object_object_add(*writer, "RequiereLive", json_object_new_boolean(false));
//    *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
//    return true;
//  }
//  return false;
//}
