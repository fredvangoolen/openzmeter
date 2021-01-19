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
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "nlohmann/json.hpp"
#include "LogFile.h"
#include "ConfigFile.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
int                                 LogFile::pFile     = 0;
LogFile::Type                       LogFile::pLevel    = LogFile::ERROR;
bool                                LogFile::pConsole  = true;
pthread_rwlock_t                    LogFile::pLock     = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t                     LogFile::pMutex    = PTHREAD_MUTEX_INITIALIZER;
bool                                LogFile::pDebug    = false;
map<string, LogFile::Component_t*>  LogFile::pComponents;
//----------------------------------------------------------------------------------------------------------------------
LogFile::LogFile() {
  string Name = "/var/log/openZmeter.log";
  json Config = ConfigFile::ReadConfig("/Log");
  if(Config.is_object()) {
    if(Config["File"].is_string()) Name = Config["File"];
    if(Config["Level"].is_number_unsigned()) pLevel = Config["Level"];
  }
  Config = json::object();
  Config["File"] = Name;
  Config["Level"] = pLevel;
  ConfigFile::WriteConfig("/Log", Config);
  if(Name.empty() == false) {
    pFile = open(Name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(pFile < 0) LogFile::Warning("Can not create or open log file (%s)", strerror(errno));
  }
}
LogFile::~LogFile() {
  pthread_mutex_lock(&pMutex);
  if(pFile != -1) close(pFile);
  pthread_mutex_unlock(&pMutex);
}
void LogFile::SetValue(const string &comp, const string &prop, const json &value) {
  pthread_rwlock_rdlock(&pLock);
  Component_t *Comp   = NULL;
  auto         CompIt = pComponents.find(comp);
  if(CompIt != pComponents.end()) {
    Comp = CompIt->second;
    pthread_rwlock_wrlock(&Comp->Lock);
    pthread_rwlock_unlock(&pLock);
  } else {
    pthread_rwlock_unlock(&pLock);
    Comp = new Component_t;
    Comp->Lock       = PTHREAD_RWLOCK_INITIALIZER;
    Comp->Properties = json::object();
    pthread_rwlock_wrlock(&Comp->Lock);
    pthread_rwlock_wrlock(&pLock);
    pComponents.insert({comp, Comp});
    pthread_rwlock_unlock(&pLock);
  }
  Comp->Properties[json::json_pointer(prop)] = value;
  pthread_rwlock_unlock(&Comp->Lock);
}
json LogFile::GetValue(const string &comp, const string &prop) {
  pthread_rwlock_rdlock(&pLock);
  auto         CompIt = pComponents.find(comp);
  if(CompIt == pComponents.end()) {
    pthread_rwlock_unlock(&pLock);
    return nullptr;
  }
  Component_t *Comp = CompIt->second;
  pthread_rwlock_rdlock(&Comp->Lock);
  pthread_rwlock_unlock(&pLock);
  json Result = Comp->Properties[json::json_pointer(prop)];
  pthread_rwlock_unlock(&Comp->Lock);
  return Result;
}
json LogFile::ToJSON() {
  return json::object();
}
json LogFile::GetComponent(const string &comp) {
  json Result = json::array();
  //Busco comp y meto los mensajes
  return Result;
}
void LogFile::EnableDebug() {
  pDebug = true;
}
void LogFile::Debug(const string &comp, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(DEBUG, comp, msg, args);
  va_end(args);
}
void LogFile::Info(const string &comp, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(INFO, comp, msg, args);
  va_end(args);
}
void LogFile::Warning(const string &comp, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(WARNING, comp, msg, args);
  va_end(args);
}
void LogFile::Error(const string &comp, const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(ERROR, comp, msg, args);
  va_end(args);
}
void LogFile::CloseConsole() {
  pConsole = false;
}
void LogFile::Msg(Type type, const string &comp, const char *msg, va_list params) {
  if((pDebug == false) && (type == DEBUG)) return;
  if(((type > pLevel) || (pFile == -1)) && (pConsole == false)) return;
  char   DateStr[32];
  char   MsgStr[1024];
  time_t Now;
  tm     TimeStamp;
  time(&Now);
  int TimeLen = strftime(DateStr, sizeof(DateStr), "[%d/%m/%Y - %X]", localtime_r(&Now, &TimeStamp));
  int MsgLen  = vsnprintf(MsgStr, sizeof(MsgStr), msg, params);
  pthread_mutex_lock(&pMutex);
  if(pConsole == true) {
    if(type == ERROR) {
      printf("\033[1;31m%s\033[0m %s\n", DateStr, MsgStr);
    } else if(type == WARNING) {
      printf("\033[1;33m%s\033[0m %s\n", DateStr, MsgStr);
    } else if((type == DEBUG) && (pDebug == true)) {
      printf("\033[1;37m%s\033[0m %s\n", DateStr, MsgStr);
    } else if(type == INFO) {
      printf("\033[1;34m%s\033[0m %s\n", DateStr, MsgStr);
    }
    fflush(stdout);
  }
  if((type <= pLevel) && (pFile != -1)) {
    write(pFile, DateStr, TimeLen);
    write(pFile, " ", 1);
    write(pFile, MsgStr, MsgLen);
    write(pFile, "\n", 1);
  }
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------