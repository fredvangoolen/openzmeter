/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once 
#include <microhttpd.h>
#include <json-c/json.h>
#include <stdlib.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer {
  private :
    typedef struct Session {
      struct Session *Next;
      char            ID[33];
      uint32_t        LastSeen;
      bool            Logged;
      uint32_t        References;
    } Session_t;
    typedef struct {
      Session_t                *Session;
      char                     *PostData;
      bool                      Deflate;
      size_t                    PostResponseLen;
      int                       ResultCode;
      Database                 *DB;
      json_object              *JSONReader;
      json_object              *JSONWriter;
      uint64_t                  StartTime;
      uint64_t                  ProcTime;
      char                     *Msg;
    } Request_t;
    static MHD_Daemon *Daemon;
    static char       *FilePath;
    static char       *Password;
    static uint64_t    LastLogin;
    static uint16_t    Port;
  private :
    static int        HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void       HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void       ExpireSessions();
    static void       SetMimeType(struct MHD_Response *response, const char *url);
    static int        SendFile(struct MHD_Connection *connection, const char *path, const char *aditionalheadertype = NULL, const char *aditionalheadervalue = NULL);
    static int        HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr);
    static int        SendRedirect(struct MHD_Connection *connection, const char *url);
    static int        SendError(struct MHD_Connection *connection, int error, const char *text);
    static Session_t *GetSession(MHD_Connection *connection);
    static Session_t *Sessions;
    static pthread_mutex_t SessionMutex;
  public :
    static bool Init(json_object *config);
    static bool Start();
    static bool stop();
    static void RemoveExpiredSessions();
};
