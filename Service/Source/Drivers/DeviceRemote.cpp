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
DeviceRemote    *DeviceRemote::pInstance = NULL;
uint32_t         DeviceRemote::pAnalyzersCount = 0;
AnalyzerRemote **DeviceRemote::pAnalyzers = NULL;
// Funciones ===========================================================================================================
DeviceRemote::DeviceRemote(json_object* config) : DeviceInterface()  {
  if(pInstance != NULL) {
    Tools::LogMsg("WARN", "Only one DeviceRemote driver allowed...\n");
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
    pAnalyzers = (AnalyzerRemote**)realloc(pAnalyzers, sizeof(AnalyzerRemote*) * (pAnalyzersCount + 1));
    Analyzer = new AnalyzerRemote(Serial, NominalVoltage, NominalFreq, Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);
    pAnalyzers[pAnalyzersCount] = Analyzer;
    pAnalyzersCount++;
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
  char *Table  = NULL;
  json_object *Find;
  if((json_object_object_get_ex(reader, "ID", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Serial = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(json, "Table", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Table = (char*)json_object_get_string(Find);
  if((Serial == NULL)  (Table == NULL)) {
    *postdata  = (char*)"'ID' and 'Table' params is requiered";
    *resultLen = strlen(*postdata);
    return false;
  }
  AnalyzerRemote *Analyzer = (AnalyzerRemote*)pInstance->FindAnalyzer(Serial);
  if(Analyzer == NULL) {
    *postdata  = (char*)"Analyzer not configured";
    *resultLen = strlen(*postdata);
    return false;  
  }
  if(strcmp("aggreg_3s", Table) == 0) {
    db->PrepareSentence("INSERT INTO SELECT * from json_populate_record(null::?, ?)")

  *writer = json_object_new_object();
  if(Analyzer->SaveJSON(reader, db) != 0) {
    *postdata  = (char*)"Can save data";
    *resultLen = strlen(*postdata);
    return false;
  }
  
    
 
  
   
  //  AnalyzerInterface::Params_t Params;
//  if(Analyzer->ParamsFromJSON(reader, &Params) == false) {
//    *postdata  = (char*)"Invalid JSON params";
//    *resultLen = strlen(*postdata);
//    return false;    
//  }
  //Guardo  
  
  
  return true;
}
AnalyzerInterface *DeviceRemote::FindAnalyzer(char *serial) {
  for(uint8_t i = 0; i < pAnalyzersCount; i++) {
    if(strcmp(serial, pAnalyzers[i]->GetID()) == 0) return pAnalyzers[i];
  }
  return NULL;
}