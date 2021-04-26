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
#include <queue>
#include <pthread.h>
#include <string>
#include "Analyzer.h"
#include "HTTPServer.h"
#include "AnalyzerSoftAggreg200MS.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftSync final {
  private :
    enum Stage { LOGIN, CONFIG, CONNECTED };
  private :
    static const uint16_t         pMaxActions = 100; //Max allowed pending tasks
  private :
    const string                  pSerial;           //Analyzer serial number
    const string                  pConfigPath;       //Analyzer config path
    const Analyzer::AnalyzerMode  pMode;             //parent mode
    const float                   pSampleRate;       //parent samplerate
  private :
    volatile bool                 pTerminate;        //Termination signal
    pthread_t                     pThread;           //Export thread
    string                        pServer;           //Export server address
    string                        pUser;             //Export server user
    string                        pPass;             //Export server password
    pthread_mutex_t               pMutexInternal;    //Internal variables lock
    pthread_mutex_t               pMutexConfig;      //Configuration lock
    queue<json>                   pPendingActions;   //Queue of pending actions
  private :
    const function<void(const string &name, const json &params)>  pHandleAction;  //Handle server actions
    const function<json(const vector<string> &series)>            pStatus;        //Get analyzer status
    const function<json(const vector<string> &series)>            pNow;           //get now data to send
  private :
    void  *Thread();
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftSync(const string &serial, const string &path, const Analyzer::AnalyzerMode mode, const float samplerate, function<void(const string &name, const json &params)> handleAction, function<json(const vector<string> &series)> status, function<json(const vector<string> &series)> now);
    ~AnalyzerSoftSync();
    void Config(const json &config);
    void Push(const string name, const json &params);
    bool ShouldPush();
};
// ---------------------------------------------------------------------------------------------------------------------