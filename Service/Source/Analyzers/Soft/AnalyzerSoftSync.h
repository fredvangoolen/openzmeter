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
#include "Analyzer.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftSync {
  private :
    enum Stage { DISABLED, LOGIN, CONFIG, CONNECTED };
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
    bool                          pRequestNow;       //Send now data
    json                          pNow;              //Now data to send
    uint64_t                      pLastSample200MS;
    uint64_t                      pLastSample3S;
    uint64_t                      pLastSample1M;
    uint64_t                      pLastSample10M;
    uint64_t                      pLastSample15M;
    uint64_t                      pLastSample1H;
    uint64_t                      pLastEvent;
    uint64_t                      pLastAlarm;
  private :
    void *Thread();
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftSync(const string &serial, const Analyzer::AnalyzerMode mode, const float samplerate);
    ~AnalyzerSoftSync();
    void Config(const json &config, const string &path);

};
// ---------------------------------------------------------------------------------------------------------------------