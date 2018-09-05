/* ================================== zMeter ===========================================================================
 * File:     DeviceRemote.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <curl/curl.h>
#include <unistd.h>
#include <math.h>
#include "DeviceRemote.h"
#include "HTTPServer.h"
// Funciones ===========================================================================================================
DeviceRemote::DeviceRemote(json_object* config) : DeviceInterface() {
  Tools::LogMsg("INFO", "Starting DeviceRemote driver...\n");
  pCurlBuff.Buffer = NULL;
  pCurlBuff.MaxSize = 0;
  pCurlBuff.Size = 0;
  pAnalyzers = NULL;
  pAnalyzersCount = 0;
  struct json_object *Find;
  char *Mode = NULL;
  if((json_object_object_get_ex(config, "Mode", &Find) == true) && (json_object_get_type(Find) == json_type_string)) Mode = (char*)json_object_get_string(Find);
  if((Mode == NULL) || ((strcmp(Mode, "Server") != 0) && (strcmp(Mode, "Client") != 0))) {
    Tools::LogMsg("ERROR", "DeviceRemote: Mode must be 'Server' or 'Client'\n");
    return;
  }
  if((json_object_object_get_ex(config, "Username", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pUserName = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "Password", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pPassword = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "RemoteHost", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pServer = (char*)json_object_get_string(Find);
  if((json_object_object_get_ex(config, "Analyzer", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSerial = (char*)json_object_get_string(Find);
  if((strcmp(Mode, "Client") == 0) && ((pUserName == NULL) || (pPassword == NULL) || (pSerial == NULL) || (pServer == NULL))) {
    Tools::LogMsg("ERROR", "DeviceRemote: Client mode need 'Username', 'Password', 'Analyzer' and 'RemoteServer' params\n");
  }
  if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
    Tools::LogMsg("ERROR", "DeviceRemote: Failed to load libcurl\n");
    return;  
  }
  if(strcmp(Mode, "Server") == 0) {
    HTTPServer::RegisterCallback((char*)"/setRemoteAnalyzer",  SetsetRemoteAnalyzerCallback);
  }
  if(strcmp(Mode, "Client") == 0) {
    if(pthread_create(&pCaptureThread, NULL, Capture_Thread_Client, this) != 0) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      return;
    }
  }
}
DeviceRemote::~DeviceRemote() {}
bool DeviceRemote::SetsetRemoteAnalyzerCallback(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
  char *Serial = NULL, *Name = NULL, *Mode = NULL;
  float NominalVoltage = NAN, NominalFreq = NAN, MaxCurrent = NAN, MinCurrent = NAN, SampleFreq = NAN;
  bool Online = false;
  struct json_object *Find;
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
  AnalyzerRemote *Analyzer = FindAnalyzer(Serial);
  if(Analyzer == NULL) {
    //AnalyzerRemote *Analyzer = new AnalyzerRemote(Serial, NominalVoltage, NominalFreq, NominalVoltage, Name, Name, MaxCurrent, MinCurrent, ModeB, SampleFreq, Online);    
    pAnalyzers = (Analyzer_t*)realloc(pAnalyzers, sizeof(Analyzer_t*) * json_object_array_length(Find));
  } else {
    Analyzer->UpdateParams(reader);
  }
  return true;
}
size_t DeviceRemote::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlBuff_t *Buff = (CurlBuff_t*)userp;
  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
    if(Buff->Buffer == NULL) {
      return 0;
    } else {
      Buff->MaxSize = Buff->Size + size * nmemb + 1;
    }
  }
  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
  Buff->Size += size * nmemb;
  Buff->Buffer[Buff->Size] = 0;
  return size * nmemb;
}
void *DeviceRemote::Capture_Thread_Client(void *args) {
  int Prio = sched_get_priority_min(SCHED_RR);
  if(Prio == -1) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    struct sched_param Param;
    Param.sched_priority = Prio;
    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) Tools::LogMsg("WARN", "Cant raise thread priority -> %s\n", __func__);
  }
  DeviceRemote *This = (DeviceRemote*)args;
  bool  Registered  = false;
  bool  Initialized = false;
  char *Session = NULL;
  json_object *Find;
  CURL *Curl = curl_easy_init();
  struct curl_slist *Headers = NULL;
  char URL[1024];
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1);
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&This->pCurlBuff);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
  while(!This->pTerminate) {
    if(Registered == false) {
      if(Headers == NULL) {
        curl_slist_free_all(Headers);
        Headers = curl_slist_append(Headers, "Accept: application/json");
        Headers = curl_slist_append(Headers, "Accept-Encoding: deflate");
        Headers = curl_slist_append(Headers, "Connection: keep-alive");
        Headers = curl_slist_append(Headers, "Content-Type: application/json");
        curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);
      }
      json_object *JSONWriter = json_object_new_object();
      size_t       JSONLen;
      json_object_object_add(JSONWriter, "Username", json_object_new_string(This->pUserName));
      json_object_object_add(JSONWriter, "Password", json_object_new_string(This->pPassword));
      json_object_object_add(JSONWriter, "UpdateLastLogin", json_object_new_boolean(false));
      This->pCurlBuff.Size = 0;
      snprintf(URL, sizeof(URL), "%s/LogIn", This->pServer);
      curl_easy_setopt(Curl, CURLOPT_URL, URL);
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
      if(curl_easy_perform(Curl) != CURLE_OK) {
        Tools::LogMsg("WARN", "DeviceRemote: failed to register on remote server\n");
      } else {
        json_object *JSONReader = json_tokener_parse(This->pCurlBuff.Buffer);
        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
          if((json_object_object_get_ex(JSONReader, "Result", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) {
            Registered = json_object_get_boolean(Find);
            if(Registered) Tools::LogMsg("INFO", "DeviceRemote: logged to remote server\n");
          }
          json_object_put(JSONReader);
        }
      }
      if(Registered == false) usleep(1000000);
      json_object_put(JSONWriter);
    } else if(Initialized == false) {
      AnalyzerInterface *Analyzer = FindAnalyzerGlobal(This->pSerial);
      if(Analyzer == NULL) {
        Tools::LogMsg("DEBUG", "DeviceRemote: analyzer is not ready jet\n");
      } else {
        snprintf(URL, sizeof(URL), "%s/setRemoteAnalyzer", This->pServer);
        json_object *JSONWriter = Analyzer->GetJSONDescriptor();
        size_t       JSONLen;
        This->pCurlBuff.Size = 0;
        curl_easy_setopt(Curl, CURLOPT_URL, URL);
        curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
        curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
        if(curl_easy_perform(Curl) != CURLE_OK) {
        }
        json_object_put(JSONWriter);
      }
      if(Initialized == false) usleep(1000000);
    } else {
      //aqui enviaria datos
    }
  }
  curl_easy_cleanup(Curl);
  return NULL;
}
AnalyzerInterface *DeviceRemote::FindAnalyzer(char *serial) {
  return NULL;
}