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
HTTPRequest::HTTPRequest(string body, bool local) : Database() {
  pJSONParams = json::parse(body, NULL, false);
  pPostData = "";
  if(!pJSONParams.is_discarded()) {
    if(pJSONParams["UserName"].is_string()) {
     if(local) pUserName = pJSONParams["UserName"];
     pJSONParams.erase("UserName");
    }
  }
}

//HTTPRequest::HTTPRequest(char *cookie, bool deflate) : Database() {
//  pStartTime = Tools::GetTime();
//  pDeflate = deflate;
//  if(deflate == true) {
//    memset(&pDeflateObj, 0, sizeof(pDeflateObj));
//    pDeflate = (deflateInit(&pDeflateObj, Z_BEST_SPEED) == Z_OK);
//  }
//  pResultCode = HTTP_OK;
//  if(cookie != NULL) {
//    ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + Bind(string(cookie)));
//    if(FecthRow()) {
//      pUserName = ResultString("username");
//      pSessionID = ResultString("id");
//      return;
//    }
//  }
//  ExecSentenceAsyncResult("INSERT INTO sessions(username, timestamp) VALUES (NULL, " + Bind(Tools::GetTime()) + ") RETURNING id");
//  if(FecthRow()) pSessionID = ResultString("id");
//}
HTTPRequest::~HTTPRequest() {
//  if(pDeflate) deflateEnd(&pDeflateObj);
}
//uint32_t HTTPRequest::size() const {
//  return pPostData.size();
//}
//string HTTPRequest::Content() const {
//  return pPostData;
//}
void HTTPRequest::FromString(const string &obj, int resultCode) {
  pPostData = obj;
  pResultCode = resultCode;
  pMimeType = "text/plain";
}
void HTTPRequest::FromJSON(const json &obj) {
  pPostData = obj.dump();
  pResultCode = HTTP_OK;
  pMimeType = "application/json";
}
//string HTTPRequest::SessionID() const {
//  return pSessionID;
//}
string HTTPRequest::UserName() const {
  return pUserName;
}
//void HTTPRequest::AppendParams(const char *input, size_t* len) {
//  pPostData += string(input, *len);
//  *len = 0;
//  return;
//}
//bool HTTPRequest::ProcessParams(bool local) {
//  pJSONParams = json::parse(pPostData, NULL, false);
//  pPostData = "";
//  if(!pJSONParams.is_discarded()) {
//    if(pJSONParams["UserName"].is_string()) {
//     if(local) pUserName = pJSONParams["UserName"];
//     pJSONParams.erase("UserName");
//    }
//    return true;
//  }
//  return false;
//}
const json &HTTPRequest::GetParams() const {
  return pJSONParams;
}
//uint64_t HTTPRequest::GetTime() const {
//  return Tools::GetTime() - pStartTime;
//}
//bool HTTPRequest::Deflate() const {
//  return pDeflate;
//}
//void HTTPRequest::CloseSession() {
//  ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + Bind(pSessionID));
//  pUserName.clear();
//}
//int  HTTPRequest::ResultCode() const {
//  return pResultCode;
//}
//string HTTPRequest::MimeType() const {
//  return pMimeType;
//}
//void HTTPRequest::MimeType(const string &type) {
//  pMimeType = type;
//}
//ssize_t HTTPRequest::WriteFunction(void *cls, uint64_t pos, char *buf, size_t max) {
//  HTTPRequest *Request = (HTTPRequest*)cls;
//  if(Request->pDeflate) {
//    if(pos == 0) {
//      Request->pDeflateObj.avail_in = Request->pPostData.size();
//      Request->pDeflateObj.next_in = (uint8_t*)Request->pPostData.c_str();
//    }
//    Request->pDeflateObj.next_out = (uint8_t*)buf;
//    Request->pDeflateObj.avail_out = max;
//    deflate(&Request->pDeflateObj, Z_FINISH);
//    if(Request->pDeflateObj.avail_out != max) {
//      return (max - Request->pDeflateObj.avail_out);
//    } else {
//      return MHD_CONTENT_READER_END_OF_STREAM;
//    }
//  } else {
//    if(max > (Request->pPostData.size() - pos)) max = Request->pPostData.size() - pos;
//    if(max == 0) return MHD_CONTENT_READER_END_OF_STREAM;
//    memcpy(buf, &Request->pPostData[pos], max);
//    return max;
//  }
//return 0;
//}
//----------------------------------------------------------------------------------------------------------------------
HTTPCallback::HTTPCallback(const string& handle, Func_t function) : HandleName(handle), Function(function) {};
//----------------------------------------------------------------------------------------------------------------------
vector<HTTPCallback> __attribute__((init_priority(200))) HTTPServer::pCallbacks;
RWLocks                                                  HTTPServer::pLock = RWLocks();
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
  uint16_t Port = 8080;
  json Config = ConfigManager::ReadConfig("/HTTPServer");
