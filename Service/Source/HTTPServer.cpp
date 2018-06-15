/* ================================== zMeter ===========================================================================
 * File:     HTTPServer.cpp
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ------------------------------------------------------------------------------------------------------------------ */
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <zlib.h>
#include "HTTPServer.h"
#include "Tools.h"
#include "Analyzer.h"
#include "NetworkManager.h"
#include "Capture.h"
#include "DeviceInterface.h"
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon             *HTTPServer::pDaemon         = NULL;
HTTPServer::Session_t  *HTTPServer::pSessions       = NULL;
HTTPServer::Callback_t *HTTPServer::pCallbacks      = NULL;
uint32_t                HTTPServer::pCallbacksCount = 0;
pthread_mutex_t         HTTPServer::pSessionMutex   = PTHREAD_MUTEX_INITIALIZER;
char                   *HTTPServer::pFilePath       = (char*)"/usr/share/openZmeter/WEB/";
char                   *HTTPServer::pPassword       = (char*)"admin";
uint64_t                HTTPServer::pLastLogin      = 0;
//----------------------------------------------------------------------------------------------------------------------
void HTTPServer::Config(json_object *config) {
  uint16_t Port = 8080;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastLogin") == 0)) {
      pLastLogin = json_object_get_int64(JSON_Val);
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) {
      Port = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "FilePath") == 0)) {
      pFilePath = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) {
      pPassword = (char*)json_object_get_string(JSON_Val);
    }
  }
  pDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, Port, NULL, NULL, 
                             &HandleClient, NULL,
                             MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
                             MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
                             MHD_OPTION_END);
  if(pDaemon == NULL) Tools::LogMsg("ERROR", "Failed to start WEB server\n");
}
void HTTPServer::Stop() {
  if(pDaemon) MHD_stop_daemon(pDaemon);
}

