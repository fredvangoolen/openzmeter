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
#include "AnalyzerAccess.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerAccess::AnalyzerAccess(const string &serial) {
  pSerial = serial;
  Database DB;
  DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES('admin', " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
}
AnalyzerAccess::~AnalyzerAccess() {
}
void AnalyzerAccess::SetTime(const string &user, const uint64_t &alarm, const uint64_t &event) {

}
json AnalyzerAccess::GetTime(const string &user) {
  json Result = json::object();
  return Result;
}
bool AnalyzerAccess::TestUser(const string &user, const Access neededAccess) {
  Database DB;
  string   Permision = (neededAccess == READ) ? "read" : (neededAccess == WRITE) ? "write" : "config";
  DB.ExecSentenceAsyncResult("SELECT " + Permision + " AS result FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(pSerial));
  bool Result = ((DB.FecthRow() == true) && (DB.ResultBool("result") == true));
  return Result;
}
json AnalyzerAccess::CheckUsers(const json &config) {
  json ConfigOUT = json::object();
  if(config.is_object() == false) {
    ConfigOUT["admin"] = "RWC";
    return ConfigOUT;
  }
  for(auto &User : config.items()) {
    if(User.value().is_string() == false) continue;
    string UserName = User.key();
    string Perm     = User.value();
    ConfigOUT[UserName] = string((Perm.find('R') != string::npos) ? "R" : "") + string((Perm.find('W') != string::npos) ? "W" : "") + string((Perm.find('C') != string::npos) ? "C" : "");
  }
  ConfigOUT["admin"] = "RWC";
  return ConfigOUT;
}
json AnalyzerAccess::GetUsers() {
  Database DB;
  json Result = json::object();
  DB.ExecSentenceAsyncResult("SELECT username, read, write, config FROM permissions WHERE serial = " + DB.Bind(pSerial));
  while(DB.FecthRow() == true) Result[DB.ResultString("username")] = string(DB.ResultBool("read") ? "R" : "") + string(DB.ResultBool("write") ? "W" : "") + string(DB.ResultBool("config") ? "C" : "");
  return Result;
}
void AnalyzerAccess::SetUsers(const json &config) {
  Database DB;
  if(config.is_object() == true) {
    for(auto &User : config.items()) {
      if(User.value().is_string() == false) continue;
      string UserName = User.key();
      string Perm     = User.value();
      DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(UserName) + ", " + DB.Bind(pSerial) + ", " + DB.Bind(Perm.find('R') != string::npos) + ", " + DB.Bind(Perm.find('W') != string::npos) + ", " + DB.Bind(Perm.find('C') != string::npos) + ", 0, 0) ON CONFLICT(username, serial) DO UPDATE SET read = EXCLUDED.read, write = EXCLUDED.write, config = EXCLUDED.config");
    }
  }
  DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES('admin', " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO UPDATE SET read = EXCLUDED.read, write = EXCLUDED.write, config = EXCLUDED.config");
}
// ---------------------------------------------------------------------------------------------------------------------