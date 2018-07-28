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
#include "NetworkManager.h"
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon             *HTTPServer::pDaemon         = NULL;
HTTPServer::Callback_t *HTTPServer::pCallbacks      = NULL;
uint32_t                HTTPServer::pCallbacksCount = 0;
HTTPServer::Mapping_t  *HTTPServer::pMappings       = NULL;
uint32_t                HTTPServer::pMappingssCount = 0;
char                   *HTTPServer::pFilePath       = (char*)"/usr/share/openZmeter/WEB/";
//----------------------------------------------------------------------------------------------------------------------
bool HTTPServer::Config(json_object *config) {
  Database DB(NULL);
  Tools::LogMsg("DEBUG", "Creating WEB service tables\n");
  DB.ExecSentence("CREATE TABLE IF NOT EXISTS users("
                  "  name      character varying NOT NULL,"
                  "  password  character varying NOT NULL,"
                  "  lastlogin bigint            NOT NULL,"
                  "  admin     boolean           NOT NULL,"
                  "CONSTRAINT users_pkey PRIMARY KEY (name)"
                  ") WITH (OIDS=FALSE)");
  DB.ExecSentence("CREATE TABLE IF NOT EXISTS sessions ("
                  "  id   serial NOT NULL,"
                  "  name character varying,"
                  "  timestamp bigint NOT NULL,"
                  "CONSTRAINT sessions_pkey PRIMARY KEY (id),"
                  "CONSTRAINT sessions_name_fkey FOREIGN KEY (name) REFERENCES users (name) MATCH SIMPLE ON UPDATE CASCADE ON DELETE CASCADE"
                  ") WITH (OIDS=FALSE)");
  DB.ExecSentence("CREATE INDEX IF NOT EXISTS sessions_timestamp_idx ON sessions USING btree (timestamp)");
  DB.ExecSentence("INSERT INTO users(name, password, lastlogin, admin) VALUES('admin', MD5('admin'), 0, true) ON CONFLICT (name) DO NOTHING");
  uint16_t Port = 8080;
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) {
      Port = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "FilePath") == 0)) {
      pFilePath = (char*)json_object_get_string(JSON_Val);
    }
  }
  pDaemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, Port, NULL, NULL, 
                             &HandleClient, NULL,
                             MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
                             MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
                             MHD_OPTION_END);
  if(pDaemon == NULL) {
    Tools::LogMsg("ERROR", "Failed to start WEB server\n");
    return false;
  }
  return true;
}
void HTTPServer::Stop() {
  if(pDaemon) MHD_stop_daemon(pDaemon);
}
void HTTPServer::RegisterCallback(char *handleName, HTTPServer::Func_t function) {
  pCallbacks = (Callback_t*)realloc(pCallbacks, sizeof(Callback_t) * (pCallbacksCount + 1));
  pCallbacks[pCallbacksCount].HandleName = handleName;
  pCallbacks[pCallbacksCount].Function  = function;
  pCallbacksCount++;
}
void HTTPServer::RegisterMapping(char *url, char *file) {
  pMappings = (Mapping_t*)realloc(pMappings, sizeof(Mapping_t) * (pMappingssCount + 1));
  pMappings[pMappingssCount].URL  = url;
  pMappings[pMappingssCount].File = file;
  pMappingssCount++;
}
char *HTTPServer::OpenSession(MHD_Connection* connection, Request_t* request) {
  if(request->DB == NULL) request->DB = new Database(NULL);
  const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");
  if(Cookie != NULL) {
    request->DB->PrepareSentence("SELECT id, name FROM sessions WHERE id = ?");
    request->DB->BindSingleString(Cookie);
    request->DB->ExecSentence();
    if(request->DB->ResultRows() == 1) {
      request->UserName = request->DB->ResultValue(0, (char*)"name");
      if(request->UserName) request->UserName = strdup(request->UserName);
      return request->DB->ResultValue(0, (char*)"id");
    }
  }
  request->DB->PrepareSentence("INSERT INTO sessions(name, timestamp) VALUES (NULL, ?) RETURNING id");
  request->DB->BindSingleUInt64(Tools::GetTime());
  request->DB->ExecSentence();
  if(request->DB->ResultRows() == 1) return request->DB->ResultValue(0, (char*)"id");
  return NULL;
}
void HTTPServer::CloseSession(MHD_Connection* connection, Request_t* request) {
  if(request->DB == NULL) request->DB = new Database(NULL);
  const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");  
  if(Cookie == NULL) return;
  request->DB->PrepareSentence("DELETE FROM sessions WHERE id = ?");
  request->DB->BindSingleString(Cookie);
  request->DB->ExecSentence();
}
void HTTPServer::RemoveExpiredSessions() {
  Database DB(NULL);
  DB.PrepareSentence("DELETE FROM sessions WHERE timestamp < ?");
  DB.BindSingleUInt64(Tools::GetTime() - 1296000000); //15 days
  DB.ExecSentence();
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
int HTTPServer::SendRedirect(struct MHD_Connection *connection, const char *url) {
  Tools::LogMsg("DEBUG", "Redirecting client to '%s'\n", url);
  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
  MHD_add_response_header(response, "Location", url);
  int Return = MHD_queue_response(connection, MHD_HTTP_SEE_OTHER, response);
  MHD_destroy_response(response);
  return Return;
}
int HTTPServer::SendFile(struct MHD_Connection *connection, const char *path, const char *aditionalheadertype, const char *aditionalheadervalue) {
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
  if(Request->UserName) free(Request->UserName);
  free(Request);
}
int HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    char *FilePath = (char*)malloc(strlen(url) + 10);
    sprintf(FilePath, "Resources%s", url);
    int Return = SendFile(connection, FilePath);
    free(FilePath);
    if(Return == MHD_YES) {
      Tools::LogMsg("DEBUG", "Client GET request '%s'\n", url);
      return MHD_YES;
    }
  }
  Request_t *Request = (Request_t*)*ptr;
  if(Request == NULL) {
    *ptr = (Request_t*)calloc(1, sizeof(Request_t));
    Request = (Request_t*)*ptr;
    char *SessionID = OpenSession(connection, Request);
    if(SessionID == NULL) return MHD_NO;
    Request->ResultCode = MHD_HTTP_OK;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
      return MHD_YES;
    } else if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
      if((strcmp(url, "/Login") == 0) && (Request->UserName != NULL)) {
        return SendRedirect(connection, "/");
      } else if((strcmp(url, "/Login") == 0) && (Request->UserName == NULL)) {
        char Cookie[256];
        sprintf(Cookie, "SESSION=%s; httponly; Path=/", SessionID);
        return SendFile(connection, (char*)"Login.html", MHD_HTTP_HEADER_SET_COOKIE, Cookie);
      } else if(Request->UserName == NULL) {
        return SendRedirect(connection, "/Login");
      } else {
        if(strcmp(url, "/Logout") == 0) {
          CloseSession(connection, Request);
          return SendRedirect(connection, "/Login");
        }
        for(uint32_t i = 0; i < pMappingssCount; i++) {
          if(strcmp(url, pMappings[i].URL) == 0) return SendFile(connection, pMappings[i].File);
        }
        return SendFile(connection, "Error404.html");
      }
    }
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
      return Return;
    }
  }
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
    char *Password = NULL, *Name = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) Password = (char*)json_object_get_string(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Username") == 0)) Name = (char*)json_object_get_string(JSON_Val);
    }
    if((Password == NULL) || (Name == NULL)) {
      Request->PostData  = (char*)"'Password' and 'Username' params requiered";
      Request->PostDataLen = strlen(Request->PostData);
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      return;
    }
    if(Request->DB == NULL) Request->DB = new Database(NULL);
    Request->DB->PrepareSentence("SELECT admin, lastlogin FROM users WHERE name = ? AND password = MD5(?)");
    Request->DB->BindSingleString(Name);
    Request->DB->BindSingleString(Password);
    Request->DB->ExecSentence();
    Request->JSONWriter = json_object_new_object();
    if(Request->DB->ResultRows() != 1) {
      json_object_object_add(Request->JSONWriter, "Result", json_object_new_boolean(false));
    } else {
      json_object_object_add(Request->JSONWriter, "Result", json_object_new_boolean(true));
      char *LastLogin = Request->DB->ResultValue(0, (char*)"lastlogin");
      if(LastLogin) json_object_object_add(Request->JSONWriter, "LastLogin", json_object_new_int64(atol(LastLogin)));
      char *Admin = Request->DB->ResultValue(0, (char*)"admin");
      if(Admin) json_object_object_add(Request->JSONWriter, "Admin", json_object_new_boolean((Admin[0] == 't') ? true : false));
      Request->DB->PrepareSentence("UPDATE users SET lastlogin = ? WHERE name = ?");
      Request->DB->BindSingleUInt64(Tools::GetTime());
      Request->DB->BindSingleString(Name);
      Request->DB->ExecSentence();
      const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "SESSION");  
      if(Cookie == NULL) return;
      Request->DB->PrepareSentence("UPDATE sessions SET name = ? WHERE id = ?");
      Request->DB->BindSingleString(Name);
      Request->DB->BindSingleString(Cookie);
      Request->DB->ExecSentence();
    }
    Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostDataLen);
    return;
  }
  if(Request->UserName == NULL) {
    struct sockaddr *Addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
    if(((sockaddr_in*)Addr)->sin_addr.s_addr != 0x0100007F) {
      Request->PostData  = (char*)"Unauthorided access to API";
      Request->PostDataLen = strlen(Request->PostData);
      Request->ResultCode = MHD_HTTP_UNAUTHORIZED;
      return;
    }
    Request->UserName = strdup("API");
  }
  for(uint32_t i = 0; i < pCallbacksCount; i++) {
    if(strcmp(pCallbacks[i].HandleName, url) == 0) {
      if(pCallbacks[i].Function(Request->UserName, Request->JSONReader, Request->DB, &Request->JSONWriter, &Request->PostData, &Request->PostDataLen) == false) Request->ResultCode = MHD_HTTP_BAD_REQUEST;
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
