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
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "Worker.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
Worker::Worker(const string &name, function<void(void*)> callback, int8_t nice) {
  pWorking   = false;
  pTerminate = false;
  pCondWake  = PTHREAD_COND_INITIALIZER;
  pCondWait  = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pRunParam  = NULL;
  pThread    = 0;
  pCallback = callback;
  if(pthread_create(&pThread, NULL, [](void *params) { return ((Worker*)params)->Thread(); }, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
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
void *Worker::Thread() {
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    while((pTerminate == false) && (pWorking == false)) pthread_cond_wait(&pCondWake, &pMutex);
    if((pTerminate == false) && (pWorking == true)) {
      pthread_mutex_unlock(&pMutex);
      pCallback(pRunParam);
      pthread_mutex_lock(&pMutex);
    }
    pWorking = false;
    pthread_cond_broadcast(&pCondWait);
    pthread_mutex_unlock(&pMutex);
  }
  return NULL;
}
void Worker::Wait() {
  pthread_mutex_lock(&pMutex);
  while((pTerminate == false) && (pWorking == true)) pthread_cond_wait(&pCondWait, &pMutex);
  pthread_mutex_unlock(&pMutex);
}
// ---------------------------------------------------------------------------------------------------------------------