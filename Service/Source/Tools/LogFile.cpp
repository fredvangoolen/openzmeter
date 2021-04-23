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
#include "Tools.h"
#include "LogFile.h"
#include "ConfigFile.h"
//----------------------------------------------------------------------------------------------------------------------
int                        LogFile::pFile         = 0;
LogFile::Type              LogFile::pLevel        = LogFile::NONE;
bool                       LogFile::pConsole      = true;
pthread_rwlock_t           LogFile::pLock         = PTHREAD_RWLOCK_INITIALIZER;
pthread_mutex_t            LogFile::pMutex        = PTHREAD_MUTEX_INITIALIZER;
bool                       LogFile::pDebugEnabled = false;
uint64_t                   LogFile::pDebugCount   = 0;
deque<LogFile::Message_t>  LogFile::pDebug;
uint64_t                   LogFile::pErrorCount   = 0;
deque<LogFile::Message_t>  LogFile::pError;
uint64_t                   LogFile::pInfoCount    = 0;
deque<LogFile::Message_t>  LogFile::pInfo;
uint64_t                   LogFile::pWarningCount = 0;
deque<LogFile::Message_t>  LogFile::pWarning;
//----------------------------------------------------------------------------------------------------------------------
LogFile::LogFile() {
  LogFile::Debug("Starting LogFile...");
  string Name = "/var/log/openZmeter.log";
  json Config = ConfigFile::ReadConfig("/Log");
  if(Config.is_object()) {
    if(Config["File"].is_string())  Name = Config["File"];
    if(Config["Level"].is_string() && ((Config["Level"] == "NONE") || (Config["Level"] == "ERROR") || (Config["Level"] == "WARNING") || (Config["Level"] == "INFO") || (Config["Level"] == "DEBUG"))) pLevel = (Config["Level"] == "NONE") ? NONE : (Config["Level"] == "ERROR") ? ERROR : (Config["Level"] == "WARNING") ? WARNING : (Config["Level"] == "INFO") ? INFO : DEBUG;
  }
  Config = json::object();
  Config["File"] = Name;
  Config["Level"] = (pLevel == NONE) ? "NONE" : (pLevel == ERROR) ? "ERROR" : (pLevel == WARNING) ? "WARNING" : (pLevel == INFO) ? "INFO" : "DEBUG";
  ConfigFile::WriteConfig("/Log", Config);
  if(Name.empty() == false) {
    pFile = open(Name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(pFile < 0) LogFile::Warning("Can not create or open log file (%s)", strerror(errno));
  }
  LogFile::Debug("LogFile started");
}
LogFile::~LogFile() {
  LogFile::Debug("Stopping LogFile...");
  pthread_mutex_lock(&pMutex);
  if(pFile != -1) close(pFile);
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("LogFile stopped");
}
json LogFile::ToJSON() {
  json Result = json::object();
  pthread_rwlock_rdlock(&pLock);
  Result["Debug_Total"]   = pDebugCount;
  Result["Warning_Total"] = pWarningCount;
  Result["Info_Total"]    = pInfoCount;
  Result["Error_Total"]   = pErrorCount;
  Result["Messages"]      = json::array();
  json Tmp = json::array();
  Tmp.push_back("Type");
  Tmp.push_back("Time");
  Tmp.push_back("Message");
  Result["Messages"].push_back(Tmp);
  for(auto &Msg : pDebug)   Result["Messages"].push_back(ToJSON(DEBUG, Msg));
  for(auto &Msg : pWarning) Result["Messages"].push_back(ToJSON(WARNING, Msg));
  for(auto &Msg : pInfo)    Result["Messages"].push_back(ToJSON(INFO, Msg));
  for(auto &Msg : pError)   Result["Messages"].push_back(ToJSON(ERROR, Msg));
  pthread_rwlock_unlock(&pLock);
  return Result;
}
json LogFile::ToJSON(const Type type, const Message_t &msg) {
  json Result = json::array();
  Result.push_back((type == DEBUG) ? "DEBUG" : (type == WARNING) ? "WARNING" : (type == INFO) ? "INFO" : "ERROR");
  Result.push_back(msg.Time);
  Result.push_back(msg.Message);
  return Result;
}
void LogFile::EnableDebug() {
  pDebugEnabled = true;
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
  va_list  Args;
  va_copy(Args, params);
  uint64_t Time   = Tools::GetTime64();
  int      MsgLen = vsnprintf(NULL, 0, msg, Args);
  va_end(Args);
  char    *MsgStr = (char*)malloc(MsgLen + 1);
  vsnprintf(MsgStr, MsgLen + 1, msg, params);
  pthread_rwlock_wrlock(&pLock);
  if(type == ERROR) {
    if(pError.size() > MaxMessages) pError.pop_front();
    pError.emplace_back(Time, MsgStr);
    pErrorCount++;
  } else if(type == WARNING) {
    if(pWarning.size() > MaxMessages) pWarning.pop_front();
    pWarning.emplace_back(Time, MsgStr);
    pWarningCount++;
  } else if(type == DEBUG) {
    if(pDebug.size() > MaxMessages) pDebug.pop_front();
    pDebug.emplace_back(Time, MsgStr);
    pDebugCount++;
  } else if(type == INFO) {
    if(pInfo.size() > MaxMessages) pInfo.pop_front();
    pInfo.emplace_back(Time, MsgStr);
    pInfoCount++;
  }
  pthread_rwlock_unlock(&pLock);
  if((pDebugEnabled == false) && (type == DEBUG)) {
    free(MsgStr);
    return;
  }
  if(((type > pLevel) || (pFile == -1)) && (pConsole == false)) {
    free(MsgStr);
    return;
  }
  time_t   Now  = Time / 1000;
  tm       TimeStamp;
  char     DateStr[32];
  int TimeLen = strftime(DateStr, sizeof(DateStr), "[%d/%m/%Y - %X]", localtime_r(&Now, &TimeStamp));
  pthread_mutex_lock(&pMutex);
  if(pConsole == true) {
    if(type == ERROR) {
      printf("\033[1;31m%s\033[0m - %s\n", DateStr, MsgStr);
    } else if(type == WARNING) {
      printf("\033[1;33m%s\033[0m - %s\n", DateStr, MsgStr);
    } else if((type == DEBUG) && (pDebugEnabled == true)) {
      printf("\033[1;37m%s\033[0m - %s\n", DateStr, MsgStr);
    } else if(type == INFO) {
      printf("\033[1;34m%s\033[0m - %s\n", DateStr, MsgStr);
    }
    fflush(stdout);
  }
  if((type <= pLevel) && (pFile != -1)) {
    write(pFile, "[", 1);
    write(pFile, DateStr, TimeLen);
    write(pFile, "] ", 2);
    write(pFile, MsgStr, MsgLen);
    write(pFile, "\n", 1);
  }
  pthread_mutex_unlock(&pMutex);
  free(MsgStr);
}
//----------------------------------------------------------------------------------------------------------------------