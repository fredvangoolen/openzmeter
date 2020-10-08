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
#include "HTTPRequestPoll.h"
#include "LogFile.h"
#include "MessageLoop.h"
//----------------------------------------------------------------------------------------------------------------------
HTTPRequestPoll::HTTPRequestPoll() {
  pRunningWorkers = 0;
  pMutex          = PTHREAD_MUTEX_INITIALIZER;
  pCond           = PTHREAD_COND_INITIALIZER;
  MessageLoop::RegisterListener("HTTPRequestPoll_PeriodicTasks", [&]() { this->PeriodicTasks(); }, 60000);
}
HTTPRequestPoll::~HTTPRequestPoll() {
  MessageLoop::UnregisterListener("HTTPRequestPoll_PeriodicTasks");
  shutdown();
}
void HTTPRequestPoll::PushWorker(HTTPRequestWorker *worker) {
  pthread_mutex_lock(&pMutex);
  pFreeWorkers.push_front(worker);
  pRunningWorkers--;
  if(pRunningWorkers == 0) pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
HTTPRequestWorker *HTTPRequestPoll::PopWorker() {
  pthread_mutex_lock(&pMutex);
  pRunningWorkers++;
  if(pFreeWorkers.size() > 0) {
    HTTPRequestWorker *Tmp = pFreeWorkers.front();
    pFreeWorkers.pop_front();
    pthread_mutex_unlock(&pMutex);
    return Tmp;
  } else {
    pthread_mutex_unlock(&pMutex);
    HTTPRequestWorker *Tmp = new HTTPRequestWorker(this);
    return Tmp;
  }
}
void HTTPRequestPoll::enqueue(function<void()> fn) {
  PopWorker()->Run(fn);
}
void HTTPRequestPoll::shutdown() {
  pthread_mutex_lock(&pMutex);
  while(pRunningWorkers > 0) pthread_cond_wait(&pCond, &pMutex);
  while(pFreeWorkers.size() > 0) {
    delete pFreeWorkers.front();
    pFreeWorkers.pop_front();
  }
  pthread_mutex_unlock(&pMutex);
}
void HTTPRequestPoll::PeriodicTasks() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.back();
    if((Time - Tmp->LastUse()) < 60000) break;
    pFreeWorkers.pop_back();
    delete Tmp;
  }
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("Adjusted HTTP workers pool to %d connections (plus %u in use)", pFreeWorkers.size(), pRunningWorkers);
}
//----------------------------------------------------------------------------------------------------------------------