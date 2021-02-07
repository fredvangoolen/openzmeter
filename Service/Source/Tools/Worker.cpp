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
#include "Tools.h"
// ---------------------------------------------------------------------------------------------------------------------
Worker::Worker(const string &name, const Worker::Prio prio) {
  pTerminate = false;
  pCondWake  = PTHREAD_COND_INITIALIZER;
  pCondWait  = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pThread    = 0;
  pCallback  = NULL;
  if(pthread_create(&pThread, NULL, [](void *params) { return ((Worker*)params)->Thread(); }, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  } else {
    SetThreadProperties(pThread, name, prio);
  }
}
Worker::~Worker() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pTerminate = true;
  if(pthread_cond_signal(&pCondWake) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if((pThread != 0) && (pthread_join(pThread, NULL) != 0)) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void Worker::Run(function<void()> callback) {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pTerminate == false) {
    pCallback = std::move(callback);
    pthread_cond_signal(&pCondWake);
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
bool Worker::IsWorking() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  bool Result = (pCallback != NULL);
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return Result;
}
void *Worker::Thread() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(pTerminate == false) {
    if(pCallback == NULL) {
      if(pthread_cond_wait(&pCondWake, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pCallback();
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pCallback = NULL;
      if(pthread_cond_broadcast(&pCondWait) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return NULL;
}
void Worker::Wait() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while((pTerminate == false) && (pCallback != NULL)) pthread_cond_wait(&pCondWait, &pMutex);
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void Worker::SetThreadNice(pthread_t &thread, const Worker::Prio prio) {

}
void Worker::SetThreadProperties(pthread_t thread, const string &name, const Worker::Prio prio) {
  if(pthread_setname_np(thread, name.c_str()) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));;
  rlimit      Limits;
  sched_param Param;
  if(getrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return SetThreadNice(thread, prio);
  }
  int Min = sched_get_priority_min(SCHED_FIFO);
  if(Min == -1) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return SetThreadNice(thread, prio);
  }
  int Max = sched_get_priority_max(SCHED_FIFO);
  if(Max == -1) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return SetThreadNice(thread, prio);
  }
  Param.sched_priority = (prio == IDLE) ? 1 : (prio == LOW) ? 10 : (prio == NORMAL) ? 30 : (prio == ABOUTNORMAL) ? 40 : 90;
  if(Param.sched_priority < Min) Param.sched_priority = Min;
  if(Param.sched_priority > Max) Param.sched_priority = Max;
  if(Limits.rlim_max < (rlim_t)Param.sched_priority) {
    Limits.rlim_max = Param.sched_priority;
    if(setrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return SetThreadNice(thread, prio);
    }
  }
  if(Limits.rlim_cur < (rlim_t)Param.sched_priority) {
    Limits.rlim_cur = Param.sched_priority;
    if(setrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return SetThreadNice(thread, prio);
    }
  }
  Param.sched_priority = Param.sched_priority;
  if(pthread_setschedparam(thread, SCHED_FIFO, &Param) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return SetThreadNice(thread, prio);
  }
}
// ---------------------------------------------------------------------------------------------------------------------