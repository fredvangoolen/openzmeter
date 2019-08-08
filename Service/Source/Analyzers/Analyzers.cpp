/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.cpp
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/ 
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "Analyzers.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) {
  pSerial      = id;
  pName        = "Unnamed analyzer";
  pLastSave    = 0;
  pSyncThread  = 0;
  pCurl        = NULL;
  pCurlHeaders = NULL;
  pCurlLocks   = PTHREAD_MUTEX_INITIALIZER;
}
Analyzer::~Analyzer() {
  DisableSync();
}
string Analyzer::Serial() const {
  return pSerial;
}
json Analyzer::GetJSONShortDescriptor() const {
  json Response;
  Response["Analyzer"] = pSerial;
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime() - pLastSave) < 20000);
  return Response;
}
bool Analyzer::CheckRights(HTTPRequest &req, const uint neededAccess) const {
// req.FromString("", MHD_HTTP_UNAUTHORIZED);
 return true;
}
json Analyzer::GetJSONSyncConfig() {
  json Response = json::object();
  Response["RemoteEnabled"] = (pCurl != NULL);
  Response["RemoteServer"] = pSyncServer;
  Response["RemoteUser"] = pSyncUser;
  Response["RemotePass"] = pSyncPass;
  return Response; 
}
bool Analyzer::EnableSync(const string &server, const string &user, const string &pass) {
  pthread_mutex_lock(&pCurlLocks);
  if(pCurl != NULL) {
    pthread_mutex_unlock(&pCurlLocks);  
    DisableSync();
    pthread_mutex_unlock(&pCurlLocks);  
  }
  pSyncServer    = server;
  pSyncUser      = user;
  pSyncPass      = pass;
  pTerminateSync = false;
  pCurlHeaders   = NULL;
  pCurl = curl_share_init();
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Accept: application/json");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Connection: keep-alive");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Content-Type: application/json");
  curl_share_setopt(pCurl, CURLSHOPT_LOCKFUNC, CurlLockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_UNLOCKFUNC, CurlUnlockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_USERDATA, &pCurlLocks);
  curl_share_setopt(pCurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);  
  if(pthread_create(&pSyncThread, NULL, Sync_Thread, (void*)this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pSyncThread = 0;
    pthread_mutex_unlock(&pCurlLocks);
    return false;
  }
  return true;  
}
void Analyzer::DisableSync() {
  pthread_mutex_lock(&pCurlLocks);
  pTerminateSync = true;
  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);
  if(pCurl != NULL) {
    curl_share_cleanup(pCurl);
    pCurl = NULL;
  }
  if(pCurlHeaders != NULL) {
    curl_slist_free_all(pCurlHeaders);
    pCurlHeaders = NULL;
  }
  pthread_mutex_unlock(&pCurlLocks);  
}
size_t Analyzer::CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlWriteBuff_t *Buffer = (CurlWriteBuff_t*)userp;
  if((Buffer->Size + size * nmemb + 1) > Buffer->MaxSize) {
    Buffer->Buffer = (char*)realloc(Buffer->Buffer, Buffer->Size + size * nmemb + 1);
    if(Buffer->Buffer == NULL) {
      return 0;
    } else {
      Buffer->MaxSize = Buffer->Size + size * nmemb + 1;
    }
  }
  memcpy(&Buffer->Buffer[Buffer->Size], contents, size * nmemb);
  Buffer->Size += size * nmemb;
  Buffer->Buffer[Buffer->Size] = 0;
  return size * nmemb;
}
void Analyzer::CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr) {
  pthread_mutex_lock((pthread_mutex_t*)userptr); 
}
void Analyzer::CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr) {
  pthread_mutex_unlock((pthread_mutex_t*)userptr); 
}
void *Analyzer::Sync_Thread(void *args) {
  Analyzer *This = (Analyzer*)args;
  pthread_setname_np(This->pSyncThread, "RemoteSync");
  CurlWriteBuff_t CurlBuff;
  long            HTTPCode;
  bool            Registered  = false;
  CURL           *Curl        = curl_easy_init();
  curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, This->pCurlHeaders);
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1L);
  curl_easy_setopt(Curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(Curl, CURLOPT_SHARE, This->pCurl);
  while(!This->pTerminateSync == false) {
    if(Registered == false) {
      json   Content = json::object();
      string URL     = This->pSyncServer + "/LogIn";
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
        if(!Content.is_discarded() && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          Registered = Content["Result"];
          if(Registered) LOGFile::Info("Analyzer '%s' logged on remote server\n", This->pSerial.c_str());
        }
      } else {
        Registered = false;
        LOGFile::Debug("Analyzer '%s' can not login on remote server\n", This->pSerial.c_str());
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
        json Content = json::parse(CurlBuff.Buffer, NULL, false);
        if(!Content.is_discarded() && Content.is_array()) {
          for(uint32_t n = 0; n < Content.size(); n++) {
            if(Content[n].is_object() && Content[n]["Sequence"].is_number() && Content[n]["Method"].is_string() && Content[n]["Params"].is_object()) {
              RemoteRequest_t *Request = new RemoteRequest_t;
              Request->Method   = Content[n]["Method"];
              Request->Params   = Content[n]["Params"];
              Request->Sequence = Content[n]["Sequence"];
              Request->Parent   = This;
              pthread_t Thread = 0;
              if(pthread_create(&Thread, NULL, Request_Thread, (void*)Request) != 0) {
                LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
                delete Request;
              } else {
                pthread_setname_np(Thread, Request->Method.c_str());
                pthread_detach(Thread);
              }
            }
          }
        }
      } else if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_UNAUTHORIZED)) {
        LOGFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...\n", This->pSerial.c_str());
        usleep(1000000);
        Registered = false;
      } else if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_NO_CONTENT)) {
        LOGFile::Debug("No pending queries in remote server for analyzar '%s'\n", This->pSerial.c_str());
      } else if((Code == CURLE_OK) && (HTTPCode == MHD_HTTP_METHOD_NOT_ALLOWED)) {
        LOGFile::Debug("Device remote is not enabled on remote server\n");
      } else {
        LOGFile::Debug("Failed request to remote server for analyzer '%s'\n", This->pSerial.c_str());
        usleep(1000000);
      }
    }
  }
  
  curl_easy_cleanup(Curl);
  return NULL;
}
void *Analyzer::Request_Thread(void *args) {
  RemoteRequest_t *This = (RemoteRequest_t*)args;
  This->Params["UserName"] = "admin";
  string ParamsText = This->Params.dump();
  size_t ParamsSize = ParamsText.size();
  LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", This->Method.c_str(), This->Parent->pSerial.c_str());
  HTTPRequest *Request = new HTTPRequest(NULL, false);
  Request->AppendParams(ParamsText.c_str(), &ParamsSize);
  Request->ProcessParams(true);
  if(This->Method == "getAnalyzer") {
    This->Parent->GetAnalyzer(*Request);
  } else if(This->Method == "setAnalyzer") {
    This->Parent->SetAnalyzer(*Request);
  } else if(This->Method == "getSeries") {   
    This->Parent->GetSeries(*Request);
  } else if(This->Method == "getSeriesNow") {
    This->Parent->GetSeriesNow(*Request);
  } else if(This->Method == "getSeriesRange") {
    This->Parent->GetSeriesRange(*Request);
  } else if(This->Method == "getEvents") {
    This->Parent->GetEvents(*Request);
  } else if(This->Method == "getEvent") {
    This->Parent->GetEvent(*Request);
  } else if(This->Method == "setEvent") {
    This->Parent->SetEvent(*Request);
  } else if(This->Method == "delEvent") {
    This->Parent->DelEvent(*Request);
  } else if(This->Method == "getRates") {
    This->Parent->GetRates(*Request);
  } else if(This->Method == "getRate") {
    This->Parent->GetRate(*Request);
  } else if(This->Method == "setRate") {
    This->Parent->SetRate(*Request);
  } else if(This->Method == "delRate") {
    This->Parent->DelRate(*Request);
  } else if(This->Method == "addRate") {
    This->Parent->AddRate(*Request);
  } else if(This->Method == "getPrices") {
    This->Parent->GetPrices(*Request);
  }
  CURL           *Curl = curl_easy_init();
  CurlWriteBuff_t CurlBuff;
  string URL = This->Parent->pSyncServer + "/setRemote";
  json Content = json::object();
  Content["Code"] = Request->ResultCode();
  Content["Sequence"] = This->Sequence;
  Content["Analyzer"] = This->Parent->pSerial;
  Content["Response"] = Request->Content();
  delete Request;
  string ContentText = Content.dump();
  curl_easy_setopt(Curl, CURLOPT_URL, URL.c_str());
  curl_easy_setopt(Curl, CURLOPT_POST, 1L);
  curl_easy_setopt(Curl, CURLOPT_SHARE, This->Parent->pCurl);
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)&CurlBuff);
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, This->Parent->pCurlHeaders);
  curl_easy_setopt(Curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, ContentText.c_str());
  curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, ContentText.size());
  CurlBuff.Size = 0;
  curl_easy_perform(Curl);
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
