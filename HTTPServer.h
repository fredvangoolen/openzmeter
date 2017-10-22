/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once 
#include <microhttpd.h>
#include <stdlib.h>
//----------------------------------------------------------------------------------------------------------------------
#define SERVER_ROOT "WEB/Resources"
class HTTPServer {
  private :
    typedef struct Session {
      struct Session *Next;
      char            ID[33];
//      uint32_t        ResourceCounter;
      time_t          Start;
      char           *User;
    } Session_t;
    static MHD_Daemon *Daemon;
    static int        HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void       ExpireSessions();
    static void       SetMimeType(struct MHD_Response *response, const char *url);
    static int        HandleStaticFiles(struct MHD_Connection *connection, const char *url, const char *method);
    static int        HandleLogin(struct MHD_Connection *connection, Session_t *session);
    static Session_t *GetSession(MHD_Connection *connection);
    static Session_t *Sessions; 
  public :
    static bool Start();
    static bool stop();
};