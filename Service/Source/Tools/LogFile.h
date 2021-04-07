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
#include <map>
#include <deque>
#include <pthread.h>
#include <stdarg.h>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class LogFile final {
  private :
    static const uint16_t MaxMessages = 100;
  private :
    enum Type { NONE, ERROR, WARNING, INFO, DEBUG };
    typedef struct Message_t {
      uint64_t  Time;
      string    Message;
      Message_t(const uint64_t time, const char *msg) { Time = time; Message = msg; }
    } Message_t;
  private :
    static deque<Message_t>  pDebug;
    static uint64_t          pDebugCount;
    static deque<Message_t>  pWarning;
    static uint64_t          pWarningCount;
    static deque<Message_t>  pInfo;
    static uint64_t          pInfoCount;
    static deque<Message_t>  pError;
    static uint64_t          pErrorCount;
    static pthread_mutex_t   pMutex;
    static pthread_rwlock_t  pLock;
    static bool              pConsole;
    static int               pFile;
    static Type              pLevel;
    static bool              pDebugEnabled;
  private :
    static void Msg(Type type, const char *msg, va_list params);
    static json ToJSON(const Type type, const Message_t &msg);
  public :
    LogFile();
    virtual ~LogFile();
  public :
    static void EnableDebug();
    static void CloseConsole();
    static json ToJSON();
    static void Info(const char *msg, ...);
    static void Warning(const char *msg, ...);
    static void Error(const char *msg, ...);
    static void Debug(const char *msg, ...);
};