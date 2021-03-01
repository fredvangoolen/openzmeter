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
Analyzer::Analyzer(const string &id, const AnalyzerMode mode, const string &user) : pSerial(id), pOwner(user), pMode(mode) {
  Database DB;
  pMembersLocks = PTHREAD_RWLOCK_INITIALIZER;
  pName         = "Unnamed analyzer";
  pLastSave     = 0;
  pConfigured   = false;
  DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES('admin', " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
  if(user != "admin") DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(user) + ", " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
}
Analyzer::~Analyzer() {
}
void Analyzer::SetTime(const string &user, const uint64_t &alarm, const uint64_t &event) {
}
json Analyzer::GetTime(const string &user) {
  json Result = json::object();
  return Result;
}
bool Analyzer::TestUser(const string &user, const Access neededAccess) {
  Database DB;
  string   Permision = (neededAccess == READ) ? "read" : (neededAccess == WRITE) ? "write" : "config";
  DB.ExecSentenceAsyncResult("SELECT " + Permision + " AS result FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(pSerial));
  bool Result = ((DB.FecthRow() == true) && (DB.ResultBool("result") == true));
  return Result;
}
json Analyzer::GetUsers() {
  Database DB;
  json Result = json::array();
  DB.ExecSentenceAsyncResult("SELECT username, read, write, config FROM permissions WHERE serial = " + DB.Bind(pSerial));
  while(DB.FecthRow() == true) {
    json Item = json::object();
    Item["User"] = DB.ResultString("username");
    Item["Read"] = DB.ResultBool("read");
    Item["Write"] = DB.ResultBool("write");
    Item["Config"] = DB.ResultBool("config");
    Result.push_back(Item);
  }
  return Result;
}
void Analyzer::SetUsers(const json &config) {
  if(config.is_array() == false) return;
  Database    DB;
  set<string> Users;
  for(auto &User : config) {
    if(User.is_object() == false) continue;
    if((User.contains("User") == false) || (User["User"].is_string() == false)) continue;
    if((User.contains("Read") == false) || (User["Read"].is_boolean() == false)) continue;
    if((User.contains("Write") == false) || (User["Write"].is_boolean() == false)) continue;
    if((User.contains("Config") == false) || (User["Config"].is_boolean() == false)) continue;
    string UserName = User["User"];
    bool   Read     = User["Read"];
    bool   Write    = User["Write"];
    bool   Config   = User["Config"];
    if((UserName == "admin") || (UserName == pOwner) || (Users.find(UserName) != Users.end())) continue;
    DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(UserName) + ", " + DB.Bind(pSerial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ", 0, 0) ON CONFLICT(username, serial) DO UPDATE SET read = EXCLUDED.read, write = EXCLUDED.write, config = EXCLUDED.config");
    Users.insert(UserName);
  }
  string UserText = "'admin', " + DB.Bind(pOwner);
  for(auto &User : Users) UserText += ", " + DB.Bind(User);
  DB.ExecSentenceNoResult("DELETE FROM permissions WHERE serial = " + DB.Bind(pSerial) + " AND username NOT IN (" + UserText + ")");
}
json Analyzer::GetJSONDescriptor() {
  json Response = json::object();
  Response["Analyzer"] = pSerial;
  pthread_rwlock_rdlock(&pMembersLocks);
  Response["Name"] = pName;
  Response["User"] = pOwner;
  Response["Online"] = ((Tools::GetTime64() - pLastSave) < 20000);
  pthread_rwlock_unlock(&pMembersLocks);
  return Response;
}
string Analyzer::Serial() {
  return pSerial;
}
bool Analyzer::Configured() {
  pthread_rwlock_rdlock(&pMembersLocks);
  bool Conf = pConfigured;
  pthread_rwlock_unlock(&pMembersLocks);
  return Conf;
}
void Analyzer::Configured(const bool conf) {
  pthread_rwlock_rdlock(&pMembersLocks);
  pConfigured = conf;
  pthread_rwlock_unlock(&pMembersLocks);
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
uint64_t Analyzer::OnlineTime() {
  pthread_rwlock_rdlock(&pMembersLocks);
  uint64_t Online = pLastSave;
  pthread_rwlock_unlock(&pMembersLocks);
  return Online;
}
//----------------------------------------------------------------------------------------------------------------------