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
  pConsumers         = 0;
  pHandledCallbacks  = 0;
  pCallbacks         = 0;
  pMaxConsumers      = maxConsumers;
  pDataReady         = false;
  pTerminate         = false;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pConsumerEnterCond = PTHREAD_COND_INITIALIZER;
  pConsumerWaitCond  = PTHREAD_COND_INITIALIZER;
  pProducerCond      = PTHREAD_COND_INITIALIZER;
}
ConsumerQueue::~ConsumerQueue() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pDataReady = false;
  pTerminate = true;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_cond_broadcast(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while((pConsumers > 0) && (pHandledCallbacks < pCallbacks)){
    if(pthread_cond_wait(&pProducerCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ConsumerQueue::Write() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pHandledCallbacks = 0;
  pDataReady        = true;
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ConsumerQueue::Wait() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while((pConsumers != 0) || (pHandledCallbacks != pCallbacks)){
    if(pthread_cond_wait(&pProducerCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  pDataReady = false;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
bool ConsumerQueue::OnWrite(function<void()> fn) {
  int Error = 0;
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if((pConsumers + pCallbacks) > pMaxConsumers) {
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  while((Error == 0) && (pDataReady == true) && (pTerminate == false)) Error = pthread_cond_wait(&pConsumerEnterCond, &pMutex);
  if(pDataReady == true) {
    if(Error != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  pConsumers++;
  while((Error == 0) && (pDataReady == false) && (pTerminate == false)) Error = pthread_cond_wait(&pConsumerWaitCond, &pMutex);
  if(pDataReady == false) {
    if(Error != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pConsumers--;
    if((pConsumers == 0) && (pCallbacks == pHandledCallbacks)) {
      if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
    return false;
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  fn();
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pConsumers--;
  if((pConsumers == 0) && (pCallbacks == pHandledCallbacks)) {
    if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return true;
}
bool ConsumerQueue::OnAllWrite(function<bool()> fn) {
  int Error = 0;
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if((pConsumers + pCallbacks) > pMaxConsumers) {
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  while((Error == 0) && (pDataReady == true) && (pTerminate == false)) Error = pthread_cond_wait(&pConsumerEnterCond, &pMutex);
  if(pDataReady == true) {
    if(Error != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  pCallbacks++;
  bool Continue = true;
  while(Continue) {
    while((Error == 0) && (pDataReady == false) && (pTerminate == false)) Error = pthread_cond_wait(&pConsumerWaitCond, &pMutex);
    if(pDataReady == false) {
      if(Error != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pCallbacks--;
      if((pConsumers == 0) && (pCallbacks == pHandledCallbacks)) {
        if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      }
      return false;
    }
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    Continue = fn();
    if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(Continue == true)  pHandledCallbacks++;
    if(Continue == false) pCallbacks--;
    if((pConsumers == 0) && (pCallbacks == pHandledCallbacks)) {
      if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  return true;
}
//----------------------------------------------------------------------------------------------------------------------