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
#include "HTTPServer.h"
#include "Tools.h"
#include <json-c/json.h>
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon            *HTTPServer::Daemon = NULL;
HTTPServer::Session_t *HTTPServer::Sessions = NULL;
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
  if(Cookie != NULL) {
    Session = Sessions;
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
    Session->User = NULL;
    Session->Next = Sessions;
    Sessions = Session;
  }
  Session->Start = time(NULL);
  return Session;
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
int  HTTPServer::SendFile(struct MHD_Connection *connection, const char *path, char *aditionalheadertype, char *aditionalheadervalue) {
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
int  HTTPServer::HandleStaticFiles(struct MHD_Connection *connection, const char *url, const char *method) {
  if(strcmp(method, MHD_HTTP_METHOD_GET) != 0) return MHD_NO;
  char *FilePath = (char*)malloc(strlen(SERVER_ROOT) + strlen(url) + 1);
  sprintf(FilePath, "%s%s", SERVER_ROOT, url);
  int Return = SendFile(connection, FilePath);
  free(FilePath);
  return Return;
}
void HTTPServer::HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe) {
  Request_t *Request = (Request_t*)*con_cls;
  if(Request == NULL) return;
  if(Request->DB) delete Request->DB;
    free(Request);
}
int  HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  if(Request == NULL) {
    Tools::LogMsg("DEBUG", "Client %s request '%s'\n", method, url);
    if(HandleStaticFiles(connection, url, method) == MHD_YES) return MHD_YES;
    *ptr = (Request_t*)calloc(1, sizeof(Request_t));
    Request = (Request_t*)*ptr;
    Request->Session = GetSession(connection);
    Request->ResultCode = MHD_HTTP_OK;
    if(strcmp (method, MHD_HTTP_METHOD_POST) == 0) return MHD_YES;
  }
  if(strcmp (method, MHD_HTTP_METHOD_POST) == 0) {
    if(*upload_data_size != 0) {
      HandlePostRequest(url, upload_data, upload_data_size, ptr);
	    *upload_data_size = 0;
  	  return MHD_YES;
	  } else {
      struct MHD_Response *response = MHD_create_response_from_buffer(Request->PostResponseLen, Request->PostData, MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      int Return = MHD_queue_response(connection, Request->ResultCode, response);
      MHD_destroy_response (response);
      return Return;
    }
  } else if(strcmp (method, MHD_HTTP_METHOD_GET) == 0) {
    if(HandleLogin(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
    if(HandleDashboard(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
  }
  //  Session_t *Session = (Session_t*)*ptr;
//  if(Session == NULL) return MHD_NO;
//  if(HandlePostRequest(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
  if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    if(Request->Session->User == NULL) return SendRedirect(connection, "/Login");
    if((strcmp(url, "/") == 0) || (strcmp(url, "/Dashboard") == 0)) {
    } else if(strcmp(url, "/Analisys_Voltage") == 0) {
    } else if(strcmp(url, "/Analisys_Current") == 0) {
    } else if(strcmp(url, "/Analisys_Energy") == 0) {
    }
  }
  return SendError(connection, MHD_HTTP_NOT_FOUND, "Page not found");
}
int  HTTPServer::HandleLogin(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, const char *method, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  if(strcmp(url, "/Login") == 0) {
    if(Request->Session->User != NULL) return SendRedirect(connection, "/Dashboard");
    char Cookie[256];
    sprintf(Cookie, "OZM_SESSION=%s; httponly; Path=/", Request->Session->ID);
    return SendFile(connection, "WEB/Views/Login.html", MHD_HTTP_HEADER_SET_COOKIE, Cookie);

    int File = -1;
    struct stat FileStat;
    const char *FilePath = "WEB/Views/Login.html";
    if((stat(FilePath, &FileStat) == 0) && (S_ISREG(FileStat.st_mode))) {
      File = open(FilePath, O_RDONLY);
    }
    if(File >= 0) {
      struct MHD_Response *response = MHD_create_response_from_fd(FileStat.st_size, File);
      SetMimeType(response, FilePath);
      char Cookie[256];
      sprintf(Cookie, "OZM_SESSION=%s; httponly; Path=/", Request->Session->ID);
      MHD_add_response_header(response, MHD_HTTP_HEADER_SET_COOKIE, Cookie);
      int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      return Return;
    }
    return MHD_NO;
  }  
  if(Request->Session->User == NULL) return SendRedirect(connection, "/Login");
  return MHD_NO;
}
int HTTPServer::HandlePostRequest(const char *url, const char *upload_data, size_t *upload_data_size, void **ptr) {
  Request_t *Request = (Request_t*)*ptr;
  json_object *JSONReader = json_tokener_parse(upload_data);
  bool Error = false;
  if(JSONReader == NULL) {
    Request->ResultCode = MHD_HTTP_BAD_REQUEST;
    Request->PostData = "Expected JSON params";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
  if(strcmp(url, "/LogIn") == 0) {
    Request->Session->User = "root";
    Request->PostData = "{\"result\":true}";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
  if(Request->Session->User == NULL) {
    Request->ResultCode = MHD_HTTP_UNAUTHORIZED;
    Request->PostData = "Unauthorided access to API";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
  if(strcmp(url, "/getNodes") == 0) {
    Request->PostData = "{ \"IDs\": [0,1], \"Names\": [\"Principal\", \"Secundario\"], \"State\" : [true, false], \"Type\": [\"3Phase\", \"1Phase\"], \"SampleFreq\": [15625, 15625], \"NominalVoltage\": [230, 230], \"MaxCurrent\": [20, 20] }";
    Request->PostResponseLen = strlen(Request->PostData);
    return MHD_YES;
  }
  if(strcmp(url, "/getSeriesData") == 0) {
    uint64_t Period = 0, From = 0, To = 0, LastSample  = 0;
    bool     IncludeFlag = false;
    uint32_t Node = 0;
    char    *Aggreg = NULL, *Series = NULL;
    json_object_object_foreach(JSONReader, JSON_Key, JSON_Val) {
      enum json_type JSON_Type = json_object_get_type(JSON_Val);
      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Series") == 0)) 
        Series = strdup(json_object_get_string(JSON_Val));
      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Aggreg") == 0))
        Aggreg = strdup(json_object_get_string(JSON_Val));
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Period") == 0))      
        Period = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "From") == 0))  
        From = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "To") == 0))
        To = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "LastSample") == 0))
        LastSample = json_object_get_int64(JSON_Val);
      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Node") == 0))
        Node = json_object_get_int(JSON_Val);
      else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "IncludeFlag") == 0))
        IncludeFlag = json_object_get_boolean(JSON_Val);
      else 
        Error = true;
    }
    if((Aggreg == NULL) || (Series == NULL) || (Error == true)) {    
      Request->ResultCode = MHD_HTTP_BAD_REQUEST;
      Request->PostData = "Invalid JSON params";
      Request->PostResponseLen = strlen(Request->PostData);
      return MHD_YES;
    }
    if(To == 0) To = Tools::GetTime();
    From = To - Period;
    char QueryText[8192];
    int  QueryLen = 0;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "SELECT row_to_json(Result) AS result FROM ( SELECT ");
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%llu AS \"From\", ", From);
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%llu AS \"To\", ", To); 
    int MaxPoints = 10000, PeriodSeconds = 3000;
    if((strcmp(Aggreg, "3s") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = "1m";
      PeriodSeconds = 60000;
    } 
    if((strcmp(Aggreg, "1m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = "10m";
      PeriodSeconds = 600000;
    } 
    if((strcmp(Aggreg, "10m") == 0) && ((Period / PeriodSeconds) > MaxPoints)) {
      Aggreg = "1h";
      PeriodSeconds = 3600000;
    } 
    From -= PeriodSeconds;
    To   += PeriodSeconds;
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%llu AS \"MaxSamples\", ", (To - From) / PeriodSeconds);
    if(strcmp(Aggreg, "3s") == 0) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'3 seconds' AS \"Aggregation_Source\", ");
    } else if(strcmp(Aggreg, "1m") == 0) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'1 minute' AS \"Aggregation_Source\", ");
    } else if(strcmp(Aggreg, "10m") == 0) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'10 minutes' AS \"Aggregation_Source\", ");
    } else if(strcmp(Aggreg, "1h") == 0) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "'1 hour' AS \"Aggregation_Source\", ");
    }
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "%s AS \"Update\", ", (LastSample != 0) ? "true" : "false");

    char *Serie = strtok(Series, ", ");
    while(Serie != NULL) {
      QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(%s) AS %s, ", Serie, Serie);
      Serie = strtok(NULL, ", ");
    }
    QueryLen += snprintf(&QueryText[QueryLen], sizeof(QueryText) - QueryLen, "array_agg(sampletime) AS \"Sampletime\" FROM aggreg_%s WHERE sampletime > %llu AND sampletime >= %llu AND sampletime <= %llu AND node = %u ORDER BY \"Sampletime\" ASC) AS Result", Aggreg, LastSample, From, To, Node);
