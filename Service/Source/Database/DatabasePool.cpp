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
#include "DatabasePool.h"
#include "nlohmann/json.h"
#include "ConfigManager.h"
#include "LogFile.h"
#include "Tools.h"
#include "Database.h"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
string                            DatabasePool::pConnectionString;
list<DatabasePool::Connection_t>  DatabasePool::pConnectionPool;
pthread_mutex_t                   DatabasePool::pMutex = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
DatabasePool::DatabasePool() {
  json Config = ConfigManager::ReadConfig("/Database");
  uint16_t Port     = 5432;
  string   Hostname = "localhost";
  string   DataBase = "openzmeter";
  string   Username = "openzmeter";
  string   Password = "openzmeter";
  if(Config.is_object()) {
    if(Config["Hostname"].is_string())      Hostname = Config["Hostname"];
    if(Config["Port"].is_number_unsigned()) Port     = Config["Port"];
    if(Config["Database"].is_string())      DataBase = Config["Database"];
    if(Config["Username"].is_string())      Username = Config["Username"];
    if(Config["Password"].is_string())      Password = Config["Password"];
  }
  Config = json::object();
  Config["Hostname"] = Hostname;
  Config["Port"]     = Port;
  Config["Database"] = DataBase;
  Config["Username"] = Username;
  Config["Password"] = Password;
  ConfigManager::WriteConfig("/Database", Config);
  pConnectionString = "host=" + Hostname + " port=" + to_string(Port) + " user=" + Username + " password=" + Password + " dbname=" + DataBase + " sslmode=allow";

  LogFile::Debug("Creating database tables");
  Database DB;
  if(DB.ExecResource("SQL/Database.sql") == false) LogFile::Error("Database is damaged, manual repair is nedded");
}
DatabasePool::~DatabasePool() {
  pthread_mutex_lock(&pMutex);
  while(pConnectionPool.size() > 0) {
    Connection_t Connection = pConnectionPool.front();
    pConnectionPool.pop_front();
    PQfinish(Connection.Connection);
  }
  pthread_mutex_unlock(&pMutex);
}
void DatabasePool::PeriodicTasks() {
  LogFile::Debug("Vacuuming database");
  Database DB;
  DB.ExecSentenceNoResult("VACUUM");
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  while(pConnectionPool.size() > 0) {
    auto Tmp = pConnectionPool.back();
    if((Time - Tmp.LastTime) < 60000) break;
    PQfinish(Tmp.Connection);
    pConnectionPool.pop_back();
  }
  LogFile::Debug("Adjusted database queries pool to %d connections", pConnectionPool.size());
  pthread_mutex_unlock(&pMutex);
}
PGconn *DatabasePool::PopConnection() {
  PGconn *Tmp = NULL;
  bool    New = false;
  pthread_mutex_lock(&pMutex);
  if(pConnectionPool.size() > 0) {
    Tmp = pConnectionPool.front().Connection;
    pConnectionPool.pop_front();
  }
  pthread_mutex_unlock(&pMutex);
  if(Tmp == NULL) {
    Tmp = PQconnectdb(pConnectionString.c_str());
    New = true;
  }
  if((Tmp != NULL) && (PQstatus(Tmp) != CONNECTION_OK)) {
    char *Save;
    LogFile::Error("Cant open database (%s)", strtok_r(PQerrorMessage(Tmp), "\r\n", &Save));
    PQfinish(Tmp);
    return NULL;
  }
  if((Tmp != NULL) && (New == true)) {
    PGresult *Result = PQexec(Tmp, "SET synchronous_commit TO OFF; SET client_min_messages = error;");
    if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
      char *Save;
      LogFile::Error("Error in SQL query (%s)", strtok_r(PQerrorMessage(Tmp), "\r\n", &Save));
      PQclear(Result);
      PQfinish(Tmp);
      return NULL;
    } else {
      PQclear(Result);
    }
  }
  return Tmp;
}
void DatabasePool::PushConnection(PGconn *conn) {
  if(conn == NULL) return;
  pthread_mutex_lock(&pMutex);
  Connection_t Connection;
  Connection.LastTime = Tools::GetTime64();
  Connection.Connection = conn;
  pConnectionPool.push_front(Connection);
  pthread_mutex_unlock(&pMutex);
}
// ---------------------------------------------------------------------------------------------------------------------