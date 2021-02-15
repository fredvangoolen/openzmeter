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
#include <math.h>
#include <sys/types.h>
#include <string.h>
#include "Analyzer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id) : pSerial(id) {
  pMembersLocks = PTHREAD_RWLOCK_INITIALIZER;
  pName         = "Unnamed analyzer";
  pLastSave     = 0;
  pAccess       = new AnalyzerAccess(id);
}
Analyzer::~Analyzer() {
  delete pAccess;
}
AnalyzerAccess *Analyzer::Access() {
  return pAccess;
}
json Analyzer::GetJSONDescriptor() {
  json Response = json::object();
  Response["Analyzer"] = pSerial;
  pthread_rwlock_rdlock(&pMembersLocks);
  Response["Name"] = pName;
  Response["Online"] = ((Tools::GetTime64() - pLastSave) < 20000);
  pthread_rwlock_unlock(&pMembersLocks);
  return Response;
}
string Analyzer::Serial() {
  return pSerial;
}
string Analyzer::Name() {
  pthread_rwlock_rdlock(&pMembersLocks);
  string Name = pName;
  pthread_rwlock_unlock(&pMembersLocks);
  return Name;
}
void Analyzer::Name(const string &name) {
  pthread_rwlock_wrlock(&pMembersLocks);
  pName = name;
  pthread_rwlock_unlock(&pMembersLocks);
}
void Analyzer::Online(const bool online) {
  pthread_rwlock_wrlock(&pMembersLocks);
  pLastSave = (online == true) ? Tools::GetTime64() : 0;
  pthread_rwlock_unlock(&pMembersLocks);
}
bool Analyzer::Online() {
  pthread_rwlock_rdlock(&pMembersLocks);
  bool Online = ((Tools::GetTime64() - pLastSave) < 2000);
  pthread_rwlock_unlock(&pMembersLocks);
  return Online;
}
//----------------------------------------------------------------------------------------------------------------------