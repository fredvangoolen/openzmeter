/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <vector>
#include <stdlib.h>
#include <zlib.h>
#include <microhttpd.h>
#include "json.hpp"
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
#define HTTP_CHUNK 32768
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class HTTPRequest : public Database {
  private :
    uint64_t    pStartTime;
    int         pResultCode;
    bool        pDeflate;
    z_stream    pDeflateObj;
    string      pSessionID;
    string      pUserName;
    string      pMimeType;
    string      pPostData;
    bool        pFreePostData;    
    json        pJSONParams;
  public :
    HTTPRequest(const char *cookie, bool deflate);
    ~HTTPRequest();
    string   SessionID();
    string   UserName();
    string   MimeType();
    uint32_t size();
    bool     Deflate();
    int      ResultCode();
    void     CloseSession();
    void     FromFile(string filename);
    bool     SetParams(string input, bool local);
    json     GetParams();
    uint64_t GetTime();
    void     FromJSON(json obj);
    void     FromString(string obj, int resultCode);
  public :
    static ssize_t WriteFunction(void *cls, uint64_t pos, char *buf, size_t max);
};
class HTTPServer {
  public :
  typedef void (*Func_t)(HTTPRequest &request);
  private :
    typedef struct {
      string HandleName;
      Func_t Function;
    } Callback_t;
  private:
    static MHD_Daemon         *pDaemon;
    static vector<Callback_t>  pCallbacks;
  private :
    static int     HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void    HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static void    ExpireSessions();
    static int     SendRedirect(struct MHD_Connection *connection, string url);
  public :
    static bool Config(json_object *config);
    static void RegisterCallback(string handleName, Func_t function);
    static void Stop();
    static void RemoveExpiredSessions();
};
