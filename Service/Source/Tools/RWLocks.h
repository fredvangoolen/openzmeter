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
#pragma once
#include <pthread.h>
#include <stdint.h>
// ---------------------------------------------------------------------------------------------------------------------
class RWLocks final {
  private :
    pthread_mutex_t   pMutex;
    pthread_cond_t    pReadersCondition;
    pthread_cond_t    pWritersCondition;
    volatile uint32_t pReaders;
    volatile uint32_t pWriters;
    bool              pWriter;
  public :
    RWLocks();
    ~RWLocks();
    void ReadLock();
    void WriteLock();
    void Unlock();
};
//----------------------------------------------------------------------------------------------------------------------