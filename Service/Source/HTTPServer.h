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
using namespace std;
#define HTTP_OK                    200
#define HTTP_NO_CONTENT            204
#define HTTP_BAD_REQUEST           400
#define HTTP_UNAUTHORIZED          401
#define HTTP_FORBIDDEN             403
#define HTTP_NOT_FOUND             404
#define HTTP_METHOD_NOT_ALLOWED    405
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_SERVICE_UNAVAILABLE   503
//----------------------------------------------------------------------------------------------------------------------
class HTTPCallback {
  public :
    typedef void (*Func_t)(const json &params, Response &response);
  public :
    string  HandleName;
    Func_t  Function;
  public :
    HTTPCallback(const string &handle, Func_t function);
};
//----------------------------------------------------------------------------------------------------------------------
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
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer final {
  private :
    static RWLocks              pLock;            //Control access to pCallbacks vector
    static vector<HTTPCallback> pCallbacks;       //List of callbacks URLs
    static string               pRelativePath;    //Relative path of server
  private:
    uint16_t       pPort;
    pthread_t      pThread;
    Server        *pServer;
  private :
    static void *Listen_Thread(void *params);
    static void  HandleGet(const Request &request, Response &response);
    static void  HandlePost(const Request &request, Response &response);
    static void  GetFunctionsCallback(const json &params, Response &response);
  public :
    static void RegisterCallback(const string &handleName, const HTTPCallback::Func_t function);
    static void UnregisterCallback(const string &handleName);
  public :
    HTTPServer();
    ~HTTPServer();
    void RemoveExpiredSessions() const;
};