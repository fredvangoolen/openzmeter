/* ================================== zMeter ==================================
 * File:     HTTPServer.h
 * Author:   Eduardo Viciana (20 de marzo de 2017)
 * ---------------------------------------------------------------------------- */
#pragma once
#include <vector>
#include <stdlib.h>
#include "nlohmann/json.hpp"
#include "Database.h"
#include "Tools.h"
//HTTPLIB config -------------------------------------------------------------------------------------------------------
#include "cpp-httplib/httplib.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace httplib;
#define HTTP_CHUNK 32768
#define HTTP_OK 200
#define HTTP_NOT_FOUND 404
#define MHD_HTTP_INTERNAL_SERVER_ERROR 500
#define MHD_HTTP_OK 200
#define MHD_HTTP_FORBIDDEN 403
#define MHD_HTTP_BAD_REQUEST 400
#define MHD_HTTP_SERVICE_UNAVAILABLE 503
#define MHD_HTTP_NO_CONTENT 204
#define MHD_HTTP_UNAUTHORIZED 401
#define MHD_HTTP_METHOD_NOT_ALLOWED 405
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class HTTPRequest final : public Database {
  public :
//    uint64_t    pStartTime;
    int         pResultCode;
    bool        pDeflate;
//    z_stream    pDeflateObj;
//    string      pSessionID;
    string      pUserName;
    string      pMimeType;
    string      pPostData;
    json        pJSONParams;
  public :
    HTTPRequest(string body, bool local);

//    HTTPRequest(char *cookie, bool deflate);
    ~HTTPRequest();
//    string      SessionID() const;
    string      UserName() const;
//    string      MimeType() const;
//    string      Content() const;
//    void        MimeType(const string &type);
//    uint32_t    size() const;
//    bool        Deflate() const;
//    int         ResultCode() const;
//    void        CloseSession();
//    void        AppendParams(const char *input, size_t *len);
//    bool        ProcessParams(bool local);
    const json &GetParams() const;
//    uint64_t    GetTime() const;
    void        FromJSON(const json &obj);
    void        FromString(const string &obj, int resultCode);
//  public :
//    static ssize_t WriteFunction(void *cls, uint64_t pos, char *buf, size_t max);
};
class HTTPCallback {
  public :
    typedef void (*Func_t)(HTTPRequest &request);
  public :
    string  HandleName;
    Func_t  Function;
  public :
    HTTPCallback(const string &handle, Func_t function);
};
class URL {
  public :
    string Protocol;
    string Host;
    string Path;
    string Query;
    string Port;
  public :
    URL(const string &url);
};
class HTTPServer final {
  private :
    static RWLocks              pLock;            //Control access to pCallbacks vector
    static vector<HTTPCallback> pCallbacks;       //List of callbacks URLs
    static string               pRelativePath;    //Relative path of server
  private:
    Server        *pServer;
//    MHD_Daemon  *pDaemon;
  private :
    static void HandleGet(const Request &request, Response &response);
    static void HandlePost(const Request &request, Response &response);
//    static int  HandleClient(void *cls, struct MHD_Connection *connection, const char *url, const char *method, const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);
//    static void HandleComplete(void *cls, struct MHD_Connection *connection, void **con_cls, enum MHD_RequestTerminationCode toe);
//    static int  SendRedirect(struct MHD_Connection *connection, const string &url);
//    static void GetFunctionsCallback(HTTPRequest &req);
  public :
    static void RegisterCallback(const string &handleName, const HTTPCallback::Func_t function);
    static void UnregisterCallback(const string &handleName);
  public :
    HTTPServer();
    ~HTTPServer();
    void RemoveExpiredSessions(Database &db) const;
};