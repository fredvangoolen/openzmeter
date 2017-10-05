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
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
MHD_Daemon            *HTTPServer::Daemon = NULL;
HTTPServer::Session_t *HTTPServer::Sessions = NULL;
//----------------------------------------------------------------------------------------------------------------------
bool HTTPServer::Start() {
  Daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, Tools::Global.HTTPPort, NULL, NULL, &HandleClient, NULL, MHD_OPTION_END);
  return true;
}
int HTTPServer::HandleClient (void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **con_cls) {
  int Return;
  if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    int File = -1;
    struct stat FileStat;
    char *FilePath = (char*)malloc(strlen(SERVER_ROOT) + strlen(url));
    sprintf(FilePath, "%s%s", SERVER_ROOT, url);
    if((stat(FilePath, &FileStat) == 0) && (S_ISREG(FileStat.st_mode))) {
      File = open(FilePath, O_RDONLY);
    }
    free(FilePath);
    if(File >= 0) {
      struct MHD_Response *response = MHD_create_response_from_fd(FileStat.st_size, File);
      Return = MHD_queue_response(connection, MHD_HTTP_OK, response);
      MHD_destroy_response (response);
      return Return;
    }
  }
  //Aqui valido que tenga sesion iniciada
GetSession (connection);
  if(strcmp(method, MHD_HTTP_METHOD_POST) != 0) {
  
    
    
    //Es una peticion de datos
  } else if(strcmp(method, MHD_HTTP_METHOD_GET) == 0) {
    
    //Si el cliente no me manda la cookie redirijo a /login
  if(strcmp(url, "/") == 0) {
  
  }
  
  }
//  int Return = HandleFile(connection, url);
//  return Return;

  struct MHD_Response *response = MHD_create_response_from_buffer(0, (void*)"", MHD_RESPMEM_PERSISTENT);
  Return = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response (response);
  return Return;
}
HTTPServer::Session_t *HTTPServer::GetSession(MHD_Connection *connection) {
  Session_t *Session;
  const char *Cookie = MHD_lookup_connection_value(connection, MHD_COOKIE_KIND, "OZM_SESSION");
  if(Cookie != NULL) {
    Session = Sessions;
    while(Session != NULL) {
	    if(strcmp(Cookie, Session->ID) == 0) break;
      Session = Session->Next;
    }
    if(Session != NULL) {
      Session->ResourceCounter++;
      return Session;
  	}
  }
  Session = (Session_t*)calloc(1, sizeof(Session_t));
  if(Session == NULL) {						
//    fprintf (stderr, "calloc error: %s\n", strerror (errno));
    return NULL; 
  }
  sprintf(Session->ID, "%08X%08X%08X%08X", (uint32_t)random(), (uint32_t)random(), (uint32_t)random(), (uint32_t)random());
  Session->ResourceCounter = 1;  
  Session->Start = time(NULL);
  Session->Next = Sessions;
  Sessions = Session;
  return Session;
}