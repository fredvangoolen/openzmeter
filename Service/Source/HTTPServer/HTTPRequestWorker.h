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
#include <stdint.h>
#include <functional>
#include <pthread.h>
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class HTTPRequestPoll;
class HTTPRequestWorker final {
  private :
    volatile bool    pTerminate;
    function<void()> pFunction;
    uint64_t         pLastTime;
    pthread_t        pThread;
    pthread_mutex_t  pMutex;
    pthread_cond_t   pCond;
    HTTPRequestPoll &pParent;
  private:
    void *Thread();
  public :
    HTTPRequestWorker(HTTPRequestPoll &parent);
    ~HTTPRequestWorker();
    void     Run(function<void()> fn);
    uint64_t LastUse() const;
};
// ---------------------------------------------------------------------------------------------------------------------