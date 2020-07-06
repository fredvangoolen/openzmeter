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
#include <termios.h>
#include <string>
#include <vector>
#include <pthread.h>
#include "nlohmann/json.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Worker final {
  private :
    void                 *pRunParam;
    volatile bool         pWorking;
    volatile bool         pTerminate;
    pthread_t             pThread;
    pthread_mutex_t       pMutex;
    pthread_cond_t        pCondWake;
    pthread_cond_t        pCondWait;
    function<void(void*)> pCallback;
  private :
    static void *Thread(void *args);
  public :
    Worker(const string &name, function<void(void*)> callback, int8_t nice = 0);
    ~Worker();
    bool IsWorking();
    void Run(void *param);
    void Wait();
};
//----------------------------------------------------------------------------------------------------------------------
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
class ProviderLocks {
  //Provider wait until all consumers ends, consumers wait until provider write
  private :
    volatile bool     pTerminate;
    volatile bool     pDataReady;
    volatile uint32_t pConsumers;
    pthread_mutex_t   pMutex;
    pthread_cond_t    pConsumerEnterCond;
    pthread_cond_t    pConsumerWaitCond;
    pthread_cond_t    pProducerCond;
  public :
    ProviderLocks();
    ~ProviderLocks();
    void ProviderWrite();
    void ProviderWait();
    bool ConsumerWait(uint32_t timeoutSec);
    void ConsumerEnds();
};
//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static termios pConsoleAttr;
  private :
    static void ResetConsole();
  public :
    static void      DisableEcho();
    static void      RunDaemon();
    static uint64_t  GetTime64();
    static string    StrTrim(const string &str);
    static void      SendUDP(const json &obj);
};
//----------------------------------------------------------------------------------------------------------------------