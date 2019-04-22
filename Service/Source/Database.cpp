/* ================================== zMeter ===========================================================================
 * File:     DB.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include "Database.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
string Database::pConnectionString;
//----------------------------------------------------------------------------------------------------------------------
Database::Database(string schema) {
  pSchema    = schema;
  pDBConn    = NULL;
  pResult    = NULL;
}
string Database::GetSchema() {
  return pSchema;
}
bool Database::Connect() {
  if((pDBConn != NULL) && (PQstatus(pDBConn) == CONNECTION_OK)) return true;
  if((pDBConn != NULL) && (PQstatus(pDBConn) == CONNECTION_BAD)) {
    PQfinish(pDBConn);
    pDBConn = NULL;
  } 
  pDBConn = PQconnectdb(pConnectionString.c_str());
  if(PQstatus(pDBConn) != CONNECTION_OK) {
    char *Save;
    LOGFile::Instance()->Error("Cant open database (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQfinish(pDBConn);
    pDBConn = NULL;
    return false;
  }
  ExecSentence("SET synchronous_commit TO OFF;"
               "SET client_min_messages = error;"
               "CREATE SCHEMA IF NOT EXISTS " + BindID(pSchema) + ";"
               "SET search_path TO " +  Bind(pSchema) + ", public");
  ClearResult();
  return true;
}
Database::~Database() {
  ClearResult();
  PQfinish(pDBConn);
}
void Database::ClearResult() {
  if(pResult != NULL) {
    PQclear(pResult);
    pResult = NULL;
  }
}
bool Database::Config(json config) {
  string Hostname = "localhost";
  uint16_t Port = 5432;
  string DataBase = "openzmeter";
  string Username = "openzmeter";
  string Password = "openzmeter";
  if(config["Hostname"].is_string()) Hostname = config["Hostname"];
  if(config["Port"].is_number_unsigned()) Port = config["Port"];
  if(config["Database"].is_string()) DataBase = config["Database"];
  if(config["Username"].is_string()) Username = config["Username"];
  if(config["Password"].is_string()) Password = config["Password"];
  pConnectionString = "host=" + Hostname + " port=" + to_string(Port) + " user=" + Username + " password=" + Password + " dbname=" + DataBase + " sslmode=allow";
  Database DB("public");
  LOGFile::Instance()->Debug("Creating database tables\n");
  return DB.ExecResource("SQL/Database.sql");
}
bool Database::ExecResource(string file) {
  size_t Len;
  ClearResult();
  char *Script = (char*)ResourceLoader::Instance()->GetFile(file, &Len);
  Script[Len - 1] = 0;
  return ExecSentence(Script);
}
bool Database::ExecSentence(string sql) {
  ClearResult();
  if(Connect() == false) return false;
  pResult = PQexec(pDBConn, sql.c_str());
  LOGFile::Instance()->Info("Exec: %s\n", sql.c_str());
  if((PQresultStatus(pResult) != PGRES_COMMAND_OK) && (PQresultStatus(pResult) != PGRES_TUPLES_OK)) {
    char *Save;
    LOGFile::Instance()->Error("Error in SQL query \"%s\" (%s)\n", sql.c_str(), strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  } else {
    return true;  
  }
}
//char *Database::ExecSentenceJSON(const char* sql, size_t *len) {
//  if(pResult != NULL) PQclear(pResult);
//  if(Connect() == false) return NULL;
//  pResult = PQexec(pDBConn, sql);
//  if(PQresultStatus(pResult) != PGRES_TUPLES_OK) {
//    char *Save;
//    LOGFile::Instance()->Error("Error in SQL query \"%s\" (%s)\n", sql, strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
//    return NULL;
//  } else {
//    if(PQntuples(pResult) == 0) return NULL;
//    if(len) *len = PQgetlength(pResult, 0, 0);
//    return (PQgetisnull(pResult, 0, 0) == true) ? NULL : PQgetvalue(pResult, 0, 0);
//  }
//}
//void Database::PrepareSentence(string sentence) {
//  pSentence = sentence;
//  pParams = queue<string>();
//}
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
string Database::Bind(bool param) {
  return (param == true) ? "true" : "false";
}
string Database::Bind(string param) {
  if(Connect() == false) return "";
  char *Scape = PQescapeLiteral(pDBConn, param.c_str(), param.length());
  string Result = Scape;
  PQfreemem(Scape);
  return Result;
}
string Database::BindID(string param) {
//  char *Scape = PQescapeLiteral(pDBConn, param.c_str(), param.length());
  return string("\"") + param + "\"";
//  PQfreemem(Scape);  
}
uint32_t Database::ResultRows() {
  if(pResult == NULL) return 0;
  return PQntuples(pResult);
}
json Database::ResultJSON(uint32_t row, string name) {
  return json::parse(ResultString(row, name));
}
string Database::ResultString(uint32_t row, string name) {
  if(pResult == NULL) return "";
  if(PQntuples(pResult) < 1) return "";
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return "";
  if(PQgetisnull(pResult, row, Col) == 1) return "";
  return PQgetvalue(pResult, row, Col);
}
float Database::ResultFloat(uint32_t row, string name) {
  if(pResult == NULL) return NAN;
  if(PQntuples(pResult) < 1) return NAN;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return NAN;
  if(PQgetisnull(pResult, row, Col) == 1) return NAN;
  return atof(PQgetvalue(pResult, row, Col));
}
uint64_t Database::ResultUInt64(uint32_t row, string name) {
  if(pResult == NULL) return 0;
  if(PQntuples(pResult) < 1) return 0;
  int32_t Col = PQfnumber(pResult, name.c_str());
  if(Col == -1) return 0;
  if(PQgetisnull(pResult, row, Col) == 1) return 0;
  return strtoull(PQgetvalue(pResult, row, Col), NULL, 10);
}
//bool Database::ExecSentence() {
//  return ExecSentence(BuildQueryString().c_str());
//}
//string Database::BuildQueryString() {
//  string Result;
//  for(std::string::iterator it = pSentence.begin(); it != pSentence.end(); it++) {
//    if(*it == '?') {
//      Result.append(pParams.front());
//      pParams.pop();
//    } else {
//      Result.push_back(*it);
//    }
//  }
//  if(pParams.size() != 0) LOGFile::Instance()->Error("There are more placeholders than params!!\n");
//  return Result;
//}
//======================================================================================================================