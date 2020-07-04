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
#include <stdlib.h>
#include "cpp-httplib/httplib.h"
#include "HTTPWorker.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using namespace httplib;
//----------------------------------------------------------------------------------------------------------------------
class HTTPTaskQueue final : public TaskQueue {
  private :
    pthread_mutex_t    pMutex;
    pthread_cond_t     pCond;
    list<HTTPWorker*>  pFreeWorkers;
    uint32_t           pRunningWorkers;
    uint64_t           pLastAdjust;
  public:
    HTTPTaskQueue();
    ~HTTPTaskQueue();
    void        PushWorker(HTTPWorker *worker);
    HTTPWorker *PopWorker();
  public :
    void enqueue(function<void()> fn) override;
    void on_idle() override;
    void shutdown() override;
};
//----------------------------------------------------------------------------------------------------------------------