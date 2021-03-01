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
#include <inttypes.h>
#include "Worker.h"
#include "AnalyzerSoftBlock.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemoteActions final {
  private :
    string              pSerial;                  //Analyzer serial
    pthread_mutex_t     pMutex;                   //Shared variables access control
    uint16_t            pMaxLen;                  //Max number of pending actions
    queue<json>         pPendingActions;          //Queue of pending actions
  private :
    bool PushAction(const string name, const json &params);
  public :
    void     DelSeriesRange(const json &params, Response &response);
    void     DelAlarm(const json &params, Response &response);
    void     DelAlarmsRange(const json &params, Response &response);
    void     DelEventsRange(const json &params, Response &response);
    void     DelEvent(const json &params, Response &response);
    json     GetAction();
    void     PopAction(const string &id);
    uint16_t PendingActions();
  public :
    AnalyzerRemoteActions(const string &serial, const uint16_t max = 100);
    ~AnalyzerRemoteActions();
};
// ---------------------------------------------------------------------------------------------------------------------