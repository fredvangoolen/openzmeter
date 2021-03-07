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
    enum Stage { DISABLED, LOGIN, CONFIG, CONNECTED };
  private :
    static const uint16_t         pMaxActions = 100; //Max allowed pending tasks
  private :
    volatile bool                 pTerminate;        //Sync terminate signal
    Stage                         pStage;            //Connection stage
    pthread_t                     pThread;           //Export thread
    pthread_mutex_t               pMutex;            //Shared variables lock
    string                        pSerial;           //Analyzer serial number
    string                        pConfigPath;       //Analyzer config path
    string                        pServer;           //Export server address
    string                        pUser;             //Export server user
    string                        pPass;             //Export server password
    float                         pSampleRate;       //parent samplerate
    Analyzer::AnalyzerMode        pMode;             //parent mode
    queue<json>                   pPendingActions;   //Queue of pending actions
  private :
    function<void(const string &name, const json &params)>  pHandleAction;  //Handle server actions
    function<json(const vector<string> &series)>            pStatus;        //Get analyzer status
    function<json(const vector<string> &series)>            pNow;           //get now data to send
  private :
    void *Thread();
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftSync(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate);
    ~AnalyzerSoftSync();
    void Config(const json &config, const string &path, function<void(const string &name, const json &params)> handleAction, function<json(const vector<string> &series)> status, function<json(const vector<string> &series)> now);
    void Push(const string name, const json &params);
    bool ShouldPush();
};
// ---------------------------------------------------------------------------------------------------------------------