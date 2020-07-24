// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "AnalyzerSoftSync.h"
// ---------------------------------------------------------------------------------------------------------------------
json AnalyzerSoftSync::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["SyncEnabled"] = false;
  ConfigOUT["Sync"] = json::object();
  ConfigOUT["Sync"]["Server"] = "";
  ConfigOUT["Sync"]["User"] = "";
  ConfigOUT["Sync"]["Pass"] = "";
  if(config.contains("SyncEnabled") && config["SyncEnabled"].is_boolean()) ConfigOUT["SyncEnabled"] = config["SyncEnabled"];
  if(config.contains("Sync") && config["Sync"].is_object()) {
    if(config["Sync"].contains("Server") && config["Sync"]["Server"].is_string()) ConfigOUT["Sync"]["Server"] = config["Sync"]["Server"];
    if(config["Sync"].contains("User") && config["Sync"]["User"].is_string()) ConfigOUT["Sync"]["User"] = config["Sync"]["User"];
    if(config["Sync"].contains("Pass") && config["Sync"]["Pass"].is_string()) ConfigOUT["Sync"]["Pass"] = config["Sync"]["Pass"];
  }
  return ConfigOUT;
}
void AnalyzerSoftSync::Config(const json &config) {
//  bool SyncEnabled = options["SyncEnabled"];
//  if(SyncEnabled == true) {
//    pSyncServer    = options["Sync"]["Server"];
//    pSyncUser      = options["Sync"]["User"];
//    pSyncPass      = options["Sync"]["Pass"];
//    pSyncTerminate = false;
//    if(pSyncThread == 0) {
//      if(pthread_create(&pSyncThread, NULL, Sync_Thread, this) != 0) {
//        LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
//        pSyncThread = 0;
//      } else {
//        pthread_setname_np(pSyncThread, "Sync");
//      }
//    }
//  } else if((SyncEnabled == false) && (pSyncThread != 0)) {
//    pSyncTerminate = true;
//    pthread_join(pSyncThread, NULL);
//    pSyncThread = 0;
//  }
}


