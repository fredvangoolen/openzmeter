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
#include "Database.h"
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
  return true;
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
  else
    Tools::LogMsg("ERROR", "No mimetype defined for request '%s'\n", url);
  MHD_add_response_header(response, "Content-Type", MimeType);
}
int  HTTPServer::SendError(struct MHD_Connection *connection, int error, const char *text) {
  Tools::LogMsg("DEBUG", "Error (%d - %s) on request\n", error, text);
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
int  HTTPServer::SendFile(struct MHD_Connection *connection, const char *url) {
  int File = -1;
  struct stat FileStat;
  if((stat(url, &FileStat) == 0) && (S_ISREG(FileStat.st_mode))) File = open(url, O_RDONLY);
  if(File == -1) return MHD_NO;
  struct MHD_Response *response = MHD_create_response_from_fd(FileStat.st_size, File);
  SetMimeType(response, url);
  MHD_add_response_header (response, "Cache-Control", "public, max-age=2592000");
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
  if(Request != NULL) {
    MHD_destroy_post_processor(Request->PostProcessor);
    free(Request);
  }
}
int  HTTPServer::HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr) {
  if(HandleStaticFiles(connection, url, method) == MHD_YES) return MHD_YES;
  Request_t *Request = (Request_t*)*ptr;
  if(Request == NULL) {
    Tools::LogMsg("DEBUG", "Client %s request '%s'\n", method, url);
    *ptr = (Request_t*)calloc(1, sizeof(Request_t));
    Request = (Request_t*)*ptr;
    Request->Session = NULL;//GetSession(connection);
    if(strcmp (method, MHD_HTTP_METHOD_POST) == 0) {
      Request->PostProcessor = MHD_create_post_processor(connection, 1024, &HandlePostRequest, Request);
      return (Request->PostProcessor == NULL) ? MHD_NO : MHD_YES;
    }
  }
  if(strcmp (method, MHD_HTTP_METHOD_POST) == 0) {
    MHD_post_process(Request->PostProcessor, upload_data, *upload_data_size);
    if (*upload_data_size != 0) {
	    *upload_data_size = 0;
  	  return MHD_YES;
	  } else {
      return SendError(connection, MHD_HTTP_LENGTH_REQUIRED, "bliu");
    }
  }
  
  


  if(HandleLogin(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
  Session_t *Session = (Session_t*)*ptr;
  if(Session == NULL) return MHD_NO;
  if(HandleDashboard(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
//  if(HandlePostRequest(connection, url, upload_data, upload_data_size, method, ptr) == MHD_YES) return MHD_YES;
  if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    if(Session->User == NULL) return SendRedirect(connection, "/Login");
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
  if(Request->Session == NULL) {
    const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "OZM_SESSION");
    if(Cookie != NULL) {
      Request->Session = Sessions;
      while(Request->Session != NULL) {
        if(strcmp(Cookie, Request->Session->ID) == 0) break;
        Request->Session = Request->Session->Next;
      }
    } 
    if(Request->Session == NULL) {
      Request->Session = (Session_t*)calloc(1, sizeof(Session_t));
      if(Request->Session == NULL) {
        Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
        return MHD_NO; 
      }
      sprintf(Request->Session->ID, "%08X%08X%08X%08X", (uint32_t)random(), (uint32_t)random(), (uint32_t)random(), (uint32_t)random());
      Request->Session->User = NULL;
      Request->Session->Start = time(NULL);
      Request->Session->Next = Sessions;
      Sessions = Request->Session;
    }
    if(Request->Session != NULL) Request->Session->Start = time(NULL);
  }
  if(strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
    if(strcmp(url, "/Login/In") == 0) {
      Request->Session->User = "root";
      char *RespText = "{\"result\":true}";
      struct MHD_Response *response = MHD_create_response_from_buffer(strlen(RespText), (void*)RespText, MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response(response);
      return Return;
    }
    //if(Session->User == NULL) return SendError(connection, MHD_HTTP_UNAUTHORIZED, "");
  } else if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    if(strcmp(url, "/Login") == 0) {
      if(Request->Session->User != NULL) return SendRedirect(connection, "/Dashboard");
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
        sprintf(Cookie, "%s=%s; httponly; Path=/", "OZM_SESSION", Request->Session->ID);
        MHD_add_response_header(response, MHD_HTTP_HEADER_SET_COOKIE, Cookie);
        int Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response (response);
        return Return;
      }
      return MHD_NO;
    }  
    if(Request->Session->User == NULL) return SendRedirect(connection, "/Login");
  }
  return MHD_NO;
}
int HTTPServer::HandlePostRequest(void *cls, enum MHD_ValueKind kind, const char *key, const char *filename, const char *content_type, const char *transfer_encoding, const char *data, uint64_t off, size_t size) {
  bool Error = false;
//  if(strcmp(method, MHD_HTTP_METHOD_POST) != 0) return MHD_NO;
  Request_t *Request = (Request_t*)cls;
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