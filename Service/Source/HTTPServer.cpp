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
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
map<string, HTTPServer::Func_t> __attribute__((init_priority(200))) HTTPServer::pCallbacks = map<string, HTTPServer::Func_t>();
RWLocks                                                             HTTPServer::pLock;
string                                                              HTTPServer::pRelativePath = "/";
//----------------------------------------------------------------------------------------------------------------------
URL::URL(const string &url) {
  typedef string::const_iterator iterator_t;
  if(url.length() == 0) return;
  iterator_t uriEnd = url.end();
  iterator_t queryStart = std::find(url.begin(), uriEnd, '?');
  iterator_t protocolStart = url.begin();
  iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');
  Port = 80;
  if(protocolEnd != uriEnd) {
    std::string prot = &*(protocolEnd);
    if((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      Protocol = string(protocolStart, protocolEnd);
      protocolEnd += 3;
      if(Protocol == "https") Port = 443;
    } else {
      protocolEnd = url.begin();
    }
  } else {
    protocolEnd = url.begin();
  }
  iterator_t hostStart = protocolEnd;
  iterator_t pathStart = std::find(hostStart, uriEnd, '/');
  iterator_t hostEnd = std::find(protocolEnd, (pathStart != uriEnd) ? pathStart : queryStart, ':');
  Host = string(hostStart, hostEnd);
  if((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {
    hostEnd++;
    iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
    string PortText = string(hostEnd, portEnd);
    Port = (PortText == "") ? 80 : atoi(PortText.c_str());
  }
  if(pathStart != uriEnd) Path = string(pathStart, queryStart);
  if(queryStart != uriEnd) Query = string(queryStart, url.end());
}
//----------------------------------------------------------------------------------------------------------------------
//HTTPTaskQueue  *HTTPTaskQueue::pInstance = NULL;
//----------------------------------------------------------------------------------------------------------------------
HTTPTaskQueue::HTTPTaskQueue() {
  pRunningWorkers = 0;
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
  pLastAdjust     = Tools::GetTime64();
}
HTTPTaskQueue::~HTTPTaskQueue() {
  shutdown();
}
void HTTPTaskQueue::enqueue(std::function<void()> fn) {
  Worker_t *Tmp = NULL;
  pthread_mutex_lock(&pMutex);
  pRunningWorkers++;
  if(pFreeWorkers.size() > 0) {
    Tmp = pFreeWorkers.front();
    pFreeWorkers.pop_front();
  }
  pthread_mutex_unlock(&pMutex);
  if(Tmp == NULL) {
    Tmp = new Worker_t;
    Tmp->Terminate = false;
    Tmp->Cond      = PTHREAD_COND_INITIALIZER;
    Tmp->Mutex     = PTHREAD_MUTEX_INITIALIZER;
    Tmp->Parent    = this;
    Tmp->Function  = fn;
    if(pthread_create(&Tmp->Thread, NULL, Thread, (void*)Tmp) != 0) {
      Tmp->Thread = 0;
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      return;
    }
    pthread_setname_np(Tmp->Thread, "HTTPWorker");
  } else {
    pthread_mutex_lock(&Tmp->Mutex);
    Tmp->Function = fn;
    pthread_cond_signal(&Tmp->Cond);
    pthread_mutex_unlock(&Tmp->Mutex);
  }
}
void *HTTPTaskQueue::Thread(void *params) {
  Worker_t *This = (Worker_t*)params;
  nice(15);
  while(true) {
    pthread_mutex_lock(&This->Mutex);
    while((This->Terminate == false) && (This->Function == NULL)) pthread_cond_wait(&This->Cond, &This->Mutex);
    if(This->Terminate == true) return NULL;
    pthread_mutex_unlock(&This->Mutex);
    This->Function();
    pthread_mutex_lock(&This->Mutex);
    This->Function = NULL;
    This->LastTime = Tools::GetTime64();
    pthread_mutex_unlock(&This->Mutex);
    pthread_mutex_lock(&This->Parent->pMutex);
    This->Parent->pFreeWorkers.push_front(This);
    This->Parent->pRunningWorkers--;
    if(This->Parent->pRunningWorkers == 0) pthread_cond_signal(&This->Parent->pCond);
    pthread_mutex_unlock(&This->Parent->pMutex);
  }
}
void HTTPTaskQueue::shutdown() {
  pthread_mutex_lock(&pMutex);
  while(pRunningWorkers > 0) pthread_cond_wait(&pCond, &pMutex);
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.front();
    pFreeWorkers.pop_front();
    pthread_mutex_lock(&Tmp->Mutex);
    Tmp->Terminate = true;
    pthread_cond_signal(&Tmp->Cond);
    pthread_mutex_unlock(&Tmp->Mutex);
    pthread_join(Tmp->Thread, NULL);
    delete Tmp;
  }
  pthread_mutex_unlock(&pMutex);
}
void HTTPTaskQueue::on_idle() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  if((pLastAdjust + 60000) > Time) {
    pthread_mutex_unlock(&pMutex);
    return;
  }
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.back();
    if((Time - Tmp->LastTime) < 60000) break;
    Tmp->Terminate = true;
    pthread_cond_signal(&Tmp->Cond);
    pthread_mutex_unlock(&Tmp->Mutex);
    pthread_join(Tmp->Thread, NULL);
    pFreeWorkers.pop_back();
    delete Tmp;
  }
  pthread_mutex_unlock(&pMutex);
  LOGFile::Debug("Adjusted HTTP workers pool to %d connections (plus %u in use)\n", pFreeWorkers.size(), pRunningWorkers);
}
//----------------------------------------------------------------------------------------------------------------------
uint16_t       HTTPServer::pPort   = 8080;
pthread_t      HTTPServer::pThread = 0;
Server        *HTTPServer::pServer = NULL;
//----------------------------------------------------------------------------------------------------------------------
HTTPServer::HTTPServer() {
  json Config = ConfigManager::ReadConfig("/HTTPServer");
  if(!Config.is_object()) Config = json::object();
  if(Config["Port"].is_number_unsigned()) pPort = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  pServer = new Server();
  pServer->Get(".*", HandleGet);
  pServer->Post(".*", HandlePost);
  pServer->set_keep_alive_max_count(100);
  pServer->set_read_timeout(5, 0);
  pServer->new_task_queue = [] { return new HTTPTaskQueue(); };
  Config = json::object();
  Config["Port"] = pPort;
  Config["RelativePath"] = pRelativePath;
  ConfigManager::WriteConfig("/HTTPServer", Config);
  if(pthread_create(&pThread, NULL, Listen_Thread, (void*)this) != 0) {
    pThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pThread, "HTTPServer");
  }
}
HTTPServer::~HTTPServer() {
  LOGFile::Debug("Stopping HTTPServer\n");
  pServer->stop();
  if(pThread != 0) pthread_join(pThread, NULL);
  delete pServer;
  LOGFile::Debug("HTTPServer stopped\n");
}
void *HTTPServer::Listen_Thread(void *params) {
  HTTPServer *This = (HTTPServer*)params;
  nice(10);
  LOGFile::Info("HTTP server starting on port %hu with relative path '%s'\n", This->pPort, This->pRelativePath.c_str());
  if(This->pServer->listen("0.0.0.0", This->pPort) == false) LOGFile::Error("HTTP port (%hu) already in use\n", This->pPort);
  return NULL;
}
void HTTPServer::FillResponse(Response &response, const string &body, const char *mime, const int status) {
  response.set_content(body, mime);
  response.status = status;
}
void HTTPServer::FillResponse(Response &response, const char *body, const size_t size, const char *mime, const int status) {
  response.set_content(body, size, mime);
  response.status = status;
}
void HTTPServer::RegisterCallback(const string &handleName, const Func_t function) {
  pLock.WriteLock();
  pCallbacks.insert({handleName, function});
  pLock.Unlock();
}
void HTTPServer::UnregisterCallback(const string &handleName) {
  pLock.WriteLock();
  pCallbacks.erase(handleName);
  pLock.Unlock();
}
string HTTPServer::ParseCookie(const string &cookie) {
  string CookieNum = "";
  auto Begin = cookie.find("SESSION=");
  if(Begin != string::npos) CookieNum = cookie.substr(Begin + 8, cookie.find(';', Begin) - Begin - 8);
  return CookieNum;
}
void HTTPServer::HandleGet(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP GET '%s'\n", URL.c_str());
  size_t Size = 0;
  const char *Tmp = (const char*)ResourceLoader::GetFile(string("WEB/") + URL, &Size);
  if(Size != 0) {
    size_t FileNameLen = URL.length();
    if((FileNameLen >= 3) && (URL.compare(FileNameLen - 3, 3, ".js")) == 0) {
      return FillResponse(response, Tmp, Size, "text/javascript", HTTP_OK);
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".png"))  == 0) {
      return FillResponse(response, Tmp, Size, "image/png", HTTP_OK);
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".ico"))  == 0) {
      return FillResponse(response, Tmp, Size, "image/x-icon", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 5, 5, ".html")) == 0) {
      return FillResponse(response, Tmp, Size, "text/html", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 5, 5, ".json")) == 0) {
      return FillResponse(response, Tmp, Size, "application/json", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 3, 3, ".md")) == 0) {
      return FillResponse(response, Tmp, Size, "text/markdown", HTTP_OK);
    }
    LOGFile::Debug("No mimetype defined for request '%s'\n", URL.c_str());
    return FillResponse(response, Tmp, Size, "application/octet-stream", HTTP_OK);
  }
  string UserName;
  if(request.has_header("Cookie")) {
    string Cookie = ParseCookie(request.get_header_value("Cookie"));
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) UserName = DB.ResultString("username");
    }
  }
  if(URL == "Login") {
    if(UserName == "") {
      size_t Len;
      const char *Content = (const char *)ResourceLoader::GetFile("WEB/Login.html", &Len);
      string Login = string(Content, Len);
      string Find = "{{ RELATIVE_PATH }}";
      Len = Login.find(Find, 0);
      if(Len != string::npos) Login = Login.replace(Len, Find.size(), pRelativePath);
      return FillResponse(response, Login, "text/html", HTTP_OK);
    } else {
      return response.set_redirect(pRelativePath.c_str());
    }
  }
  if(UserName == "") {
    return response.set_redirect(string(pRelativePath + "Login").c_str());
  } else {
    size_t Len;
    const char *Content = (const char *)ResourceLoader::GetFile("WEB/Index.html", &Len);
    string Index = string(Content, Len);
    string Find = "{{ VERSION }}";
    string Version = __BUILD__;
    Len = Index.find(Find, 0);
    if(Len != string::npos) Index = Index.replace(Len, Find.size(), Version);
    Find = "{{ RELATIVE_PATH }}";
    Len = Index.find(Find, 0);
    if(Len != string::npos) Index = Index.replace(Len, Find.size(), pRelativePath);
    return FillResponse(response, Index, "text/html", HTTP_OK);
  }
}
void HTTPServer::HandlePost(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP POST '%s'\n", URL.c_str());
  json Params = json::parse(request.body, NULL, false);
  if(Params.is_discarded()) return FillResponse(response, "Malformed JSON params", "text/plain", HTTP_BAD_REQUEST);
  if(URL == "logIn") {
    if((Params["Password"].is_string() == false) || (Params["Username"].is_string() == false)) {
      return FillResponse(response, "'Password' and 'Username' params requiered", "text/plain", HTTP_BAD_REQUEST);
    } else {
      Database DB;
      json     Response        = json::object();
      string   User            = Params["Username"];
      string   Pass            = Params["Password"];
      bool     UpdateLastLogin = true;
      DB.ExecSentenceAsyncResult("SELECT lastlogin FROM users WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(Pass) + ")");
      if(DB.FecthRow()) {
        Response["Result"] = true;
        Response["LastLogin"] = DB.ResultUInt64("lastlogin");
        if(Params["UpdateLastLogin"].is_boolean()) UpdateLastLogin = Params["UpdateLastLogin"];
        DB.ExecSentenceAsyncResult("INSERT INTO sessions(username, timestamp) VALUES (" + DB.Bind(User) + ", " + DB.Bind(Tools::GetTime64()) + ") RETURNING id");
        if(DB.FecthRow()) response.set_header("Set-Cookie", "SESSION=" + DB.ResultString("id"));
        if(UpdateLastLogin) DB.ExecSentenceNoResult("UPDATE users SET lastlogin = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(User));
      } else {
        Response["Result"] = false;
      }
      return FillResponse(response, Response.dump(), "application/json", HTTP_OK);
    }
  }
  if(URL == "logOut") {
    if(request.has_header("Cookie")) {
      string Cookie = ParseCookie(request.get_header_value("Cookie"));
      if(Cookie != "") {
        Database DB;
        DB.ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + DB.Bind(Cookie));
      }
    }
    return response.set_redirect(string(pRelativePath + "Login").c_str());
  }
  if(request.has_header("Cookie")) {
    Params.erase("UserName");
    string Cookie = ParseCookie(request.get_header_value("Cookie"));
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) Params["UserName"] = DB.ResultString("username");
    }
  } else if(request.has_header("REMOTE_ADDR")) {
    if(request.get_header_value("REMOTE_ADDR") != "127.0.0.1") Params.erase("UserName");
  }
  if(Params.contains("UserName") == false) return FillResponse(response, "Unauthorided access to API", "text/plain", HTTP_UNAUTHORIZED);
  pLock.ReadLock();
  auto Callback = pCallbacks.find(URL);
  if(Callback != pCallbacks.end()) {
    Callback->second(Params, response);
    pLock.Unlock();
    return;
  }
  pLock.Unlock();
  FillResponse(response, "Method not found", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void HTTPServer::RemoveExpiredSessions() const {
  if(Tools::GetTime64() < 1296000000) return;
  Database DB;
  DB.ExecSentenceNoResult("DELETE FROM sessions WHERE timestamp < " + DB.Bind(Tools::GetTime64() - 1296000000));
}