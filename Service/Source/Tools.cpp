/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <sys/time.h>
#include <stdarg.h>
#include <fcntl.h>
#include "Tools.h"



//añadido para probar el envio
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
//----------------------------------------------------------------------------------------------------------------------
RWLocks::RWLocks() {
  pMutex = PTHREAD_MUTEX_INITIALIZER;
  pReadersCondition = PTHREAD_COND_INITIALIZER;
  pWritersCondition = PTHREAD_COND_INITIALIZER;
  pWriters = 0;
  pReaders = 0;
  pActiveWriter = false;
}
void RWLocks::ReadLock() {
  pthread_mutex_lock(&pMutex);
  while(pWriters != 0) pthread_cond_wait(&pReadersCondition, &pMutex);
  pReaders++;
  pthread_mutex_unlock(&pMutex);
}
void RWLocks::ReadUnlock() {
  pthread_mutex_lock(&pMutex);
  pReaders--;
  pthread_mutex_unlock(&pMutex);
  pthread_cond_signal(&pWritersCondition);
}
void RWLocks::WriteLock() {
  pthread_mutex_lock(&pMutex);
  pWriters++;
  while((pReaders != 0) || (pActiveWriter != false)) pthread_cond_wait(&pWritersCondition, &pMutex);
  pActiveWriter = true;
  pthread_mutex_unlock(&pMutex);
}
void RWLocks::WriteUnlock() {
  pthread_mutex_lock(&pMutex);
  pWriters--;
  pActiveWriter = false;
  if(pWriters > 0) 
    pthread_cond_signal(&pWritersCondition);
  else
    pthread_cond_broadcast(&pReadersCondition);
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------
InstanceLocks::InstanceLocks() {
  pMutex = PTHREAD_MUTEX_INITIALIZER;
  pCond = PTHREAD_COND_INITIALIZER;
  pReferences = 0;
}
void InstanceLocks::WaitInstances() {
  pthread_mutex_lock(&pMutex);
  while(pReferences != 0) pthread_cond_wait(&pCond, &pMutex);
  pthread_mutex_unlock(&pMutex);
}
void InstanceLocks::Reference() {
  pthread_mutex_lock(&pMutex);
  pReferences++;
  pthread_mutex_unlock(&pMutex);
}
void InstanceLocks::Dereference() {
  pthread_mutex_lock(&pMutex);
  pReferences--;
  if(pReferences == 0) pthread_cond_signal(&pCond);
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
    if(pTerminate == true) return false;
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
WorkerLocks::WorkerLocks() {
  pWorking = false;
  pTerminate = false;
  pCond = PTHREAD_COND_INITIALIZER;
  pMutex = PTHREAD_MUTEX_INITIALIZER;
}
void WorkerLocks::Abort(pthread_t *thread) {
  pTerminate = true;
  pthread_cond_signal(&pCond);
  if(*thread == 0) return;
  if(pthread_join(*thread, NULL) != 0) LOGFile::Error("%s:%d -> (Unknown)\n", __func__, __LINE__);
  *thread = 0;
}
void WorkerLocks::WakeWorker() {
  pthread_mutex_lock(&pMutex);
  if(pTerminate == true) return;
  pWorking = true;
  pthread_mutex_unlock(&pMutex);
  pthread_cond_signal(&pCond);
}
bool WorkerLocks::WorkerWaitTask() {
  pthread_mutex_lock(&pMutex);
  while(pWorking == false) {
    if(pTerminate == true) {
      pTerminate = false;
      pthread_mutex_unlock(&pMutex);
      return false;
    }
    pthread_cond_wait(&pCond, &pMutex);
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
void WorkerLocks::WorkerEndsTask() {
  pthread_mutex_lock(&pMutex);
  pWorking = false;
  pthread_mutex_unlock(&pMutex);
  pthread_cond_signal(&pCond);
}
void WorkerLocks::WaitWorker() {
  pthread_mutex_lock(&pMutex);
  while(pWorking == true) {
    if(pTerminate == true) return;
    pthread_cond_wait(&pCond, &pMutex);
  } 
  pthread_mutex_unlock(&pMutex);
}
bool WorkerLocks::IsWorking() {
  bool Result;
  pthread_mutex_lock(&pMutex);
  Result = (pWorking == false);
  pthread_mutex_unlock(&pMutex);
  return Result;
}
//----------------------------------------------------------------------------------------------------------------------
ConfigManager   *ConfigManager::pInstance = NULL;
pthread_mutex_t  ConfigManager::pMutex = PTHREAD_MUTEX_INITIALIZER;
ConfigManager::ConfigManager(string name) {
  pthread_mutex_lock(&pMutex);
  if(pInstance != NULL) {
    delete pInstance;
    pInstance = NULL;
  }
  pFile = open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(pFile < 0) {
    LOGFile::Error("Can not open config file (%s)\n", strerror(errno));
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
  pInstance = this;
  pthread_mutex_unlock(&pMutex);
}
ConfigManager::~ConfigManager() {
  pthread_mutex_lock(&pMutex);
  close(pFile);
  pInstance = NULL;
  pthread_mutex_unlock(&pMutex);
}
bool ConfigManager::WriteConfig(const string &path, const json &val) {
  pthread_mutex_lock(&pMutex);
  LOGFile::Debug("Saving '%s' settings\n", path.c_str());
  if(pInstance == NULL) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  
  LOGFile::Debug("merge path: %s\n", path.c_str());
  LOGFile::Debug("merge contents: %s\n", val.dump().c_str());
  pInstance->pConfig[json::json_pointer(path)].merge_patch(val);
  LOGFile::Debug("merge result: %s\n",   pInstance->pConfig[json::json_pointer(path)].dump().c_str());
  if(pInstance->pFile < 0) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  lseek(pInstance->pFile, 0, SEEK_SET);
  string Write = pInstance->pConfig.dump(2);
  if(write(pInstance->pFile, Write.c_str(), Write.size()) < 0) {
    LOGFile::Error("Can not write configuration to file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(ftruncate(pInstance->pFile, Write.size()) < 0 ) {
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
  if((pInstance == NULL) || (pInstance->pConfig.is_object() == false)) {
    pthread_mutex_unlock(&pMutex);
    return nullptr;
  }
  json Result = pInstance->pConfig[json::json_pointer(path)];
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
LOGFile        *LOGFile::pInstance = NULL;
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
  pthread_mutex_lock(&pMutex);
  if(pInstance != NULL) delete pInstance;
  pInstance = this;
  pthread_mutex_unlock(&pMutex);
}
LOGFile::~LOGFile() {
  pthread_mutex_lock(&pMutex);
  pInstance = NULL;
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
  if(pInstance != NULL) {
    if((type < pInstance->pLevel) && (pInstance->pFile != NULL)) fprintf(pInstance->pFile, "[%s]  %s", DateStr, MsgStr);
  }
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
  for(uint32_t i = 0; i < pFiles.size(); i++) {
    if(pFiles[i].Name != name) continue;
    *size = pFiles[i].Size;
    return pFiles[i].Ptr;
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------

 
//añadido para probar el envio
//  #include <arpa/inet.h>
//      #include <netinet/in.h>
//      #include <stdio.h>
//      #include <sys/types.h>
//      #include <sys/socket.h>
//      #include <unistd.h>

//----------------------------------------------------------------------------------------------------------------------
uint64_t Tools::GetTime() {
  struct timeval Time;
  if(gettimeofday(&Time, NULL) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000;
}
void Tools::SendUDP(const json &obj) {
  struct sockaddr_in Client;
  int Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(Socket == -1) {
    LOGFile::Error("cant open socket\n");
    return;
  }
  memset((char *)&Client, 0, sizeof(Client));
  Client.sin_family = AF_INET;
  Client.sin_port = htons(8888);
  if(inet_aton("127.0.0.1", &Client.sin_addr) == 0) {
    LOGFile::Error("inet_aton() failed\n");
    close(Socket);
    return;
  }
  string Data = obj.dump();
  if(sendto(Socket, Data.c_str(), Data.size(), 0, (sockaddr*)&Client, sizeof(Client)) == -1) {
    LOGFile::Error("sendto()\n");
  }
  close(Socket);
}