void HTTPServer::RegisterCallback(char* handleName, HTTPServer::Func_t function) {
  pCallbacks = (Callback_t*)realloc(pCallbacks, sizeof(Callback_t) * (pCallbacksCount + 1));
  pCallbacks[pCallbacksCount].HandlName = handleName;
  pCallbacks[pCallbacksCount].Function  = function;
  pCallbacksCount++;
}
HTTPServer::Session_t *HTTPServer::GetSession(MHD_Connection* connection) {
  const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");
  Session_t *Session = NULL;
  pthread_mutex_lock(&pSessionMutex);
  if(Cookie != NULL) {
    Session         = pSessions;
    while(Session != NULL) {
      if(strcmp(Cookie, Session->ID) == 0) break;
      Session = Session->Next;
    }
  } 
  if(Session == NULL) {
    Session = (Session_t*)calloc(1, sizeof(Session_t));
    if(Session == NULL) {
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      return NULL; 
    }
    sprintf(Session->ID, "%08X%08X%08X%08X", (uint32_t)random(), (uint32_t)random(), (uint32_t)random(), (uint32_t)random());
    Tools::LogMsg("DEBUG", "New HTTP client with ID %s\n", Session->ID);
    Session->Logged = false;
    Session->Next = pSessions;
    pSessions = Session;
  }
  Session->LastSeen = time(NULL);
  Session->References++;
  pthread_mutex_unlock(&pSessionMutex);
  return Session;
}
void HTTPServer::RemoveExpiredSessions() {
  pthread_mutex_lock(&pSessionMutex);
  Session_t *Session = pSessions;
  Session_t *Prev = NULL;
  uint32_t Time = time(NULL);
  while(Session != NULL) {
    if((Session->LastSeen < (Time - 86400)) && (Session->References != 0)) {
      Tools::LogMsg("DEBUG", "Remove expired session %s\n", Session->ID);
      Session_t *Tmp = Session;
      if(Prev == NULL) {
        pSessions = Session->Next;
      } else {
        Prev->Next = Session->Next;
      }
      Session = Session->Next;
      free(Tmp);
    } else {
      Prev = Session;
      Session = Session->Next;
    }
  }
  pthread_mutex_unlock(&pSessionMutex);
}
void HTTPServer::SetMimeType(struct MHD_Response *response, const char *url) {
  char *MimeType = NULL;
  int FileNameLen = strlen(url);
  if((strlen(url) >= 3) && (strcmp(&url[FileNameLen - 3], ".js")) == 0) 
    MimeType = (char*)"text/javascript";
  else if((strlen(url) >= 4) && (strcmp(&url[FileNameLen - 4], ".css")) == 0) 
    MimeType = (char*)"text/css";
  else if((strlen(url) >= 4) && (strcmp(&url[FileNameLen - 4], ".ico")) == 0) 
    MimeType = (char*)"image/x-icon";
  else if((strlen(url) >= 4) && (strcmp(&url[FileNameLen - 4], ".png")) == 0) 
    MimeType = (char*)"image/png";
  else if((strlen(url) >= 4) && (strcmp(&url[FileNameLen - 4], ".svg")) == 0) 
    MimeType = (char*)"image/svg+xml";
  else if((strlen(url) >= 5) && (strcmp(&url[FileNameLen - 5], ".html")) == 0) 
    MimeType = (char*)"text/html";
  else if((strlen(url) >= 5) && (strcmp(&url[FileNameLen - 6], ".woff2")) == 0) 
    MimeType = (char*)"font/woff2";
  else {
    MimeType = (char*)"application/octet-stream";
    Tools::LogMsg("DEBUG", "No mimetype defined for request '%s', '%s' used\n", url, MimeType);
  }
  MHD_add_response_header(response, "Content-Type", MimeType);
}
int  HTTPServer::SendError(struct MHD_Connection *connection, int error, const char *text) {
  int Len = strlen(text);
  if(Len == 0)
    Tools::LogMsg("DEBUG", "Error %d on request\n", error);
  else
    Tools::LogMsg("DEBUG", "Error %d (%s) on request\n", error, text);
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(text), (void*)text, MHD_RESPMEM_PERSISTENT);
  int Return = MHD_queue_response(connection, error, response);
  MHD_destroy_response(response);
  return Return;
}
int  HTTPServer::SendRedirect(struct MHD_Connection *connection, const char *url) {
  Tools::LogMsg("DEBUG", "Redirecting client to '%s'\n", url);
  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, "Location", url);
  int Return = MHD_queue_response(connection, MHD_HTTP_SEE_OTHER, response);
  MHD_destroy_response(response);
  return Return;
}
int  HTTPServer::SendFile(struct MHD_Connection *connection, const char *path, const char *aditionalheadertype, const char *aditionalheadervalue) {
  int File = -1;
  struct stat FileStat;
  char *Path = (char*)malloc(sizeof(char) * (strlen(path) + strlen(pFilePath) + 1));
  sprintf(Path, "%s%s", pFilePath, path);
  if((stat(Path, &FileStat) == 0) && (S_ISREG(FileStat.st_mode))) File = open(Path, O_RDONLY);
  if(File == -1) return MHD_NO;
  struct MHD_Response *response = MHD_create_response_from_fd(FileStat.st_size, File);
  SetMimeType(response, path);
  if((aditionalheadertype != NULL) && (aditionalheadervalue != NULL )) MHD_add_response_header(response, aditionalheadertype, aditionalheadervalue);
  MHD_add_response_header(response, "Cache-Control", "public, max-age=2592000");
  int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  free(Path);
  return Return;
}
void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  Request_t *Request = (Request_t*)*con_cls;
  if(Request == NULL) return;
  if(Request->DB) delete Request->DB;
  if(Request->JSONReader) json_object_put(Request->JSONReader);
  if(Request->JSONWriter) json_object_put(Request->JSONWriter);
  if(Request->Deflate) free(Request->PostData);
  if(Request->URL) {
    uint64_t EndTime = Tools::GetTime();
    Tools::LogMsg("DEBUG", "POST request '%s' takes %" PRIu64 "mS (%.2fkB send in %" PRIu64 "mS)\n", Request->URL, Request->ProcTime - Request->StartTime, Request->PostDataLen / 1024.0, EndTime - Request->StartTime);
    free(Request->URL);
  }
  Request->Session->References++;
  free(Request);
}
int  HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  if(Request == NULL) {
    Tools::LogMsg("DEBUG", "Client %s request '%s'\n", method, url);
    if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
      char *FilePath = (char*)malloc(strlen(url) + 10);
      sprintf(FilePath, "Resources%s", url);
      int Return = SendFile(connection, FilePath);
      free(FilePath);
      if(Return == MHD_YES) return MHD_YES;
    }
    *ptr = (Request_t*)calloc(1, sizeof(Request_t));
    Request = (Request_t*)*ptr;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
      Request->URL = strdup(url);
      Request->StartTime = Tools::GetTime();
    }
    Request->Session = GetSession(connection);
    Request->ResultCode = MHD_HTTP_OK;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) return MHD_YES;
  }
  if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
    if(*upload_data_size != 0) {
      HandlePostRequest(connection, url, upload_data, upload_data_size, ptr);
	    *upload_data_size = 0;
  	  return MHD_YES;
	  } else {
      char *AcceptEncoding = (char*)MHD_lookup_connection_value(connection, MHD_HEADER_KIND, MHD_HTTP_HEADER_ACCEPT_ENCODING);
      Request->Deflate = ((AcceptEncoding != NULL) && (strstr(AcceptEncoding, "deflate") != NULL) && (Request->PostDataLen > 256));
      if(Request->Deflate) {
        char *Tmp = (char*)malloc(Request->PostDataLen);
        z_stream Deflate;
        Deflate.zalloc = Z_NULL;
        Deflate.zfree = Z_NULL;
        Deflate.opaque = Z_NULL;
        Deflate.avail_in = Request->PostDataLen;
        Deflate.next_in = (uint8_t*)Request->PostData;
        Deflate.avail_out = Request->PostDataLen;
        Deflate.next_out = (uint8_t*)Tmp;
        deflateInit(&Deflate, Z_BEST_SPEED);
        deflate(&Deflate, Z_FINISH);
        deflateEnd(&Deflate);
        Request->PostDataLen = Deflate.total_out;
        Request->PostData = Tmp;      
      }
      struct MHD_Response *response = MHD_create_response_from_buffer(Request->PostDataLen, Request->PostData, MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      if(Request->Deflate) MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
      int Return = MHD_queue_response(connection, Request->ResultCode, response);
      MHD_destroy_response(response);
      Request->ProcTime = Tools::GetTime();
      return Return;
    }
  } else if(strcmp (method, MHD_HTTP_METHOD_GET) == 0) {
    if(strcmp(url, "/Login") == 0) {
      if(Request->Session->Logged == true) return SendRedirect(connection, "/Dashboard");
      char Cookie[256];
      sprintf(Cookie, "SESSION=%s; httponly; Path=/", Request->Session->ID);
      return SendFile(connection, (char*)"Login.html", MHD_HTTP_HEADER_SET_COOKIE, Cookie);
    }
    if(strncmp(url, "/MiniViews/", 11) == 0) {
      char *FilePath = (char*)malloc(strlen(url) + 6);
      sprintf(FilePath, "%s.html", &url[1]);
      int Return = SendFile(connection, FilePath);
      free(FilePath);
      if(Return == MHD_YES) return MHD_YES;
    }
    if(Request->Session->Logged == false) return SendRedirect(connection, "/Login");
    if((strcmp(url, "/Dashboard") == 0) || (strcmp(url, "/") == 0)) return SendFile(connection,  "Dashboard.html");
    if(strcmp(url, "/Analysis/Frequency") == 0) return SendFile(connection, "Analysis_Frequency.html");
    if(strcmp(url, "/Analysis/Voltage") == 0) return SendFile(connection, "Analysis_Voltage.html");
    if(strcmp(url, "/Analysis/Current") == 0) return SendFile(connection, "Analysis_Current.html");
    if(strcmp(url, "/Analysis/EnergyPower") == 0) return SendFile(connection, "Analysis_EnergyPower.html");
    if(strcmp(url, "/Events/ITIC") == 0) return SendFile(connection, "Events_ITIC.html");
    if(strcmp(url, "/Events/Disturbances") == 0) return SendFile(connection, "Events_Disturbances.html");
    if(strcmp(url, "/Events/EN50160") == 0) return SendFile(connection, "Events_EN50160.html");
    if(strcmp(url, "/Stats") == 0) return SendFile(connection, "Stats.html");
    if(strcmp(url, "/Settings") == 0) return SendFile(connection, "Settings.html");
    if(strcmp(url, "/Help") == 0) return SendFile(connection, "Help.html");
    if(strcmp(url, "/Logout") == 0) {
      Request->Session->Logged = false;
      return SendRedirect(connection, "/Login");
    }
  }
  return SendError(connection, MHD_HTTP_NOT_FOUND, "Page not found");
}
void HTTPServer::HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  Request->JSONReader = json_tokener_parse(upload_data);
  if((Request->JSONReader == NULL) || (json_object_get_type(Request->JSONReader) != json_type_object)) {
    Request->PostData  = (char*)"No valid JSON request";
    Request->PostDataLen = strlen(Request->PostData);
    Request->ResultCode = MHD_HTTP_BAD_REQUEST;
    return;
  }
  if(strcmp(url, "/LogIn") == 0) {
    char *Password = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) 
        Password = (char*)json_object_get_string(JSON_Val);
    }
    if(Password == NULL) {
      Request->PostData  = (char*)"'Password' param requiered";
      Request->PostDataLen = strlen(Request->PostData);
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      return;
    }
    Request->JSONWriter = json_object_new_object();
    if(strcmp(Password, pPassword) != 0) {
      json_object_object_add(Request->JSONWriter, "result", json_object_new_boolean(false));
    } else {
      Request->Session->Logged = true;
      json_object_object_add(Request->JSONWriter, "result", json_object_new_boolean(true));    
    }
    Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostDataLen);
    return;
  }
  if(Request->Session->Logged == false) {
    struct sockaddr *Addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    if(((sockaddr_in*)Addr)->sin_addr.s_addr != 0x0100007F) {
      Request->PostData  = (char*)"Unauthorided access to API";
      Request->PostDataLen = strlen(Request->PostData);
      Request->ResultCode = MHD_HTTP_UNAUTHORIZED;
      return;
    }
  }
  for(uint32_t i = 0; i < pCallbacksCount; i++) {
    if(strcmp(pCallbacks[i].HandlName, url) == 0) {
      if(pCallbacks[i].Function(&Request->DB, Request->JSONReader, &Request->JSONWriter, &Request->PostData, &Request->PostDataLen) == false) Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      return;
    }
  }
  Request->ResultCode = MHD_HTTP_METHOD_NOT_ALLOWED;


