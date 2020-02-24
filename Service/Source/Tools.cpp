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
#include <sys/time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Worker::Worker(const string &name, void *param, void (*callback)(void*, void*)) {
  pWorking       = false;
  pTerminate     = false;
  pCond          = PTHREAD_COND_INITIALIZER;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
  pCreationParam = param;
  pRunParam      = NULL;
  pCallback      = callback;
  if(pthread_create(&pThread, NULL, Thread, (void*)this) != 0) {
    pThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, name.c_str());
}
Worker::~Worker() {
  pTerminate = true;
  pthread_cond_signal(&pCond);
  if(pThread == 0) return;
  if(pthread_join(pThread, NULL) != 0) LOGFile::Error("%s:%d -> (Unknown)\n", __func__, __LINE__);
}
void Worker::Run(void *param) {
  pthread_mutex_lock(&pMutex);
  if(pTerminate == true) {
    pthread_mutex_unlock(&pMutex);
    return;
  }
  pWorking = true;
  pRunParam = param;
  pthread_mutex_unlock(&pMutex);
  pthread_cond_signal(&pCond);
}
bool Worker::IsWorking() {
  bool Result;
  pthread_mutex_lock(&pMutex);
  Result = pWorking;
  pthread_mutex_unlock(&pMutex);
  return Result;
}
void *Worker::Thread(void *args) {
  Worker *This = (Worker*)args;
  while(This->pTerminate == false) {
    pthread_mutex_lock(&This->pMutex);
    while(This->pWorking == false) {
      if(This->pTerminate == true) {
        This->pWorking = false;
        pthread_mutex_unlock(&This->pMutex);
        return NULL;
      }
      pthread_cond_wait(&This->pCond, &This->pMutex);
    }
    pthread_mutex_unlock(&This->pMutex);
    This->pCallback(This->pCreationParam, This->pRunParam);
    pthread_mutex_lock(&This->pMutex);
    This->pWorking = false;
    pthread_mutex_unlock(&This->pMutex);
    pthread_cond_signal(&This->pCond);
  }
  return NULL;
}
void Worker::Wait() {
  pthread_mutex_lock(&pMutex);
  while(pWorking == true) pthread_cond_wait(&pCond, &pMutex);
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
RWLocks::RWLocks() {
  pMutex = PTHREAD_MUTEX_INITIALIZER;
  pReadersCondition = PTHREAD_COND_INITIALIZER;
  pWritersCondition = PTHREAD_COND_INITIALIZER;
  pWriters          = 0;
  pReaders          = 0;
  pWriter           = false;
}
RWLocks::~RWLocks() {
  pthread_mutex_destroy(&pMutex);
  pthread_cond_destroy(&pReadersCondition);
  pthread_cond_destroy(&pWritersCondition);
}
void RWLocks::ReadLock() {
  pthread_mutex_lock(&pMutex);
  while((pWriters > 0) || (pWriter == true)) pthread_cond_wait(&pReadersCondition, &pMutex);
  pReaders++;
  if(pReaders > 10) {
    printf("%d lectores\n", pReaders);
  }
  pthread_mutex_unlock(&pMutex);
}
bool RWLocks::TryReadLock() {
  pthread_mutex_lock(&pMutex);
  if((pWriters > 0) || (pWriter == true)) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  while((pWriters > 0) || (pWriter == true)) pthread_cond_wait(&pReadersCondition, &pMutex);
  pReaders++;
  if(pReaders > 10) {
    printf("%d lectores\n", pReaders);
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
void RWLocks::WriteLock() {
  pthread_mutex_lock(&pMutex);
  pWriters++;
  while((pReaders > 0) || (pWriter == true)) pthread_cond_wait(&pWritersCondition, &pMutex);
  pWriters--;
  pWriter = true;
  pthread_mutex_unlock(&pMutex);
}
void RWLocks::Unlock() {
  pthread_mutex_lock(&pMutex);
  if(pWriter == true) {
    pWriter = false;
  } else {
    pReaders--;
  }
  if(pWriters > 0) {
    pthread_cond_signal(&pWritersCondition);
  } else if(pReaders > 0) {
    pthread_cond_broadcast(&pReadersCondition);
  }
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
ProviderLocks::ProviderLocks() {
  pWaitingThreads      = 0;
  pDataReady           = false;
  pTerminate           = false;
  pWaitingThreadsMutex = PTHREAD_MUTEX_INITIALIZER;
  pDataReadyMutex      = PTHREAD_MUTEX_INITIALIZER;
  pWaitingThreadsCond  = PTHREAD_COND_INITIALIZER;
  pDataReadyCond       = PTHREAD_COND_INITIALIZER;
}
void ProviderLocks::Abort() {
  pTerminate = true;
  pthread_cond_broadcast(&pDataReadyCond);
}
void ProviderLocks::ProviderWrite() {
  pthread_mutex_lock(&pDataReadyMutex);
  pDataReady = true;
  pthread_mutex_unlock(&pDataReadyMutex);
  pthread_cond_broadcast(&pDataReadyCond);
}
void ProviderLocks::ProviderWait() {
  pthread_mutex_lock(&pWaitingThreadsMutex);
  while(pWaitingThreads != 0) pthread_cond_wait(&pWaitingThreadsCond, &pWaitingThreadsMutex);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  pthread_mutex_lock(&pDataReadyMutex);
  pDataReady = false;
  pthread_mutex_unlock(&pDataReadyMutex);
}
bool ProviderLocks::ConsumerWait(uint timeoutMS) {
  int Error = 0;
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads++;
  pthread_mutex_unlock(&pWaitingThreadsMutex);
  pthread_mutex_lock(&pDataReadyMutex);
  while((pDataReady == false) && (Error == 0)) {
    if(pTerminate == true) {
      pthread_mutex_unlock(&pDataReadyMutex);
      ConsumerEnds();
      return false;
    }
    struct timeval  Current;
    struct timespec Timeout;
    gettimeofday(&Current, NULL);
    Timeout.tv_sec  = Current.tv_sec + (timeoutMS / 1000);
    Timeout.tv_nsec = (timeoutMS % 1000) * 1000000;
    Error = pthread_cond_timedwait(&pDataReadyCond, &pDataReadyMutex, &Timeout);
  }
  pthread_mutex_unlock(&pDataReadyMutex);
  if(Error != 0) {
    ConsumerEnds();
    return false;
  }
  return true;
}
void ProviderLocks::ConsumerEnds() {
  pthread_mutex_lock(&pWaitingThreadsMutex);
  pWaitingThreads--;
  pthread_cond_signal(&pWaitingThreadsCond);
  pthread_mutex_unlock(&pWaitingThreadsMutex);
}
//----------------------------------------------------------------------------------------------------------------------
int             ConfigManager::pFile   = -1;
pthread_mutex_t ConfigManager::pMutex  = PTHREAD_MUTEX_INITIALIZER;
json            ConfigManager::pConfig = nullptr;
//----------------------------------------------------------------------------------------------------------------------
ConfigManager::ConfigManager(const string &name) {
  pthread_mutex_lock(&pMutex);
  pFile = open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(pFile < 0) {
    LOGFile::Error("Can not open config file (%s)\n", strerror(errno));
    pFile = -1;
    pthread_mutex_unlock(&pMutex);
    return;
  }
  string Read;
  char Buff[256];
  ssize_t Len;
  do {
    Len = read(pFile, Buff, sizeof(Buff));
    Read += string(Buff, Len);
  } while(Len > 0);
  pConfig = json::parse(Read, NULL, false);
  if(pConfig.is_discarded()) pConfig = json::object();
  pthread_mutex_unlock(&pMutex);
}
ConfigManager::~ConfigManager() {
  pthread_mutex_lock(&pMutex);
  if(pFile != -1) close(pFile);
  pFile = -1;
  pthread_mutex_unlock(&pMutex);
}
bool ConfigManager::WriteConfig(const string &path, const json &val) {
  pthread_mutex_lock(&pMutex);
  LOGFile::Debug("Saving '%s' settings\n", path.c_str());
  if(pFile == -1) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pConfig[json::json_pointer(path)] = val;
  lseek(pFile, 0, SEEK_SET);
  string Write = pConfig.dump(2);
  if(write(pFile, Write.c_str(), Write.size()) < 0) {
    LOGFile::Error("Can not write configuration to file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(ftruncate(pFile, Write.size()) < 0 ) {
    LOGFile::Error("Can not write configuration to file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
json ConfigManager::ReadConfig(const string &path) {
  pthread_mutex_lock(&pMutex);
  LOGFile::Debug("Reading '%s' settings\n", path.c_str());
  if((pFile == -1) || (pConfig.is_object() == false)) {
    pthread_mutex_unlock(&pMutex);
    return nullptr;
  }
  json Result = pConfig[json::json_pointer(path)];
  pthread_mutex_unlock(&pMutex);
  return Result;
}
//----------------------------------------------------------------------------------------------------------------------
PIDFile::PIDFile() {
  pFile = "/var/run/openZmeter.pid";
  json Config = ConfigManager::ReadConfig("/PidFile");
  if(Config.is_string()) pFile = Config;
  char PID_Number[16];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(!pFile.empty()) remove(pFile.c_str());
  int File = open(pFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) LOGFile::Warning("Can not create PID file (%s)\n", strerror(errno));
  if(write(File, PID_Number, Len) <= 0) LOGFile::Warning("Can not create PID file (%s)\n", strerror(errno));
  if(File > 0) close(File);
}
PIDFile::~PIDFile() {
  if(!pFile.empty()) remove(pFile.c_str());
}
//----------------------------------------------------------------------------------------------------------------------
FILE           *LOGFile::pFile     = NULL;
LOGFile::Type   LOGFile::pLevel    = LOGFile::DEBUG;
bool            LOGFile::pConsole  = true;
pthread_mutex_t LOGFile::pMutex    = PTHREAD_MUTEX_INITIALIZER;
LOGFile::LOGFile() {
  string Name = "/var/log/openZmeter.log";
  pLevel = DEBUG;
  json Config = ConfigManager::ReadConfig("/Log");
  if(Config.is_object()) {
    if(Config["File"].is_string()) Name = Config["File"];
    if(Config["Level"].is_number_unsigned()) pLevel = Config["Level"];
  }
  Config = json::object();
  Config["File"] = Name;
  Config["Level"] = pLevel;
  ConfigManager::WriteConfig("/Log", Config);
  pFile = NULL;
  if(Name.empty() == false) {
    pFile = fopen(Name.c_str(), "at");
    if(!pFile) pFile = fopen(Name.c_str(), "wt");
    if(pFile == NULL) LOGFile::Warning("Can not create or open log file (%s)\n", strerror(errno));
  }
}
LOGFile::~LOGFile() {
  pthread_mutex_lock(&pMutex);
  if(pFile != NULL) fclose(pFile);
  pthread_mutex_unlock(&pMutex);
}
void LOGFile::Debug(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(DEBUG, msg, args);
  va_end(args);
}
void LOGFile::Info(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(INFO, msg, args);
  va_end(args);
}
void LOGFile::Warning(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(WARNING, msg, args);
  va_end(args);
}
void LOGFile::Error(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(ERROR, msg, args);
  va_end(args);
}
void LOGFile::Msg(Type type, const char *msg, va_list params) {
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[32];
  char MsgStr[1024];
  pthread_mutex_lock(&pMutex);
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X]", TimeStamp);
  vsnprintf(MsgStr, sizeof(MsgStr), msg, params);
  if(pConsole == true) {
    if(type == ERROR) {
      if(printf("\033[1;31m%s\033[0m %s", DateStr, MsgStr) == 0) pConsole = false;
    } else if(type == WARNING) {
      if(printf("\033[1;33m%s\033[0m %s", DateStr, MsgStr) == 0) pConsole = false;
    } else if(type == DEBUG) {
      if(printf("\033[1;37m%s\033[0m %s", DateStr, MsgStr) == 0) pConsole = false;
    } else if(type == INFO) {
      if(printf("\033[1;34m%s\033[0m %s", DateStr, MsgStr) == 0) pConsole = false;
    }
  }
  if(pConsole == true) fflush(stdout);
  if((type < pLevel) && (pFile != NULL)) fprintf(pFile, "[%s]  %s", DateStr, MsgStr);
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
vector<ResourceLoader::File_t> ResourceLoader::pFiles;;
uint8_t *ResourceLoader::GetFile(string name, size_t *size) {
  *size = 0;
  if(pFiles.size() == 0) {
    char *Ptr = _binary_ResourceFiles_start;
    uint32_t FileCount = atoi(Ptr);
    pFiles.reserve(FileCount);
    Ptr = &Ptr[strlen(Ptr) + 1];
    for(uint32_t i = 0; i < FileCount; i++) {
      File_t File;
      File.Name = Ptr;
      Ptr = &Ptr[strlen(Ptr) + 1];
      File.Size = atoi(Ptr);
      Ptr = &Ptr[strlen(Ptr) + 1];
      File.Ptr = (uint8_t*)Ptr;
      Ptr = &Ptr[File.Size];
      pFiles.push_back(File);
    }
  }
  for(File_t &File : pFiles) {
    if(File.Name != name) continue;
    *size = File.Size;
    return File.Ptr;
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------
termios Tools::pConsoleAttr;
//----------------------------------------------------------------------------------------------------------------------
void Tools::ResetConsole() {
  tcsetattr (STDIN_FILENO, TCSANOW, &pConsoleAttr);
}
void Tools::DisableEcho() {
  struct termios tattr;
  if(!isatty(STDIN_FILENO)) return;
  tcgetattr(STDIN_FILENO, &pConsoleAttr);
  atexit(ResetConsole);
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON | ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}
uint64_t Tools::GetTime() {
  struct timeval Time;
  if(gettimeofday(&Time, NULL) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000;
}