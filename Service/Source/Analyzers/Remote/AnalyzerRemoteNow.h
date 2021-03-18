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
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemoteNow final {
  private :
    static const uint32_t      pTimeout = 60000;         //Request persistance time (ms)
  private :
    const string               pSerial;                  //Analyzer serial
  private :
    json                       pNow;                     //Cached version of now data
    json                       pStatus;                  //Cached version of status data
    map<string, uint64_t>      pNowRequest;              //Avaliable series last request rime
    map<string, uint64_t>      pStatusRequest;           //avaliable series last request time
    pthread_rwlock_t           pLock;                    //Control access to shared variables
  public :
    void GetSeriesNow(const json &params, Response &response);
    void GetStatus(const json &params, Response &response, const json extraProps = json::object());
  public :
    void SetRemote(const json &params, json &response);
  public :
    AnalyzerRemoteNow(const string &serial);
    ~AnalyzerRemoteNow();
};
// ---------------------------------------------------------------------------------------------------------------------