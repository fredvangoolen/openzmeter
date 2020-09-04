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
#include "LogFile.h"
#include "ResourceFile.h"
#include "ConfigFile.h"
#include "ResourceFile.h"
#include "DatabasePool.h"
//----------------------------------------------------------------------------------------------------------------------
Database::Database(const string &schema) {
  pResult    = NULL;
  pLoadedRow = false;
  pDBConn    = DatabasePool::PopConnection();
  if(ExecSentenceNoResult("CREATE SCHEMA IF NOT EXISTS " + BindID(schema) + "; SET search_path TO " +  Bind(schema) + ", public") == false) {
    PQfinish(pDBConn);
    pDBConn = NULL;
    return;
  }
}
Database::~Database() {
  ClearAsync();
  if(pDBConn != NULL) DatabasePool::PushConnection(pDBConn);
}
bool Database::FecthRow() {
  pLoadedRow = false;
  if(pResult != NULL) PQclear(pResult);
  if(pDBConn == NULL) return false;
  pResult = PQgetResult(pDBConn);
  if(PQresultStatus(pResult) != PGRES_SINGLE_TUPLE) {
    char *Save;
    if(PQresultStatus(pResult) != PGRES_TUPLES_OK) LogFile::Error("Error in SQL query (%s)", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  pLoadedRow = true;
  pRowPos++;
  return true;
}
bool Database::LoadedRow() {
  return pLoadedRow;
}
uint32_t Database::RowIndex() const {
  return pRowPos;
}
bool Database::ExecResource(const string &file) {
  size_t Len;
  ClearAsync();
  char *Script = (char*)ResourceFile::GetFile(file, &Len);
  Script[Len - 1] = 0;
  return ExecSentenceNoResult(Script);
}
bool Database::ExecSentenceNoResult(const string &sql) {
  if(pDBConn == NULL) return false;
  ClearAsync();
  PGresult *Result = PQexec(pDBConn, sql.c_str());
  if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
    char *Save;
    LogFile::Error("Error in SQL query (%s)", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
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
    LogFile::Error("Error in SQL query (%s)", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  if(PQsetSingleRowMode(pDBConn) == 0) {
    char *Save;
    LogFile::Error("Error in SQL query (%s)", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  }
  pRowPos = 0;
  return true;
}
void Database::ClearAsync() {
  if(pResult == NULL) return;
  PQrequestCancel(pDBConn);
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
string Database::Bind(const float *params, const uint32_t lenA, const uint32_t lenB) const {
  string Result = "'{";
  for(uint32_t i = 0; i < lenA; i++) {
    Result.append((i == 0) ? "{" : ", {");
    for(uint32_t n = 0; n < lenB; n++) Result.append(((n == 0) ? "" : ", ") + (isnan(params[i * lenB + n]) ? string("NULL") : to_string(params[i * lenB + n])));
    Result.append("}");
  }
  Result.append("}'");
  return Result;
}
string Database::Bind(const uint16_t *params, const uint32_t len) const {
  string Result = "'{";
  for(uint i = 0; i < len; i++) {
    Result.append(isfinite(params[i]) ? to_string(params[i]) : string("NULL"));
    Result.append((i < (len - 1)) ? ", " : "}'");
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
//----------------------------------------------------------------------------------------------------------------------