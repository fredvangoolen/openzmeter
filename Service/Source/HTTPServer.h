/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <vector>
#include <stdlib.h>
#include <zlib.h>
#include <microhttpd.h>
#include "nlohmann/json.hpp"
#include "Database.h"
#include "Tools.h"
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
    json        pJSONParams;
  public :
    HTTPRequest(char *cookie, bool deflate);
    ~HTTPRequest();
    string      SessionID() const;
    string      UserName() const;
    string      MimeType() const;
    string      Content() const;
    void        MimeType(const string &type);
    uint32_t    size() const;
    bool        Deflate() const;
    int         ResultCode() const;
    void        CloseSession();
    void        FromFile(const string &filename);
    void        AppendParams(const char *input, size_t *len);
    bool        ProcessParams(bool local);
    const json &GetParams() const;
    uint64_t    GetTime() const;
    void        FromJSON(const json &obj);
    void        FromString(const string &obj, int resultCode);
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
  private :
    static RWLocks            pLock;            //Control access to pCallbacks vector
    static vector<Callback_t> pCallbacks;       //List of callbacks URLs
    static string             pRelativePath;    //Relative path of server
  private:
    MHD_Daemon  *pDaemon;
  private :
    static void PingCallback(HTTPRequest &req);
    static int  HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
    static void HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
    static int  SendRedirect(struct MHD_Connection *connection, const string &url);
  public :
    static void RegisterCallback(const string &handleName, const Func_t function);
    static void UnregisterCallback(const string &handleName);
  public :
    HTTPServer();
    virtual ~HTTPServer();
    void     RemoveExpiredSessions() const;
};