/* ================================== zMeter ===========================================================================
 * File:     DB.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "Database.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
string Database::pConnectionString;
//----------------------------------------------------------------------------------------------------------------------
Database::Database(string schema) {
  if(pConnectionString.empty() == true) ReadConfig();
  pSchema    = schema;
  pResult    = NULL;
  pDBConn    = PQconnectdb(pConnectionString.c_str());
  if(PQstatus(pDBConn) != CONNECTION_OK) {
    char *Save;
    LOGFile::Error("Cant open database (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQfinish(pDBConn);
    pDBConn = NULL;
    return;
  }
  if(ExecSentenceNoResult("SET synchronous_commit TO OFF; SET client_min_messages = error; CREATE SCHEMA IF NOT EXISTS " + BindID(pSchema) + "; SET search_path TO " +  Bind(pSchema) + ", public") == false) {
    PQfinish(pDBConn);
    pDBConn = NULL;
    return;
  }
}
string Database::GetSchema() {
  return pSchema;
}
Database::~Database() {
  ClearAsync();
  if(pDBConn) PQfinish(pDBConn);
}
void Database::ReadConfig() {
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
  Config["Port"] = Port;;
  Config["Database"] = DataBase;
  Config["Username"] = Username;
  Config["Password"] = Password;
  ConfigManager::WriteConfig("/Database", Config);
  pConnectionString = "host=" + Hostname + " port=" + to_string(Port) + " user=" + Username + " password=" + Password + " dbname=" + DataBase + " sslmode=allow";
}
bool Database::FecthRow() {
  if(pResult != NULL) PQclear(pResult);  
  pResult = PQgetResult(pDBConn);
  return (PQresultStatus(pResult) == PGRES_SINGLE_TUPLE);
}
bool Database::ExecResource(string file) {
  size_t Len;
  ClearAsync();
  char *Script = (char*)ResourceLoader::GetFile(file, &Len);
  Script[Len - 1] = 0;
  return ExecSentenceNoResult(Script);
}
bool Database::ExecSentenceNoResult(string sql) {
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
bool Database::ExecSentenceAsyncResult(string sql) {
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
string Database::Bind(uint32_t param) {
  return to_string(param);
}
string Database::Bind(uint64_t param) {
  return to_string(param);
}
string Database::Bind(float param) {
  return isnan(param) ? string("NULL") : to_string(param);
}
string Database::Bind(const vector< vector<float> > &params) {
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
string Database::Bind(const vector<uint16_t> &params) {
  string Result = "'{";
  for(uint i = 0; i < params.size(); i++) {
    Result.append(to_string(params[i]));
    Result.append((i < (params.size() - 1)) ? ", " : "}'");
  }
  return Result;
}
string Database::Bind(const vector<float> &params) {
  string Result = "'{";
  for(uint i = 0; i < params.size(); i++) {
    Result.append(isnan(params[i]) ? string("NULL") : to_string(params[i]));
    Result.append((i < (params.size() - 1)) ? ", " : "}'");
  }
  return Result;
}
string Database::Bind(const vector<vector<complex<float> > > &params) {
  string Result = "'{";
  for(uint i = 0; i < params.size(); i++) {
    Result.append("{");
    for(uint n = 0; n < params[i].size(); n++) {
      Result.append(isnan(params[i][n].real()) ? string("NULL") : to_string(sqrt(norm(params[i][n]))));
      Result.append((n < (params[i].size() - 1)) ? ", " : "}");
    }
    if(i < (params.size() - 1)) Result.append(", ");
  }
  Result.append("}'");
  return Result;
}
string Database::Bind(bool param) {
  return (param == true) ? "true" : "false";
}
string Database::Bind(string param) {
  if(pDBConn == NULL) return "";
  char *Scape = PQescapeLiteral(pDBConn, param.c_str(), param.length());
  string Result = Scape;
  PQfreemem(Scape);
  return Result;
}
string Database::BindID(string param) {
  if(pDBConn == NULL) return "";
  char *Scape = PQescapeIdentifier(pDBConn, param.c_str(), param.length());
  string Result = Scape;
  PQfreemem(Scape);
  return Result;
}
json Database::ResultJSON(string name) {
  return json::parse(ResultString(name));
}
string Database::ResultString(string name) {
  if(pResult == NULL) return "";
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return "";
  if(PQgetisnull(pResult, 0, Col) == 1) return "";
  return PQgetvalue(pResult, 0, Col);
}
float Database::ResultFloat(string name) {
  if(pResult == NULL) return NAN;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return NAN;
  if(PQgetisnull(pResult, 0, Col) == 1) return NAN;
  return atof(PQgetvalue(pResult, 0, Col));
}
uint64_t Database::ResultUInt64(string name) {
  if(pResult == NULL) return 0;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return 0;
  if(PQgetisnull(pResult, 0, Col) == 1) return 0;
  return strtoull(PQgetvalue(pResult, 0, Col), NULL, 10);
}
//======================================================================================================================