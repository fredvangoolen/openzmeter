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
#include "NetworkManager.h"
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon                     *HTTPServer::pDaemon = NULL;
string                          HTTPServer::pDefaultPage;
vector<HTTPServer::Callback_t>  HTTPServer::pCallbacks;
vector<HTTPServer::Mapping_t>   HTTPServer::pMappings;
//----------------------------------------------------------------------------------------------------------------------
HTTPRequest::HTTPRequest(const char *cookie, bool deflate) : Database(NULL) {
  if(deflate == true) {
    memset(&pDeflateObj, 0, sizeof(pDeflateObj));
    pDeflate = (deflateInit(&pDeflateObj, Z_BEST_SPEED) == Z_OK);
  }
  pResultCode = MHD_HTTP_OK;
  if(cookie != NULL) {
    PrepareSentence("SELECT id, username FROM sessions WHERE id = ?");
    BindSingleString(cookie);
    ExecSentence();
    if(ResultRows() == 1) {
      pUserName = (ResultValue(0, "username") != NULL) ? ResultValue(0, "username") : "";
      pSessionID = ResultValue(0, "id");
      return;
    }
  }
  PrepareSentence("INSERT INTO sessions(username, timestamp) VALUES (NULL, ?) RETURNING id");
  BindSingleUInt64(Tools::GetTime());
  ExecSentence();
  if(ResultRows() == 1) pSessionID = ResultValue(0, (char*)"id");
}
HTTPRequest::~HTTPRequest() {
  if(pDeflate) deflateEnd(&pDeflateObj);
}
void HTTPRequest::FromFile(string name) {
#ifdef OZM_DEBUG
  string Path = "Resources/" + name;
  std::ifstream File(Path, ios::binary);
  pResultCode = MHD_HTTP_NOT_FOUND;
  if(File) {
    File.seekg (0, File.end);
    int Size = File.tellg();
    File.seekg (0, File.beg);
    if(Size > 0) { 
      pPostData = string(Size, 0);
      File.seekg(0, ios::beg);
      File.read(&pPostData[0], pPostData.size());
      pResultCode = MHD_HTTP_OK;
    }
  }
#else
  pPostData = (char*)Tools::GetFile(name, &pPostDataLen);
#endif
  size_t FileNameLen = name.length();
  if((FileNameLen >= 3) && (name.compare(FileNameLen - 3, 3, ".js")) == 0)        pMimeType = "text/javascript";
  else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".css"))  == 0) pMimeType = "text/css";
  else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".ico"))  == 0) pMimeType = "image/x-icon";
  else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".png"))  == 0) pMimeType = "image/png";
  else if((FileNameLen >= 4) && (name.compare(FileNameLen - 4, 4, ".svg"))  == 0) pMimeType = "image/svg+xml";
  else if((FileNameLen >= 5) && (name.compare(FileNameLen - 5, 5, ".html")) == 0) pMimeType = "text/html";
