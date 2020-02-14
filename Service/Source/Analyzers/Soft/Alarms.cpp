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
AnalyzerSoft::Alarms::Alarms(AnalyzerSoft *parent, const json &config) {
//  json Config = ConfigManager::ReadConfig(configPath);
  for(auto &Tmp : config) pAlarms.emplace_back(pLastAlarmID++, Tmp);
}
void *AnalyzerSoft::Alarms::Alarms_Thread(void* args) {
  Alarms *This = (Alarms*)args;
  while(This->pTerminate == false) {
    if(This->pWorkerLocks.WorkerWaitTask() == false) break;
    This->pMemberLocks.ReadLock();
    printf("chech alarms\n");
    for(auto Alarm : This->pAlarms) {
    }
    This->pMemberLocks.ReadUnlock();
    This->pWorkerLocks.WorkerEndsTask();
  }
  return NULL;
}
AnalyzerSoft::Alarms::~Alarms() {
  pTerminate = true;
  pWorkerLocks.Abort(&pAlarmThread);
  pMemberLocks.WriteLock();
  pAlarms.clear();
  pMemberLocks.WriteUnlock();
}

void AnalyzerSoft::Alarms::Start(AnalyzerSoft::Params_t &aggreg) {
  pParams = &aggreg;
  pWorkerLocks.WakeWorker();
}
void AnalyzerSoft::Alarms::Wait() {
  pWorkerLocks.WaitWorker();
}
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
