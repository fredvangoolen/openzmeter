#include "Sync.h"
//------------------------------------------------------------------------------
AnalyzerSoft::Sync::CurlBuff::CurlBuff() {
  pBuffer = (char*)malloc(1);
  pSize = 1;
}
AnalyzerSoft::Sync::CurlBuff::~CurlBuff() {
  if(pBuffer != NULL) free(pBuffer);
}
bool AnalyzerSoft::Sync::CurlBuff::Append(void *data, uint32_t size) {
  pSize = pSize + size;
  pBuffer = (char*)realloc(pBuffer, pSize + size);
  if(pBuffer == NULL) return false;
  memcpy(&pBuffer[pSize - size - 1], data, size);
  pBuffer[pSize - 1] = 0;
  return true;
}
char *AnalyzerSoft::Sync::CurlBuff::Data() {
  return pBuffer;
}
AnalyzerSoft::Sync::Sync(AnalyzerSoft *parent, const string &server, const string &user, const string &pass) {
  pParent       = parent;
  pServer       = server;
  pUser         = user;
  pPass         = pass;
  pTerminate    = false;
  pLogged       = false;
  pCurl         = curl_share_init();
  pCurlHeaders  = NULL;
  pThreadsCount = 0;
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Accept: application/json");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Connection: keep-alive");
  pCurlHeaders = curl_slist_append(pCurlHeaders, "Content-Type: application/json");
  pthread_mutex_init(&pCurlLocks, NULL);
  curl_share_setopt(pCurl, CURLSHOPT_USERDATA, &pCurlLocks);
  curl_share_setopt(pCurl, CURLSHOPT_LOCKFUNC, CurlLockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_UNLOCKFUNC, CurlUnlockCallback);
  curl_share_setopt(pCurl, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE);
  if(pthread_create(&pThread, NULL, Sync_Thread, this) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pThread = 0;
  }
  if(pThread != 0) pthread_setname_np(pThread, "RemoteSync");
}
AnalyzerSoft::Sync::~Sync() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pLocks.ReadLock();
  while(pThreadsCount > 0) {
    pLocks.ReadUnlock();
    usleep(100000);
    pLocks.ReadLock();
  }
  pLocks.ReadUnlock();
  if(pCurl != NULL) curl_share_cleanup(pCurl);
  if(pCurlHeaders != NULL) curl_slist_free_all(pCurlHeaders);
}
bool AnalyzerSoft::Sync::Logged() {
  pLocks.ReadLock();
  bool Result = pLogged;
  pLocks.ReadUnlock();
  return Result;
}
void AnalyzerSoft::Sync::Configure(const string &server, const string &user, const string &pass) {
  pLocks.WriteLock();
  pServer    = server;
  pUser      = user;
  pPass      = pass;
  pLogged = false;
  pLocks.WriteUnlock();
}
void AnalyzerSoft::Sync::CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr) {
  pthread_mutex_lock((pthread_mutex_t*)userptr); 
}
void AnalyzerSoft::Sync::CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr) {
  pthread_mutex_unlock((pthread_mutex_t*)userptr); 
}
size_t AnalyzerSoft::Sync::CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
  CurlBuff *Buffer = (CurlBuff*)userp;
  if(Buffer->Append(contents, size * nmemb) == false) return 0;
  return size * nmemb;
}
uint16_t AnalyzerSoft::Sync::CurlSendPost(const string &url, const string &contents, AnalyzerSoft::Sync::CurlBuff *buffer) {
  CURL      *Curl = NULL;
  Curl = curl_easy_init();
  curl_easy_setopt(Curl, CURLOPT_SHARE, pCurl);
  curl_easy_setopt(Curl, CURLOPT_HTTPHEADER, pCurlHeaders);
  curl_easy_setopt(Curl, CURLOPT_COOKIEFILE, "");
  curl_easy_setopt(Curl, CURLOPT_POST, 1L);
  curl_easy_setopt(Curl, CURLOPT_ACCEPT_ENCODING, "");
  curl_easy_setopt(Curl, CURLOPT_WRITEDATA, (void*)buffer);
  curl_easy_setopt(Curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(Curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(Curl, CURLOPT_POSTFIELDS, contents.c_str());
  curl_easy_setopt(Curl, CURLOPT_POSTFIELDSIZE, contents.size());
  curl_easy_setopt(Curl, CURLOPT_TIMEOUT, 30L);
  long Code = curl_easy_perform(Curl);
  if((Code == CURLE_OK) || (Code == CURLE_PARTIAL_FILE)) {
    long HTTPCode;
    curl_easy_getinfo(Curl, CURLINFO_RESPONSE_CODE, &HTTPCode);
    curl_easy_cleanup(Curl);
    return HTTPCode;
  }
  curl_easy_cleanup(Curl);
  return Code;
}
void *AnalyzerSoft::Sync::Sync_Thread(void *args) {
  Sync *This = (Sync*)args;
  while(This->pTerminate == false) {
    This->pLocks.ReadLock();
    CurlBuff Buffer;
    if(This->pLogged == false) {
      json   Content = json::object();
      Content["Username"] = This->pUser;
      Content["Password"] = This->pPass;
      Content["UpdateLastLogin"] = false;
      uint32_t Code = This->CurlSendPost(This->pServer + "/LogIn", Content.dump(), &Buffer);
      if(Code == MHD_HTTP_OK) {
        json Content = json::parse(Buffer.Data(), NULL, false);
        if(!Content.is_discarded() && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          This->pLogged = Content["Result"];
          if(This->pLogged) LOGFile::Info("Analyzer '%s' logged on remote server\n", This->pParent->pSerial.c_str());
        }
      } else {
        This->pLogged = false;
        LOGFile::Debug("Analyzer '%s' can not login on remote server\n", This->pParent->pSerial.c_str());
      }
      if(This->pLogged == false) usleep(1000000);
    } else {
      json   Content = json::object();
      Content["Analyzer"] = This->pParent->pSerial;
      Content["Name"] = This->pParent->pName;
      uint32_t Code = This->CurlSendPost(This->pServer + "/getRemote", Content.dump(), &Buffer);
      if(Code == MHD_HTTP_OK) {
        json Content = json::parse(Buffer.Data(), NULL, false);
        if(!Content.is_discarded() && Content.is_array()) {
          for(uint32_t n = 0; n < Content.size(); n++) {
            if(Content[n].is_object() && Content[n]["Sequence"].is_number() && Content[n]["Method"].is_string() && Content[n]["Params"].is_object()) {
              Request_t *Request = new Request_t;
              Request->Method   = Content[n]["Method"];
              Request->Params   = Content[n]["Params"];
              Request->Sequence = Content[n]["Sequence"];
              Request->Sync     = This;
              Request->Analyzer = This->pParent;
              pthread_t Thread = 0;
              if(pthread_create(&Thread, NULL, Request_Thread, (void*)Request) != 0) {
                LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
                delete Request;
              } else {
                This->pThreadsCount++;
                pthread_detach(Thread);
              }
            }
          }
        }
      } else if(Code == MHD_HTTP_UNAUTHORIZED) {
        LOGFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...\n", This->pParent->pSerial.c_str());
        usleep(1000000);
        This->pLogged = false;
      } else if((Code == MHD_HTTP_NO_CONTENT)) {
        LOGFile::Debug("No pending queries in remote server for analyzar '%s'\n", This->pParent->pSerial.c_str());
      } else if((Code == MHD_HTTP_METHOD_NOT_ALLOWED)) {
        LOGFile::Debug("Device remote is not enabled on remote server\n");
        usleep(1000000);
      } else {
        LOGFile::Error("Failed request to remote server for analyzer '%s' (Code: %u)\n", This->pParent->pSerial.c_str(), Code);
        usleep(1000000);
      }
    }
    This->pLocks.ReadUnlock();
  }

  return NULL;
}
void *AnalyzerSoft::Sync::Request_Thread(void *args) {
  Request_t *This = (Request_t*)args;
  This->Params["UserName"] = "admin";
  string ParamsText = This->Params.dump();
  size_t ParamsSize = ParamsText.size();
  This->Sync->pLocks.ReadLock();
  LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", This->Method.c_str(), This->Analyzer->pSerial.c_str());
  HTTPRequest *Request = new HTTPRequest(NULL, false);
  Request->AppendParams(ParamsText.c_str(), &ParamsSize);
  Request->ProcessParams(true);
  if(This->Method == "getAnalyzer") {
    This->Analyzer->GetAnalyzer(*Request);
  } else if(This->Method == "setAnalyzer") {
    This->Analyzer->SetAnalyzer(*Request);
  } else if(This->Method == "getSeries") {   
    This->Analyzer->GetSeries(*Request);
  } else if(This->Method == "getSeriesNow") {
    This->Analyzer->GetSeriesNow(*Request);
  } else if(This->Method == "getSeriesRange") {
    This->Analyzer->GetSeriesRange(*Request);
  } else if(This->Method == "getEvents") {
    This->Analyzer->GetEvents(*Request);
  } else if(This->Method == "getEvent") {
    This->Analyzer->GetEvent(*Request);
  } else if(This->Method == "setEvent") {
    This->Analyzer->SetEvent(*Request);
  } else if(This->Method == "delEvent") {
    This->Analyzer->DelEvent(*Request);
  } else if(This->Method == "getRates") {
    This->Analyzer->GetRates(*Request);
  } else if(This->Method == "getRate") {
    This->Analyzer->GetRate(*Request);
  } else if(This->Method == "setRate") {
    This->Analyzer->SetRate(*Request);
  } else if(This->Method == "delRate") {
    This->Analyzer->DelRate(*Request);
  } else if(This->Method == "addRate") {
    This->Analyzer->AddRate(*Request);
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(*Request);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(*Request);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(*Request);
  } else if(This->Method == "getAlarm") {
    This->Analyzer->GetAlarm(*Request);
  } else if(This->Method == "setAlarm") {
    This->Analyzer->SetAlarm(*Request);
  } else if(This->Method == "addAlarm") {
    This->Analyzer->AddAlarm(*Request);
  } else if(This->Method == "delAlarm") {
    This->Analyzer->DelAlarm(*Request);
  }
  CurlBuff Buffer;
  json Content = json::object();
  Content["Code"] = Request->ResultCode();
  Content["Sequence"] = This->Sequence;
  Content["Analyzer"] = This->Analyzer->pSerial;
  Content["Response"] = Request->Content();
  uint32_t Code = This->Sync->CurlSendPost(This->Sync->pServer +  "/setRemote", Content.dump(), &Buffer);
  if(Code == MHD_HTTP_OK) {
    LOGFile::Debug("Remote request '%s' for analyzer '%s' completed\n", This->Method.c_str(), This->Analyzer->pSerial.c_str());
  } else {
    LOGFile::Error("Remote request '%s' for analyzer '%s' failed\n", This->Method.c_str(), This->Analyzer->pSerial.c_str());
  }
  This->Sync->pLocks.ReadUnlock();
  This->Sync->pLocks.WriteLock();
  This->Sync->pThreadsCount--;
  This->Sync->pLocks.WriteUnlock();
  delete Request;
  return NULL;
}
