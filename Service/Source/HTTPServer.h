/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <vector>
#include <stdlib.h>
#include <json-c/json.h>
#include <zlib.h>
#include <microhttpd.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
#define HTTP_CHUNK 32768
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class HTTPServer {
  public :
  typedef bool (*Func_t)(string user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
  private :
    typedef struct {
      string UserName;
      string SessionID;
      char           *PostData;
      size_t          PostDataLen;
      bool           *FreePostData;
      int             ResultCode;
      bool            Deflate;
      Database       *DB;
      json_object    *JSONReader;
      json_object    *JSONWriter;
      z_stream        DeflateObj;
    } Request_t;
    typedef struct {
      string HandleName;
      Func_t Function;
    } Callback_t;
    typedef struct {
      string URL;
      string File;
    } Mapping_t;
  private:
    static MHD_Daemon         *pDaemon;
    static string              pDefaultPage;
    static vector<Callback_t>  pCallbacks;
    static vector<Mapping_t>   pMappings;
  private :
    static int     HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void    HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void    ExpireSessions();
    static void    SetMimeType(struct MHD_Response *response, string url);
    static int     SendRedirect(struct MHD_Connection *connection, string url);
    static void    OpenSession(MHD_Connection *connection, Request_t *request);
    static void    CloseSession(MHD_Connection *connection, Request_t *request);
    static ssize_t WriteFunction(void *cls, uint64_t pos, char *buf, size_t max);
  public :
    static bool Config(json_object *config);
    static void RegisterDefault(string def);
    static void RegisterCallback(string handleName, Func_t function);
    static void RegisterMapping(string url, string file);
    static void Stop();
    static void RemoveExpiredSessions();
};
