/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include <curl/curl.h>
#include "Analyzers.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) {
  pSerial = id;
  pName = "Unnamed analyzer";
  pLastSave = 0;
}
Analyzer::~Analyzer() {
}
string Analyzer::Serial() const {
  return pSerial;
}
json Analyzer::GetJSONShortDescriptor() const {
  json Response;
  Response["ID"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  return Response;
}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
size_t Analyzer::CurlReadCallback(void *contents, size_t size, size_t nmemb, void *userp) {
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
void *Analyzer::Sync_Thread(void *args) {
  Analyzer *This = (Analyzer*)args;
  pthread_setname_np(This->pSyncThread, "RemoteSync");
  CurlBuff_t   CurlBuff;
//  json_object *Find;
//  size_t       JSONLen;
  long         HTTPCode;
//  char         Tmp[1024];
//  uint64_t     Last3S      = 0;
//  uint64_t     Last1M      = 0;
//  uint64_t     Last10M     = 0;
//  uint64_t     Last1H      = 0;
//  uint64_t     LastEvent   = 0;
//  uint64_t     LastPrice   = 0;
  bool         Registered  = false;
//  bool         Initialized = false;
  CURL        *Curl        = curl_easy_init();
  curl_slist  *Headers     = NULL;
//  Database     DB(This->GetID());
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1L);
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlReadCallback);
  while(!This->pTerminateSync) {
    if(Headers == NULL) {
      curl_slist_free_all(Headers);
      Headers = curl_slist_append(Headers, "Accept: application/json");
      Headers = curl_slist_append(Headers, "Accept-Encoding: deflate");
      Headers = curl_slist_append(Headers, "Connection: keep-alive");
      Headers = curl_slist_append(Headers, "Content-Type: application/json");
      curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);
    }
    if(Registered == false) {
      json   Content = json::object();
      string URL     = This->pSyncServer + "/Login";
      CurlBuff.Size  = 0;
      Content["Username"] = This->pSyncUser;
      Content["Password"] = This->pSyncPass;
      Content["UpdateLastLogin"] = false;
      string ContentText = Content.dump();
      curl_easy_setopt(Curl, CURLOPT_URL, URL.c_str());
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, ContentText.c_str());
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, ContentText.size());
      CURLcode Code = curl_easy_perform(Curl);
      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
      if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_OK)) {
        json Content = json::parse(CurlBuff.Buffer, NULL, false);
        if(!Content.is_discarded() && Content.is_object() && Content["Result"].is_boolean()) {
          Registered = Content["Result"];
          if(Registered) LOGFile::Info("INFO", "AnalyzerRemote: logged to remote server\n");
        }
      } else {
        LOGFile::Debug("AnalyzerRemote: failed to register on remote server\n");
      }        
      if(Registered == false) usleep(1000000);
    } else {
      json   Content = json::object();
      string URL     = This->pSyncServer + "/getRemote";
      CurlBuff.Size = 0;
      Content["Analyzer"] = This->pSerial;
      string ContentText = Content.dump();
      curl_easy_setopt(Curl, CURLOPT_URL, URL.c_str());
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, ContentText.c_str());
      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, ContentText.size());
      CURLcode Code = curl_easy_perform(Curl);
      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
      if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_OK)) {
        LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", "getAnalyzer", This->pSerial.c_str());
        
        //Se supone que me ha llegado un array con peticiones que habia pendientes
//        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//          if((json_object_object_get_ex(JSONReader, "aggreg_3s", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last3S = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_1m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1M = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_10m", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last10M = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "aggreg_1h", &Find) == true) && (json_object_get_type(Find) == json_type_int)) Last1H = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "events", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastEvent = json_object_get_int64(Find);
//          if((json_object_object_get_ex(JSONReader, "prices", &Find) == true) && (json_object_get_type(Find) == json_type_int)) LastPrice = json_object_get_int64(Find);
//          Tools::LogMsg("INFO", "AnalyzerRemote: configured on remote server\n");
//          Initialized = true;
//        }
      } else if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_UNAUTHORIZED)) {
        LOGFile::Debug("Non authorized on remote server for analyzar '%s', trying to login again...\n", This->pSerial.c_str());
        usleep(1000000);
        Registered = false;
      } else if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_NO_CONTENT)) {
        LOGFile::Debug("No pending queries in remote server for analyzar '%s'\n", This->pSerial.c_str());
      } else {
        LOGFile::Debug("Failed request to remote server for analyzer '%s'\n", This->pSerial.c_str());
      }
    }
  }
  curl_easy_cleanup(Curl);
  return NULL;
}


