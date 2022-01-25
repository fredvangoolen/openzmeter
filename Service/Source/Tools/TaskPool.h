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
#pragma once
#include <queue>
#include <pthread.h>
#include <stdint.h>
#include <functional>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class TaskPool {
  private :
    typedef struct {
      pthread_t             Thread;
      pthread_mutex_t       Mutex;
      pthread_cond_t        Cond;
      function<void()>      Callback;
      uint64_t              LastUse;
      volatile bool         Terminate;
    } Task_t;
  public :
    enum Prio { IDLE, LOW, NORMAL, ABOUTNORMAL, HIGH, HIGHEST };
    typedef Task_t* Task;
  private :
    static int             pMinPrio;
    static int             pMaxPrio;
    static volatile bool   pUsePrio;
    static pthread_mutex_t pMutex;
    static pthread_cond_t  pCond;
    static deque<Task_t*>  pFreeTasks;
    static uint32_t        pRunningTasks;
  private :
    static void  PeriodicTasks();
    static void *Thread(Task_t *task);
  public:
    TaskPool();
    ~TaskPool();
  public :
    static Task Run(function<void()> fn, Prio prio);
    static void Wait(Task &task);
    static void SetThreadProperties(pthread_t thread, const Prio prio);
    static void SetThreadNice(pthread_t &thread, const Prio prio);
};
// ---------------------------------------------------------------------------------------------------------------------