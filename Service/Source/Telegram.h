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
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.h"
#include "Tools.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Telegram final {
  private :
    static string          pToken;
    static pthread_t       pThread;
    static pthread_mutex_t pMutex;
    static bool            pTerminate;
  private :
    static void  GetTelegramCallback(const json &params, Response &response);
    static void  SetTelegramCallback(const json &params, Response &response);
    static void *Thread_Func(void *params);
  public :
    Telegram();
    ~Telegram();
};