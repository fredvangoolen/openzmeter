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
#include <list>
#include <functional>
#include <stdint.h>
#include <pthread.h>
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class ConsumerQueue {
  private :
    uint16_t                pHandledCallbacks;
    uint16_t                pCallbacks;
    uint16_t                pMaxCallbacks;
    uint16_t                pDataSequence;
    bool                    pConsumerEnter;
    pthread_mutex_t         pMutex;
    pthread_cond_t          pConsumerEnterCond;
    pthread_cond_t          pConsumerWaitCond;
    pthread_cond_t          pProducerCond;
  public :
    ConsumerQueue(const uint16_t maxConsumers);
    ~ConsumerQueue();
    void  Write();
    void  Wait();
    bool  OnWrite(function<bool()> fn);
};
//----------------------------------------------------------------------------------------------------------------------