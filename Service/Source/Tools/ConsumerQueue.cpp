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
#include "ConsumerQueue.h"
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
ConsumerQueue::ConsumerQueue(const uint16_t maxConsumers) {
  pMaxCallbacks      = maxConsumers;
  pCallbacks         = 0;
  pHandledCallbacks  = 0;
  pDataSequence      = 0;
  pConsumerEnter     = true;
  pTerminate         = false;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pConsumerEnterCond = PTHREAD_COND_INITIALIZER;
  pConsumerWaitCond  = PTHREAD_COND_INITIALIZER;
  pProducerCond      = PTHREAD_COND_INITIALIZER;
}
ConsumerQueue::~ConsumerQueue() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pTerminate = true;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ConsumerQueue::Write() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pConsumerEnter    = false;
  pHandledCallbacks = 0;
  pDataSequence++;
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ConsumerQueue::Wait() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -040> %s", __FILE__, __LINE__, strerror(errno));
  while(pCallbacks != pHandledCallbacks) {
    if(pthread_cond_wait(&pProducerCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  pConsumerEnter = true;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
bool ConsumerQueue::OnWrite(function<bool()> fn) {
  int Error = 0;
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pCallbacks > pMaxCallbacks) {
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  while((Error == 0) && (pTerminate == false) && (pConsumerEnter == false)) Error = pthread_cond_wait(&pConsumerEnterCond, &pMutex);
  if((Error != 0) || (pTerminate == true)) {
    if(Error != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  pCallbacks++;
  uint16_t Sequence = pDataSequence;
  bool     Continue = true;
  while(Continue) {
    timespec To = { .tv_sec = time(NULL) + 5, .tv_nsec = 0 };
    while((Error == 0) && (pTerminate == false) && (pDataSequence == Sequence)) Error = pthread_cond_timedwait(&pConsumerWaitCond, &pMutex, &To);
    if((Error != 0) || (pTerminate == true)) {
      if((Error != 0) && (Error != ETIMEDOUT)) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pCallbacks--;
      Continue = false;
    } else {
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      bool Result = fn();
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      if(Result == true) {
        pHandledCallbacks++;
        Sequence++;
      } else {
        Continue = false;
        pCallbacks--;
      }
    }
    if(pCallbacks == pHandledCallbacks) {
      if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return (Error == 0);
}
//----------------------------------------------------------------------------------------------------------------------