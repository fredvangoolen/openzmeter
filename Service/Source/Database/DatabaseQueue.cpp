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
#include "DatabaseQueue.h"
#include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
DatabaseQueue::DatabaseQueue(const string &schema, const bool create) : Database(schema, create) {
  pThread    = 0;
  pMutex     = PTHREAD_MUTEX_INITIALIZER;
  pCond      = PTHREAD_COND_INITIALIZER;
  pTerminate = false;
  if(pthread_create(&pThread, NULL, [](void *params) { return ((DatabaseQueue*)params)->Thread(); }, (void*)this) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  pthread_setname_np(pThread, "DatabaseQueue");
}
DatabaseQueue::~DatabaseQueue() {
  pTerminate = true;
  pthread_cond_signal(&pCond);
  if(pThread != 0) pthread_join(pThread, NULL);
}
void DatabaseQueue::ExecSentenceNoResult(const string &sql) {
  pthread_mutex_lock(&pMutex);
  pQueue.emplace(sql);
  pthread_cond_signal(&pCond);
  pthread_mutex_unlock(&pMutex);
}
void *DatabaseQueue::Thread() {
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    while((pTerminate == false) && (pQueue.empty() == true)) pthread_cond_wait(&pCond, &pMutex);
    if(pTerminate == true) return NULL;
    string Query = pQueue.front();
    pQueue.pop();
    pthread_mutex_unlock(&pMutex);
    Database::ExecSentenceAsyncResult(Query);
  }
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------