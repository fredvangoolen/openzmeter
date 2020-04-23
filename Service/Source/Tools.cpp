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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Worker::Worker(const string &name, void *param, void (*callback)(void*, void*), int8_t nice) {
  pWorking       = false;
  pTerminate     = false;
  pCondWake      = PTHREAD_COND_INITIALIZER;
  pCondWait      = PTHREAD_COND_INITIALIZER;
  pMutex         = PTHREAD_MUTEX_INITIALIZER;
  pCreationParam = param;
  pRunParam      = NULL;
  pCallback      = callback;
  pThread        = 0;
  if(pthread_create(&pThread, NULL, Thread, (void*)this) != 0) {
    pThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return;
  }
  if(nice != 0) setpriority(PRIO_PROCESS, pThread, -nice);
  pthread_setname_np(pThread, name.c_str());
}
Worker::~Worker() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCondWake);
  pthread_mutex_unlock(&pMutex);
  if(pThread != 0) pthread_join(pThread, NULL);
}
void Worker::Run(void *param) {
  pthread_mutex_lock(&pMutex);
  pWorking = true;
  pRunParam = param;
  pthread_cond_signal(&pCondWake);
  pthread_mutex_unlock(&pMutex);
}
bool Worker::IsWorking() {
  pthread_mutex_lock(&pMutex);
  bool Result = pWorking;
  pthread_mutex_unlock(&pMutex);
  return Result;
}
void *Worker::Thread(void *args) {
  Worker *This = (Worker*)args;
  while(This->pTerminate == false) {
    pthread_mutex_lock(&This->pMutex);
    while((This->pTerminate == false) && (This->pWorking == false)) pthread_cond_wait(&This->pCondWake, &This->pMutex);
    if((This->pTerminate == false) && (This->pWorking == true)) {
      pthread_mutex_unlock(&This->pMutex);
      This->pCallback(This->pCreationParam, This->pRunParam);
      pthread_mutex_lock(&This->pMutex);
    }
    This->pWorking = false;
    pthread_cond_broadcast(&This->pCondWait);
    pthread_mutex_unlock(&This->pMutex);
  }
  return NULL;
}
void Worker::Wait() {
  pthread_mutex_lock(&pMutex);
  while((pTerminate == false) && (pWorking == true)) pthread_cond_wait(&pCondWait, &pMutex);
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
}
void RWLocks::ReadLock() {
  pthread_mutex_lock(&pMutex);
  while((pWriters > 0) || (pWriter == true)) pthread_cond_wait(&pReadersCondition, &pMutex);
  pReaders++;
  pthread_mutex_unlock(&pMutex);
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
  } else {
    pthread_cond_broadcast(&pReadersCondition);
  }
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
ProviderLocks::ProviderLocks() {
  pConsumers         = 0;
  pDataReady         = false;
  pTerminate         = false;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pConsumerEnterCond = PTHREAD_COND_INITIALIZER;
  pConsumerWaitCond  = PTHREAD_COND_INITIALIZER;
  pProducerCond      = PTHREAD_COND_INITIALIZER;
}
ProviderLocks::~ProviderLocks() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_broadcast(&pConsumerEnterCond);
  pthread_cond_broadcast(&pConsumerWaitCond);
  pthread_cond_broadcast(&pProducerCond);
  pthread_mutex_unlock(&pMutex);
}
void ProviderLocks::ProviderWrite() {
  pthread_mutex_lock(&pMutex);
  pDataReady = true;
  pthread_cond_broadcast(&pConsumerWaitCond);
  pthread_mutex_unlock(&pMutex);
}
void ProviderLocks::ProviderWait() {
  pthread_mutex_lock(&pMutex);
  while(pConsumers > 0) pthread_cond_wait(&pProducerCond, &pMutex);
  pDataReady = false;
  pthread_cond_broadcast(&pConsumerEnterCond);
  pthread_mutex_unlock(&pMutex);
}
bool ProviderLocks::ConsumerWait(uint32_t timeoutSec) {
  int Error = 0;
  timespec To;
  clock_gettime(CLOCK_REALTIME, &To);
  To.tv_sec += timeoutSec;
  pthread_mutex_lock(&pMutex);
  while((Error == 0) && (pDataReady == true) && (pTerminate == false)) Error = pthread_cond_timedwait(&pConsumerEnterCond, &pMutex, &To);
  if((Error != 0) && (Error != ETIMEDOUT)) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  } else if((pTerminate == true) || (Error == ETIMEDOUT)) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pConsumers++;
  while((Error == 0) && (pDataReady == false) && (pTerminate == false)) Error = pthread_cond_timedwait(&pConsumerWaitCond, &pMutex, &To);
  if((Error != 0) && (Error != ETIMEDOUT)) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  } else if((pTerminate == true) || (Error == ETIMEDOUT)) {
    pConsumers--;
    if(pConsumers == 0) pthread_cond_signal(&pProducerCond);
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
void ProviderLocks::ConsumerEnds() {
  pthread_mutex_lock(&pMutex);
  pConsumers--;
  if(pConsumers == 0) pthread_cond_signal(&pProducerCond);
  pthread_mutex_unlock(&pMutex);
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
bool            LOGFile::pDebug    = false;
//----------------------------------------------------------------------------------------------------------------------
LOGFile::LOGFile() {
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
void LOGFile::EnableDebug() {
  pDebug = true;
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
void LOGFile::CloseConsole() {
  pConsole = false;
}
void LOGFile::Msg(Type type, const char *msg, va_list params) {
  if(((type > pLevel) || (pFile == NULL)) && (pConsole == false)) return;
  char   DateStr[32];
  char   MsgStr[1024];
  time_t Now;
  tm     TimeStamp;
  time(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X]", localtime_r(&Now, &TimeStamp));
  vsnprintf(MsgStr, sizeof(MsgStr), msg, params);
  pthread_mutex_lock(&pMutex);
  if(pConsole == true) {
    if(type == ERROR) {
      printf("\033[1;31m%s\033[0m %s", DateStr, MsgStr);
    } else if(type == WARNING) {
      printf("\033[1;33m%s\033[0m %s", DateStr, MsgStr);
    } else if((type == DEBUG) && (pDebug == true)) {
      printf("\033[1;37m%s\033[0m %s", DateStr, MsgStr);
    } else if(type == INFO) {
      printf("\033[1;34m%s\033[0m %s", DateStr, MsgStr);
    }
    fflush(stdout);
  }
  if((type <= pLevel) && (pFile != NULL)) fprintf(pFile, "[%s]  %s", DateStr, MsgStr);
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
vector<ResourceLoader::File_t> ResourceLoader::pFiles;
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
uint64_t Tools::GetTime64() {
  struct timespec Time;
  if(clock_gettime(CLOCK_REALTIME, &Time) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
//  struct timeval Time;
//  if(gettimeofday(&Time, NULL) != 0) {
//    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return 0;
//  }
//  return (uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000;
}
void Tools::RunDaemon() {
  LOGFile::Info("Move to background task\n");
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  LOGFile::CloseConsole();
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}