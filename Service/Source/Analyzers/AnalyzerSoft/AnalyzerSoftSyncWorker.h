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
#include <pthread.h>
#include "nlohmann/json.hpp"
#include "URL.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft;
class AnalyzerSoftSync;
class AnalyzerSoftSyncWorker final {
  private :
    volatile bool      pTerminate;
    string             pCookie;
    uint16_t           pPort;
    string             pHost;
    string             pPath;
    string             pSequence;
    string             pMethod;
    json               pParams;
    uint64_t           pLastTime;
    pthread_t          pThread;
    pthread_mutex_t    pMutex;
    pthread_cond_t     pCond;
    AnalyzerSoftSync  *pParent;
    AnalyzerSoft      *pAnalyzer;
    Client            *pClient;
  private:
    void *Thread();
    void  ResetClient();
  public :
    AnalyzerSoftSyncWorker(AnalyzerSoftSync *parent, AnalyzerSoft *analyzer);
    ~AnalyzerSoftSyncWorker();
    void     Run(const string &cookie, const URL &url, const string &sequence, const string &method, const json &params);
    uint64_t LastUse() const;
};
// ---------------------------------------------------------------------------------------------------------------------