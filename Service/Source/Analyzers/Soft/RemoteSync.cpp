/* ================================== zMeter ===========================================================================
 * File:     RemoteSync.cpp
 * Author:   Eduardo Viciana (agosto de 2019)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "RemoteSync.h"

RemoteSync::RemoteSync(string serial) {
  //  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
}

RemoteSync::RemoteSync(const RemoteSync& orig) {
}

RemoteSync::~RemoteSync() {
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
//size_t AnalyzerSoft::Curl_Read_Callback(void *contents, size_t size, size_t nmemb, void *userp) {
//  CurlBuff_t *Buff = (CurlBuff_t*)userp;
//  if((Buff->Size + size * nmemb + 1) > Buff->MaxSize) {
//    Buff->Buffer = (char*)realloc(Buff->Buffer, Buff->Size + size * nmemb + 1);
//    if(Buff->Buffer == NULL) {
//      return 0;
//    } else {
//      Buff->MaxSize = Buff->Size + size * nmemb + 1;
//    }
//  }
//  memcpy(&Buff->Buffer[Buff->Size], contents, size * nmemb);
//  Buff->Size += size * nmemb;
//  Buff->Buffer[Buff->Size] = 0;
//  return size * nmemb;
//}
//void *AnalyzerSoft::Sync_Thread(void *args) {
//  int Prio = sched_get_priority_min(SCHED_RR);
//  if(Prio == -1) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//  } else {
//    struct sched_param Param;
//    Param.sched_priority = Prio;
//    if(sched_setscheduler(0, SCHED_RR, &Param) != 0) LOGFile::Warning("Cant raise thread priority -> %s\n", __func__);
//  }
//  AnalyzerSoft *This = (AnalyzerSoft*)args;
//  pthread_setname_np(This->pSyncThread, "Sync");
//  CurlBuff_t   CurlBuff;
//  json_object *Find;
//  size_t       JSONLen;
//  long         HTTPCode;
//  char         Tmp[1024];
//  uint64_t     Last3S      = 0;
//  uint64_t     Last1M      = 0;
//  uint64_t     Last10M     = 0;
//  uint64_t     Last1H      = 0;
//  uint64_t     LastEvent   = 0;
//  uint64_t     LastPrice   = 0;
//  bool         Registered  = false;
//  bool         Initialized = false;
//  CURL        *Curl        = curl_easy_init();
//  curl_slist  *Headers     = NULL;
//  Database     DB(This->GetID());
//  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
//  curl_easy_setopt(Curl, CURLOPT_POST, 1);
//  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
//  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, Curl_Read_Callback);
//  while(!This->pTerminate) {
//    if(Registered == false) {
//      if(Headers == NULL) {
//        curl_slist_free_all(Headers);
//        Headers = curl_slist_append(Headers, "Accept: application/json");
//        Headers = curl_slist_append(Headers, "Accept-Encoding: deflate");
//        Headers = curl_slist_append(Headers, "Connection: keep-alive");
//        Headers = curl_slist_append(Headers, "Content-Type: application/json");
//        curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, Headers);
//      }
//      json_object *JSONWriter = json_object_new_object();
//      json_object_object_add(JSONWriter, "Username", json_object_new_string(This->pSyncName));
//      json_object_object_add(JSONWriter, "Password", json_object_new_string(This->pSyncPassword));
//      json_object_object_add(JSONWriter, "UpdateLastLogin", json_object_new_boolean(false));
//      CurlBuff.Size = 0;
//      snprintf(Tmp, sizeof(Tmp), "%s/LogIn", This->pSyncServer);
//      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//      CURLcode Code = curl_easy_perform(Curl);
//      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//      if((Code != CURLE_OK) || (HTTPCode != 200)) {
//        LOGFile::Warning("AnalyzerRemote: failed to register on remote server\n");
//      } else {
//        json_object *JSONReader = json_tokener_parse(CurlBuff.Buffer);
//        if((JSONReader != NULL) && (json_object_get_type(JSONReader) == json_type_object)) {
//          if((json_object_object_get_ex(JSONReader, "Result", &Find) == true) && (json_object_get_type(Find) == json_type_boolean)) {
//            Registered = json_object_get_boolean(Find);
//            if(Registered) Tools::LogMsg("INFO", "AnalyzerRemote: logged to remote server\n");
//          }
//          json_object_put(JSONReader);
//        }
//      }
//      if(Registered == false) usleep(1000000);
//      json_object_put(JSONWriter);
//    } else if(Initialized == false) {
//      snprintf(Tmp, sizeof(Tmp), "%s/setRemoteAnalyzer", This->pSyncServer);
//      json_object *JSONWriter = This->GetJSONDescriptor();
//      CurlBuff.Size = 0;
//      curl_easy_setopt(Curl, CURLOPT_URL, Tmp);
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, json_object_to_json_string_length(JSONWriter, JSON_C_TO_STRING_PLAIN, &JSONLen));
//      curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, JSONLen);
//      CURLcode Code = curl_easy_perform(Curl);
//      curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
//      if((Code != CURLE_OK) || (HTTPCode != 200)) {
//        LOGFile::Warning("AnalyzerRemote: failed to config on remote server\n");
//      } else {
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
//      }
//      json_object_put(JSONWriter);
//      if(Initialized == false) usleep(1000000);
//    } else {
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
//    }
//  }
//  curl_easy_cleanup(Curl);
//  return NULL;
//}