//void AnalyzerSoft::ConfigRemote(json_object* config) {
//  json_object *Find;
//  if((json_object_object_get_ex(config, "Username", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncName = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(config, "Password", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncPassword = (char*)json_object_get_string(Find);
//  if((json_object_object_get_ex(config, "RemoteHost", &Find) == true) && (json_object_get_type(Find) == json_type_string)) pSyncServer = (char*)json_object_get_string(Find);
//  if((pSyncName == NULL) || (pSyncPassword == NULL) || (pSyncServer == NULL)) {
//    LOGFile::Error("AnayzerSoft: Remote functionality mode need 'Username', 'Password' and 'RemoteServer' params\n");
//    return;
//  }
//  if(curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
//    LOGFile::Error("AnayzerSoft: Failed to load libcurl\n");
//    return;  
//  }
//  if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return;
//  }
//}
//void AnalyzerSoft::ConfigPrices(json_object* config) {
//  pPrices = PricesInterface::Config(GetSchema(), config);
//}


//      json_object_put(JSONWriter);
//      if(Initialized == false) usleep(1000000);
//      bool      Live  = false;
//      char     *Table = (char*)"aggreg_3s";
//      uint64_t *Time  = &Last3S;
////      sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_3s' AS \"Table\", * FROM aggreg_3s WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//      char *Result = DB.ExecSentenceJSON(Tmp, &JSONLen);
//      if(Result == NULL) {
//        Table = (char*)"aggreg_1m";
//        Time  = &Last1M;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1m' AS \"Aggregation_Source\", * FROM (SELECT * FROM aggreg_1m NATURAL LEFT JOIN aggreg_ext_1m) AS aggreg_1m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result == NULL) {
//        Table = (char*)"aggreg_10m";
//        Time  = &Last10M;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_10m' AS \"Aggregation_Source\", * FROM aggreg_10m WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result == NULL) {
//        Table = (char*)"aggreg_10m";
//        Time  = &Last1H;
////        sprintf(Tmp, "SELECT row_to_json(Result), sampletime FROM (SELECT '%s' AS \"ID\", 'aggreg_1h' AS \"Aggregation_Source\", * FROM aggreg_1h WHERE sampletime > %" PRIu64 " ORDER BY sampletime ASC LIMIT 1) AS Result", This->pID, Last3S);
//        Result = DB.ExecSentenceJSON(Tmp, &JSONLen);        
//      }
//      if(Result != NULL) {
//        snprintf(Tmp, sizeof(Tmp), "%s/setRemoteData", This->pSyncServer);
//        CurlBuff.Size = 0;
//        curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//        curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, Result);
//        curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//        CURLcode Code = curl_easy_perform(Curl);
//        curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//        if((Code != CURLE_OK) || (HTTPCode != 200)) {
//          LOGFile::Warning("AnalyzerRemote: failed send data to server\n");
//        } else {
//          json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//          if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//            *Time = atol(DB.ResultValue(0, (char*)"sampletime"));
//            if((json_object_object_get_ex(JSONReader, "Live", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) Live = json_object_get_int64(Find);
//            LOGFile::Debug("AnalyzerRemote: sampletime %" PRIu64 " saved in remote '%s' (Live: %s)\n", *Time, Table, (Live ? "true" : "false"));
//          }
//        }
//      } else {
//        usleep(1000000);
//      }
