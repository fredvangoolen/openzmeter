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
#include "HTTPWorker.h"
#include "HTTPTaskQueue.h"
#include "Tools.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
HTTPWorker::HTTPWorker(HTTPTaskQueue *parent) {
  pTerminate = false;
  pCond      = PTHREAD_COND_INITIALIZER;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pParent    = parent;
  pFunction  = NULL;
  if(pthread_create(&pThread, NULL, Thread, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, "HTTPWorker");
}
void HTTPWorker::Run(function<void()> fn) {
  pthread_mutex_lock(&pMutex);
  pFunction = fn;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
void *HTTPWorker::Thread(void *params) {
  HTTPWorker *This = (HTTPWorker*)params;
  nice(15);
  while(true) {
    pthread_mutex_lock(&This->pMutex);
    while((This->pTerminate == false) && (This->pFunction == NULL)) pthread_cond_wait(&This->pCond, &This->pMutex);
    if(This->pTerminate == true) return NULL;
    pthread_mutex_unlock(&This->pMutex);
    This->pFunction();
    pthread_mutex_lock(&This->pMutex);
    This->pFunction = NULL;
    This->pLastTime = Tools::GetTime64();
    pthread_mutex_unlock(&This->pMutex);
    This->pParent->PushWorker(This);
  }
}
HTTPWorker::~HTTPWorker() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
  pthread_join(pThread, NULL);
}
uint64_t HTTPWorker::LastUse() const {
  return pLastTime;
}
// ---------------------------------------------------------------------------------------------------------------------