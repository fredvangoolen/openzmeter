/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <stdlib.h>
#include <json-c/json.h>
#include <microhttpd.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer {
  public :
  typedef bool (*Func_t)(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  private :
    typedef struct {
      char           *UserName;
      char           *SessionID;
      char           *PostData;
      size_t          PostDataLen;
      int             ResultCode;
      bool            Deflate;
      Database       *DB;
      json_object    *JSONReader;
      json_object    *JSONWriter;
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
    static Callback_t     *pCallbacks;
    static uint32_t        pCallbacksCount;
    static Mapping_t      *pMappings;
    static char           *pDefaultPage;
    static uint32_t        pMappingssCount;
  private :
    static int   HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void  HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void  ExpireSessions();
    static void  SetMimeType(struct MHD_Response *response, const char *url);
    static int   SendFile(struct MHD_Connection *connection, const char *path);
    static void  HandlePostRequest(struct MHD_Connection *connection, const char *url, const char *upload_data, size_t *upload_data_size, void **ptr);
    static int   SendRedirect(struct MHD_Connection *connection, const char *url);
    static int   SendError(struct MHD_Connection *connection, int error, const char *text);
    static void  OpenSession(MHD_Connection *connection, Request_t *request);
    static void  CloseSession(MHD_Connection *connection, Request_t *request);
  public :
    static bool Config(json_object *config);
    static void RegisterDefault(char *def);
    static void RegisterCallback(char *handleName, Func_t function);
    static void RegisterMapping(char *url, char *file);
    static void Stop();
    static void RemoveExpiredSessions();
};
