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
#include "HTTPServer.h"
//======================================================================================================================
DeviceRemote     __attribute__((init_priority(500)))         *DeviceRemote::pInstance = NULL;
uint32_t                   DeviceRemote::pAnalyzersCount = 0;
DeviceRemote::Analyzer_t  *DeviceRemote::pAnalyzers = NULL;
pthread_mutex_t            DeviceRemote::pListMutex = PTHREAD_MUTEX_INITIALIZER;
// Funciones ===========================================================================================================
DeviceRemote::DeviceRemote(json_object* config) : DeviceInterface()  {
  if(pInstance != NULL) {
    LOGFile::Warning("Only one DeviceRemote driver allowed...\n");
    return;
  }
  Tools::LogMsg("INFO", "Starting DeviceRemote driver...\n");
  pInstance = this;
  HTTPServer::RegisterCallback((char*)"/setRemoteAnalyzer",  SetRemoteAnalyzerCallback);
  HTTPServer::RegisterCallback((char*)"/setRemoteData",  SetRemoteDataCallback);
}
DeviceRemote::~DeviceRemote() {}
bool DeviceRemote::SetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  char *Serial = NULL, *Name = NULL, *Mode = NULL;
  float NominalVoltage = NAN, NominalFreq = NAN, MaxCurrent = NAN, MinCurrent = NAN, SampleFreq = NAN;
  bool Online = false;
  json_object *Find;
  if((json_object_object_get_ex(reader, "ID", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(reader, "NominalVoltage", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalVoltage = json_object_get_double(Find);
  if((json_object_object_get_ex(reader, "NominalFreq", &Find) == true) && (json_object_get_type(Find) == json_type_double)) NominalFreq = json_object_get_double(Find);
  if((json_object_object_get_ex(reader, "Name", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Name = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(reader, "MaxCurrent", &Find) == true) && (json_object_get_type(Find) == json_type_double)) MaxCurrent = json_object_get_double(Find);
  if((json_object_object_get_ex(reader, "MinCurrent", &Find) == true) && (json_object_get_type(Find) == json_type_double)) MinCurrent = json_object_get_double(Find);
  if((json_object_object_get_ex(reader, "Mode", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Mode = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(reader, "SampleFreq", &Find) == true) && (json_object_get_type(Find) == json_type_double)) SampleFreq = json_object_get_double(Find);
  if((json_object_object_get_ex(reader, "Online", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Online = json_object_get_boolean(Find);
  AnalyzerInterface::AnalyzerMode ModeB;
  if(strcmp(Mode, "1PHASE") == 0) {
    ModeB = AnalyzerInterface::PHASE1;
  } else if(strcmp(Mode, "3PHASE") == 0) {
    ModeB = AnalyzerInterface::PHASE3;
  } else if(strcmp(Mode, "3PHASEN") == 0) {
    ModeB = AnalyzerInterface::PHASE3N;
  } else {
    *postdata  = (char*)"'Mode' param must be '1PHASE', '3PHASE' or '3PHASEN'";
    *resultLen = strlen(*postdata);
    return false;
  }
  if(Serial == NULL) {
    *postdata  = (char*)"'ID' param is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->FindAnalyzer(Serial);
  if(Analyzer == NULL) {
    pthread_mutex_lock(&pListMutex);
    pAnalyzers = (Analyzer_t*)realloc(pAnalyzers, sizeof(Analyzer_t) * (pAnalyzersCount + 1));
    pAnalyzers[pAnalyzersCount].ID   = strdup(Serial);
    pAnalyzers[pAnalyzersCount].Name = strdup(Name);
    Analyzer = new AnalyzerRemote(pAnalyzers[pAnalyzersCount].ID, NominalVoltage, NominalFreq, pAnalyzers[pAnalyzersCount].Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);
    pAnalyzers[pAnalyzersCount].Analyzer = Analyzer;
    pAnalyzersCount++;
    pthread_mutex_unlock(&pListMutex);
  } else {
    Analyzer->UpdateConfig(NominalVoltage, NominalFreq, Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);
  }
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer->GetID());
  db->ExecSentence();
  *postdata = db->ExecSentenceJSON("SELECT row_to_json(Result) FROM (SELECT (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_3s) AS aggreg_3s, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_1m) AS aggreg_1m, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_10m) AS aggreg_10m, (SELECT COALESCE(MAX(sampletime), 0) FROM aggreg_1h) AS aggreg_1h, (SELECT COALESCE(MAX(eventtime), 0) FROM events) AS events) AS Result", resultLen);
  if(*postdata == NULL) {
    *postdata  = (char*)"Config failed";
    *resultLen = strlen(*postdata);
    return false;
  }
  return true;
}
bool DeviceRemote::SetRemoteDataCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  char *Serial = NULL;
  Tools::JSONString(reader, (char*)"ID", &Serial, false);
  if(Serial == NULL) {
    *postdata  = (char*)"'ID' params is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->FindAnalyzer(Serial);
  if(Analyzer == NULL) {
    *postdata  = (char*)"Analyzer not configured";
    *resultLen = strlen(*postdata);
    return false;  
  }
  db->PrepareSentence("SET search_path TO ?");
  db->BindSingleString(Analyzer->GetID());
  db->ExecSentence();
  if(Analyzer->SaveJSON(reader, db) == false) {
    *postdata  = (char*)"Invalid or imcomplete data to save";
    *resultLen = strlen(*postdata);
    return false;
  } else {
    *writer = json_object_new_object();
    json_object_object_add(*writer, "RequiereLive", json_object_new_boolean(false));
    *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
    return true;
  }
  return false;
}
AnalyzerInterface *DeviceRemote::FindAnalyzer(char *serial) {
  pthread_mutex_lock(&pListMutex);
  AnalyzerInterface *Result = NULL;
  for(uint8_t i = 0; i < pAnalyzersCount; i++) {
    if(strcmp(serial, pAnalyzers[i].ID) == 0) {
      Result = pAnalyzers[i].Analyzer;
      break;
    }
  }
  pthread_mutex_unlock(&pListMutex);
  return Result;
}