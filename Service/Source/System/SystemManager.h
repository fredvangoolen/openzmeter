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
#include "nlohmann/json.hpp"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------

#ifdef OPENWRT // ======================================================================================================
class SystemManager final {
  private :
    static json             pStatus;
    static pthread_mutex_t  pMutex;
  private :
    static void     PeriodicTasks();
    static void     PowerOff();
    static uint64_t ParseProcMem(const string &name, const string &buff);
    static bool     ReadFile(const string filename, string &res);
    static void     GetSystemCallback(const json &params, Response &response);
    static void     GetLogCallback(const json &params, Response &response);
    static void     GetSystemLogCallback(const json &params, Response &response);
  public :
    static string Exec(const string &command, int *resultCode = NULL);
    static json   uBusCall(const string &path, const string &method, const json &params = nullptr);
  public :
    static json   ToJSON();
    static string SystemLog();
  public :
    SystemManager();
    ~SystemManager();
};
#else // ===============================================================================================================
class SystemManager final {
  private :
    static void GetLogCallback(const json &params, Response &response);
  public :
    static json   ToJSON();
    static string SystemLog();
  public :
    SystemManager();
    ~SystemManager();
};
#endif // ==============================================================================================================