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
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <fstream>
#include "Database.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
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

  LOGFile::Debug("Creating database tables\n");
  Database DB;
  if(DB.ExecResource("SQL/Database.sql") == false) LOGFile::Error("Database is damaged, manual repair is nedded\n");
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
void DatabasePool::AdjustWorkers() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  while(pConnectionPool.size() > 0) {
    auto Tmp = pConnectionPool.back();
    if((Time - Tmp.LastTime) < 60000) break;
    PQfinish(Tmp.Connection);
    pConnectionPool.pop_back();
  }
  LOGFile::Debug("Adjusted database queries pool to %d connections\n", pConnectionPool.size());
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
    LOGFile::Error("Cant open database (%s)\n", strtok_r(PQerrorMessage(Tmp), "\r\n", &Save));
    PQfinish(Tmp);
    return NULL;
  }
  if((Tmp != NULL) && (New == true)) {
    PGresult *Result = PQexec(Tmp, "SET synchronous_commit TO OFF; SET client_min_messages = error;");
    if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
      char *Save;
      LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(Tmp), "\r\n", &Save));
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
Database::Database(const string &schema) {
  pResult    = NULL;
  pDBConn    = DatabasePool::PopConnection();
  if(ExecSentenceNoResult("CREATE SCHEMA IF NOT EXISTS " + BindID(schema) + "; SET search_path TO " +  Bind(schema) + ", public") == false) {
    PQfinish(pDBConn);
    pDBConn = NULL;
    return;
  }
}
Database::~Database() {
  ClearAsync();
  DatabasePool::PushConnection(pDBConn);
}
bool Database::FecthRow() {
  if(pResult != NULL) PQclear(pResult);
  if(pDBConn == NULL) return false;
  pResult = PQgetResult(pDBConn);
  if(PQresultStatus(pResult) != PGRES_SINGLE_TUPLE) {
    char *Save;
    if(PQresultStatus(pResult) != PGRES_TUPLES_OK) LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  return true;
}
bool Database::ExecResource(const string &file) {
  size_t Len;
  ClearAsync();
  char *Script = (char*)ResourceLoader::GetFile(file, &Len);
  Script[Len - 1] = 0;
  return ExecSentenceNoResult(Script);
}
bool Database::ExecSentenceNoResult(const string &sql) {
  if(pDBConn == NULL) return false;
  ClearAsync();
  PGresult *Result = PQexec(pDBConn, sql.c_str());
  if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
    char *Save;
    LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    printf("%s\n", sql.c_str());
    PQclear(Result);
    return false;
  } else {
    PQclear(Result);
    return true;
  }
}
bool Database::ExecSentenceAsyncResult(const string &sql) {
  if(pDBConn == NULL) return false;
  ClearAsync();
  if(PQsendQuery(pDBConn, sql.c_str()) == 0) {
    char *Save;
    LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  if(PQsetSingleRowMode(pDBConn) == 0) {
    char *Save;
    LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  return true;
}
void Database::ClearAsync() {
  if(pResult == NULL) return;
  do {
    PQclear(pResult);
    pResult = PQgetResult(pDBConn);
  } while(pResult != NULL);
}
string Database::Bind(const uint8_t param) const {
  return to_string(param);
}
string Database::Bind(const uint16_t param) const {
  return to_string(param);
}
string Database::Bind(const uint32_t param) const {
  return to_string(param);
}
string Database::Bind(const int64_t param) const {
  return to_string(param);
}
string Database::Bind(const uint64_t param) const {
  return to_string(param);
}
string Database::Bind(const float param) const {
  return isfinite(param) ? to_string(param) : string("NULL");
}
//string Database::Bind(const vector< vector<float> > &params) const {
//  string Result = "'{";
//  for(uint32_t i = 0; i < params.size(); i++) {
//    Result.append("{");
//    for(uint32_t n = 0; n < params[i].size(); n++) {
//      Result.append(isfinite(params[i][n]) ? to_string(params[i][n]) : string("NULL"));
//      if(n < (params[i].size() - 1)) Result.append(", ");
//    }
//    Result.append("}");
//    if(i < (params.size() - 1)) Result.append(", ");
//  }
//  Result.append("}'");
//  return Result;
//}
string Database::Bind(const float *params, const uint32_t lenA, const uint32_t lenB) const {
  string Result = "'{";
  for(uint32_t i = 0; i < lenA; i++) {
    Result.append("{");
    for(uint32_t n = 0; n < lenB; n++) {
      Result.append(isnan(params[i * lenB + n]) ? string("NULL") : to_string(params[i * lenB + n]));
      Result.append((n < (lenB - 1)) ? ", " : "}");
    }
    if(i < (lenA - 1)) Result.append(", ");
  }
  Result.append("}'");
  return Result;
}
//string Database::Bind(const vector<uint16_t> &params) const {
//  string Result = "'{";
//  for(uint i = 0; i < params.size(); i++) {
//    Result.append(isfinite(params[i]) ? to_string(params[i]) : string("NULL"));
//    Result.append((i < (params.size() - 1)) ? ", " : "}'");
//  }
//  return Result;
//}
string Database::Bind(const uint16_t *params, const uint32_t len) const {
  string Result = "'{";
  for(uint i = 0; i < len; i++) {
    Result.append(isfinite(params[i]) ? to_string(params[i]) : string("NULL"));
    Result.append((i < (len - 1)) ? ", " : "}'");
  }
  return Result;
}
//string Database::Bind(const vector<float> &params) const {
//  string Result = "'{";
//  for(uint i = 0; i < params.size(); i++) {
//    Result.append(isfinite(params[i]) ? to_string(params[i]) : string("NULL"));
//    Result.append((i < (params.size() - 1)) ? ", " : "}'");
//  }
//  return Result;
//}
string Database::Bind(const float *params, const uint32_t len) const {
  string Result = "'{";
  for(uint32_t i = 0; i < len; i++) {
    Result.append(isnan(params[i]) ? string("NULL") : to_string(params[i]));
    Result.append((i < (len - 1)) ? ", " : "}'");
  }
  return Result;
}
string Database::Bind(const bool param) const {
  return (param == true) ? "true" : "false";
}
string Database::Bind(const string &param) const {
  if(pDBConn == NULL) return "";
  char *Scape = PQescapeLiteral(pDBConn, param.c_str(), param.length());
  string Result = Scape;
  PQfreemem(Scape);
  return Result;
}
string Database::BindID(const string &param) const {
  if(pDBConn == NULL) return "";
  char *Scape = PQescapeIdentifier(pDBConn, param.c_str(), param.length());
  string Result = Scape;
  PQfreemem(Scape);
  return Result;
}
json Database::ResultJSON(const string &name) const {
  json Result = json::parse(ResultString(name), NULL, false);
  if(Result.is_discarded()) return nullptr;
  return Result;
}
string Database::ResultString(const string &name) const {
  if(pResult == NULL) return "";
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return "";
  if(PQgetisnull(pResult, 0, Col) == 1) return "";
  return PQgetvalue(pResult, 0, Col);
}
float Database::ResultFloat(const string &name) const {
  if(pResult == NULL) return NAN;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return NAN;
  if(PQgetisnull(pResult, 0, Col) == 1) return NAN;
  return atof(PQgetvalue(pResult, 0, Col));
}
uint64_t Database::ResultUInt64(const string &name) const {
  if(pResult == NULL) return 0;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return 0;
  if(PQgetisnull(pResult, 0, Col) == 1) return 0;
  return strtoull(PQgetvalue(pResult, 0, Col), NULL, 10);
}
//======================================================================================================================