//  } else if(strcmp(url, "/getSysInfo") == 0) {
//    char *Type = NULL;
//    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Type") == 0)) 
//        Type = (char*)json_object_get_string(JSON_Val);
//      else
//        return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON params");
//    }
//    if(Type == NULL) return SendError(connection, MHD_HTTP_BAD_REQUEST, "Requiered param not specified");
//    if(strcmp(Type, "Performance") == 0) {
//      Request->JSONWriter = Tools::GetJSONSysInfo();
//      if(Request->JSONWriter == NULL) {
//        Request->ResultCode = MHD_HTTP_BAD_REQUEST;
//      } else {
//        Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostDataLen);
//      }
//    } else if(strcmp(Type, "Connectivity") == 0) {
//      Request->JSONWriter = NetworkManager::GetJSONNetworks();
//      if(Request->JSONWriter == NULL) {
//        Request->ResultCode = MHD_HTTP_BAD_REQUEST;
//      } else {
//        Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostDataLen);
//      }
//    }
//    return MHD_YES;
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
//    uint32_t Node = 0;
//    uint64_t LastEvent = 0, From = 0, To = 0, Period = 0;
//    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastEvent") == 0))
//        LastEvent = json_object_get_int64(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
//        Period = json_object_get_int64(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
//        To = json_object_get_int64(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
//        Node = json_object_get_int(JSON_Val);
//      else 
//        return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON params");
//    }
//    if(To == 0) To = Tools::GetTime();
//    From = To - Period;
//    char QueryText[8192];
//    int  QueryLen = 0;
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(starttime ORDER BY starttime ASC) AS \"Starttime\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(duration ORDER BY starttime ASC) AS \"Duration\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(type ORDER BY starttime ASC) AS \"Type\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(voltage ORDER BY starttime ASC) AS \"Voltage\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(percent ORDER BY starttime ASC) AS \"Percentage\" ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM events WHERE starttime > %" PRIu64 " AND starttime >= %" PRIu64 " AND starttime <= %" PRIu64 " AND node = %u) AS Result", LastEvent, From, To, Node);
//    Request->DB = new Database();
//    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostDataLen);
//    return MHD_YES;
//  } else if(strcmp(url, "/getSamplesRange") == 0) {
//    uint32_t Node = 0;
//    char *Aggreg = NULL;
//    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
//        Node = json_object_get_int(JSON_Val);
//      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
//        Aggreg = (char*)json_object_get_string(JSON_Val);
//      else
//        return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalid JSON params");
//    }
//    if(Aggreg == NULL) return SendError(connection, MHD_HTTP_BAD_REQUEST, "Requiered param not specified");
//    char QueryText[8192];
//    int  QueryLen = 0;
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "max(sampletime) AS \"To\", ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "min(sampletime) AS \"From\" ");
//    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM aggreg_%s WHERE node = %u) AS Result", Aggreg, Node);
//    Request->DB = new Database();
//    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostDataLen);
//    return MHD_YES;
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
}