//    printf(QueryText);
    Request->DB = new Database();
    Request->PostData = Request->DB->ExecSentenceJSON(QueryText, &Request->PostResponseLen);
    return MHD_YES;
    //    if(Text != NULL) {


//    echo $row['result'];
//    return;

  

  }


  if(Request->PostData == NULL) {
    Request->PostData = strdup("bliu");
    Request->PostResponseLen = 4;
  }
  return MHD_YES;
//  if((upload_data != NULL) && (Request->PostData == NULL)) Request->PostData = strdup(upload_data);
//  if((*upload_data_size != 0) || (upload_data != NULL) || (Request->PostData == NULL)) return MHD_YES;

//  json_object *JSONReader = json_tokener_parse((char*)*ptr);
//  if(JSONReader == NULL) return SendError(connection, MHD_HTTP_BAD_REQUEST, "Expected JSON params");
//  if(strcmp(url, "/GetRows") == 0) {
//    char    *Table = NULL, *Variables = NULL;
//    uint64_t Period = 0, From = 0, To = 0;
//    json_object_object_foreach(JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "variables") == 0)) 
//        Variables = strdup(json_object_get_string(JSON_Val));
//      else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "table") == 0))
//        Table = strdup(json_object_get_string(JSON_Val));
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "period") == 0))      
//        Period = json_object_get_int64(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "from") == 0))  
//        From = json_object_get_int64(JSON_Val);
//      else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "to") == 0))
//        To = json_object_get_int64(JSON_Val);
//      else 
//        Error = true;
//    }
//    free(JSONReader);
//    free(*ptr);
//    if((Variables == NULL) || (Table != NULL) || (Error == true)) return SendError(connection, MHD_HTTP_BAD_REQUEST, "Invalids params.");
//    Database DB;
//    int Len;
//    char *Text = DB.ExecSentenceJSON("SELECT row_to_json(TmpB) FROM ("
//                                     " SELECT array_agg(sampletime) AS sampletime, array_agg(rms_i) AS rms_i"
//                                     " FROM (SELECT sampletime, rms_i FROM agreg_10c WHERE sampletime > 1508917614689 ORDER BY sampletime ASC LIMIT 200) as TmpA) AS TmpB;", &Len);
//    free(Variables);
//    free(Table);
//    if(Text != NULL) {
//      struct MHD_Response *response = MHD_create_response_from_buffer(Len, Text, MHD_RESPMEM_MUST_FREE);
//      MHD_add_response_header(response, "Content-Type", "application/json");
//      int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
//      MHD_destroy_response (response);
//      return Return;
//    }      
//  } else   if(strcmp(url, "/GetDevices") == 0) {
//    
//  } else   if(strcmp(url, "/GetLastValues") == 0) {
//    json_object *JSONWriter = json_object_new_object();
//    json_object_object_foreach(JSONReader, JSON_Key, JSON_Val) {
//      enum json_type JSON_Type = json_object_get_type(JSON_Val);
//      if(JSON_Type == json_type_array) {
//        int arraylen = json_object_array_length(JSONReader);
//        
//        
//        //Variables = strdup(json_object_get_string(JSON_Val));
//      }
//    }
//    free(JSONReader);
//    free(*ptr);
//
//
//
//
////    if((Variables != NULL) && (Table != NULL)) {
////      Database DB;
////      int Len;
////      char *Text = DB.ExecSentenceJSON("SELECT row_to_json(TmpB) FROM ("
////                                       " SELECT array_agg(sampletime) AS sampletime, array_agg(rms_i) AS rms_i"
////                                       " FROM (SELECT sampletime, rms_i FROM agreg_10c WHERE sampletime > 1508917614689 ORDER BY sampletime ASC LIMIT 200) as TmpA) AS TmpB;", &Len);
////      free(Variables);
////      free(Table);
////      if(Text != NULL) {
////        struct MHD_Response *response = MHD_create_response_from_buffer(Len, Text, MHD_RESPMEM_MUST_FREE);
////        MHD_add_response_header(response, "Content-Type", "application/json");
////        int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
////        MHD_destroy_response (response);
////        return Return;
////      }      
////    }
//    struct MHD_Response *response = MHD_create_response_from_buffer(2, (void*)"{}", MHD_RESPMEM_PERSISTENT);
//    MHD_add_response_header(response, "Content-Type", "application/json");
//    int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
//    MHD_destroy_response (response);
//    return Return;
//  }
//  struct MHD_Response *response = MHD_create_response_from_buffer(2, (void*)"{}", MHD_RESPMEM_PERSISTENT);
//  MHD_add_response_header(response, "Content-Type", "application/json");
//  int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
//  MHD_destroy_response (response);
//  return Return;
}
int HTTPServer::HandleDashboard(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, const char *method, void **ptr) {
  if((strcmp(method, MHD_HTTP_METHOD_GET) != 0) || ((strcmp(url, "/Dashboard") != 0) && (strcmp(url, "/") != 0))) return MHD_NO;
  int Return = SendFile(connection,  "WEB/Views/Dashboard.html");
  return Return;
}