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
constexpr float   Analyzer::FrequencyRanges[];
const     uint8_t Analyzer::FrequencyRangesLen;
constexpr float   Analyzer::VoltageRanges[];
const     uint8_t Analyzer::VoltageRangesLen;
constexpr float   Analyzer::THDRanges[];
const     uint8_t Analyzer::THDRangesLen;
constexpr float   Analyzer::HarmonicsAllowed[];
const     uint8_t Analyzer::HarmonicsAllowedLen;
constexpr float   Analyzer::UnbalanceRanges[];
const     uint8_t Analyzer::UnbalanceRangesLen;
//----------------------------------------------------------------------------------------------------------------------
Analyzer::Analyzer(const string &id, const AnalyzerMode mode, const string &user) : pSerial(id), pOwner(user), pMode(mode) {
  pMembersLocks = PTHREAD_RWLOCK_INITIALIZER;
  pName         = "Unnamed analyzer";
  pState        = UNCONFIGURED;
}
Analyzer::~Analyzer() {
}
json Analyzer::GetJSONDescriptor() {
  json Response = json::object();
  Response["Analyzer"] = pSerial;
  Response["User"] = pOwner;
  pthread_rwlock_rdlock(&pMembersLocks);
  Response["Name"] = pName;
  Response["State"] = (State() == UNCONFIGURED) ? "UNCONFIGURED" : (State() == OFFLINE) ? "OFFLINE" : (State() == SYNCING) ? "SYNCING" : "ONLINE";
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
void Analyzer::State(const Analyzer::AnalyzerState state) {
  pthread_rwlock_wrlock(&pMembersLocks);
  pState    = state;
  pLastTime = Tools::GetTime64();
  pthread_rwlock_unlock(&pMembersLocks);
}
Analyzer::AnalyzerState Analyzer::State() {
  uint64_t Now = Tools::GetTime64() - 10000;
  pthread_rwlock_rdlock(&pMembersLocks);
  AnalyzerState State = pState;
  if((State != UNCONFIGURED) && (pLastTime < Now)) State = OFFLINE;
  pthread_rwlock_unlock(&pMembersLocks);
  return State;
}
//----------------------------------------------------------------------------------------------------------------------