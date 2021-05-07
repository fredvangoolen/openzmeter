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
#include "AnalyzerRemoteActions.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerRemoteActions::AnalyzerRemoteActions(function<void(const string &name, const json &params)> handleAction) {
  pHandleAction = handleAction;
  pLock         = PTHREAD_RWLOCK_INITIALIZER;
}
AnalyzerRemoteActions::~AnalyzerRemoteActions() {
}
bool AnalyzerRemoteActions::ShouldPush() {
  pthread_rwlock_rdlock(&pLock);
  uint16_t Count = pPendingActions.size();
  pthread_rwlock_unlock(&pLock);
  return (Count < pMaxActions);
}
void AnalyzerRemoteActions::Push(const string name, const json &params) {
  json Action = json::object();
  Action["Type"] = name;
  Action["Params"] = params;
  if(Action["Params"].contains("UserName")) Action["Params"].erase("UserName");
  pthread_rwlock_wrlock(&pLock);
  pPendingActions.push(Action);
  pthread_rwlock_unlock(&pLock);
}
uint16_t AnalyzerRemoteActions::Count() {
  pthread_rwlock_rdlock(&pLock);
  uint16_t Size = pPendingActions.size();
  pthread_rwlock_unlock(&pLock);
  return Size;
}
void AnalyzerRemoteActions::SetRemote(const json &params, json &response) {
  if(params.contains("Actions") && params["Actions"].is_array()) {
    for(auto &Action : params["Actions"]) {
      Response Resp;
      if((Action.contains("Type") == false) || (Action["Type"].is_string() == false) || (Action.contains("Params") == false)) continue;
      pHandleAction(Action["Type"], Action["Params"]);
    }
  }
  pthread_rwlock_wrlock(&pLock);
  if(pPendingActions.size() > 0) {
    response["Actions"] = json::array();
    while(pPendingActions.empty() == false) {
      response["Actions"].push_back(pPendingActions.front());
      pPendingActions.pop();
    }
  }
  pthread_rwlock_unlock(&pLock);
}
// ---------------------------------------------------------------------------------------------------------------------