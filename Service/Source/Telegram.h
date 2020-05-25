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
#include "cctz/time_zone.h"
#include "nlohmann/json.h"
#include "Tools.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
#define TELEGRAM_POLLING_SEC    3
#define TELEGRAM_UPDATE_LIMIT 100
//----------------------------------------------------------------------------------------------------------------------
class Telegram final {
  private :
    typedef struct {
      uint64_t              LastTime;     //Last time event
      string                Token;
      json                  JSON;
      Worker               *Thread;
      SSLClient            *Client;
    } Worker_t;
  private :
    static json                pStatus;
    static string              pToken;
    static pthread_t           pThread;
    static pthread_mutex_t     pMutex;
    volatile static bool       pTerminate;
    static list<Worker_t*>     pFreeWorkers;
    static uint32_t            pUsedWorkers;
    static Headers             pHeaders;
  private :
    static void  GetTelegramCallback(const json &params, Response &response);
    static void  SetTelegramCallback(const json &params, Response &response);
    static void  Configure(const json &config);
    static json  CheckConfig(const json &config);
    static void *Thread_Func(void *params);
  public :
    Telegram();
    ~Telegram();
  public :
    static void  SendAlarm(const string &analyzer, const time_zone &tz, const uint64_t timestamp, const string &name, const uint8_t priority, const bool triggered);
    static bool  SendMessage(const json &msg, Client *client);
    static void  PeriodicTasks();
//----------------------------------------------------------------------------------------------------------------------
    static void  HandleUpdate(SSLClient *client, const json &JSON);
};