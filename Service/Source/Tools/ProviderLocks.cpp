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
//#include <sys/time.h>
//#include <stdarg.h>
//#include <fcntl.h>
//#include <sys/stat.h>
//#include <sys/types.h>
//#include <sys/resource.h>
//#include <uuid/uuid.h>
//#include <unistd.h>
#include "ProviderLocks.h"
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
ProviderLocks::ProviderLocks() {
  pConsumers         = 0;
  pDataReady         = false;
  pTerminate         = false;
  pMutex             = PTHREAD_MUTEX_INITIALIZER;
  pConsumerEnterCond = PTHREAD_COND_INITIALIZER;
  pConsumerWaitCond  = PTHREAD_COND_INITIALIZER;
  pProducerCond      = PTHREAD_COND_INITIALIZER;
}
ProviderLocks::~ProviderLocks() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pTerminate = true;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_cond_broadcast(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ProviderLocks::ProviderWrite() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pDataReady = true;
  if(pthread_cond_broadcast(&pConsumerWaitCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ProviderLocks::ProviderWait() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while(pConsumers > 0) {
    if(pthread_cond_wait(&pProducerCond, &pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  pDataReady = false;
  if(pthread_cond_broadcast(&pConsumerEnterCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
bool ProviderLocks::ConsumerWait(uint32_t timeoutSec) {
  int Error = 0;
  timespec To;
  if(clock_gettime(CLOCK_REALTIME, &To) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  To.tv_sec += timeoutSec;
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  while((Error == 0) && (pDataReady == true) && (pTerminate == false)) Error = pthread_cond_timedwait(&pConsumerEnterCond, &pMutex, &To);
  if((Error != 0) && (Error != ETIMEDOUT)) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  } else if((pTerminate == true) || (Error == ETIMEDOUT)) {
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  pConsumers++;
  while((Error == 0) && (pDataReady == false) && (pTerminate == false)) Error = pthread_cond_timedwait(&pConsumerWaitCond, &pMutex, &To);
  if((Error != 0) && (Error != ETIMEDOUT)) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  } else if((pTerminate == true) || (Error == ETIMEDOUT)) {
    pConsumers--;
    if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return false;
  }
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return true;
}
void ProviderLocks::ConsumerEnds() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pConsumers--;
  if(pthread_cond_signal(&pProducerCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
//----------------------------------------------------------------------------------------------------------------------