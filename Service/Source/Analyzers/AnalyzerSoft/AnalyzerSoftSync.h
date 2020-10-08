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
#include <map>
#include <pthread.h>
#include <string>
#include "HTTPServer.h"
#include "AnalyzerSoftSyncWorker.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft;
class AnalyzerSoftSync {
  private :
    bool                          pTerminate;        //Sync terminate signal
    pthread_t                     pThread;           //Export thread
    pthread_cond_t                pCond;             //Wait workers condition
    pthread_mutex_t               pMutex;            //Shared variables lock
    AnalyzerSoft                 *pParent;           //Parent analyzer
    string                        pServer;           //Export server address
    string                        pUser;             //Export server user
    string                        pPass;             //Export server password
    string                        pName;             //Analyzer name
    list<AnalyzerSoftSyncWorker*> pFreeWorkers;      //Avaliable workers
    uint32_t                      pRunningWorkers;   //Currently active workers
  private :
    void                   *Thread();
    void                    PeriodicTasks();
    bool                    Login(const string &path, Client *client, const string &user, const string &pass, string &cookie);
    AnalyzerSoftSyncWorker *PopWorker();
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftSync(AnalyzerSoft *parent);
    ~AnalyzerSoftSync();
    void Config(const json &config, const string &name);
    void PushWorker(AnalyzerSoftSyncWorker *worker);
};
// ---------------------------------------------------------------------------------------------------------------------