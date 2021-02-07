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
#include "HTTPRequestWorker.h"
#include "HTTPRequestPoll.h"
#include "Tools.h"
#include "LogFile.h"
#include "Worker.h"
// ---------------------------------------------------------------------------------------------------------------------
HTTPRequestWorker::HTTPRequestWorker(HTTPRequestPoll *parent) {
  pTerminate = false;
  pCond      = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pParent    = parent;
  pFunction  = NULL;
  if(pthread_create(&pThread, NULL, [](void *params) { return ((HTTPRequestWorker*)params)->Thread(); }, this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  } else {
    Worker::SetThreadProperties(pThread, "HTTPWorker", Worker::LOW);
  }
}
void HTTPRequestWorker::Run(function<void()> fn) {
  pthread_mutex_lock(&pMutex);
  pFunction = fn;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
void *HTTPRequestWorker::Thread() {
  while(true) {
    pthread_mutex_lock(&pMutex);
    while((pTerminate == false) && (pFunction == NULL)) pthread_cond_wait(&pCond, &pMutex);
    if(pTerminate == true) return NULL;
    pthread_mutex_unlock(&pMutex);
    pFunction();
    pthread_mutex_lock(&pMutex);
    pFunction = NULL;
    pLastTime = Tools::GetTime64();
    pthread_mutex_unlock(&pMutex);
    pParent->PushWorker(this);
  }
}
HTTPRequestWorker::~HTTPRequestWorker() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
  pthread_join(pThread, NULL);
}
uint64_t HTTPRequestWorker::LastUse() const {
  return pLastTime;
}
// ---------------------------------------------------------------------------------------------------------------------