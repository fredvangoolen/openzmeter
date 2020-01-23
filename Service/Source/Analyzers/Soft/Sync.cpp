#include "Sync.h"
#include <iostream>
//------------------------------------------------------------------------------
AnalyzerSoft::Sync::Sync(AnalyzerSoft *parent) {
  pParent       = parent;
  pTerminate    = false;
  pCookie       = "";
  pThreadsCount = 0;
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
}
bool AnalyzerSoft::Sync::Logged() {
  pLocks.ReadLock();
  bool Result = !pCookie.empty();
  pLocks.ReadUnlock();
  return Result;
}
void AnalyzerSoft::Sync::Configure(const string &server, const string &user, const string &pass) {
  pLocks.WriteLock();
  URL Server(server);
  if(Server.Port.empty()) {
    pPort = 80;
  } else {
    pPort = atoi(Server.Port.c_str());
  }
  pPath      = Server.Path;
  pServer    = Server.Host;
  pUser      = user;
  pPass      = pass;
  pCookie    = "";
  pLocks.WriteUnlock();
}
void *AnalyzerSoft::Sync::Sync_Thread(void *args) {
  Sync   *This = (Sync*)args;
  Client *HTTPClient = NULL;
  while(This->pTerminate == false) {
    This->pLocks.ReadLock();
    if(This->pCookie.empty()) {
      Headers Header  = {{"Accept", "application/json"}, {"Connection", "keep-alive"}};
      string  Path    = This->pPath + "/LogIn";
      json    Content = json::object();
      Content["Username"] = This->pUser;
      Content["Password"] = This->pPass;
      Content["UpdateLastLogin"] = false;
      if(HTTPClient != NULL) delete HTTPClient;
      HTTPClient = new Client(This->pServer.c_str(), This->pPort, 30);
      This->pLocks.ReadUnlock();
      auto Response = HTTPClient->Post(Path.c_str(), Header, Content.dump(), "application/json", true);
      This->pLocks.WriteLock();
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if(!Content.is_discarded() && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          if(Response->has_header("Set-Cookie") == false) {
            This->pCookie = "";
          } else {
            string Cookie = Response->get_header_value("Set-Cookie");
            This->pCookie = "";
            for(unsigned n = 0; n < Cookie.length(); n++) {
              if(Cookie[n] == ';') break;
              This->pCookie += Cookie[n];
            }
          }
          if(This->pCookie.empty() == false) LOGFile::Info("Analyzer '%s' logged on remote server\n", This->pParent->Serial.c_str());
        }
      } else {
        This->pCookie = "";
        LOGFile::Debug("Analyzer '%s' can not login on remote server\n", This->pParent->Serial.c_str());
      }
      if(This->pCookie.empty()) {
        This->pLocks.WriteUnlock();
        usleep(1000000);
      } else {
        This->pLocks.WriteUnlock();
      }
    } else {
      string Path    = This->pPath + "/getRemote";
      json   Content = json::object();
      Headers Header = {{"Accept", "application/json"}, {"Connection", "keep-alive"}, {"Cookie", This->pCookie}};
      Content["Analyzer"] = This->pParent->Serial;
      Content["Name"] = This->pParent->pName;
      This->pLocks.ReadUnlock();
      LOGFile::Debug("Analyzer '%s'request pending queries to server\n", This->pParent->Serial.c_str());
      auto Response = HTTPClient->Post(Path.c_str(), Header, Content.dump(), "application/json", true);
      This->pLocks.WriteLock();
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if(!Content.is_discarded() && Content.is_array()) {
          LOGFile::Debug("Analyzer '%s' receive %u queries from remote server\n", This->pParent->Serial.c_str(), Content.size());
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
                pthread_setname_np(Thread, Request->Method.c_str());
                This->pThreadsCount++;
                pthread_detach(Thread);
              }
            }
          }
        }
        This->pLocks.WriteUnlock();
      } else if((Response != nullptr) && (Response->status == HTTP_UNAUTHORIZED)) {
        This->pCookie = "";
        This->pLocks.WriteUnlock();
        LOGFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...\n", This->pParent->Serial.c_str());
        usleep(1000000);
      } else if((Response != nullptr) && (Response->status == HTTP_NO_CONTENT)) {
        This->pLocks.WriteUnlock();
        LOGFile::Debug("No pending queries in remote server for analyzar '%s'\n", This->pParent->Serial.c_str());
      } else if((Response != nullptr) && (Response->status == HTTP_METHOD_NOT_ALLOWED)) {
        This->pLocks.WriteUnlock();
        LOGFile::Debug("Device remote is not enabled on remote server\n");
        usleep(1000000);
      } else if((Response != nullptr) && (Response->status == HTTP_BAD_REQUEST)) {
        This->pLocks.WriteUnlock();
        LOGFile::Debug("Analyzer '%s' is not authorized in remote server\n", This->pParent->Serial.c_str());
        usleep(1000000);
      } else if((Response != nullptr) && (Response->status == HTTP_SERVICE_UNAVAILABLE)) {
        This->pLocks.WriteUnlock();
        LOGFile::Debug("Analyzer '%s' send request while remote server already have one\n", This->pParent->Serial.c_str());
        usleep(1000000);
      } else {
        This->pLocks.WriteUnlock();
        LOGFile::Debug("Failed request to remote server for analyzer '%s'\n", This->pParent->Serial.c_str());
        usleep(1000000);
      }
    }
  }
  if(HTTPClient != NULL) delete HTTPClient;
  return NULL;
}
void *AnalyzerSoft::Sync::Request_Thread(void *args) {
  Request_t *This = (Request_t*)args;
  This->Params["UserName"] = "admin";
//  string ParamsText = This->Params.dump();
//  size_t ParamsSize = ParamsText.size();
  LOGFile::Debug("Remote server request query '%s' for analyzar '%s'\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
//  HTTPRequest Request(ParamsText.c_str());
//  Request.AppendParams(ParamsText.c_str(), &ParamsSize);
//  Request.ProcessParams(true);
  Response Resp;
  if(This->Method == "getAnalyzer") {
    This->Analyzer->GetAnalyzer(This->Params, Resp);
  } else if(This->Method == "setAnalyzer") {
    This->Analyzer->SetAnalyzer(This->Params, Resp);
  } else if(This->Method == "getSeries") {
    This->Analyzer->GetSeries(This->Params, Resp);
  } else if(This->Method == "getSeriesNow") {
    This->Analyzer->GetSeriesNow(This->Params, Resp);
  } else if(This->Method == "getSeriesRange") {
    This->Analyzer->GetSeriesRange(This->Params, Resp);
  } else if(This->Method == "getEvents") {
    This->Analyzer->GetEvents(This->Params, Resp);
  } else if(This->Method == "getEvent") {
    This->Analyzer->GetEvent(This->Params, Resp);
  } else if(This->Method == "setEvent") {
    This->Analyzer->SetEvent(This->Params, Resp);
  } else if(This->Method == "delEvent") {
    This->Analyzer->DelEvent(This->Params, Resp);
  } else if(This->Method == "getRates") {
    This->Analyzer->GetRates(This->Params, Resp);
  } else if(This->Method == "getRate") {
    This->Analyzer->GetRate(This->Params, Resp);
  } else if(This->Method == "setRate") {
    This->Analyzer->SetRate(This->Params, Resp);
  } else if(This->Method == "delRate") {
    This->Analyzer->DelRate(This->Params, Resp);
  } else if(This->Method == "addRate") {
    This->Analyzer->AddRate(This->Params, Resp);
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(This->Params, Resp);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(This->Params, Resp);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(This->Params, Resp);
  } else if(This->Method == "getAlarm") {
    This->Analyzer->GetAlarm(This->Params, Resp);
  } else if(This->Method == "setAlarm") {
    This->Analyzer->SetAlarm(This->Params, Resp);
  } else if(This->Method == "addAlarm") {
    This->Analyzer->AddAlarm(This->Params, Resp);
  } else if(This->Method == "delAlarm") {
    This->Analyzer->DelAlarm(This->Params, Resp);
  }
  json Content = json::object();
  Content["Code"] = Resp.status;
  Content["Sequence"] = This->Sequence;
  Content["Analyzer"] = This->Analyzer->Serial;
  Content["Response"] = Resp.body;
  This->Sync->pLocks.ReadLock();
  Headers Header = {{"Accept", "application/json"}, {"Connection", "keep-alive"}, {"Cookie", This->Sync->pCookie}};
  string  Path   = This->Sync->pPath + "/setRemote";
  Client HTTPClient(This->Sync->pServer.c_str(), This->Sync->pPort);
  This->Sync->pLocks.ReadUnlock();
  auto Response = HTTPClient.Post(Path.c_str(), Header, Content.dump(), "application/json", true);
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    LOGFile::Debug("Remote request '%s' for analyzer '%s' completed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  } else {
    LOGFile::Error("Remote request '%s' for analyzer '%s' failed\n", This->Method.c_str(), This->Analyzer->Serial.c_str());
  }
  This->Sync->pLocks.WriteLock();
  This->Sync->pThreadsCount--;
  This->Sync->pLocks.WriteUnlock();
  delete This;
  return NULL;
}
