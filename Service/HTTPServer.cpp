/* ================================== zMeter ==================================
 * File:     HTTPServer.cpp
 * Author:   Pedro Sanchez (3 de marzo de 2015)
 * Modified: Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
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
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon            *HTTPServer::Daemon = NULL;
HTTPServer::Session_t *HTTPServer::Sessions = NULL;
pthread_mutex_t        HTTPServer::SessionMutex = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
bool HTTPServer::Start() {
  Daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, Tools::Global.HTTPPort, NULL, NULL, 
                            &HandleClient, NULL,
                            MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 15,
                            MHD_OPTION_NOTIFY_COMPLETED, &HandleComplete, NULL,
                            MHD_OPTION_END);
  if(Daemon == NULL) {
    Tools::LogMsg("ERROR", "Failed to start WEB server\n");
  }
  return (Daemon != NULL);
}
HTTPServer::Session_t *HTTPServer::GetSession(MHD_Connection* connection) {
  const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "OZM_SESSION");
  Session_t *Session = NULL;
  pthread_mutex_lock(&SessionMutex);
  if(Cookie != NULL) {
    Session         = Sessions;
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
    Session->User = NULL;
    Session->Next = Sessions;
    Sessions = Session;
  }
  Session->LastSeen = time(NULL);
  Session->References++;
  pthread_mutex_unlock(&SessionMutex);
  return Session;
}
void HTTPServer::RemoveExpiredSessions() {
  pthread_mutex_lock(&SessionMutex);
  Session_t *Session = Sessions;
  Session_t *Prev = NULL;
  uint32_t Time = time(NULL);
  while(Session != NULL) {
    if((Session->LastSeen < (Time - 86400)) && (Session->References != 0)) {
      Tools::LogMsg("DEBUG", "Remove expired session %s\n", Session->ID);
      Session_t *Tmp = Session;
      if(Prev == NULL) {
        Sessions = Session->Next;
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
  pthread_mutex_unlock(&SessionMutex);
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
  else if((strlen(url) >= 5) && (strcmp(&url[FileNameLen - 5], ".html")) == 0) 
    MimeType = (char*)"text/html";
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
  MHD_destroy_response (response);
  return Return;
}
int  HTTPServer::SendFile(struct MHD_Connection *connection, const char *path, const char *aditionalheadertype, const char *aditionalheadervalue) {
  int File = -1;
  struct stat FileStat;
  if((stat(path, &FileStat) == 0) && (S_ISREG(FileStat.st_mode))) File = open(path, O_RDONLY);
  if(File == -1) return MHD_NO;
  struct MHD_Response *response = MHD_create_response_from_fd(FileStat.st_size, File);
  SetMimeType(response, path);
  if((aditionalheadertype != NULL) && (aditionalheadervalue != NULL )) MHD_add_response_header(response, aditionalheadertype, aditionalheadervalue);
  MHD_add_response_header(response, "Cache-Control", "public, max-age=2592000");
  int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
  MHD_destroy_response(response);
  return Return;
}
void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  Request_t *Request = (Request_t*)*con_cls;
  if(Request == NULL) return;
  if(Request->DB) delete Request->DB;
  if(Request->JSONReader) json_object_put(Request->JSONReader);
  if(Request->JSONWriter) json_object_put(Request->JSONWriter);
  if(Request->Deflate) free(Request->PostData);
  if(Request->Msg) {
    uint64_t EndTime = Tools::GetTime();
    Tools::LogMsg("DEBUG", "POST request '%s' takes %" PRIu64 "mS (%.2fkB send in %" PRIu64 "mS)\n", Request->Msg, Request->ProcTime - Request->StartTime, Request->PostResponseLen / 1024.0, EndTime - Request->StartTime);
    free(Request->Msg);
  }
  Request->Session->References++;
  free(Request);
}
int  HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  if(Request == NULL) {
    Tools::LogMsg("DEBUG", "Client %s request '%s'\n", method, url);
    if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
      static const char *Root_Path = "WEB/Resources";
      char *FilePath = (char*)malloc(strlen(Root_Path) + strlen(url) + 1);
      sprintf(FilePath, "%s%s", Root_Path, url);
      int Return = SendFile(connection, FilePath);
      free(FilePath);
      if(Return == MHD_YES) return MHD_YES;
    }
    *ptr = (Request_t*)calloc(1, sizeof(Request_t));
    Request = (Request_t*)*ptr;
    if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
      Request->Msg = strdup(url);
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
      Request->Deflate = ((AcceptEncoding != NULL) && (strstr(AcceptEncoding, "deflate") != NULL) && (Request->PostResponseLen > 256));
      if(Request->Deflate) {
        char *Tmp = (char*)malloc(Request->PostResponseLen);
        z_stream Deflate;
        Deflate.zalloc = Z_NULL;
        Deflate.zfree = Z_NULL;
        Deflate.opaque = Z_NULL;
        Deflate.avail_in = Request->PostResponseLen;
        Deflate.next_in = (uint8_t*)Request->PostData;
        Deflate.avail_out = Request->PostResponseLen;
        Deflate.next_out = (uint8_t*)Tmp;
        deflateInit(&Deflate, Z_BEST_SPEED);
        deflate(&Deflate, Z_FINISH);
        deflateEnd(&Deflate);
        Request->PostResponseLen = Deflate.total_out;
        Request->PostData = Tmp;      
      }
      struct MHD_Response *response = MHD_create_response_from_buffer(Request->PostResponseLen, Request->PostData, MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      if(Request->Deflate) MHD_add_response_header(response, MHD_HTTP_HEADER_CONTENT_ENCODING, "deflate");
      int Return = MHD_queue_response(connection, Request->ResultCode, response);
      MHD_destroy_response(response);
      Request->ProcTime = Tools::GetTime();
      return Return;
    }
  } else if(strcmp (method, MHD_HTTP_METHOD_GET) == 0) {
    if(strcmp(url, "/Login") == 0) {
      if(Request->Session->User != NULL) return SendRedirect(connection, "/Dashboard");
      char Cookie[256];
      sprintf(Cookie, "OZM_SESSION=%s; httponly; Path=/", Request->Session->ID);
      return SendFile(connection, (char*)"WEB/Login.html", MHD_HTTP_HEADER_SET_COOKIE, Cookie);
    }
    if(strncmp(url, "/MiniViews/", 11) == 0) {
      static const char *Root_Path = "WEB";
      char *FilePath = (char*)malloc(strlen(Root_Path) + strlen(url) + 6);
      sprintf(FilePath, "%s%s.html", Root_Path, url);
      int Return = SendFile(connection, FilePath);
      free(FilePath);
      if(Return == MHD_YES) return MHD_YES;
    }
    if(Request->Session->User == NULL) return SendRedirect(connection, "/Login");
    if((strcmp(url, "/Dashboard") == 0) || (strcmp(url, "/") == 0)) return SendFile(connection,  "WEB/Dashboard.html");
    if(strcmp(url, "/Analysis/Frequency") == 0) return SendFile(connection,  "WEB/Analysis_Frequency.html");
    if(strcmp(url, "/Analysis/Voltage") == 0) return SendFile(connection,  "WEB/Analysis_Voltage.html");
    if(strcmp(url, "/Analysis/Current") == 0) return SendFile(connection,  "WEB/Analysis_Current.html");
    if(strcmp(url, "/Analysis/EnergyPower") == 0) return SendFile(connection,  "WEB/Analysis_EnergyPower.html");
    if(strcmp(url, "/Events/ITIC") == 0) return SendFile(connection,  "WEB/Events_ITIC.html");
    if(strcmp(url, "/Events/Disturbances") == 0) return SendFile(connection,  "WEB/Events_Disturbances.html");
    if(strcmp(url, "/Events/EN50160") == 0) return SendFile(connection,  "WEB/Events_EN50160.html");
    if(strcmp(url, "/Stats") == 0) return SendFile(connection,  "WEB/Stats.html");
    if(strcmp(url, "/Help") == 0) return SendFile(connection,  "WEB/Help.html");
  }
  return SendError(connection, MHD_HTTP_NOT_FOUND, "Page not found");
}
int HTTPServer::HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  Request->JSONReader = json_tokener_parse(upload_data);
  bool Error = false;
  if((Request->JSONReader == NULL) || (json_object_get_type(Request->JSONReader) != json_type_object)) {
    Request->ResultCode = MHD_HTTP_BAD_REQUEST;
    Request->PostData = (char*)"Expected JSON params";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
  if(strcmp(url, "/LogIn") == 0) {
    Request->Session->User = (char*)"root";
    Request->PostData = (char*)"{\"result\":true}";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
//  if(Request->Session->User == NULL) {
//    struct sockaddr *Addr = MHD_get_connection_info(connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS)->client_addr;
//    if(((sockaddr_in*)Addr)->sin_addr.s_addr != 0x0100007F) {
//      Request->ResultCode = MHD_HTTP_UNAUTHORIZED;
//      Request->PostData = (char*)"Unauthorided access to API";
//      Request->PostResponseLen = strlen(Request->PostData);
//      return MHD_YES;
//    }
//  }
  if(strcmp(url, "/getNodes") == 0) {
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      (void)JSON_Key; (void)JSON_Val;      
      Error = true;
    }
    if(Error == true) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    Request->JSONWriter = json_object_new_object();
    json_object_object_add(Request->JSONWriter, "NominalVoltage", json_object_new_int(Tools::Electrical.NominalVoltage));
    json_object_object_add(Request->JSONWriter, "NominalFreq", json_object_new_int(Tools::Electrical.NominalFrequency));
    json_object *Devices = json_object_new_array();
    for(uint32_t i = 0; i < Tools::Devices.Len; i++) {
      json_object *Obj = json_object_new_object();
      json_object_object_add(Obj, "Name", json_object_new_string(Tools::Devices.Items[i].Name));
      json_object_object_add(Obj, "Type", json_object_new_string(Tools::Devices.Items[i].Instance->Triphase() ? "3Phase" : "1Phase"));
      json_object_object_add(Obj, "SampleFreq", json_object_new_int(Tools::Devices.Items[i].Instance->SampleRate()));
      json_object_object_add(Obj, "MaxCurrent", json_object_new_int(20));
      json_object_array_add(Devices, Obj);
    }
    json_object_object_add(Request->JSONWriter, "Devices", Devices);
    Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostResponseLen);
    return MHD_YES;
  }
  if(strcmp(url, "/getSeries") == 0) {
    uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
    uint32_t Node = 0;
    char    *Aggreg = NULL, *Series = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
        Series = (char*)json_object_get_string(JSON_Val);
      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
        Aggreg = (char*)json_object_get_string(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
        Period = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
        To = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastSample") == 0))
        LastSample = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else 
        Error = true;
    }
    if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0)) || (Series == NULL) || (Error == true)) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    if(To == 0) To = Tools::GetTime();
    From = To - Period;
    char QueryText[8192];
    int  QueryLen = 0;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
    uint32_t MaxPoints = 10000, PeriodSeconds = 3000;
    if(strcmp(Aggreg, "1m")  == 0) PeriodSeconds = 60000;
    if(strcmp(Aggreg, "10m") == 0) PeriodSeconds = 600000;
    if(strcmp(Aggreg, "1h")  == 0) PeriodSeconds = 3600000;
    if((strcmp(Aggreg, "3s") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = (char*)"1m";
      PeriodSeconds = 60000;
    } 
    if((strcmp(Aggreg, "1m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = (char*)"10m";
      PeriodSeconds = 600000;
    } 
    if((strcmp(Aggreg, "10m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = (char*)"1h";
      PeriodSeconds = 3600000;
    } 
    From -= PeriodSeconds;
    To   += PeriodSeconds;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"MaxSamples\", ", (To - From) / PeriodSeconds);
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'%s' AS \"Aggregation_Source\", ", Aggreg);
    char *Save;
    char *Serie = strtok_r(Series, ", ", &Save);
    while(Serie != NULL) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(%s ORDER BY sampletime ASC) AS %s, ", Serie, Serie);
      Serie = strtok_r(NULL, ", ", &Save);
    }
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(sampletime ORDER BY sampletime ASC) AS \"Sampletime\" FROM aggreg_%s WHERE sampletime > %" PRIu64 " AND sampletime >= %" PRIu64 " AND sampletime <= %" PRIu64 " AND node = %u) AS Result", Aggreg, LastSample, From, To, Node);
    Request->DB = new Database();
    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostResponseLen);
    return MHD_YES;
  } else if(strcmp(url, "/getSeriesAggreg") == 0) {
    uint64_t Period = 0, From = 0, To = 0;
    uint32_t Node = 0;
    char    *Aggreg = NULL, *Series = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
        Series = (char*)json_object_get_string(JSON_Val);
      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
        Aggreg = (char*)json_object_get_string(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
        Period = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
        To = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else 
        Error = true;
    }
    if((Aggreg == NULL) || ((strcmp(Aggreg, "3s") != 0) && (strcmp(Aggreg, "1m") != 0) && (strcmp(Aggreg, "10m") != 0) && (strcmp(Aggreg, "1h") != 0)) || (Series == NULL) || (Error == true)) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    if(To == 0) To = Tools::GetTime();
    From = To - Period;
    char QueryText[8192];
    int  QueryLen = 0;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT COUNT(*) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u) AS \"Samples\", ", Aggreg, From, To, Node);
    char *Save;
    char *Serie = strtok_r(Series, ", ", &Save);
    while(Serie != NULL) {
      char *Oper = NULL, *Tmp1 = strchr(Serie, '('), *Tmp2 = strrchr(Serie, ')');
      if((Tmp1 != NULL) && (Tmp2 != NULL)) {
        Oper = Serie;
        Tmp1[0] = 0;
        Serie = &Tmp1[1];
        Tmp2[0] = 0;
      }
      if((strcmp(Serie, "rms_v") == 0) || (strcmp(Serie, "rms_i") == 0) || (strcmp(Serie, "active_power") == 0) || (strcmp(Serie, "apparent_power") == 0) || (strcmp(Serie, "power_factor") == 0) || (strcmp(Serie, "active_energy") == 0) || (strcmp(Serie, "fundamenta_active_power") == 0) || (strcmp(Serie, "fundamental_reactive_power") == 0) || (strcmp(Serie, "fundamental_reactive_energy") == 0) || (strcmp(Serie, "fundamental_apparent_power") == 0) || (strcmp(Serie, "fundamental_power_factor") == 0) || (strcmp(Serie, "thd_v") == 0) || (strcmp(Serie, "thd_i") == 0) || (strcmp(Serie, "phi") == 0) || (strcmp(Serie, "harmonics_i") == 0) || (strcmp(Serie, "harmonics_v") == 0)) {
        if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
          QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT %s(elem) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u GROUP BY rn ORDER BY rn) as %s_%s,", Oper, Aggreg, Serie, From, To, Node, Oper, Serie);
        } else if(strcmp(Oper, "rms") == 0) {
          QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "ARRAY(SELECT SQRT(SUM(POW(elem, 2.0)) / COUNT(*)) FROM aggreg_%s, unnest(%s) WITH ORDINALITY x(elem, rn) WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u GROUP BY rn ORDER BY rn) as %s_%s,", Aggreg, Serie, From, To, Node, Oper, Serie);
        }
      } else if(strcmp(Serie, "freq") == 0) {
        if((strcmp(Oper, "sum") == 0) || (strcmp(Oper, "max") == 0) || (strcmp(Oper, "min") == 0) || (strcmp(Oper, "avg") == 0)) {
          QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT %s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u) AS %s_%s, ", Oper, Serie, Aggreg, From, To, Node, Oper, Serie);
        } else if(strcmp(Oper, "rms") == 0) {
          QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT SQRT(SUM(POW(%s, 2.0)) / COUNT(*)) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u) AS %s_%s, ", Serie, Aggreg, From, To, Node, Oper, Serie);
        }
      } else if(strcmp(Serie, "flag") == 0) {
        if((strcmp(Oper, "and") == 0) || (strcmp(Oper, "or") == 0)) {
          QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "(SELECT bool_%s(%s) FROM aggreg_%s WHERE sampletime >= %" PRIu64 " AND sampletime < %" PRIu64 " AND node = %u) AS %s_%s, ", Oper, Serie, Aggreg, From, To, Node, Oper, Serie);
        }
      }
      Serie = strtok_r(NULL, ", ", &Save);
    }
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\") AS Result", To); 
    Request->DB = new Database();
    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostResponseLen);
    return MHD_YES;
  } else if(strcmp(url, "/getSeriesNow") == 0) {
    uint32_t Node = 0;
    char    *Series = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
        Series = (char*)json_object_get_string(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else 
        Error = true;
    }
    if((Series == NULL) || (Error == true)) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    if(Node < Tools::Devices.Len) {
      Request->JSONWriter = Tools::Devices.Items[Node].Instance->GetJSONCopy(Series);
      if(Request->JSONWriter == NULL) {
        Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      } else {
        Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostResponseLen);
      }
      return MHD_YES;
    }
  } else if(strcmp(url, "/getSysInfo") == 0) {
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      (void)JSON_Key; (void)JSON_Val;      
      Error = true;
    }
    struct sysinfo Info;
    struct statvfs Disk;
    if(sysinfo(&Info) != 0) {
      Error = true;
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    }
    if(statvfs("/", &Disk) != 0) {
      Error = true;
      Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    }
    if(Error == true) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    Request->JSONWriter = json_object_new_object();
    json_object_object_add(Request->JSONWriter, "UpTime", json_object_new_int(Info.uptime));
    json_object_object_add(Request->JSONWriter, "Swap_Total", json_object_new_int64(Info.totalswap * Info.mem_unit));
    json_object_object_add(Request->JSONWriter, "Swap_Free", json_object_new_int64(Info.freeswap * Info.mem_unit));
    json_object_object_add(Request->JSONWriter, "RAM_Total", json_object_new_int64(Info.totalram * Info.mem_unit));
    json_object_object_add(Request->JSONWriter, "RAM_Free", json_object_new_int64(Info.freeram* Info.mem_unit));
    json_object_object_add(Request->JSONWriter, "CPU_Usage", json_object_new_double(Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)));
    json_object_object_add(Request->JSONWriter, "CPU_Count", json_object_new_int(get_nprocs()));
    json_object_object_add(Request->JSONWriter, "Disk_Total", json_object_new_int64(Disk.f_blocks * Disk.f_bsize));
    json_object_object_add(Request->JSONWriter, "Disk_Free", json_object_new_int64(Disk.f_bfree * Disk.f_bsize));
    json_object_object_add(Request->JSONWriter, "Battery_Percent", json_object_new_int(100));
    Request->PostData = (char*)json_object_to_json_string_length(Request->JSONWriter, JSON_C_TO_STRING_PLAIN, &Request->PostResponseLen);
    return MHD_YES;
  } else if(strcmp(url, "/getEvents") == 0) {
    uint32_t Node = 0;
    uint64_t LastEvent = 0, From = 0, To = 0, Period = 0;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastEvent") == 0))
        LastEvent = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
        Period = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
        To = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else 
        Error = true;
    }
    if(Error == true) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    if(To == 0) To = Tools::GetTime();
    From = To - Period;
    char QueryText[8192];
    int  QueryLen = 0;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"From\", ", From);
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%" PRIu64 " AS \"To\", ", To); 
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(starttime) AS \"Starttime\", ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(duration) AS \"Duration\", ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(type) AS \"Type\", ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(voltage) AS \"Voltage\", ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(percent) AS \"Percentage\" ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM events WHERE starttime > %" PRIu64 " AND starttime >= %" PRIu64 " AND starttime <= %" PRIu64 " AND node = %u ORDER BY \"Starttime\" ASC) AS Result", LastEvent, From, To, Node);
    Request->DB = new Database();
    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostResponseLen);
    return MHD_YES;
    
    
  } else if(strcmp(url, "/getSamplesRange") == 0) {
    uint32_t Node = 0;
    char *Aggreg = NULL;
    json_object_object_foreach(Request->JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
        Aggreg = (char*)json_object_get_string(JSON_Val);
      else
        Error = true;
    }
    if((Error == true) || (Aggreg == NULL)) {
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = (char*)"Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    char QueryText[8192];
    int  QueryLen = 0;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "max(sampletime) AS \"To\", ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "min(sampletime) AS \"From\" ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "FROM aggreg_%s WHERE node = %u) AS Result", Aggreg, Node);
    Request->DB = new Database();
    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostResponseLen);
    return MHD_YES;
  }
  Request->ResultCode = MHD_HTTP_BAD_REQUEST;
  Request->PostData = (char*)"Invalid command";
  Request->PostResponseLen = strlen(Request->PostData);
  return MHD_YES;
}
