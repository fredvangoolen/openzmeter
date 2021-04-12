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
#include "HTTPRequest.h"
#include "nlohmann/json.hpp"
#include "cctz/time_zone.h"
#include "nlohmann/json.hpp"
#include "TaskPool.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace cctz;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
#define TELEGRAM_POLLING_SEC    3
#define TELEGRAM_UPDATE_LIMIT 100
//----------------------------------------------------------------------------------------------------------------------
class TelegramManager final {
  private :
    static pthread_mutex_t     pMutex;
    static pthread_mutex_t     pMutexConfig;
    static pthread_cond_t      pCond;
    static json                pStatus;
    static string              pToken;
    static pthread_t           pThread;
    volatile static bool       pTerminate;
    static uint32_t            pRunningWorkers;
  private :
    static void  GetTelegramCallback(HTTPRequest &req);
    static void  SetTelegramCallback(HTTPRequest &req);
    static void  Config(const json &config);
    static json  CheckConfig(const json &config);
    static json  SendRequest(SSLClient &client, const string &url, const json &params);
    static json  GetMe(SSLClient &client);
    static void *Thread_Func(void *params);
  public :
    TelegramManager();
    ~TelegramManager();
  public :
    static void  SendAlarm(const string &analyzer, const time_zone &tz, const uint64_t timestamp, const string &name, const uint8_t priority, const bool triggered);
    static bool  SendMessage(const json &msg, Client *client);
//----------------------------------------------------------------------------------------------------------------------
    static void  HandleUpdate(SSLClient *client, const json &JSON);
};