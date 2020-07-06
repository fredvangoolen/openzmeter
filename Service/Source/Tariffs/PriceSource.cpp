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
#include "PriceSource.h"
//----------------------------------------------------------------------------------------------------------------------
map<string, PriceSource::Module_t>  __attribute__((init_priority(200))) PriceSource::pModules;
//----------------------------------------------------------------------------------------------------------------------
PriceSource *PriceSource::CreateInstance(const string &name) {
  auto Tmp = pModules.find(name);
  if(Tmp == pModules.end()) return NULL;
  return Tmp->second.Constructor();
}
json PriceSource::GetVarsInfo() {
  json Result = json::object();
  for(auto M : pModules) Result[M.first] = M.second.Info();
  return Result;
}
PriceSource::PriceSource() {
}
PriceSource::~PriceSource() {
}
//----------------------------------------------------------------------------------------------------------------------