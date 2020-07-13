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
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
Worker::Worker(const string &name, function<void(void*)> callback, int8_t nice) {
  pWorking   = false;
  pTerminate = false;
  pCondWake  = PTHREAD_COND_INITIALIZER;
  pCondWait  = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pRunParam  = NULL;
  pThread    = 0;
  pCallback = callback;
  if(pthread_create(&pThread, NULL, Thread, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
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
      This->pCallback(This->pRunParam);
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
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  } else if((pTerminate == true) || (Error == ETIMEDOUT)) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pConsumers++;
  while((Error == 0) && (pDataReady == false) && (pTerminate == false)) Error = pthread_cond_timedwait(&pConsumerWaitCond, &pMutex, &To);
  if((Error != 0) && (Error != ETIMEDOUT)) {
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
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
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_nsec / 1000000;
}
string Tools::StrTrim(const string &str) {
  string chars = "\t\n\v\f\r ";
  size_t first = str.find_first_not_of(chars);
  if(first == std::string::npos) return "";
  size_t last = str.find_last_not_of(chars);
  return str.substr(first, (last-first+1));
}
void Tools::RunDaemon() {
  LogFile::Info("Move to background task");
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  LogFile::CloseConsole();
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}