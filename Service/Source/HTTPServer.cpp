/* ================================== zMeter ===========================================================================
 * File:     HTTPServer.cpp
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
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
HTTPRequest::HTTPRequest(char *cookie, bool deflate) : Database() {
  pStartTime = Tools::GetTime();
  pDeflate = deflate;
  if(deflate == true) {
    memset(&pDeflateObj, 0, sizeof(pDeflateObj));
    pDeflate = (deflateInit(&pDeflateObj, Z_BEST_SPEED) == Z_OK);
  }
  pResultCode = MHD_HTTP_OK;
  if(cookie != NULL) {
    ExecSentenceAsyncResult("SELECT id, username FROM sessions WHERE id = " + Bind(string(cookie)));
    if(FecthRow()) {
      pUserName = ResultString("username");
      pSessionID = ResultString("id");
      return;
    }
  }
  ExecSentenceAsyncResult("INSERT INTO sessions(username, timestamp) VALUES (NULL, " + Bind(Tools::GetTime()) + ") RETURNING id");
  if(FecthRow()) pSessionID = ResultString("id");
}
HTTPRequest::~HTTPRequest() {
  if(pDeflate) deflateEnd(&pDeflateObj);
}
uint32_t HTTPRequest::size() const {
  return pPostData.size();
}
string HTTPRequest::Content() const {
  return pPostData;
}
void HTTPRequest::FromFile(const string &name) {
  size_t Size = 0;
  char *Tmp = (char*)ResourceLoader::GetFile(name, &Size);
  pResultCode = (Size == 0) ? MHD_HTTP_NOT_FOUND : MHD_HTTP_OK;
  pPostData = string(Tmp, Size);
  if(pResultCode == MHD_HTTP_OK) {
    size_t FileNameLen = name.length();
    if((FileNameLen >= 3) && (name.compare(FileNameLen - 3, 3, ".js")) == 0)        pMimeType = "text/javascript";
    else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".css"))  == 0) pMimeType = "text/css";
    else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".ico"))  == 0) pMimeType = "image/x-icon";
    else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".png"))  == 0) pMimeType = "image/png";
    else if((FileNameLen >= 5) && (name.compare(FileNameLen - 5, 5, ".html")) == 0) pMimeType = "text/html";
    else {
      pMimeType = "application/octet-stream";
      LOGFile::Debug("No mimetype defined for request '%s'\n", name.c_str());
    }
  }
}
void HTTPRequest::FromString(const string &obj, int resultCode) {
  pPostData = obj;
  pResultCode = resultCode;
  pMimeType = "text/plain";
}
void HTTPRequest::FromJSON(const json &obj) {
  pPostData = obj.dump();
  pResultCode = MHD_HTTP_OK;
  pMimeType = "application/json";
}
string HTTPRequest::SessionID() const {
  return pSessionID;
}
string HTTPRequest::UserName() const {
  return pUserName;
}
void HTTPRequest::AppendParams(const char *input, size_t* len) {
  pPostData += string(input, *len);
  *len = 0;
  return;
}
bool HTTPRequest::ProcessParams(bool local) {
  pJSONParams = json::parse(pPostData, NULL, false);
  pPostData = "";
  if(!pJSONParams.is_discarded()) {
    if(pJSONParams["UserName"].is_string()) {
     if(local) pUserName = pJSONParams["UserName"];
     pJSONParams.erase("UserName");
    }
    return true;
  }
  return false;
}
const json &HTTPRequest::GetParams() const {
  return pJSONParams;
}
uint64_t HTTPRequest::GetTime() const {
  return Tools::GetTime() - pStartTime;
}
bool HTTPRequest::Deflate() const {
  return pDeflate;
}
void HTTPRequest::CloseSession() {
  ExecSentenceNoResult("DELETE FROM sessions WHERE id = " + Bind(pSessionID));
  pUserName.clear();
}
int  HTTPRequest::ResultCode() const {
  return pResultCode;
}
string HTTPRequest::MimeType() const {
  return pMimeType;
}
void HTTPRequest::MimeType(const string &type) {
  pMimeType = type;
}
ssize_t HTTPRequest::WriteFunction(void *cls, uint64_t pos, char *buf, size_t max) {
  HTTPRequest *Request = (HTTPRequest*)cls;
  if(Request->pDeflate) {
    if(pos == 0) {
      Request->pDeflateObj.avail_in = Request->pPostData.size();
      Request->pDeflateObj.next_in = (uint8_t*)Request->pPostData.c_str();
    }
    Request->pDeflateObj.next_out = (uint8_t*)buf;
    Request->pDeflateObj.avail_out = max;
    deflate(&Request->pDeflateObj, Z_FINISH);
    if(Request->pDeflateObj.avail_out != max) {
      return (max - Request->pDeflateObj.avail_out); 
    } else {
      return MHD_CONTENT_READER_END_OF_STREAM;
    }
  } else {
    if(max > (Request->pPostData.size() - pos)) max = Request->pPostData.size() - pos; 
    if(max == 0) return MHD_CONTENT_READER_END_OF_STREAM;
    memcpy(buf, &Request->pPostData[pos], max);
    return max;
  }
}
//----------------------------------------------------------------------------------------------------------------------
vector<HTTPServer::Callback_t> __attribute__((init_priority(200))) HTTPServer::pCallbacks;
RWLocks                        HTTPServer::pLock = RWLocks();
string                         HTTPServer::pRelativePath = "/";
//----------------------------------------------------------------------------------------------------------------------
HTTPServer::HTTPServer() {
  pDaemon = NULL;
  uint16_t Port = 8080;
  json Config = ConfigManager::ReadConfig("/HTTPServer");
  if(!Config.is_object()) Config = json::object();
  if(Config["Port"].is_number_unsigned()) Port = Config["Port"];
  if(Config["RelativePath"].is_string()) pRelativePath = Config["RelativePath"];
  pDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, Port, NULL, NULL, 
                             &HandleClient, NULL,
                             MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int)60,
                             MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
                             MHD_OPTION_END);
  Config = json::object();
  Config["Port"] = Port;
  Config["RelativePath"] = pRelativePath;
  ConfigManager::WriteConfig("/HTTPServer", Config); 
  if(pDaemon == NULL) {
    LOGFile::Error("HTTP port (%hu) already in use\n", Port);
  } else {
    LOGFile::Info("HTTP server started on port %hu with relative path '%s'\n", Port, pRelativePath.c_str());
  }
}
HTTPServer::~HTTPServer() {
  if(pDaemon) MHD_stop_daemon(pDaemon);
}
void HTTPServer::RegisterCallback(const string &handleName, const HTTPServer::Func_t function) {
  Callback_t Item = {handleName, function};
  pLock.WriteLock();
  for(uint n = 0; n < pCallbacks.size(); n++) {
    if(pCallbacks[n].HandleName != handleName) continue;
    pCallbacks[n] = Item;
    pLock.WriteUnlock();
    return;
  }
  pCallbacks.push_back(Item);
  pLock.WriteUnlock();
}
void HTTPServer::UnregisterCallback(const string &handleName) {
  pLock.WriteLock();
  for(vector<Callback_t>::iterator it = pCallbacks.begin(); it < pCallbacks.end(); it++) {
    if(it->HandleName != handleName) continue;
    pCallbacks.erase(it);
    break;
  }
  pLock.WriteUnlock();
}
void HTTPServer::RemoveExpiredSessions(Database &db) const {
  if(Tools::GetTime() < 1296000000) return;
  db.ExecSentenceNoResult("DELETE FROM sessions WHERE (timestamp < " + db.Bind(Tools::GetTime() - 1296000000) + " AND username IS NOT NULL) OR (timestamp < " + db.Bind(Tools::GetTime() - 60000) + " AND username IS NULL)");
}
int HTTPServer::SendRedirect(struct MHD_Connection *connection, const string &url) {
  LOGFile::Debug("Redirecting client to '%s'\n", (pRelativePath + url).c_str());
  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, "Location", (pRelativePath + url).c_str());
  int Return = MHD_queue_response(connection, MHD_HTTP_SEE_OTHER, response);
  MHD_destroy_response(response);
  return Return;
}
void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  if(*con_cls != NULL) delete (HTTPRequest*)*con_cls;
}
int HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  HTTPRequest *Request = (HTTPRequest*)*ptr;
  string URL = string(url).erase(0, pRelativePath.size());
  if(Request == NULL) {
    LOGFile::Debug("HTTP %s request '%s'\n", method, URL.c_str());
    string FilePath;
    char *AcceptEncoding = (char*)MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING);
    char *Cookie         = (char*)MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");
    bool  Deflate        = ((AcceptEncoding != NULL) && (strstr(AcceptEncoding, "deflate") != NULL));
    bool  EnableCache    = true;
    Request = new HTTPRequest(Cookie, Deflate);
    *ptr = Request;
    if(Request->SessionID().empty()) return MHD_NO;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) return MHD_YES;
    if((strcmp(method, MHD_HTTP_METHOD_GET) == 0) || (strcmp(method, MHD_HTTP_METHOD_HEAD) == 0)) Request->FromFile(string("WEB/") + URL.c_str());
    if((Request->ResultCode() != MHD_HTTP_OK) && (strcmp(method, MHD_HTTP_METHOD_GET) == 0)) {
      if(URL == "Logout") {
        Request->CloseSession();
        return SendRedirect(connection, "Login");
      } else if((URL == "Login") && (Request->UserName().empty())) {
        size_t Len;
        const char *Content = (const char *)ResourceLoader::GetFile("WEB/Login.html", &Len);
        string Login = string(Content, Len);
        string Find = "{{ RELATIVE_PATH }}";
        Len = Login.find(Find, 0);
        if(Len != string::npos) Login = Login.replace(Len, Find.size(), pRelativePath);
        Request->FromString(Login, MHD_HTTP_OK);
        Request->MimeType("text/html");
      } else if(Request->UserName().empty()) {
        return SendRedirect(connection, "Login");
      } else if((URL == "Login") && !Request->UserName().empty()) {
        return SendRedirect(connection, "");
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
        Request->FromString(Index, MHD_HTTP_OK);
        Request->MimeType("text/html");
        EnableCache = false;
      }
    }
    MHD_Response *Response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN, 8192, &Request->WriteFunction, Request, NULL);
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CACHE_CONTROL, EnableCache ? "public, max-age=2592000" : "no-cache");
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_TYPE, Request->MimeType().c_str());
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_ENCODING, Request->Deflate() ? "deflate" : "");
    int Return = MHD_queue_response(connection, Request->ResultCode(), Response);
    MHD_destroy_response(Response);
    return Return;
  }
  if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
    if(*upload_data_size != 0) {
      Request->AppendParams(upload_data, upload_data_size);
      return MHD_YES;
    }
    struct sockaddr *Addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    if(Request->ProcessParams((((sockaddr_in*)Addr)->sin_addr.s_addr == 0x0100007F)) == false) {
      Request->FromString("No valid JSON request", MHD_HTTP_BAD_REQUEST);
    } else {
      if(URL == "LogIn") {
        json Params = Request->GetParams();
        bool UpdateLastLogin = true;
        if(Params["UpdateLastLogin"].is_boolean()) UpdateLastLogin = Params["UpdateLastLogin"];
        if((Params["Password"].is_string() == false) || (Params["Username"].is_string() == false)) {
          Request->FromString("'Password' and 'Username' params requiered", MHD_HTTP_BAD_REQUEST);
        } else {
          string User = Params["Username"];
          string Pass = Params["Password"];
          Request->ExecSentenceAsyncResult("SELECT lastlogin FROM users WHERE username = " + Request->Bind(User) + " AND password = MD5(" + Request->Bind(Pass) + ")");
          json Response;
          if(Request->FecthRow()) {
            Response["Result"] = true;
            Response["LastLogin"] = Request->ResultUInt64("lastlogin");
            Request->ExecSentenceNoResult("UPDATE sessions SET username = " + Request->Bind(User) + " WHERE id = " + Request->Bind(Request->SessionID()) + "; " +
                       (UpdateLastLogin ? "UPDATE users SET lastlogin = " + Request->Bind(Tools::GetTime()) + " WHERE username = " + Request->Bind(User) : string("")));
          } else {
            Response["Result"] = false;
          }
          Request->FromJSON(Response);
        }  
      } else {
        if(Request->UserName().empty()) {
          Request->FromString("Unauthorided access to API", MHD_HTTP_UNAUTHORIZED);
        } else {
          bool Handled = false;
          pLock.ReadLock();
          for(vector<Callback_t>::iterator it = pCallbacks.begin() ; it != pCallbacks.end(); ++it) {
            if(it->HandleName != URL) continue;
            pLock.ReadUnlock();
            it->Function(*Request);
            Handled = true;
            break;
          }
          if(Handled == false) {
            pLock.ReadUnlock();
            Request->FromString("Method not found", MHD_HTTP_METHOD_NOT_ALLOWED);
          }
        }
      }
      MHD_Response *Response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN, 8192, &Request->WriteFunction, Request, NULL);
      if(URL == "LogIn") {
        string Cookie = "SESSION=" + Request->SessionID() + "; httponly; Path=" + pRelativePath;
        MHD_add_response_header(Response, MHD_HTTP_HEADER_SET_COOKIE, Cookie.c_str());
      }
      MHD_add_response_header(Response, "Process-Time", string(to_string(Request->GetTime()) + " ms").c_str());
      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_TYPE, Request->MimeType().c_str());
      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_ENCODING, Request->Deflate() ? "deflate" : "");
      int Return = MHD_queue_response(connection, Request->ResultCode(), Response);
      MHD_destroy_response(Response);        
      return Return;
    }
  }
  return MHD_NO;
}
