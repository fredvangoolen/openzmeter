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
#include "LogFile.h"
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
  LogFile::Debug("Starting Analyzer...");
  pName         = "Unnamed analyzer";
  pState        = UNCONFIGURED;
  LogFile::Debug("Analyzer started");
}
Analyzer::~Analyzer() {
}
string Analyzer::Serial() {
  return pSerial;
}
string Analyzer::Owner() {
  return pOwner;
}
string Analyzer::Name() {
  pMembersLocks.ReadLock();
  string Name = pName;
  pMembersLocks.Unlock();
  return Name;
}
void Analyzer::Name(const string &name) {
  pMembersLocks.WriteLock();
  pName = name;
  pMembersLocks.Unlock();
}
void Analyzer::State(const Analyzer::AnalyzerState state) {
  pMembersLocks.WriteLock();
  pState    = state;
  pLastTime = Tools::GetTime64();
  pMembersLocks.Unlock();
}
Analyzer::AnalyzerState Analyzer::State() {
  uint64_t Now = Tools::GetTime64() - 10000;
  pMembersLocks.ReadLock();
  AnalyzerState State = pState;
  if((State != UNCONFIGURED) && (pLastTime < Now)) State = OFFLINE;
  pMembersLocks.Unlock();
  return State;
}
uint8_t Analyzer::VoltageChannels(const AnalyzerMode mode) {
  if(mode == Analyzer::PHASE1)  return 1;
  if(mode == Analyzer::PHASE2)  return 2;
  if(mode == Analyzer::PHASE2N) return 2;
  if(mode == Analyzer::PHASE3)  return 3;
  if(mode == Analyzer::PHASE3N) return 3;
  return 0;
}
uint8_t Analyzer::CurrentChannels(const AnalyzerMode mode) {
  if(mode == Analyzer::PHASE1)  return 1;
  if(mode == Analyzer::PHASE2)  return 2;
  if(mode == Analyzer::PHASE2N) return 3;
  if(mode == Analyzer::PHASE3)  return 3;
  if(mode == Analyzer::PHASE3N) return 4;
  return 0;
}
string Analyzer::ModeToString(const AnalyzerMode mode) {
  if(mode == PHASE1)  return "1PHASE";
  if(mode == PHASE2)  return "2PHASE";
  if(mode == PHASE2N) return "2PHASE+N";
  if(mode == PHASE3)  return "3PHASE";
  if(mode == PHASE3N) return "3PHASE+N";
  return "";
}
Analyzer::AnalyzerMode Analyzer::StringToMode(const string mode) {
  if(mode == "1PHASE")   return Analyzer::PHASE1;
  if(mode == "2PHASE")   return Analyzer::PHASE2;
  if(mode == "2PHASE+N") return Analyzer::PHASE2N;
  if(mode == "3PHASE")   return Analyzer::PHASE3;
  if(mode == "3PHASE+N") return Analyzer::PHASE3N;
  return Analyzer::INVALID;
}
//----------------------------------------------------------------------------------------------------------------------