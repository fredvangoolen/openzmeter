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
#include <functional>
#include <queue>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemoteActions final {
  private :
    static const uint16_t      pMaxActions = 100;                           //Max allowed pending tasks
  private :
    pthread_rwlock_t           pLock;                                       //Actions queue lock
    queue<json>                pPendingActions;                             //Queue of pending actions
  private :
    function<void(const string &name, const json &params)>  pHandleAction;  //Handle server actions
  public :
    AnalyzerRemoteActions(function<void(const string &name, const json &params)> handleAction);
    ~AnalyzerRemoteActions();
    void     SetRemote(const json &params, json &response);
    bool     ShouldPush();
    void     Push(const string name, const json &params);
    uint16_t Count();
};
// ---------------------------------------------------------------------------------------------------------------------