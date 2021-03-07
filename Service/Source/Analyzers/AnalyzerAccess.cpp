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
#include <set>
#include "AnalyzerAccess.h"
#include "Database.h"
#include "LogFile.h"
#include "SystemManager.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerAccess::AnalyzerAccess(const string &serial, const string &owner) : pSerial(serial), pOwner(owner) {
  Database DB;
  DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES('admin', " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
  if(pOwner != "admin") DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(pOwner) + ", " + DB.Bind(pSerial) + ", true, true, true, 0, 0) ON CONFLICT(username, serial) DO NOTHING");
}
AnalyzerAccess::~AnalyzerAccess() {
}
bool AnalyzerAccess::TestUser(const Access neededAccess, const string &user) const {
  Database DB;
  string   Permision = (neededAccess == READ) ? "read" : (neededAccess == WRITE) ? "write" : "config";
  DB.ExecSentenceAsyncResult("SELECT " + Permision + " AS result FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(pSerial));
  bool Result = ((DB.FecthRow() == true) && (DB.ResultBool("result") == true));
  return Result;
}
json AnalyzerAccess::Users() const {
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
json AnalyzerAccess::CheckConfig(const json &config, const string &owner) {
  json        ConfigOUT = json::array();
  set<string> Users;
  ConfigOUT.push_back({ {"User", "admin"}, {"Read", true}, {"Write", true}, {"Config", true} });
  Users.insert("admin");
  if(owner != "admin") {
    ConfigOUT.push_back({ {"User", owner}, {"Read", true}, {"Write", true}, {"Config", true} });
    Users.insert(owner);
  }
  if(config.is_array() == false) return ConfigOUT;
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
    if((UserName == "admin") || (UserName == owner) || (Users.find(UserName) != Users.end())) continue;
    ConfigOUT.push_back({ {"User", UserName}, {"Read", Read}, {"Write", Write}, {"Config", Config} });
    Users.insert(UserName);
  }
  return ConfigOUT;
}
void AnalyzerAccess::Config(const json &config) const {
  Database DB;
  string   Users;
  for(auto &User : config) {
    string UserName = User["User"];
    bool   Read     = User["Read"];
    bool   Write    = User["Write"];
    bool   Config   = User["Config"];
    DB.ExecSentenceNoResult("INSERT INTO permissions(username, serial, read, write, config, eventsread, alarmsread) VALUES(" + DB.Bind(UserName) + ", " + DB.Bind(pSerial) + ", " + DB.Bind(Read) + ", " + DB.Bind(Write) + ", " + DB.Bind(Config) + ", 0, 0) ON CONFLICT(username, serial) DO UPDATE SET read = EXCLUDED.read, write = EXCLUDED.write, config = EXCLUDED.config");
    Users.append((Users.size() == 0) ? DB.Bind(UserName) : ", " + DB.Bind(UserName));
  }
  DB.ExecSentenceNoResult("DELETE FROM permissions WHERE serial = " + DB.Bind(pSerial) + " AND username NOT IN (" + Users + ")");
}
json AnalyzerAccess::GetStatus(const vector<string> &series, const string &user) const {
  json     Result = json::object();
  uint64_t To     = Tools::GetTime64();
  Result["Time"]  = To;
  for(auto &Serie : series) {
    if(Serie == "Log") {
      Result["Log"] = LogFile::ToJSON();
    } else if(Serie == "System") {
      Result["System"] = SystemManager::ToJSON();
    } else if(Serie == "Events") {
      Result["Events"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT eventsread, (SELECT count(*) FROM events WHERE changetime > eventsread) AS events FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(pSerial));
      if(DB.FecthRow() == false) {
        Result["Events"]["LastRead"] = 0;
        Result["Events"]["Count"]    = 0;
      } else {
        Result["Events"]["LastRead"] = DB.ResultUInt64("eventsread");
        Result["Events"]["Count"]    = DB.ResultUInt64("events");
      }
    } else if(Serie == "Alarms") {
      Result["Alarms"] = json::object();
      Database DB(pSerial);
      DB.ExecSentenceAsyncResult("SELECT alarmsread, (SELECT count(*) FROM alarms WHERE changetime > alarmsread) AS alarms FROM permissions WHERE username = " + DB.Bind(user) + " AND serial = " + DB.Bind(pSerial));
      Result["Unread_Alarms"]["To"]    = To;
      if(DB.FecthRow() == false) {
        Result["Alarms"]["LastRead"] = 0;
        Result["Alarms"]["Count"]    = 0;
      } else {
        Result["Alarms"]["LastRead"] = DB.ResultUInt64("alarmsread");
        Result["Alarms"]["Count"]    = DB.ResultUInt64("alarms");
      }
    } else if(Serie == "Storage") {
      Database DB;
      DB.ExecSentenceAsyncResult("WITH sizes AS (SELECT tablename, pg_total_relation_size(quote_ident(schemaname) || '.' || quote_ident(tablename)) AS size FROM pg_tables WHERE schemaname = " + DB.Bind(pSerial) + ") SELECT json_build_object("
                                 "'Aggreg_200MS', (SELECT size FROM sizes WHERE tablename = 'aggreg_200ms'),"
                                 "'Aggreg_3S', (SELECT size FROM sizes WHERE tablename = 'aggreg_3s'),"
                                 "'Aggreg_1M', (SELECT SUM(size) FROM sizes WHERE tablename IN ('aggreg_1m', 'aggreg_ext_1m')),"
                                 "'Aggreg_10M', (SELECT size FROM sizes WHERE tablename = 'aggreg_10m'),"
                                 "'Aggreg_15M', (SELECT size FROM sizes WHERE tablename = 'aggreg_15m'),"
                                 "'Aggreg_1H', (SELECT size FROM sizes WHERE tablename = 'aggreg_1h'),"
                                 "'Events', (SELECT SUM(size) FROM sizes WHERE tablename IN ('events', 'events_ext')),"
                                 "'Alarms', (SELECT size FROM sizes WHERE tablename = 'alarms')) as tables");
      if(DB.FecthRow() == true) Result["Storage"] = DB.ResultJSON("tables");
    } else {
      return nullptr;
    }
  }
  return Result;
}
// ---------------------------------------------------------------------------------------------------------------------