/*

//  pSyncTerminate = true;
//  if(pSyncThread != 0) pthread_join(pSyncThread, NULL);

void *AnalyzerSoftSync::Sync_Thread(void *params) {
  AnalyzerSoft *This = (AnalyzerSoft*)params;
  nice(10);
  string Cookie, User, Pass, Server;
  uint32_t ThreadsCount = 0;
  pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_cond_t  Cond  = PTHREAD_COND_INITIALIZER;
  while(This->pSyncTerminate == false) {
    This->pAnalyzerLock.ReadLock();
    if((This->pSyncUser != User) || (This->pSyncPass != Pass) || (This->pSyncServer != Server)) {
      Cookie = "";
      User   = This->pSyncUser;
      Pass   = This->pSyncPass;
      Server = This->pSyncServer;
    }
    This->pAnalyzerLock.Unlock();
    URL Url(Server);
    Client HTTPClient(Url.Host(), Url.Port());
    HTTPClient.set_compress(true);
    HTTPClient.set_read_timeout(30);
    HTTPClient.set_write_timeout(30);
    HTTPClient.set_connection_timeout(30);
    if(Cookie == "") {
      Headers Header  = {{"Accept", "application/json"}};
      json    Content = json::object();
      Content["Username"] = User;
      Content["Password"] = Pass;
      Content["UpdateLastLogin"] = false;
      auto Response = HTTPClient.Post(string(Url.Path() + "/logIn").c_str(), Header, Content.dump(), "application/json");
      if((Response != nullptr) && (Response->status == HTTP_OK)) {
        json Content = json::parse(Response->body, NULL, false);
        if((Response->has_header("Set-Cookie") == true) && (Content.is_discarded() == false) && Content.is_object() && Content["Result"].is_boolean() && (Content["Result"] == true)) {
          string Tmp = Response->get_header_value("Set-Cookie");
          for(uint8_t n = 0; (n < Tmp.length()) && (Tmp.length() < 100); n++) {
            if(Tmp[n] == ';') break;
            Cookie += Tmp[n];
          }
        }
      }
      if(Cookie != "") {
        LogFile::Debug("Analyzer '%s' logged on remote server", This->pSerial.c_str());
      } else {
        LogFile::Debug("Analyzer '%s' can not login on remote server", This->pSerial.c_str());
        usleep(1000000);
      }
    } else {
      json   Content = json::object();
      Content["Serial"] = "REMOTE";
      Content["Analyzer"] = This->pSerial;
      Content["Name"] = This->GetName();
      Request Req;
      Req.method = "POST";
      Req.path = Url.Path() + "/getRemote";
      Req.headers =  {{"Accept", "application/json"}, {"Cookie", Cookie}, { "Accept", "text/event-stream" }, {"Content-Type", "application/json"}};
      Req.body = Content.dump();
      Req.progress = nullptr;
      Req.response_handler = [&](const Response &response) -> bool {
        if(This->pSyncTerminate == true) return false;
        if(response.status == HTTP_UNAUTHORIZED) {
          Cookie = "";
          LogFile::Debug("Analyzer '%s' not authorized on remote server, trying to login again...", This->pSerial.c_str());
          usleep(1000000);
          return false;
        } else if(response.status == HTTP_OK) {
          return true;
        } else {
          LogFile::Debug("Failed request to remote server in analyzer '%s' (ERROR: %d)", This->pSerial.c_str(), response.status);
          usleep(1000000);
          return false;
        }
      };
      Req.content_receiver = [&](const char *data, uint64_t len) -> bool {
        if(This->pSyncTerminate == true) return false;
        if((len <= 5) || (strncmp("data:", data, 5) != 0)) return false;
        string Msg(&data[5], len - 5);
        json Content = json::parse(Msg, NULL, false);
        if((Content.is_discarded() == true) || (Content.is_object() == false)) return true;
        LogFile::Debug("Analyzer '%s' receive querie from remote server", This->pSerial.c_str());
        if((Content["Sequence"].is_number() == false) || (Content["Method"].is_string() == false) || (Content["Params"].is_object() == false)) return false;
        Sync_t *Request = new Sync_t;
        Request->Count        = &ThreadsCount;
        Request->Mutex        = &Mutex;
        Request->Cond         = &Cond;
        Request->Cookie       = Cookie;
        Request->Server       = Server;
        Request->Analyzer     = This;
        Request->Method       = Content["Method"];
        Request->Params       = Content["Params"];
        Request->Sequence     = Content["Sequence"];
        pthread_t Thread;
        pthread_mutex_lock(&Mutex);
        if(pthread_create(&Thread, NULL, Sync_RespThread, (void*)Request) != 0) {
          LogFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
          delete Request;
        } else {
          pthread_setname_np(Thread, "SyncWorker");
          ThreadsCount++;
          pthread_detach(Thread);
        }
        pthread_mutex_unlock(&Mutex);
        return true;
      };
      auto Res = std::make_shared<Response>();
      if(HTTPClient.send(Req, *Res) == false) usleep(1000000);
    }
  }
  pthread_mutex_lock(&Mutex);
  while(ThreadsCount > 0) {
    LogFile::Debug("Analyzer '%s' flusing remote request (%d pending)", This->pSerial.c_str(), ThreadsCount);
    pthread_cond_wait(&Cond, &Mutex);
  }
  pthread_mutex_unlock(&Mutex);
  return NULL;
}
void *AnalyzerSoft::Sync_RespThread(void *args) {
  Sync_t *This = (Sync_t*)args;
  This->Params["UserName"] = "admin";
  LogFile::Debug("Remote server request query '%s' for analyzar '%s'", This->Method.c_str(), This->Analyzer->pSerial.c_str());
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
  } else if(This->Method == "getPrices") {
    This->Analyzer->GetPrices(This->Params, Resp);
  } else if(This->Method == "testAlarm") {
    This->Analyzer->TestAlarm(This->Params, Resp);
  } else if(This->Method == "getAlarms") {
    This->Analyzer->GetAlarms(This->Params, Resp);
  }
  URL Url(This->Server);
  Headers Header  = {{"Accept", "application/json"}, {"Cookie", This->Cookie}};
  json    Content = json::object();
  Content["Code"] = Resp.status;
  Content["Sequence"] = This->Sequence;
  Content["Serial"] = "REMOTE";
  Content["Analyzer"] = This->Analyzer->pSerial;
  Content["Response"] = Resp.body;
  Content["Mime"] = Resp.has_header("Content-Type") ? Resp.get_header_value("Content-Type") : "text/plain";
  Client HTTPClient(Url.Host().c_str(), Url.Port());
  HTTPClient.set_compress(true);
  auto Response = HTTPClient.Post(string(Url.Path() + "/setRemote").c_str(), Header, Content.dump(), "application/json");
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    LogFile::Debug("Remote request '%s' for analyzer '%s' completed", This->Method.c_str(), This->Analyzer->pSerial.c_str());
  } else {
    if(Response != nullptr)
      LogFile::Error("Remote request '%s' for analyzer '%s' failed (ERROR: %d)", This->Method.c_str(), This->Analyzer->pSerial.c_str(), Response->status);
    else
      LogFile::Error("Remote request '%s' for analyzer '%s' failed (can not connect)", This->Method.c_str(), This->Analyzer->pSerial.c_str());
  }
  pthread_mutex_lock(This->Mutex);
  *This->Count = *This->Count - 1;
  pthread_cond_signal(This->Cond);
  pthread_mutex_unlock(This->Mutex);
  delete This;
  return NULL;
}
*/