//  else if((FileNameLen >= 5) && (strcmp(&url[FileNameLen - 6], ".woff2")) == 0) 
//    MHD_add_response_header(response, "Content-Type", "font/woff2");
  else {
    pMimeType = "application/octet-stream";
    Tools::LogMsg("DEBUG", "No mimetype defined for request '%s'\n", name);
  }
}
void HTTPRequest::FromString(string obj, int resultCode) {
  pPostData = obj;
  pResultCode = resultCode;
  pMimeType = "text/plain";
}
void HTTPRequest::FromJSON(json obj) {
  pPostData = obj.dump();
  pResultCode = MHD_HTTP_OK;
  pMimeType = "application/json";
}
string HTTPRequest::SessionID() {
  return pSessionID;
}
string HTTPRequest::UserName() {
  return pUserName;
}
bool HTTPRequest::SetParams(string input, bool local) {
  pJSONParams = json::parse(input, NULL, false);
  if(!pJSONParams.is_discarded()) {
    if(local && pJSONParams["UserName"].is_string()) {
     pUserName = pJSONParams["UserName"];
     pJSONParams.erase("UserName");
    }
    return true;
  }
  return false;
}
json HTTPRequest::GetParams() {
  return pJSONParams;
}
bool HTTPRequest::Deflate() {
  return pDeflate;
}
void HTTPRequest::CloseSession() {
  PrepareSentence("DELETE FROM sessions WHERE id = ?");
  BindSingleString(pSessionID.c_str());
  ExecSentence();
}
int  HTTPRequest::ResultCode() {
  return pResultCode;
}
string HTTPRequest::MimeType() {
  return pMimeType;
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
bool HTTPServer::Config(json_object *config) {
  Database DB(NULL);
  uint16_t Port = 8080;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) Port = json_object_get_int(JSON_Val);
  }
  pDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, Port, NULL, NULL, 
                             &HandleClient, NULL,
                             MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
                             MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
                             MHD_OPTION_END);
  if(pDaemon == NULL) return false;
  return true;
}
void HTTPServer::Stop() {
  if(pDaemon) MHD_stop_daemon(pDaemon);
}
void HTTPServer::RegisterDefault(string def) {
  pDefaultPage = def;
}
void HTTPServer::RegisterCallback(string handleName, HTTPServer::Func_t function) {
  Callback_t Item = {handleName, function};
  pCallbacks.push_back(Item);
}
void HTTPServer::RegisterMapping(string url, string file) {
  Mapping_t Item = {url, file};
  pMappings.push_back(Item);
}
void HTTPServer::RemoveExpiredSessions() {
  Database DB(NULL);
  DB.PrepareSentence("DELETE FROM sessions WHERE (timestamp < ? AND username IS NOT NULL) OR (timestamp < ? AND username IS NULL)");
  DB.BindSingleUInt64(Tools::GetTime() - 1296000000); //15 days
  DB.BindSingleUInt64(Tools::GetTime() - 60000);      //1 minute
  DB.ExecSentence();
}
int HTTPServer::SendRedirect(struct MHD_Connection *connection, string url) {
  Tools::LogMsg("DEBUG", "Redirecting client to '%s'\n", url);
  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, "Location", url.c_str());
  int Return = MHD_queue_response(connection, MHD_HTTP_SEE_OTHER, response);
  MHD_destroy_response(response);
  return Return;
}
void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  if(*con_cls != NULL) delete (HTTPRequest*)*con_cls;
}
int HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  HTTPRequest *Request = (HTTPRequest*)*ptr;
  if(Request == NULL) {
    Tools::LogMsg("DEBUG", "HTTP %s request '%s'\n", method, url);
    string FilePath;
    char *AcceptEncoding = (char*)MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING);
    char *Cookie         = (char*)MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");
    bool  Deflate        = ((AcceptEncoding != NULL) && (strstr(AcceptEncoding, "deflate") != NULL));
    Request = new HTTPRequest(Cookie, Deflate);
    *ptr = Request;
    if(Request->SessionID().empty()) return MHD_NO;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) return MHD_YES;
    if((strcmp(method, MHD_HTTP_METHOD_GET) == 0) || (strcmp(method, MHD_HTTP_METHOD_HEAD) == 0)) Request->FromFile(string("WEB") + url);
    if((Request->ResultCode() != MHD_HTTP_OK) && (strcmp(method, MHD_HTTP_METHOD_GET) == 0)) {
      if(((strcmp(url, "/Login") == 0) || (strcmp(url, "/") == 0)) && (!Request->UserName().empty())) {
        return SendRedirect(connection, pDefaultPage);
      } else if(strcmp(url, "/Logout") == 0) {
        Request->CloseSession();
        return SendRedirect(connection, "/Login");
      } else if((strcmp(url, "/Login") == 0) && (Request->UserName().empty())) {
        Request->FromFile("WEB/Login.html");
      } else if(Request->UserName().empty()) {
        return SendRedirect(connection, "/Login");
      } else {
        for(vector<Mapping_t>::iterator it = pMappings.begin() ; it != pMappings.end(); ++it) {
          if(it->URL.compare(url) == 0) Request->FromFile("WEB" + it->File);
        }    
      }
    }
    MHD_Response *Response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN, 8192, &Request->WriteFunction, Request, NULL);
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CACHE_CONTROL, "public, max-age=2592000");
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_TYPE, Request->MimeType().c_str());
    MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_ENCODING, Request->Deflate() ? "deflate" : "");
    int Return = MHD_queue_response(connection, Request->ResultCode(), Response);
    MHD_destroy_response(Response);
    return Return;
  }
  if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
    if(*upload_data_size != 0) {
      struct sockaddr *Addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
      if(Request->SetParams(string(upload_data, *upload_data_size), (((sockaddr_in*)Addr)->sin_addr.s_addr == 0x0100007F)) == false) {
        Request->FromString("No valid JSON request", MHD_HTTP_BAD_REQUEST);
      } else {
        if(strcmp(url, "/LogIn") == 0) {
          json Params = Request->GetParams();
          bool UpdateLastLogin = true;
          if(Params["UpdateLastLogin"].is_boolean()) UpdateLastLogin = Params["UpdateLastLogin"];
          if((Params["Password"].is_string() == false) || (Params["Username"].is_string() == false)) {
            Request->FromString("'Password' and 'Username' params requiered", MHD_HTTP_BAD_REQUEST);
          } else {
            Request->PrepareSentence("SELECT lastlogin FROM users WHERE username = ? AND password = ?");
            Request->BindSingleString(Params["Username"].get<string>().c_str());
            Request->BindSingleString(Params["Password"].get<string>().c_str());
            Request->ExecSentence();
            json Response;
            if(Request->ResultRows() != 1) {
              Response["Result"] = false;
            } else {
              Response["Result"] = true;
              Response["LastLogin"] = atoll(Request->ResultValue(0, "lastlogin"));
              if(UpdateLastLogin) {
                Request->PrepareSentence("UPDATE users SET lastlogin = ? WHERE username = ?");
                Request->BindSingleUInt64(Tools::GetTime());
                Request->BindSingleString(Params["Username"].get<string>().c_str());
                Request->ExecSentence();
              }
              Request->PrepareSentence("UPDATE sessions SET username = ? WHERE id = ?");
              Request->BindSingleString(Params["Username"].get<string>().c_str());
              Request->BindSingleString(Request->SessionID().c_str());
              Request->ExecSentence();
            }
            Request->FromJSON(Response);
          }  
        } else {
          if(Request->UserName().empty()) {
            Request->FromString("Unauthorided access to API", MHD_HTTP_UNAUTHORIZED);
          } else {
            bool Handled = false;
            for(vector<Callback_t>::iterator it = pCallbacks.begin() ; it != pCallbacks.end(); ++it) {
              if(it->HandleName.compare(url) == 0) {
                it->Function(*Request);
                Handled = true;
                break;
              }
            }
            if(Handled == false) Request->FromString("Method not found", MHD_HTTP_METHOD_NOT_ALLOWED);
          }
        }
      }
	    *upload_data_size = 0;
  	  return MHD_YES;
	  } else {
      MHD_Response *Response = MHD_create_response_from_callback(MHD_SIZE_UNKNOWN, 8192, &Request->WriteFunction, Request, NULL);
      if(strcmp(url, "/LogIn") == 0) {
        string Cookie = "SESSION=" + Request->SessionID() +"; httponly; Path=/";
        MHD_add_response_header(Response, MHD_HTTP_HEADER_SET_COOKIE, Cookie.c_str());
      }
      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_TYPE, Request->MimeType().c_str());
      MHD_add_response_header(Response, MHD_HTTP_HEADER_CONTENT_ENCODING, Request->Deflate() ? "deflate" : "");
      int Return = MHD_queue_response(connection, Request->ResultCode(), Response);
      MHD_destroy_response(Response);        
      return Return;
    }
  }
  return MHD_NO;
}
//void HTTPServer::HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr) {



