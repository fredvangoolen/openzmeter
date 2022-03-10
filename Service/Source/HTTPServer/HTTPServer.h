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
#include <functional>
#include <list>
#include <stdlib.h>
#include "nlohmann/json.hpp"
#include "cpp-httplib/httplib.h"
#include "Database.h"
#include "Tools.h"
#include "RWLocks.h"
#include "HTTPRequest.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace httplib;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
#ifndef __BUILD__
#define __BUILD__ "UNKNOWN"
#endif
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer final : private Server {
  private :
    typedef struct {
      function<void(HTTPRequest &)> Func;
      string                        APIFile;
    } Callback_t;
  private :
    static RWLocks                      pLock;            //Control access to pCallbacks vector
    static map<string, string>          pComponents;      //Components openAPI descriptor
    static map<string, Callback_t>      pCallbacks;       //List of callbacks without reader
    static string                       pRelativePath;    //Relative path of server
    static string                       pLogoImg;         //Aditional logo image
  private:
    static uint16_t                     pPort;
    static pthread_t                    pThread;
  private :
    static string PatchFile(const string &file, const string &find, const string &replace);
    static string PatchString(string &src, const string &find, const string &replace);
    static string MimeType(const string &file);
    static void   PeriodicTasks();
    static void   HandleGet(const Request &request, Response &response);
    static void   HandlePost(const Request &request, Response &response);
    static void   GetVersionCallback(HTTPRequest &req);
    static void   LogInCallback(HTTPRequest &req);
    static void   LogOutCallback(HTTPRequest &req);
    static void   GetAPICallback(HTTPRequest &req);
  private :
    void *Thread();
  public :
    static void   RegisterCallback(const string &handleName, const function<void(HTTPRequest &)> func, const string &apiFile = "");
    static void   UnregisterCallback(const string &handleName);
    static void   RegisterDoc(const string &name, const json &doc);
    static void   UnregisterDoc(const string &name);
  public :
    HTTPServer();
    ~HTTPServer();
};
//----------------------------------------------------------------------------------------------------------------------