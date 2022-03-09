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
#include "RWLocks.h"
#include "stdio.h"
//----------------------------------------------------------------------------------------------------------------------
RWLocks::RWLocks() {
  pMutex = PTHREAD_MUTEX_INITIALIZER;
  pReadersCondition = PTHREAD_COND_INITIALIZER;
  pWritersCondition = PTHREAD_COND_INITIALIZER;
  pWriters          = 0;
  pReaders          = 0;
  pWriter           = false;
}
RWLocks::~RWLocks() {
}
void RWLocks::ReadLock() {
  pthread_mutex_lock(&pMutex);
  while((pWriters > 0) || (pWriter == true)) pthread_cond_wait(&pReadersCondition, &pMutex);
  pReaders++;
  pthread_mutex_unlock(&pMutex);
}
void RWLocks::WriteLock() {
  pthread_mutex_lock(&pMutex);
  pWriters++;
  while((pReaders > 0) || (pWriter == true)) pthread_cond_wait(&pWritersCondition, &pMutex);
  pWriters--;
  pWriter = true;
  pthread_mutex_unlock(&pMutex);
}
void RWLocks::Unlock() {
  pthread_mutex_lock(&pMutex);
  if(pWriter == true) {
    pWriter = false;
  } else {
    pReaders--;
  }
  if(pWriters > 0) {
    pthread_cond_signal(&pWritersCondition);
  } else {
    pthread_cond_broadcast(&pReadersCondition);
  }
  pthread_mutex_unlock(&pMutex);
}
//----------------------------------------------------------------------------------------------------------------------