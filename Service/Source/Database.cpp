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
string                             Database::pConnectionString;
queue<Database::PoolConnection_t>  Database::pConnectionPool;
pthread_mutex_t                    Database::pMutex       = PTHREAD_MUTEX_INITIALIZER;
bool                               Database::pPoolEnabled = false;
//----------------------------------------------------------------------------------------------------------------------
Database::Database(const string &schema) {
  if(pConnectionString.empty() == true) {
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
  }
  pSchema    = schema;
  pResult    = NULL;
  pDBConn    = NULL;
  bool New   = false;
  pthread_mutex_lock(&pMutex);
  if(pPoolEnabled == true) {
    if(pConnectionPool.size() > 0) {
      pDBConn    = pConnectionPool.front().Connection;
      pUsageTime = pConnectionPool.front().UsageTime;
      pLastTime  = Tools::GetTime();
      pFreeTime  = pConnectionPool.front().FreeTime + (pLastTime - pConnectionPool.front().LastTime);
      pConnectionPool.pop();
    }
  }
  pthread_mutex_unlock(&pMutex);
  if(pDBConn == NULL) {
    New        = true;
    pUsageTime = 0;
    pFreeTime  = 0;
    pLastTime  = Tools::GetTime();
    pDBConn    = PQconnectdb(pConnectionString.c_str());
  }
  if(PQstatus(pDBConn) != CONNECTION_OK) {
    char *Save;
    LOGFile::Error("Cant open database (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQfinish(pDBConn);
    pDBConn = NULL;
    return;
  }
  if(New == true) {
    if(ExecSentenceNoResult("SET synchronous_commit TO OFF; SET client_min_messages = error; CREATE SCHEMA IF NOT EXISTS " + BindID(pSchema) + "; SET search_path TO " +  Bind(pSchema) + ", public") == false) {
      PQfinish(pDBConn);
      pDBConn = NULL;
      return;
    }
  } else {
    if(ExecSentenceNoResult("CREATE SCHEMA IF NOT EXISTS " + BindID(pSchema) + "; SET search_path TO " +  Bind(pSchema) + ", public") == false) {
      PQfinish(pDBConn);
      pDBConn = NULL;
      return;
    }
  }
}
void Database::EnablePool() {
  pthread_mutex_lock(&pMutex);
  pPoolEnabled = true;
  pthread_mutex_unlock(&pMutex);
}
void Database::DisablePool() {
  pthread_mutex_lock(&pMutex);
  while(pConnectionPool.size() > 0) {
    PoolConnection_t Connection = pConnectionPool.front();
    pConnectionPool.pop();
    PQfinish(Connection.Connection);
  }
  pPoolEnabled = false;
  pthread_mutex_unlock(&pMutex);
}
void Database::RemoveExpiredSessions() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime();
  while(pConnectionPool.size() > 0) {
    pConnectionPool.front().FreeTime;
    pConnectionPool.front().FreeTime = pConnectionPool.front().FreeTime + (Time - pConnectionPool.front().LastTime);
    pConnectionPool.front().LastTime = Time;
    if(pConnectionPool.front().FreeTime < pConnectionPool.front().UsageTime) break;
    PQfinish(pConnectionPool.front().Connection);
    pConnectionPool.pop();
  }
  LOGFile::Debug("Adjusted queries pool to %d connections\n", pConnectionPool.size());
  pthread_mutex_unlock(&pMutex);
}
Database::~Database() {
  ClearAsync();
  if(pDBConn) {
    pthread_mutex_lock(&pMutex);
    if(pPoolEnabled == true) {
      PoolConnection_t Connection;
      Connection.FreeTime   = pFreeTime;
      Connection.LastTime   = Tools::GetTime();
      Connection.UsageTime  = pUsageTime + (Connection.LastTime - pLastTime);
      Connection.Connection = pDBConn;
      pConnectionPool.push(Connection);
    } else {
      PQfinish(pDBConn);
    }
    pthread_mutex_unlock(&pMutex);
  }
}
bool Database::FecthRow() {
  if(pResult != NULL) PQclear(pResult);
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
  printf("%s\n", sql.c_str());
  if(pDBConn == NULL) return false;
  ClearAsync();
  PGresult *Result = PQexec(pDBConn, sql.c_str());
  if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
    char *Save;
    LOGFile::Error("Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
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
string Database::Bind(const uint32_t param) const {
  return to_string(param);
}
string Database::Bind(const uint64_t param) const {
  return to_string(param);
}
string Database::Bind(const float param) const {
  return isnan(param) ? string("NULL") : to_string(param);
}
string Database::Bind(const vector< vector<float> > &params) const {
  string Result = "'{";
  for(uint32_t i = 0; i < params.size(); i++) {
    Result.append("{");
    for(uint32_t n = 0; n < params[i].size(); n++) {
      Result.append(isnan(params[i][n]) ? string("NULL") : to_string(params[i][n]));
      Result.append((n < (params[i].size() - 1)) ? ", " : "}");
    }
    if(i < (params.size() - 1)) Result.append(", ");
  }
  Result.append("}'");
  return Result;
}
string Database::Bind(const float *params, const uint32_t lenA, const uint32_t lenB) const {
  string Result = "'{";
  for(uint32_t i = 0; i < lenA; i++) {
    Result.append("{");
    for(uint32_t n = 0; n < lenB; n++) {
      Result.append(isnan(params[i * lenA + n]) ? string("NULL") : to_string(params[i * lenA + n]));
      Result.append((n < (lenB - 1)) ? ", " : "}");
    }
    if(i < (lenA - 1)) Result.append(", ");
  }
  Result.append("}'");
  return Result;
}
string Database::Bind(const vector<uint16_t> &params) const {
  string Result = "'{";
  for(uint i = 0; i < params.size(); i++) {
    Result.append(to_string(params[i]));
    Result.append((i < (params.size() - 1)) ? ", " : "}'");
  }
  return Result;
}
string Database::Bind(const vector<float> &params) const {
  string Result = "'{";
  for(uint i = 0; i < params.size(); i++) {
    Result.append(isnan(params[i]) ? string("NULL") : to_string(params[i]));
    Result.append((i < (params.size() - 1)) ? ", " : "}'");
  }
  return Result;
}
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