/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once 
#include <json-c/json.h>
#include <microhttpd.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer {
  public :
  typedef bool (*Func_t)(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  private :
//    typedef struct Session {
//      struct Session *Next;
//      char            ID[33];
//      uint32_t        LastSeen;
//      char           *UserName;
//      uint32_t        References;
//    } Session_t;
    typedef struct {
      char           *UserName;
//      Session_t                *Session;
//      char                     *URL;
      char                     *PostData;
      size_t                    PostDataLen;
      int                       ResultCode;
      bool                      Deflate;
//      uint64_t                  StartTime;
//      uint64_t                  ProcTime;
      Database                 *DB;
      json_object              *JSONReader;
      json_object              *JSONWriter;
    } Request_t;
    typedef struct {
      char   *HandleName;
      Func_t  Function;
    } Callback_t;
    typedef struct {
      char *URL;
      char *File;
    } Mapping_t;
  private:
    static MHD_Daemon     *pDaemon;
    static char           *pFilePath;
//    static char           *pPassword;
//    static uint64_t        pLastLogin;
//    static Session_t      *pSessions;
//    static pthread_mutex_t pSessionMutex;
    static Callback_t     *pCallbacks;
    static uint32_t        pCallbacksCount;
        static Mapping_t *pMappings;
    static uint32_t        pMappingssCount;
  private :
    static int   HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void  HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void  ExpireSessions();
    static void  SetMimeType(struct MHD_Response *response, const char *url);
    static int   SendFile(struct MHD_Connection *connection, const char *path, const char *aditionalheadertype = NULL, const char *aditionalheadervalue = NULL);
    static void  HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr);
    static int   SendRedirect(struct MHD_Connection *connection, const char *url);
    static int   SendError(struct MHD_Connection *connection, int error, const char *text);
    static char *OpenSession(MHD_Connection *connection, Request_t *request);
    static void  CloseSession(MHD_Connection *connection, Request_t *request);
  public :
    static void Config(json_object *config);
    static void RegisterCallback(char *handleName, Func_t function);
    static void RegisterMapping(char *url, char *file);
    static void Stop();
    static void RemoveExpiredSessions();
};