//        char Cookie[256];
//        sprintf(Cookie, "SESSION=%s; httponly; Path=/", SessionID);
//return SendFile(connection, (char*)"Login.html", MHD_HTTP_HEADER_SET_COOKIE, Cookie);
//if((aditionalheadertype != NULL) && (aditionalheadervalue != NULL )) MHD_add_response_header(response, aditionalheadertype, aditionalheadervalue);



//  Request_t *Request = (Request_t*)*ptr;
  


//  } else if(strcmp(url, "/setSysInfo") == 0) {
//    char        *Type   = NULL;
//    json_object *Params = NULL;
//    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Type") == 0)) 
//        Type = (char*)json_object_get_string(JSON_Val);
//      else if((JSON_Type == json_type_object) && (strcmp(JSON_Key, "Params") == 0))
//        Params = JSON_Val;
//      else
//        return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON params");
//    }
//    if((Type == NULL) || (Params == NULL)) return SendError(connection, MHD_HTTP_BAD_REQUEST, "Requiered param not specified");
//    if(strcmp(Type, "Connectivity") == 0) {
//      Request->JSONWriter = json_object_new_object();
//      json_object_object_add(Request->JSONWriter, "result", json_object_new_boolean(NetworkManager::SetJSONNetworkConfig(Params)));
//      Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostDataLen);
//    }
//    return MHD_YES;
//  } else if(strcmp(url, "/getEvents") == 0) {
//  } else if(strcmp(url, "/getEventSamples") == 0) {
//    uint32_t Node = 0;
//    uint64_t Starttime = 0;
//    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
//        Node = json_object_get_int(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Starttime") == 0))
//        Starttime = json_object_get_int64(JSON_Val);
//      else
//        return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON params");
//    }
////    if((Error == true) || (Node < 0) || (Node >= Tools::Devices.Len)) goto InvalidParams;
//    char QueryText[8192];
//    int  QueryLen = 0;
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
////    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%u AS \"Envelope\", ", (uint32_t)(EVENT_ENVELOPETIME * Tools::Devices.Items[Node].Instance->SampleRate()));
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "duration AS \"Duration\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "samples AS \"Samples\" ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM events WHERE node = %u AND Starttime = %" PRIu64 ") AS Result", Node, Starttime);
//    Request->DB = new Database();
//    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostDataLen);
//    return MHD_YES;
//  }
//  Request->ResultCode = MHD_HTTP_BAD_REQUEST;
//  Request->PostData = (char*)"Invalid command";
//  Request->PostDataLen = strlen(Request->PostData);
//  return MHD_YES;
////InvalidParams:
////  Request->ResultCode = MHD_HTTP_BAD_REQUEST;
////  Request->PostData = (char*)"Invalid JSON params";
////  Request->PostResponseLen = strlen(Request->PostData);
////  return MHD_YES;  
//}
