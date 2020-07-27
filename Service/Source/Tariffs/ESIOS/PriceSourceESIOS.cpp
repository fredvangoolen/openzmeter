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
#include "cctz/time_zone.h"
#include "Database.h"
#include "PriceSourceESIOS.h"
#include "ESIOSFetch.h"
#include "PriceSource.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace cctz;
//----------------------------------------------------------------------------------------------------------------------
string PriceSourceESIOS::pQueryColumns;
bool   PriceSourceESIOS::pRegistered = PriceSourceESIOS::Register();
//----------------------------------------------------------------------------------------------------------------------
bool PriceSourceESIOS::Register() {
  PriceSource::pModules.insert( { "ESP_ESIOS", { CreateInstance, GetVarsInfo } } );
  for(uint16_t V = 0; V < ESIOSFetch::pVariables.size(); V++) pQueryColumns += ESIOSFetch::pVariables[V].VarName + ", ";
  pQueryColumns += "timestamp";
  return true;
}
PriceSource *PriceSourceESIOS::CreateInstance() {
  return new PriceSourceESIOS();
}
void PriceSourceESIOS::PrepareRange(const uint64_t from, const uint64_t to) {
  pDB.ExecSentenceAsyncResult("SELECT " + pQueryColumns + " FROM esios WHERE timestamp >= " + pDB.Bind(from) + " AND timestamp <=" + pDB.Bind(to) + " ORDER BY timestamp ASC");
  pDB.FecthRow();
}
void PriceSourceESIOS::GetParams(const uint64_t sampletime, map<string, float> &vars) {
  while((pDB.LoadedRow() == true) && ((pDB.ResultUInt64("timestamp") / 3600000) < (sampletime / 3600000))) pDB.FecthRow();
  if((pDB.LoadedRow() == true) && ((pDB.ResultUInt64("timestamp") / 3600000) == (sampletime / 3600000))) {
    for(uint16_t V = 0; V < ESIOSFetch::pVariables.size(); V++) {
      float Tmp = pDB.ResultFloat(ESIOSFetch::pVariables[V].VarName);
      vars[ESIOSFetch::pVariables[V].VarName] = isnan(Tmp) ? 0 : Tmp;
    }
  } else {
    for(uint16_t V = 0; V < ESIOSFetch::pVariables.size(); V++) vars[ESIOSFetch::pVariables[V].VarName] = 0.0;
  }
}
json PriceSourceESIOS::GetVarsInfo() {
  json Tmp = json::array();
  uint64_t Now = Tools::GetTime64();
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT " + pQueryColumns + " FROM esios WHERE timestamp <= " + DB.Bind(Now) + " ORDER BY timestamp DESC LIMIT 1");
  bool HaveData = DB.FecthRow();
  for(uint16_t V = 0; V < ESIOSFetch::pVariables.size(); V++) {
    json Var = json::object();
    Var["Name"] = ESIOSFetch::pVariables[V].VarName;
    Var["Description"] = ESIOSFetch::pVariables[V].Description;
    if(HaveData == true) {
      float Tmp = DB.ResultFloat(ESIOSFetch::pVariables[V].VarName);
      Var["Value"] = isnan(Tmp) ? 0 : Tmp;
    } else {
      Var["Value"] = 0.0;
    }
    Tmp.push_back(Var);
  }
  return Tmp;
}
//----------------------------------------------------------------------------------------------------------------------