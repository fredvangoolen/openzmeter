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
#include <list>
#include <pthread.h>
#include <stdarg.h>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class LogFile final {
  private :
    enum Type { ERROR = 0, WARNING = 1, INFO = 2, DEBUG = 3 };
    typedef struct {
      uint64_t  Time;
      string    Message;
    } Message_t;
    typedef struct {
      json                 Properties;
      map<Type, Message_t> Messages;
      pthread_rwlock_t     Lock;
    } Component_t;
  private :
    static map<string, Component_t*> pComponents;
    static pthread_mutex_t           pMutex;
    static pthread_rwlock_t          pLock;
    static bool                      pConsole;
    static int                       pFile;
    static Type                      pLevel;
    static bool                      pDebug;
  private :
    static void Msg(Type type, const string &comp, const char *msg, va_list params);
  public :
    LogFile();
    virtual ~LogFile();
  public :
    static void EnableDebug();
    static void CloseConsole();
    static json GetComponent(const string &comp);
    static json ToJSON();
    static void SetValue(const string &comp, const string &prop, const json &value);
    static json GetValue(const string &comp, const string &prop);
    static void Info(const string &comp, const char *msg, ...);
    static void Warning(const string &comp, const char *msg, ...);
    static void Error(const string &comp, const char *msg, ...);
    static void Debug(const string &comp, const char *msg, ...);
};