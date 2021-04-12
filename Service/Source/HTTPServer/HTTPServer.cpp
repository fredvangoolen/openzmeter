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
#include "LogFile.h"
#include "ConfigFile.h"
#include "ResourceFile.h"
#include "HTTPRequestPool.h"
#include "MessageLoop.h"
#include "TaskPool.h"
//----------------------------------------------------------------------------------------------------------------------
map<string, HTTPServer::Callback_t> __attribute__((init_priority(200))) HTTPServer::pCallbacks;
map<string, string>                 __attribute__((init_priority(200))) HTTPServer::pComponents;
pthread_rwlock_t                                                        HTTPServer::pLock = PTHREAD_RWLOCK_INITIALIZER;
string                                                                  HTTPServer::pRelativePath = "/";
string                                                                  HTTPServer::pLogoImg = "iVBORw0KGgoAAAANSUhEUgAAAPgAAAAcCAMAAAB72oKtAAABFFBMVEUAAAD////////////////////////////86eH////////////////////////////pcBz//////////v7////////////////////////ocBz////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////mbB7////////////pcBrpcBzmbx3pcBvpcBzocBvpcRzpcBvyrYDocBvobxvocBzocRzpcBzpbhzmbx3pbxvsbx3ncBznch7piz3////ocBxhu/u6AAAAWnRSTlMA5P3f7gXr5gP16PnbQ+H34/LYGQjV0s0/LuDColVMKBCJWyK3ppd+OhPGv4QyqxYNCp6aaSWyjnIryZNtNmMedhBHvHpQ7z7q5bwc+E7WM8imY0WGdiiSdy5XSb+iAAAIGklEQVRYw9VXZ7cSMRCd7LKNLUh7gFSp0qXDE33Ye+/r//8fZibZgkeeejzHo/cLmyE7mTuZuckC4cbYLfYhBnPnuq3YuDZhtuKMpvC38PRKDI/fwW/C1OAXUJ+kfN+3K/XA0Lim+xzlmhxnh75A6mDC38HLrzE8fAS/hfpC/5Utup0KaG2FoaT6Ep0Gjlc4lmjfgL8B80qc+Gv4HTTzSd+Bn2OpRLTmuKG3EpHhbMrzx/wYyn+F+auHssofct5PnsHvII/7Az/FfZ8QMS+m44b8VPP8I5Q1+Ht48Pjr1+eff69YeLzKHn6Km8QmOZT0RlCjCrAGBWEY5KpoUObN29KygF9Ao7npLG5npXZe3y5hN+os9jHBzN0dVRbb6yQaF7XNTWhUu51u9Sitd67yDX+JT333fgVa+3ln4/I36odKp1eHAK3lzc6oKdTZbSY5naXrug0g7LbdSq8q6jTLzUWzuOn0aqCVOaueBtqeWl2pQg9LhXspqqII4BrPQw1zOfIJS/gZ+hNGeskqyOOgJ3x1mETvRrshE7PwqLISmTHvyoTle116xXJKcW3HQv/IH8Z62tfLBm5BOlMsJ/EhWQHC3TOy26xr8hzolsKfLctKXFDDOzQ3xe7j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";
uint16_t                                                                HTTPServer::pPort   = 80;
pthread_t                                                               HTTPServer::pThread = 0;
//----------------------------------------------------------------------------------------------------------------------
HTTPServer::HTTPServer() {
  json   Config = ConfigFile::ReadConfig("/HTTPServer");
  string Logo   = pLogoImg;
  if(!Config.is_object()) Config = json::object();
  if(Config["Logo"].is_string()) Logo = Config["Logo"];
  if(Config["Port"].is_number_unsigned()) pPort = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  Get(".*", HandleGet);
  Post(".*", HandlePost);
  set_keep_alive_max_count(UINT32_MAX);
  new_task_queue = [] { return new HTTPRequestPool(); };
  Config = json::object();
  if(Logo != pLogoImg) Config["Logo"] = Logo;
  Config["Port"] = pPort;
  Config["RelativePath"] = pRelativePath;
  ConfigFile::WriteConfig("/HTTPServer", Config);
  pLogoImg = Tools::DecodeBase64(Logo);
  RegisterCallback("getVersion", GetVersionCallback, "API/GetVersion.json");
  RegisterCallback("getAPI", GetAPICallback, "API/GetAPI.json");
  MessageLoop::RegisterListener("HTTPServer_PeriodicTasks", PeriodicTasks, 60000);
  if(pthread_create(&pThread, NULL, [](void *params) { return ((HTTPServer*)params)->Thread(); }, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  } else {
    if(pthread_setname_np(pThread, "HTTPServer") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    TaskPool::SetThreadProperties(pThread, TaskPool::NORMAL);
  }
}
HTTPServer::~HTTPServer() {
  LogFile::Debug("Stopping HTTPServer");
  MessageLoop::UnregisterListener("HTTPServer_PeriodicTasks");
  UnregisterCallback("getVersion");
  UnregisterCallback("getAPI");
  stop();
  if(pThread != 0) pthread_join(pThread, NULL);
  LogFile::Debug("HTTPServer stopped");
}
void HTTPServer::PeriodicTasks() {
  if(Tools::GetTime64() < 1296000000) return;
  Database DB;
  DB.ExecSentenceNoResult("DELETE FROM sessions WHERE timestamp < " + DB.Bind(Tools::GetTime64() - 1296000000));
}
void *HTTPServer::Thread() {
  LogFile::Info("HTTP server starting on port %hu with relative path '%s'", pPort, pRelativePath.c_str());
  if(listen("0.0.0.0", pPort) == false) LogFile::Error("HTTP port (%hu) already in use", pPort);
  return NULL;
}
void HTTPServer::RegisterCallback(const string &handleName, const function<void(HTTPRequest &)> function, const string &apiFile) {
  pthread_rwlock_wrlock(&pLock);
  Callback_t Callback;
  Callback.Func = function;
  Callback.APIFile = apiFile;
  pCallbacks.insert({handleName, Callback});
  pthread_rwlock_unlock(&pLock);
}
void HTTPServer::UnregisterCallback(const string &handleName) {
  pthread_rwlock_wrlock(&pLock);
  pCallbacks.erase(handleName);
  pthread_rwlock_unlock(&pLock);
}
void HTTPServer::RegisterDoc(const string &name, const json &content) {
  pthread_rwlock_wrlock(&pLock);
  pComponents.insert({name, content});
  pthread_rwlock_unlock(&pLock);
}
void HTTPServer::UnregisterDoc(const string &name) {
  pthread_rwlock_wrlock(&pLock);
  pComponents.erase(name);
  pthread_rwlock_unlock(&pLock);
}
string HTTPServer::MimeType(const string &file) {
  size_t FileNameLen = file.length();
  if((FileNameLen >= 3) && (file.compare(FileNameLen - 3, 3, ".js")) == 0) {
    return "text/javascript";
  } else if((FileNameLen >= 4) && (file.compare(FileNameLen - 4, 4, ".png"))  == 0) {
    return "image/png";
  } else if((FileNameLen >= 4) && (file.compare(FileNameLen - 4, 4, ".ico"))  == 0) {
    return "image/x-icon";
  } else if((FileNameLen >= 5) && (file.compare(FileNameLen - 5, 5, ".html")) == 0) {
    return "text/html";
  } else if((FileNameLen >= 5) && (file.compare(FileNameLen - 3, 3, ".md")) == 0) {
    return "text/markdown";
  }
  LogFile::Debug("No mimetype defined for request '%s'", file.c_str());
  return "text/plain";
}
string HTTPServer::PatchFile(const string &file, const string &find, const string &replace) {
  size_t Len;
  const char *Content = (const char *)ResourceFile::GetFile(file, &Len);
  string Tmp = string(Content, Len);
  return PatchString(Tmp, find, replace);
}
string HTTPServer::PatchString(string &src, const string &find, const string &replace) {
  size_t Len = src.find(find, 0);
  return (Len != string::npos) ? src.replace(Len, find.size(), replace) : src;
}
void HTTPServer::GetVersionCallback(HTTPRequest &req) {
  json Result = json::object();
  Result["Branch"]    = __BUILD_BRANCH__;
  Result["Arch"]      = __BUILD_ARCH__;
  Result["Build"]     = __BUILD_DATE__;
  Result["Functions"] = json::array();
  pthread_rwlock_rdlock(&pLock);
  for(auto &Callback : pCallbacks) Result["Functions"].push_back(Callback.first);
  pthread_rwlock_unlock(&pLock);
  req.Success(Result);
}
void HTTPServer::GetAPICallback(HTTPRequest &req) {
  json Result = json::parse(ResourceFile::GetFile("API/Base.json"));
  Result["servers"].push_back({{ "url", pRelativePath }});
  Result["info"]["version"] = __BUILD__;
  pthread_rwlock_rdlock(&pLock);
  for(auto &Callback : pCallbacks) {
    if(Callback.second.APIFile.empty() == false) Result.merge_patch(json::parse(ResourceFile::GetFile(Callback.second.APIFile)));
  }
  for(auto &Component : pComponents) Result.merge_patch(json::parse(ResourceFile::GetFile(Component.second)));
  pthread_rwlock_unlock(&pLock);
  req.Success(Result);
}
void HTTPServer::LogInCallback(HTTPRequest &req) {
  if(!req.CheckParamString("Password") || !req.CheckParamString("User") || !req.CheckParamBool("UpdateLastLogin")) return;
  Database DB;
  json     Response = json::object();
  string   User     = req.Param("User");
  string   Pass     = req.Param("Password");
  bool     Update   = req.Param("UpdateLastLogin");
  DB.ExecSentenceAsyncResult("SELECT lastlogin FROM users WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(Pass) + ")");
  if(DB.FecthRow()) {
    Response["LastLogin"] = DB.ResultUInt64("lastlogin");
    DB.ExecSentenceAsyncResult("INSERT INTO sessions(id, username, timestamp) VALUES (" + DB.Bind(Tools::GetUUID()) + "," + DB.Bind(User) + ", " + DB.Bind(Tools::GetTime64()) + ") RETURNING id");
    if(DB.FecthRow()) req.SetHeader("Set-Cookie", "SESSION=" + DB.ResultString("id"));
    if(Update) DB.ExecSentenceNoResult("UPDATE users SET lastlogin = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(User));
    return req.Success(Response);
  }
  return req.Error("", HTTP_UNAUTHORIZED);
}
void HTTPServer::LogOutCallback(HTTPRequest &req) {
  if(!req.Cookie().empty()) {
    Database DB;
    DB.ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + DB.Bind(req.Cookie()));
  }
  return req.Success(json::object());
}
void HTTPServer::HandleGet(const Request &request, Response &response) {
  string URL      = string(request.path).erase(0, pRelativePath.size());
  size_t Size     = 0;
  string UserName = "";
  LogFile::Debug("HTTP GET '%s'", URL.c_str());
  const char *Tmp = (const char*)ResourceFile::GetFile(string("WEB/") + URL, &Size);
  if(Size != 0) {
    response.set_header("Cache-Control", "public, max-age=604800, immutable");
    response.set_header("X-Uncompressed-Length", to_string(Size));
    auto Func = [Tmp, Size](uint64_t offset, DataSink &sink) -> bool {
      size_t      Pending = Size;
      const char *SendPtr = Tmp;
      while((Pending > 0) && (sink.is_writable() == true)) {
        size_t PkgSize = (Pending > 4096) ? 4096 : Pending;
        sink.write(SendPtr, PkgSize);
        Pending = Pending - PkgSize;
        SendPtr = &SendPtr[PkgSize];
      }
      sink.done();
      return true;
    };
    return response.set_chunked_content_provider(MimeType(URL).c_str(), Func);
  }
  if(request.has_header("Cookie")) {
    string Cookie = "";
    string Header = request.get_header_value("Cookie");
    auto Begin = Header.find("SESSION=");
    if(Begin != string::npos) Cookie = Header.substr(Begin + 8, Header.find(';', Begin) - Begin - 8);
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) UserName = DB.ResultString("username");
    }
  }
  if(URL == "Login") {
    if(UserName != "") return response.set_redirect(pRelativePath.c_str());
    response.set_content(PatchFile("WEB/Login.html", "{{ RELATIVE_PATH }}", pRelativePath), "text/html");
    response.status = HTTP_OK;
    return;
  } else if(URL == "MainLogo.png") {
    response.set_content(pLogoImg, "image/png");
    response.status = HTTP_OK;
    return;
  }
  if(UserName == "") return response.set_redirect(string(pRelativePath + "Login").c_str());
  string File = PatchFile("WEB/Index.html", "{{ VERSION }}", __BUILD__);
  response.set_content(PatchString(File, "{{ RELATIVE_PATH }}", pRelativePath), "text/html");
  response.status = HTTP_OK;
  return;
}
void HTTPServer::HandlePost(const Request &request, Response &response) {
  string       URL = string(request.path).erase(0, pRelativePath.size());
  HTTPRequest  Req(request, &response);
  if(Req.Status() != HTTP_NOT_FOUND) return;
  LogFile::Debug("HTTP POST '%s'", URL.c_str());
  if(URL == "logIn") return LogInCallback(Req);
  if(Req.UserName().empty()) return Req.Error("", HTTP_UNAUTHORIZED);
  if(URL == "logOut") return LogOutCallback(Req);
  pthread_rwlock_rdlock(&pLock);
  auto Callback = pCallbacks.find(URL);
  if(Callback != pCallbacks.end()) {
    Callback->second.Func(Req);
    pthread_rwlock_unlock(&pLock);
    return;
  }
  pthread_rwlock_unlock(&pLock);
}
//----------------------------------------------------------------------------------------------------------------------