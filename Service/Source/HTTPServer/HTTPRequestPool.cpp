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
#include "Tools.h"
#include "HTTPRequestPool.h"
#include "LogFile.h"
#include "MessageLoop.h"
#include "TaskPool.h"
//----------------------------------------------------------------------------------------------------------------------
HTTPRequestPool::HTTPRequestPool() {
  pRunningWorkers = 0;
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
}
HTTPRequestPool::~HTTPRequestPool() {
  shutdown();
}
void HTTPRequestPool::enqueue(function<void()> fn) {
  auto Func = [&, fn]() {
    if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pRunningWorkers++;
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    fn();
    if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pRunningWorkers--;
    if(pRunningWorkers == 0) {
      if(pthread_cond_signal(&pCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  };
  TaskPool::Run(Func, TaskPool::IDLE);
}
void HTTPRequestPool::shutdown() {
  pthread_mutex_lock(&pMutex);
  while(pRunningWorkers > 0) pthread_cond_wait(&pCond, &pMutex);
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------