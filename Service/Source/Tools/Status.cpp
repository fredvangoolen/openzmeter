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
#include "Status.h"
//----------------------------------------------------------------------------------------------------------------------
//queue<string>    SysSupport::pSystemActions;
RWLocks  Status::pLock;
json     Status::pStatus = json::object();
//----------------------------------------------------------------------------------------------------------------------
Status::Status() {
}
Status::~Status() {
}
void Status::SetValue(const string &path, const json &value) {
  pLock.WriteLock();
  pStatus[json::json_pointer(path)] = value;
  pLock.Unlock();
}
json Status::GetValue(const string &path) {
  pLock.ReadLock();
  json Result = pStatus[json::json_pointer(path)];
  pLock.Unlock();
  return Result;
}
json Status::GetJSON() {
  pLock.ReadLock();
  json Result = pStatus;
  pLock.Unlock();
  return Result;
}
//----------------------------------------------------------------------------------------------------------------------