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
#include "PriceSourceNone.h"
//----------------------------------------------------------------------------------------------------------------------
bool  PriceSourceNone::pRegistered = Register();
//----------------------------------------------------------------------------------------------------------------------
bool PriceSourceNone::Register() {
  pModules.insert( { "NONE", { Instance, VarsInfo, VarsFill } } );
  return true;
}
PriceSource *PriceSourceNone::Instance() {
  return new PriceSourceNone();
}
json PriceSourceNone::VarsInfo() {
  json Result = json::array();
  return Result;
}
void PriceSourceNone::VarsFill(map<string, float> &params) {
}
void PriceSourceNone::GetSample(const uint64_t sampletime, map<string, float> &vars) {
}
//----------------------------------------------------------------------------------------------------------------------