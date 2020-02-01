/* ================================== zMeter ===========================================================================
 * File:     HTTPServer.cpp
 * Eduardo Viciana (2020)
 * ------------------------------------------------------------------------------------------------------------------ */
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
HTTPCallback::HTTPCallback(const string& handle, Func_t function) : HandleName(handle), Function(function) {
};
void HTTPCallback::FillResponse(Response &response, const string &body, const char *mime, const int status) {
  response.set_content(body, mime);
  response.status = status;
}
void HTTPCallback::FillResponse(Response &response, const char *body, const size_t size, const char *mime, const int status) {
  response.set_content(body, size, mime);
  response.status = status;
}
//----------------------------------------------------------------------------------------------------------------------
vector<HTTPCallback> __attribute__((init_priority(200))) HTTPServer::pCallbacks;
pthread_rwlock_t                                         HTTPServer::pLock = PTHREAD_RWLOCK_INITIALIZER;
string                                                   HTTPServer::pRelativePath = "/";
//----------------------------------------------------------------------------------------------------------------------
URL::URL(const string &url) {
  typedef string::const_iterator iterator_t;
  if(url.length() == 0) return;
  iterator_t uriEnd = url.end();
  iterator_t queryStart = std::find(url.begin(), uriEnd, '?');
  iterator_t protocolStart = url.begin();
  iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');
  if(protocolEnd != uriEnd) {
    std::string prot = &*(protocolEnd);
    if((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      Protocol = string(protocolStart, protocolEnd);
      protocolEnd += 3;
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
    Port = string(hostEnd, portEnd);
  }
  if(pathStart != uriEnd) Path = string(pathStart, queryStart);
  if(queryStart != uriEnd) Query = string(queryStart, url.end());
}
//----------------------------------------------------------------------------------------------------------------------
HTTPServer::HTTPServer() {
  pServer = NULL;
  pPort   = 8080;
  pThread = 0;
  json Config = ConfigManager::ReadConfig("/HTTPServer");
  if(!Config.is_object()) Config = json::object();
  if(Config["Port"].is_number_unsigned()) pPort = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  pServer = new Server();
  pServer->Get(".*", HandleGet);
  pServer->Post(".*", HandlePost);
  Config = json::object();
  Config["Port"] = pPort;
  Config["RelativePath"] = pRelativePath;
  ConfigManager::WriteConfig("/HTTPServer", Config);
  pthread_create(&pThread, NULL, Listen_Thread, (void*)this);
  pthread_setname_np(pThread, "HTTPServer");
}
HTTPServer::~HTTPServer() {
  pServer->stop();
  if(pThread != 0) pthread_join(pThread, NULL);
}
void *HTTPServer::Listen_Thread(void *params) {
  HTTPServer *This = (HTTPServer*)params;
  LOGFile::Info("HTTP server starting on port %hu with relative path '%s'\n", This->pPort, This->pRelativePath.c_str());
  if(This->pServer->listen("0.0.0.0", This->pPort) == false) LOGFile::Error("HTTP port (%hu) already in use\n", This->pPort);
  return NULL;
}
void HTTPServer::RegisterCallback(const string &handleName, const HTTPCallback::Func_t function) {
  pthread_rwlock_wrlock(&pLock);
  for(HTTPCallback &Callback : pCallbacks) {
    if(Callback.HandleName != handleName) continue;
    Callback.Function = function;
    pthread_rwlock_unlock(&pLock);
    return;
  }
  pCallbacks.emplace_back(handleName, function);
  pthread_rwlock_unlock(&pLock);
}
void HTTPServer::UnregisterCallback(const string &handleName) {
  pthread_rwlock_wrlock(&pLock);
  for(auto it = pCallbacks.begin(); it < pCallbacks.end(); it++) {
    if(it->HandleName != handleName) continue;
    pthread_rwlock_unlock(&pLock);
    return;
  }
  pthread_rwlock_unlock(&pLock);
}
void HTTPServer::HandleGet(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP GET '%s'\n", URL.c_str());
  size_t Size = 0;
  const char *Tmp = (const char*)ResourceLoader::GetFile(string("WEB/") + URL, &Size);
  if(Size != 0) {
    size_t FileNameLen = URL.length();
    if((FileNameLen >= 3) && (URL.compare(FileNameLen - 3, 3, ".js")) == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "text/javascript", HTTP_OK);
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".png"))  == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "image/png", HTTP_OK);
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".ico"))  == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "image/x-icon", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 5, 5, ".html")) == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "text/html", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 5, 5, ".json")) == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "application/json", HTTP_OK);
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 3, 3, ".md")) == 0) {
      return HTTPCallback::FillResponse(response, Tmp, Size, "text/markdown", HTTP_OK);
    }
    LOGFile::Debug("No mimetype defined for request '%s'\n", URL.c_str());
    return HTTPCallback::FillResponse(response, Tmp, Size, "application/octet-stream", HTTP_OK);
  }
  string UserName;
  if(request.has_header("Cookie")) {
    string Cookie = request.get_header_value("Cookie").substr(8);
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
      return HTTPCallback::FillResponse(response, Login, "text/html", HTTP_OK);
    } else {
      return response.set_redirect(pRelativePath.c_str());
    }
  }
  if(UserName == "") {
    return response.set_redirect("Login");
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
    return HTTPCallback::FillResponse(response, Index, "text/html", HTTP_OK);
  }
}
void HTTPServer::HandlePost(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP POST '%s'\n", URL.c_str());
  json Params = json::parse(request.body, NULL, false);
  if(Params.is_discarded()) return HTTPCallback::FillResponse(response, "Malformed JSON params", "text/plain", HTTP_BAD_REQUEST);
  if(URL == "logIn") {
    if((Params["Password"].is_string() == false) || (Params["Username"].is_string() == false)) {
      return HTTPCallback::FillResponse(response, "'Password' and 'Username' params requiered", "text/plain", HTTP_BAD_REQUEST);
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
        DB.ExecSentenceAsyncResult("INSERT INTO sessions(username, timestamp) VALUES (" + DB.Bind(User) + ", " + DB.Bind(Tools::GetTime()) + ") RETURNING id");
        if(DB.FecthRow()) response.set_header("Set-Cookie", "SESSION=" + DB.ResultString("id"));
        if(UpdateLastLogin) DB.ExecSentenceNoResult("UPDATE users SET lastlogin = " + DB.Bind(Tools::GetTime()) + " WHERE username = " + DB.Bind(User));
      } else {
        Response["Result"] = false;
      }
      return HTTPCallback::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
    }
  }
  if(URL == "logOut") {
    if(request.has_header("Cookie")) {
      string Cookie = request.get_header_value("Cookie").substr(8);
      if(Cookie != "") {
        Database DB;
        DB.ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + DB.Bind(Cookie));
      }
    }
    return response.set_redirect(string(pRelativePath + "Login").c_str());
  }
  if(request.has_header("Cookie")) {
    Params.erase("UserName");
    string Cookie = request.get_header_value("Cookie").substr(8);
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) Params["UserName"] = DB.ResultString("username");
    }
  } else if(request.has_header("REMOTE_ADDR")) {
    if(request.get_header_value("REMOTE_ADDR") != "127.0.0.1") Params.erase("UserName");
  }
  if(Params.contains("UserName") == false) return HTTPCallback::FillResponse(response, "Unauthorided access to API", "text/plain", HTTP_UNAUTHORIZED);
  pthread_rwlock_rdlock(&pLock);
  for(HTTPCallback &Callback : pCallbacks) {
    if(Callback.HandleName != URL) continue;
    Callback.Function(Params, response);
    pthread_rwlock_unlock(&pLock);
    return;
  }
  pthread_rwlock_unlock(&pLock);
  HTTPCallback::FillResponse(response, "Method not found", "text/plain", HTTP_METHOD_NOT_ALLOWED);
}
void HTTPServer::RemoveExpiredSessions() const {
  if(Tools::GetTime() < 1296000000) return;
  Database DB;
  DB.ExecSentenceNoResult("DELETE FROM sessions WHERE timestamp < " + DB.Bind(Tools::GetTime() - 1296000000));
}