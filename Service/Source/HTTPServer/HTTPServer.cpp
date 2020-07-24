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
#include "ConfigManager.h"
#include "ResourceManager.h"
#include "HTTPTaskQueue.h"
//----------------------------------------------------------------------------------------------------------------------
map<string, HTTPServer::Func_t>  __attribute__((init_priority(200))) HTTPServer::pCallbacks;
RWLocks                                                              HTTPServer::pLock;
string                                                               HTTPServer::pRelativePath = "/";
uint16_t                                                              HTTPServer::pPort   = 8080;
pthread_t                        HTTPServer::pThread = 0;
//----------------------------------------------------------------------------------------------------------------------
HTTPServer::HTTPServer() {
  json Config = ConfigManager::ReadConfig("/HTTPServer");
  if(!Config.is_object()) Config = json::object();
  if(Config["Port"].is_number_unsigned()) pPort = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  Get(".*", HandleGet);
  Post(".*", HandlePost);
  set_keep_alive_max_count(UINT32_MAX);
  set_read_timeout(30);
//  set_compression_level(9);
  new_task_queue = [] { return new HTTPTaskQueue(); };
  Config = json::object();
  Config["Port"] = pPort;
  Config["RelativePath"] = pRelativePath;
  ConfigManager::WriteConfig("/HTTPServer", Config);
  if(pthread_create(&pThread, NULL, Listen_Thread, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pThread, "HTTPServer");
  }
}
HTTPServer::~HTTPServer() {
  LogFile::Debug("Stopping HTTPServer");
  stop();
  if(pThread != 0) pthread_join(pThread, NULL);
  LogFile::Debug("HTTPServer stopped");
}
void *HTTPServer::Listen_Thread(void *params) {
  HTTPServer *This = (HTTPServer*)params;
  nice(10);
  LogFile::Info("HTTP server starting on port %hu with relative path '%s'", This->pPort, This->pRelativePath.c_str());
  if(This->listen("0.0.0.0", This->pPort) == false) LogFile::Error("HTTP port (%hu) already in use", This->pPort);
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
const char *HTTPServer::MimeType(const string &file) {
  size_t FileNameLen = file.length();
  if((FileNameLen >= 3) && (file.compare(FileNameLen - 3, 3, ".js")) == 0) {
    return "text/javascript";
  } else if((FileNameLen >= 4) && (file.compare(FileNameLen - 4, 4, ".png"))  == 0) {
    return "image/png";
  } else if((FileNameLen >= 4) && (file.compare(FileNameLen - 4, 4, ".ico"))  == 0) {
    return "image/x-icon";
  } else if((FileNameLen >= 5) && (file.compare(FileNameLen - 5, 5, ".html")) == 0) {
    return "text/html";
  } else if((FileNameLen >= 5) && (file.compare(FileNameLen - 5, 5, ".json")) == 0) {
    return "application/json";
  } else if((FileNameLen >= 5) && (file.compare(FileNameLen - 3, 3, ".md")) == 0) {
    return "text/markdown";
  }
  LogFile::Debug("No mimetype defined for request '%s'", file.c_str());
  return "text/plain";
}
string HTTPServer::PatchFile(const string &file, const string &find, const string &replace) {
  size_t Len;
  const char *Content = (const char *)ResourceManager::GetFile(file, &Len);
  string Tmp = string(Content, Len);
  return PatchString(Tmp, find, replace);
}
string HTTPServer::PatchString(string &src, const string &find, const string &replace) {
  size_t Len = src.find(find, 0);
  return (Len != string::npos) ? src.replace(Len, find.size(), replace) : src;
}
void HTTPServer::LogIn(const json &params, Response &response) {
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return FillResponse(response, "'Password' param requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("UserName") == false) || (params["UserName"].is_string() == false)) return FillResponse(response, "'UserName' param requiered", "text/plain", HTTP_BAD_REQUEST);
  Database DB;
  json     Response        = json::object();
  string   User            = params["UserName"];
  string   Pass            = params["Password"];
  bool     UpdateLastLogin = true;
  DB.ExecSentenceAsyncResult("SELECT lastlogin FROM users WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(Pass) + ")");
  if(DB.FecthRow()) {
    Response["LastLogin"] = DB.ResultUInt64("lastlogin");
    if((params.contains("UpdateLastLogin") == true) && (params["UpdateLastLogin"].is_boolean() == true)) UpdateLastLogin = params["UpdateLastLogin"];
    DB.ExecSentenceAsyncResult("INSERT INTO sessions(id, username, timestamp) VALUES (" + DB.Bind(Tools::GetUUID()) + "," + DB.Bind(User) + ", " + DB.Bind(Tools::GetTime64()) + ") RETURNING id");
    if(DB.FecthRow()) response.set_header("Set-Cookie", "SESSION=" + DB.ResultString("id"));
    if(UpdateLastLogin) DB.ExecSentenceNoResult("UPDATE users SET lastlogin = " + DB.Bind(Tools::GetTime64()) + " WHERE username = " + DB.Bind(User));
    return FillResponse(response, Response.dump(), "application/json", HTTP_OK);
  } else {
    return FillResponse(response, "", "application/json", HTTP_UNAUTHORIZED);
  }
}
void HTTPServer::LogOut(const Request &request, Response &response) {
  if(request.has_header("Cookie")) {
    string Cookie = ParseCookie(request.get_header_value("Cookie"));
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + DB.Bind(Cookie));
    }
  }
  return FillResponse(response, "", "application/json", HTTP_OK);
}
void HTTPServer::HandleGet(const Request &request, Response &response) {
  string URL      = string(request.path).erase(0, pRelativePath.size());
  size_t Size     = 0;
  string UserName = "";
  LogFile::Debug("HTTP GET '%s'", URL.c_str());
  const char *Tmp = (const char*)ResourceManager::GetFile(string("WEB/") + URL, &Size);
  if(Size != 0) return FillResponse(response, Tmp, Size, MimeType(URL), HTTP_OK);
  if(request.has_header("Cookie")) {
    string Cookie = ParseCookie(request.get_header_value("Cookie"));
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) UserName = DB.ResultString("username");
    }
  }
  if(URL == "Login") {
    if(UserName != "") return response.set_redirect(pRelativePath.c_str());
    return FillResponse(response, PatchFile("WEB/Login.html", "{{ RELATIVE_PATH }}", pRelativePath), "text/html", HTTP_OK);
  }
  if(UserName == "") return response.set_redirect(string(pRelativePath + "Login").c_str());
  string File = PatchFile("WEB/Index.html", "{{ VERSION }}", __BUILD__);
  return FillResponse(response, PatchString(File, "{{ RELATIVE_PATH }}", pRelativePath), "text/html", HTTP_OK);
}
void HTTPServer::HandlePost(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LogFile::Debug("HTTP POST '%s'", URL.c_str());
  response.set_header("Connection", "Keep-Alive");
  json Params = json::parse(request.body, NULL, false);
  if(Params.is_discarded()) return FillResponse(response, "Malformed JSON params", "text/plain", HTTP_BAD_REQUEST);
  if(URL == "logIn") return LogIn(Params, response);
  if(URL == "logOut") return LogOut(request, response);
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
void HTTPServer::PeriodicTasks() {
  if(Tools::GetTime64() < 1296000000) return;
  Database DB;
  DB.ExecSentenceNoResult("DELETE FROM sessions WHERE timestamp < " + DB.Bind(Tools::GetTime64() - 1296000000));
}