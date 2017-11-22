/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once 
#include <microhttpd.h>
#include <stdlib.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
#define SERVER_ROOT "WEB/Resources"
class HTTPServer {
  private :
    typedef struct Session {
      struct Session *Next;
      char            ID[33];
      time_t          Start;
      char           *User;
    } Session_t;
    typedef struct {
      Session_t                *Session;
      char                     *PostData;
      int                       PostResponseLen;
      int                       ResultCode;
      Database                 *DB;
    } Request_t;
    static MHD_Daemon *Daemon;
  private :
    static int        HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void       HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void       ExpireSessions();
    static void       SetMimeType(struct MHD_Response *response, const char *url);
    static int        SendFile(struct MHD_Connection *connection, const char *path, char *aditionalheadertype = NULL, char *aditionalheadervalue = NULL);
    static int        HandleStaticFiles(struct MHD_Connection *connection, const char *url, const char *method);
    static int        HandleLogin(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, const char *method, void **ptr);
    static int        HandlePostRequest(const char *url, const char *upload_data, size_t *upload_data_size, void **ptr);
    static int        HandleDashboard(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, const char *method, void **ptr);
    static int        SendRedirect(struct MHD_Connection *connection, const char *url);
    static int        SendError(struct MHD_Connection *connection, int error, const char *text);
    static Session_t *GetSession(MHD_Connection *connection);
    static Session_t *Sessions; 
  public :
    static bool Start();
    static bool stop();
};