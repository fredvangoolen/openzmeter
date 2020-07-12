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
#include "nlohmann/json.h"
#include "LogFile.h"
#include "ConfigManager.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
int             LogFile::pFile     = 0;
LogFile::Type   LogFile::pLevel    = LogFile::ERROR;
bool            LogFile::pConsole  = true;
pthread_mutex_t LogFile::pMutex    = PTHREAD_MUTEX_INITIALIZER;
bool            LogFile::pDebug    = false;
//----------------------------------------------------------------------------------------------------------------------
LogFile::LogFile() {
  string Name = "/var/log/openZmeter.log";
  json Config = ConfigManager::ReadConfig("/Log");
  if(Config.is_object()) {
    if(Config["File"].is_string()) Name = Config["File"];
    if(Config["Level"].is_number_unsigned()) pLevel = Config["Level"];
  }
  Config = json::object();
  Config["File"] = Name;
  Config["Level"] = pLevel;
  ConfigManager::WriteConfig("/Log", Config);
  if(Name.empty() == false) {
    pFile = open(Name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(pFile < 0) LogFile::Warning("Can not create or open log file (%s)\n", strerror(errno));
  }
}
LogFile::~LogFile() {
  pthread_mutex_lock(&pMutex);
  if(pFile != -1) close(pFile);
  pthread_mutex_unlock(&pMutex);
}
void LogFile::EnableDebug() {
  pDebug = true;
}
void LogFile::Debug(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(DEBUG, msg, args);
  va_end(args);
}
void LogFile::Info(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(INFO, msg, args);
  va_end(args);
}
void LogFile::Warning(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(WARNING, msg, args);
  va_end(args);
}
void LogFile::Error(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(ERROR, msg, args);
  va_end(args);
}
void LogFile::CloseConsole() {
  pConsole = false;
}
void LogFile::Msg(Type type, const char *msg, va_list params) {
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