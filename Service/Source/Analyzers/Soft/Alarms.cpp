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
#include <condition_variable>

#include "Alarms.h"
//----------------------------------------------------------------------------------------------------------------------
void AnalyzerSoft::Alarms::SetAlarm(const json &params, Response &response) {
  json Result = json::object();
  uint32_t AlarmID = params["AlarmID"];
  pMemberLocks.WriteLock();
  for(auto &Alarm : pAlarms) {
    if(Alarm.GetID() != AlarmID) continue;
    Alarm.Configure(Alarm::CheckConfig(params));
    json Config = ConfigManager::ReadConfig(pParent->pConfigPath);
    Config["Alarms"] = json::array();
    pMemberLocks.ReadLock();
    for(auto Alarm : pAlarms) Config["Alarms"].push_back(Alarm.GetFullDescriptor());
    pMemberLocks.ReadUnlock();
    ConfigManager::WriteConfig(pParent->pConfigPath, Config);
    response.set_content("", "text/plain");
    response.status = HTTP_OK;
    return;
  }
  pMemberLocks.WriteUnlock();
  response.set_content("AlarmID not found", "text/plain");
  response.status = HTTP_BAD_REQUEST;
}
