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
#include <pthread.h>
#include <functional>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class Worker {
  private :
    volatile bool         pTerminate;
    pthread_t             pThread;
    pthread_mutex_t       pMutex;
    pthread_cond_t        pCondWake;
    pthread_cond_t        pCondWait;
    function<void()>      pCallback;
  public :
    enum Prio { IDLE, LOW, NORMAL, ABOUTNORMAL, HIGH, HIGHEST };
  private :
    void *Thread();
  private :
    static void SetThreadNice(pthread_t &thread, const Prio prio);
  public :
    static void SetThreadProperties(pthread_t thread, const string &name, const Prio prio);
  public :
    Worker(const string &name, const Prio prio);
    ~Worker();
    bool IsWorking();
    void Run(function<void()> callback);
    void Wait();
};
// ---------------------------------------------------------------------------------------------------------------------