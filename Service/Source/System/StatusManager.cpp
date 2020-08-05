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
#include "StatusManager.h"
//#include "ConfigFile.h"
//#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
//queue<string>    SysSupport::pSystemActions;
RWLocks  StatusManager::pLock;
json     StatusManager::pStatus = json::object();
//----------------------------------------------------------------------------------------------------------------------
StatusManager::StatusManager() {
}
StatusManager::~StatusManager() {
}
void StatusManager::SetValue(const string &path, const json &value) {
  pLock.WriteLock();
  pStatus[json::json_pointer(path)] = value;
  pLock.Unlock();
}
json StatusManager::GetValue(const string &path) {
  pLock.ReadLock();
  json Result = pStatus[json::json_pointer(path)];
  pLock.Unlock();
  return Result;
}
json StatusManager::GetJSON() {
  pLock.ReadLock();
  json Result = pStatus;
  pLock.Unlock();
  return Result;



//  Result["BT_Enabled"] = pBT_Enabled;
//  Result["TimeSet"] = pTimeSet;
//  Result["Battery_Volt"] = pSystemStatus.Battery_Volt;
//  Result["Battery_Level"] = pSystemStatus.Battery_Level;
//  Result["Updates"] = pSystemStatus.Updates;
//  Result["Events"] = pSystemStatus.Events;
}
//----------------------------------------------------------------------------------------------------------------------