//  RegisterCallback("getFunctions", GetFunctionsCallback);
  if(!Config.is_object()) Config = json::object();
  if(Config["Port"].is_number_unsigned()) Port = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  pServer = new Server();
  pServer->Get(".*", HandleGet);
  pServer->Post(".*", HandlePost);
//  pServer->Post("*.", HandlePost);
//  pDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, Port, NULL, NULL,
//                             &HandleClient, NULL,
//                             MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)60,
//                             MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
//                             MHD_OPTION_END);
  Config = json::object();
  Config["Port"] = Port;
  Config["RelativePath"] = pRelativePath;
  ConfigManager::WriteConfig("/HTTPServer", Config);
  if(pServer->listen("0.0.0.0", Port) == false) {
    LOGFile::Error("HTTP port (%hu) already in use\n", Port);
 } else {
    LOGFile::Info("HTTP server started on port %hu with relative path '%s'\n", Port, pRelativePath.c_str());
  }
}
HTTPServer::~HTTPServer() {
//  if(pDaemon) MHD_stop_daemon(pDaemon);
}
void HTTPServer::RegisterCallback(const string &handleName, const HTTPCallback::Func_t function) {
  pLock.WriteLock();
  for(HTTPCallback &Callback : pCallbacks) {
    if(Callback.HandleName != handleName) continue;
    Callback.Function = function;
    pLock.WriteUnlock();
    return;
  }
  pCallbacks.emplace_back(handleName, function);
  pLock.WriteUnlock();
}
void HTTPServer::UnregisterCallback(const string &handleName) {
  pLock.WriteLock();
  for(auto it = pCallbacks.begin(); it < pCallbacks.end(); it++) {
    if(it->HandleName != handleName) continue;
    pCallbacks.erase(it);
    break;
  }
  pLock.WriteUnlock();
}
void HTTPServer::HandleGet(const Request &request, Response &response) {
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP GET '%s'\n", URL.c_str());
  size_t Size = 0;
  char *Tmp = (char*)ResourceLoader::GetFile(string("WEB/") + URL, &Size);
  if(Size != 0) {
    size_t FileNameLen = URL.length();
    if((FileNameLen >= 3) && (URL.compare(FileNameLen - 3, 3, ".js")) == 0) {
      response.set_content(Tmp, Size, "text/javascript");
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".png"))  == 0) {
      response.set_content(Tmp, Size, "image/png");
    } else if((FileNameLen >= 4) && (URL.compare(FileNameLen - 4, 4, ".ico"))  == 0) {
      response.set_content(Tmp, Size, "image/x-icon");
    } else if((FileNameLen >= 5) && (URL.compare(FileNameLen - 5, 5, ".html")) == 0) {
      response.set_content(Tmp, Size, "text/html");
    } else {
      response.set_content(Tmp, Size, "application/octet-stream");
      LOGFile::Debug("No mimetype defined for request '%s'\n", URL.c_str());
    }
    response.status = 200;
    return;
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
      response.set_content(Login, "text/html");
      response.status = HTTP_OK;
    } else {
      response.set_redirect(pRelativePath.c_str());
    }
    return;
  }
  if(UserName == "") {
    response.set_redirect("Login");
    return;
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
    response.set_content(Index, "text/html");
    response.status = HTTP_OK;
    return;
  }
}
void HTTPServer::HandlePost(const Request &request, Response &response) {
  HTTPRequest Req(request.body, false);
  string URL = string(request.path).erase(0, pRelativePath.size());
  LOGFile::Debug("HTTP POST '%s'\n", URL.c_str());
  if(URL == "LogIn") {
    json Params = Req.GetParams();
    if((Params["Password"].is_string() == false) || (Params["Username"].is_string() == false)) {
      response.set_content("'Password' and 'Username' params requiered", "text/plain");
      response.status = MHD_HTTP_BAD_REQUEST;
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
      response.set_content(Response.dump(), "application/json");
      response.status = HTTP_OK;
    }
    return;
  }
  if(URL == "LogOut") {
    if(request.has_header("Cookie")) {
      string Cookie = request.get_header_value("Cookie").substr(8);
      if(Cookie != "") {
        Database DB;
        DB.ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + DB.Bind(Cookie));
      }
    }
    response.set_redirect(string(pRelativePath + "Login").c_str());
    return;
  }
  string UserName;
  if(request.has_header("Cookie")) {
    string Cookie = request.get_header_value("Cookie").substr(8);
    if(Cookie != "") {
      Database DB;
      DB.ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + DB.Bind(Cookie));
      if(DB.FecthRow()) UserName = DB.ResultString("username");
    }
  } else if(request.has_header("REMOTE_ADDR")) {
    if(request.get_header_value("REMOTE_ADDR") == "127.0.0.1") {
      UserName = "admin";
    }
  }
  if(UserName == "") {
    response.set_content("Unauthorided access to API", "text/plain");
    response.status = MHD_HTTP_UNAUTHORIZED;
    return;
  }
  Req.pUserName = UserName;
  pLock.ReadLock();
  for(HTTPCallback &Callback : pCallbacks) {
    if(Callback.HandleName != URL) continue;
    Callback.Function(Req);
    pLock.ReadUnlock();
    response.set_content(Req.pPostData, Req.pMimeType.c_str());
    response.status = Req.pResultCode;
    return;
  }
  pLock.ReadUnlock();
  response.set_content("Method not found", "text/plain");
  response.status = MHD_HTTP_METHOD_NOT_ALLOWED;
