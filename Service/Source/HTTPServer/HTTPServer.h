// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <vector>
#include <list>
#include <stdlib.h>
#include "nlohmann/json.h"
#include "cpp-httplib/httplib.h"
#include "Database.h"
#include "Tools.h"
#include "RWLocks.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace httplib;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
#define HTTP_OK                    200
#define HTTP_NO_CONTENT            204
#define HTTP_BAD_REQUEST           400
#define HTTP_UNAUTHORIZED          401
#define HTTP_FORBIDDEN             403
#define HTTP_NOT_FOUND             404
#define HTTP_METHOD_NOT_ALLOWED    405
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_SERVICE_UNAVAILABLE   503
#define HTTP_GATEWAY_TIMEOUT       504
#ifndef __BUILD__
#define __BUILD__ "UNKNOWN"
#endif
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer final : private Server {
  private :
    typedef void (*Func_t)(const json &params, Response &response);
  private :
    static RWLocks              pLock;            //Control access to pCallbacks vector
    static map<string, Func_t>  pCallbacks;       //List of callbacks URLs
    static string               pRelativePath;    //Relative path of server
  private:
    static uint16_t             pPort;
    static pthread_t            pThread;
  private :
    static void        PeriodicTasks();
    static string      ParseCookie(const string &cookie);
    static string      PatchFile(const string &file, const string &find, const string &replace);
    static string      PatchString(string &src, const string &find, const string &replace);
    static const char *MimeType(const string &file);
    static void        HandleGet(const Request &request, Response &response);
    static void        HandlePost(const Request &request, Response &response);
    static void        GetVersionCallback(const json &params, Response &response);
    static void        LogInCallback(const json &params, Response &response);
    static void        LogOutCallback(const Request &request, Response &response);
  private :
    void *Thread();
  public :
    static void   RegisterCallback(const string &handleName, const Func_t function);
    static void   UnregisterCallback(const string &handleName);
    static void   FillResponse(Response &response, const string &body, const char *mime, const int status);
    static void   FillResponse(Response &response, const char *body, const size_t size, const char *mime, const int status);
  public :
    HTTPServer();
    ~HTTPServer();
};