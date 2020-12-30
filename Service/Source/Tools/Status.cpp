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
pthread_rwlock_t  Status::pLock = PTHREAD_RWLOCK_INITIALIZER;
json              Status::pStatus = json::object();
//----------------------------------------------------------------------------------------------------------------------
Status::Status() {
}
Status::~Status() {
}
void Status::SetValue(const string &path, const json &value) {
  pthread_rwlock_wrlock(&pLock);
  pStatus[json::json_pointer(path)] = value;
  pthread_rwlock_unlock(&pLock);
}
json Status::GetValue(const string &path) {
  pthread_rwlock_rdlock(&pLock);
  json Result = pStatus[json::json_pointer(path)];
  pthread_rwlock_unlock(&pLock);
  return Result;
}
json Status::GetJSON() {
  pthread_rwlock_rdlock(&pLock);
  json Result = pStatus;
  pthread_rwlock_unlock(&pLock);
  return Result;
}
//----------------------------------------------------------------------------------------------------------------------