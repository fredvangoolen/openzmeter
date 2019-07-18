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
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::pRegistered = DeviceRemote::GetInstances();
// Funciones ===========================================================================================================
bool DeviceRemote::GetInstances() {
  LOGFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'\n");
  DeviceManager::AddDevice(new DeviceRemote());
  return true;
}
DeviceRemote::DeviceRemote() {
  pSerial = "REMOTE";
  pDriver = "Remote";
  pName   = "Allow repplication of remote openZmeter devices in local machine";
  pMutex  = PTHREAD_MUTEX_INITIALIZER;
}
bool DeviceRemote::Start() {
  pthread_mutex_lock(&pMutex);
  if(pState == RUNNING) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  LOGFile::Info("Loading 'Remote' driver configuration\n");
  bool AutoStart = false;
  json Config = ConfigManager::ReadConfig("/Devices/" + pSerial);
  if(Config.is_object() && Config["Serials"].is_array()) {
    pVoltage = Config["Voltage"];
  }
  if((pVoltage < 85) || (pVoltage > 300)) pVoltage = 230.0;
  if((pFrequency < 10) || (pFrequency > 80)) pFrequency = 50.0;
  if((pSampleRate < (pFrequency * 100))) pSampleRate = (pFrequency * 100);
  Config = json::object();
  Config["Voltage"] = pVoltage;
  Config["Frequency"] = pFrequency;
  Config["SampleRate"] = pSampleRate;
  Config["Mode"] = (pMode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pMode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N";
  Config["AutoStart"] = AutoStart;
  ConfigManager::WriteConfig("/Devices/" + pSerial, Config);
  if(pState == UNCONFIGURED) {
    pState = STOPPED;
    if(AutoStart == false) {
      pthread_mutex_unlock(&pMutex);
      return true;
    }
  }
  LOGFile::Info("Starting 'Virtual' driver\n");
  pTerminate = false;
  pAnalyzer = new AnalyzerSoft(pSerial + "_" + ((pMode == AnalyzerInterface::PHASE1) ? "1PHASE" : (pMode == AnalyzerInterface::PHASE3) ? "3PHASE" : "3PHASE+N"), pMode, pSampleRate);
  if(pthread_create(&pCaptureThread, NULL, Capture_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    delete pAnalyzer;
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pState = RUNNING;
  pthread_mutex_unlock(&pMutex);
  return true;

  
  
  
  
  
  
  
  
  
  
  HTTPServer::RegisterCallback((char*)"/setRemoteAnalyzer", SetRemoteAnalyzerCallback);
  HTTPServer::RegisterCallback((char*)"/setRemoteData",  SetRemoteDataCallback);
  LOGFile::Info("Starting 'Remote' driver\n");
  return false;
}
bool DeviceRemote::Stop() {
  return false;
}
uint32_t DeviceRemote::AnalyzersCount() {
  pthread_mutex_lock(&pMutex);
  uint Analyzers = pAnalyzers.size();
  pthread_mutex_unlock(&pMutex);
  return Analyzers;
}
AnalyzerInterface *DeviceRemote::FindAnalyzer(string serial) {
  pthread_mutex_lock(&pMutex);
  AnalyzerInterface *Result = NULL;
  for(list<AnalyzerRemote*>::iterator it = pAnalyzers.begin(); it != pAnalyzers.end(); it++) {
    if(serial == (*it)->Serial()) {
      Result = *it;
      break;
    }
  }
  pthread_mutex_unlock(&pMutex);
  return Result;
}



DeviceRemote::~DeviceRemote() {
  while(pAnalyzers.size()) {
    AnalyzerRemote *Analyzer = pAnalyzers.front();
    delete Analyzer;
    pAnalyzers.pop_front();
  }
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
