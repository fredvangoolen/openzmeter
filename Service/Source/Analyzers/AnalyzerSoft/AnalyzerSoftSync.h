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
#include <string>
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftSync {
  enum SyncState   { NOT_LOGGED, LOGGED, CONNECTED, TERMINATE };

  private : //Readed in thread, changed in configure
    pthread_t                  pSyncThread;              //Export thread
    bool                       pSyncTerminate;           //Sync terminate signal
    string                     pSyncServer;              //Export server address
    string                     pSyncUser;                //Export server user
    string                     pSyncPass;                //Export server password


        typedef struct {
      pthread_mutex_t         *Mutex;                    //Export list locks
      pthread_cond_t          *Cond;                     //Wakeup parent after last thread ends
      uint32_t                *Count;                    //Count of running sync threads
      string                   Server;                   //
      string                   Cookie;                   //
      string                   Method;                   //
      json                     Params;                   //
      uint64_t                 Sequence;                 //
      //AnalyzerSoft            *Analyzer;                 //
    } Sync_t;
    static void *Sync_Thread(void *params);
    static void *Sync_RespThread(void *params);

public :
  static json CheckConfig(const json &config);
  public :
    void Config(const json &config);

};