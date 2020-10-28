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
#include "LogFile.h"
#include "URL.h"
#include "AnalyzerSoftSync.h"
#include "AnalyzerSoft.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftSync::AnalyzerSoftSync(AnalyzerSoft *parent) {
  pRunningWorkers = 0;
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
  pParent         = parent;
  pThread         = 0;
  pReAuth         = false;
}
AnalyzerSoftSync::~AnalyzerSoftSync() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pthread_mutex_lock(&pMutex);
  while(pRunningWorkers > 0) pthread_cond_wait(&pCond, &pMutex);
  while(pFreeWorkers.size() > 0) {
    delete pFreeWorkers.front();
    pFreeWorkers.pop_front();
  }
  pthread_mutex_unlock(&pMutex);
}
json AnalyzerSoftSync::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Server"] = "";
  ConfigOUT["User"] = "";
  ConfigOUT["Pass"] = "";
  if(config.is_object() == true) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Server") && config["Server"].is_string()) ConfigOUT["Server"] = config["Server"];
    if(config.contains("User") && config["User"].is_string()) ConfigOUT["User"] = config["User"];
    if(config.contains("Pass") && config["Pass"].is_string()) ConfigOUT["Pass"] = config["Pass"];
  }
  return ConfigOUT;
}
void AnalyzerSoftSync::Config(const json &config, const string &name) {
  pthread_mutex_lock(&pMutex);
  bool Enabled = config["Enabled"];
  if(Enabled == true) {
    pServer    = config["Server"];
    pUser      = config["User"];
    pPass      = config["Pass"];
    pTerminate = false;
    pName      = name;
    if(pThread == 0) {
      if(pthread_create(&pThread, NULL, [](void *params) { return ((AnalyzerSoftSync*)params)->Thread(); }, this) != 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        pThread = 0;
      } else {
        pthread_setname_np(pThread, "Sync");
      }
    }
  } else if(Enabled == false) {
    if(pThread != 0) {
      pTerminate = true;
      pthread_join(pThread, NULL);
      pThread = 0;
    }
  }
  pthread_mutex_unlock(&pMutex);
}
bool AnalyzerSoftSync::Login(const string &path, Client *client, const string &user, const string &pass, string &cookie) {
  if(cookie.empty() == false) return true;
  Headers Header  = { {"Accept", "application/json"} };
  json    Content = json::object();
  Content["UserName"]        = user;
  Content["Password"]        = pass;
  Content["UpdateLastLogin"] = false;
  auto Response = client->Post((path + "/logIn").c_str(), Header, Content.dump(), "application/json");
  if((Response != nullptr) && (Response->status == HTTP_OK) && (Response->has_header("Set-Cookie") == true)) {
    LogFile::Debug("Analyzer '%s' logged on remote server", pParent->Serial().c_str());
    cookie = Response->get_header_value("Set-Cookie");
    return true;
  } else {
    LogFile::Debug("Analyzer '%s' can not login on remote server", pParent->Serial().c_str());
    usleep(1000000);
    return false;
  }
}
void *AnalyzerSoftSync::Thread() {
  nice(10);
  string  Cookie, User, Pass, Server, Name;
  URL    *Url        = NULL;
  Client *HTTPClient = NULL;
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    if((pReAuth == true) || (pUser != User) || (pName != Name) || (pPass != Pass) || (pServer != Server) || (HTTPClient == NULL) || (Url == NULL)) {
      Cookie  = "";
      pReAuth = false;
      User    = pUser;
      Pass    = pPass;
      Server  = pServer;
      Name    = pName;
      if(Url != NULL) {
        delete Url;
        Url = NULL;
      }
      Url = new URL(Server);
      if(HTTPClient != NULL) {
        delete HTTPClient;
        HTTPClient = NULL;
      }
      HTTPClient = new Client(Url->Host(), Url->Port());
      HTTPClient->set_compress(true);
      HTTPClient->set_keep_alive(true);
      HTTPClient->set_read_timeout(30);
      HTTPClient->set_write_timeout(30);
      HTTPClient->set_connection_timeout(30);
    }
    pthread_mutex_unlock(&pMutex);
    if(Login(Url->Path(), HTTPClient, User, Pass, Cookie) == false) continue;
    string   Buffer;
    Request  Req;
    Response Res;
    json     Content     = json::object();
    Content["Analyzer"]  = pParent->Serial();
    Content["Name"]      = Name;
    Req.method           = "POST";
    Req.path             = Url->Path() + "/getRemote";
    Req.headers          = {{"Cookie", Cookie}, { "Accept", "text/event-stream" }, {"Content-Type", "application/json"}};
    Req.body             = Content.dump();
    Req.response_handler = [&](const Response &response) -> bool {
      if(pTerminate == true) return false;
      if(response.status == HTTP_OK) return true;
      LogFile::Debug("Failed request to remote server in analyzer '%s' (ERROR: %d)", pParent->Serial().c_str(), response.status);
      usleep(1000000);
      return false;
    };
    Req.progress = [&](uint64_t current, uint64_t total) -> bool {
      if((current == 0) && (total > 0x00100000)) return false;
      if(current == 0) {
        Buffer.clear();
        Buffer.reserve(total);
      } else if(current == total) {
        if((Buffer.size() > 5) && (Buffer.substr(0, 6) == "data: ")) {
          Buffer = Buffer.substr(6);
          json Content = json::parse(Buffer, NULL, false);
          if(!Content.is_discarded() && Content.is_object() && Content["Sequence"].is_string() && Content["Method"].is_string()) {
            LogFile::Debug("Analyzer '%s' receive query from remote server", pParent->Serial().c_str());
            string ID     = Content["Sequence"];
            string Method = Content["Method"];
            Content.erase("Sequence");
            Content.erase("Method");
            PopWorker()->Run(Cookie, *Url, ID, Method, Content);
          }
        }
        Buffer.clear();
      }
      return true;
     };
    Req.content_receiver = [&](const char *data, uint64_t len) -> bool {
      pthread_mutex_lock(&pMutex);
      if((pTerminate == true) || (pReAuth == true) || (pUser != User) || (pName != Name) || (pPass != Pass) || (pServer != Server)) {
        pthread_mutex_unlock(&pMutex);
        return false;
      }
      pthread_mutex_unlock(&pMutex);
      Buffer.append(data, len);
      return true;
    };
    if(HTTPClient->send(Req, Res) == false) usleep(1000000);
  }
  if(HTTPClient != NULL) delete HTTPClient;
  if(Url != NULL)        delete Url;
  return NULL;
}
void AnalyzerSoftSync::PeriodicTasks() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.back();
    if((Time - Tmp->LastUse()) < 60000) break;
    pFreeWorkers.pop_back();
    delete Tmp;
  }
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("Adjusted HTTP workers pool to %d connections (plus %u in use)", pFreeWorkers.size(), pRunningWorkers);
}
void AnalyzerSoftSync::PushWorker(AnalyzerSoftSyncWorker *worker, const bool fail) {
  pthread_mutex_lock(&pMutex);
  if(fail == true) pReAuth = true;
  pFreeWorkers.push_front(worker);
  pRunningWorkers--;
  if(pRunningWorkers == 0) pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
AnalyzerSoftSyncWorker *AnalyzerSoftSync::PopWorker() {
  pthread_mutex_lock(&pMutex);
  pRunningWorkers++;
  if(pFreeWorkers.size() > 0) {
    AnalyzerSoftSyncWorker *Tmp = pFreeWorkers.front();
    pFreeWorkers.pop_front();
    pthread_mutex_unlock(&pMutex);
    return Tmp;
  } else {
    pthread_mutex_unlock(&pMutex);
    AnalyzerSoftSyncWorker *Tmp = new AnalyzerSoftSyncWorker(this, pParent);
    return Tmp;
  }
}
// ---------------------------------------------------------------------------------------------------------------------