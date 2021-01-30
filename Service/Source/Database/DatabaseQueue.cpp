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
  if(pthread_setname_np(pThread, "DatabaseQueue") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
DatabaseQueue::~DatabaseQueue() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pTerminate = true;
  if(pthread_cond_signal(&pCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if((pThread != 0) && (pthread_join(pThread, NULL) != 0)) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void DatabaseQueue::ExecSentenceNoResult(const string &sql) {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pQueue.emplace(sql);
  if(pthread_cond_signal(&pCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void *DatabaseQueue::Thread() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(pTerminate == false) {
    if(pQueue.empty() == true) {
      if(pthread_cond_wait(&pCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      string Query = pQueue.front();
      pQueue.pop();
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      Database::ExecSentenceNoResult(Query);
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------