//        }
//      }
//      MHD_Response *Response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN, 8192, &Request->WriteFunction, Request, NULL);
//      if(URL == "LogIn") {

//      }
//      MHD_add_response_header(Response, "Process-Time", string(to_string(Request->GetTime()) + " ms").c_str());
//      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_TYPE, Request->MimeType().c_str());
//      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_ENCODING, Request->Deflate() ? "deflate" : "");
//      int Return = MHD_queue_response(connection, Request->ResultCode(), Response);
//      MHD_destroy_response(Response);
//      return Return;
//    }

//  return MHD_NO;

}
//void HTTPServer::GetFunctionsCallback(HTTPRequest &req) {
//  json Response = json::array();
//  pLock.ReadLock();
//  for(HTTPCallback &Callback : pCallbacks) Response.push_back(Callback.HandleName);
//  pLock.ReadUnlock();
//  req.FromJSON(Response);
//}
void HTTPServer::RemoveExpiredSessions(Database &db) const {
  if(Tools::GetTime() < 1296000000) return;
  db.ExecSentenceNoResult("DELETE FROM sessions WHERE (timestamp < " + db.Bind(Tools::GetTime() - 1296000000) + " AND username IS NOT NULL) OR (timestamp < " + db.Bind(Tools::GetTime() - 60000) + " AND username IS NULL)");
}
//int HTTPServer::SendRedirect(struct MHD_Connection *connection, const string &url) {
//  LOGFile::Debug("Redirecting client to '%s'\n", (pRelativePath + url).c_str());
//  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
//  MHD_add_response_header(response, "Location", (pRelativePath + url).c_str());
//  int Return = MHD_queue_response(connection, MHD_HTTP_SEE_OTHER, response);
//  MHD_destroy_response(response);
//  return Return;
//}
//void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
//  if(*con_cls != NULL) delete (HTTPRequest*)*con_cls;
//  pthread_setname_np(pthread_self(), "HTTP_Wait");
//}
//int HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
//  HTTPRequest *Request = (HTTPRequest*)*ptr;
//  string URL = string(url).erase(0, pRelativePath.size());
//  pthread_setname_np(pthread_self(), URL.c_str());
//  if(Request == NULL) {
//    LOGFile::Debug("HTTP %s request '%s'\n", method, URL.c_str());
//    string FilePath;
//    char *AcceptEncoding = (char*)MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING);
//    char *Cookie         = (char*)MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");
//    bool  Deflate        = ((AcceptEncoding != NULL) && (strstr(AcceptEncoding, "deflate") != NULL));
//    bool  EnableCache    = true;
//    Request = new HTTPRequest(Cookie, Deflate);
//    *ptr = Request;
//    if(Request->SessionID().empty()) return MHD_NO;
//  }
//}