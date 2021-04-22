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
#include "TaskPool.h"
#include "LogFile.h"
#include "Tools.h"
#include "MessageLoop.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t          TaskPool::pMutex        = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t           TaskPool::pCond         = PTHREAD_COND_INITIALIZER;
uint32_t                 TaskPool::pRunningTasks = 0;
int                      TaskPool::pMinPrio      = -1;
int                      TaskPool::pMaxPrio      = -1;
volatile bool            TaskPool::pUsePrio      = false;
deque<TaskPool::Task_t*> TaskPool::pFreeTasks;
// ---------------------------------------------------------------------------------------------------------------------
TaskPool::TaskPool() {
  LogFile::Debug("Starting TaskPool...");
  rlimit Limits;
  if(getrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  pMinPrio = sched_get_priority_min(SCHED_FIFO);
  if(pMinPrio == -1) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  if(Limits.rlim_cur < (rlim_t)pMinPrio) {
    Limits.rlim_cur = pMinPrio;
    if(setrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return;
    }
  }
  pMaxPrio = sched_get_priority_max(SCHED_FIFO);
  if(pMaxPrio == -1) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(Limits.rlim_max < (rlim_t)pMaxPrio) {
    Limits.rlim_max = pMaxPrio;
    if(setrlimit(RLIMIT_RTPRIO, &Limits) != 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      return;
    }
  }
  MessageLoop::RegisterListener("TaskPool_PeriodicTasks", [&]() { this->PeriodicTasks(); }, 60000);
  LogFile::Debug("TaskPool started");
}
TaskPool::~TaskPool() {
  LogFile::Debug("Stoping TaskPool...");
  pthread_mutex_lock(&pMutex);
  MessageLoop::UnregisterListener("TaskPool_PeriodicTasks");
  while(pRunningTasks > 0) pthread_cond_wait(&pCond, &pMutex);
  while(pFreeTasks.size() > 0) {
    Task_t *Tmp = pFreeTasks.front();
    if(pthread_mutex_lock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    Tmp->Terminate = true;
    if(pthread_cond_signal(&Tmp->Cond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_join(Tmp->Thread, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pFreeTasks.pop_front();
    delete Tmp;
  }
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("TaskPool stoped");
}
TaskPool::Task_t *TaskPool::Run(function<void()> callback, Prio prio) {
  pthread_mutex_lock(&pMutex);
  pRunningTasks++;
  Task_t *Tmp = NULL;
  if(pFreeTasks.size() > 0) {
    Tmp = pFreeTasks.front();
    pFreeTasks.pop_front();
  }
  pthread_mutex_unlock(&pMutex);
  if(Tmp == NULL) {
    Tmp = new Task_t;
    Tmp->Cond      = PTHREAD_COND_INITIALIZER;
    Tmp->Mutex     = PTHREAD_MUTEX_INITIALIZER;
    Tmp->Thread    = 0;
    Tmp->Callback  = NULL;
    Tmp->Terminate = false;
    if(pthread_create(&Tmp->Thread, NULL, [](void *params) { return Thread((Task_t*)params); }, (void*)Tmp) != 0) {
      Tmp->Thread = 0;
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      if(pthread_setname_np(Tmp->Thread, "Worker") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  SetThreadProperties(Tmp->Thread, prio);
  if(pthread_mutex_lock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  Tmp->Callback = std::move(callback);
  if(pthread_cond_signal(&Tmp->Cond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return Tmp;
}
void *TaskPool::Thread(Task_t *task) {
  if(pthread_mutex_lock(&task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(task->Terminate == false) {
    if(task->Callback == NULL) {
      if(pthread_cond_wait(&task->Cond, &task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      task->Callback();
      task->Callback = NULL;
      task->LastUse  = Tools::GetTime64();
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pFreeTasks.push_back(task);
      pRunningTasks--;
      if(pRunningTasks == 0) {
        if(pthread_cond_signal(&pCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      }
      pthread_mutex_unlock(&pMutex);
      if(pthread_cond_signal(&task->Cond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  if(pthread_mutex_unlock(&task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return NULL;
}
void TaskPool::Wait(Task &task) {
  if(pthread_mutex_lock(&task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(task->Callback != NULL) {
    if(pthread_cond_wait(&task->Cond, &task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  if(pthread_mutex_unlock(&task->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  task = NULL;
}
void TaskPool::SetThreadNice(pthread_t &thread, const Prio prio) {
}
void TaskPool::SetThreadProperties(pthread_t thread, const Prio prio) {
  if((pMinPrio == -1) || (pMaxPrio == -1) || (pUsePrio == false)) return SetThreadNice(thread, prio);
  sched_param Param;
  Param.sched_priority = (prio == IDLE) ? 1 : (prio == LOW) ? 10 : (prio == NORMAL) ? 30 : (prio == ABOUTNORMAL) ? 40 : 90;
  if(Param.sched_priority < pMinPrio) Param.sched_priority = pMinPrio;
  if(Param.sched_priority > pMaxPrio) Param.sched_priority = pMaxPrio;
  if(pthread_setschedparam(thread, SCHED_FIFO, &Param) != 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pUsePrio = false;
    return SetThreadNice(thread, prio);
  }
}
void TaskPool::PeriodicTasks() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  uint64_t Time = Tools::GetTime64();
  while(pFreeTasks.size() > 0) {
    auto Tmp = pFreeTasks.back();
    if((Time - Tmp->LastUse) < 60000) break;
    if(pthread_mutex_lock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    Tmp->Terminate = true;
    if(pthread_cond_signal(&Tmp->Cond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&Tmp->Mutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_join(Tmp->Thread, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pFreeTasks.pop_back();
    delete Tmp;
  }
  LogFile::Debug("Adjusted TaskPool to %d workers (plus %u in use)", pFreeTasks.size(), pRunningTasks);
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
// ---------------------------------------------------------------------------------------------------------------------