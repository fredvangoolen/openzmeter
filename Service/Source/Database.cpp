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
char *Database::pDatabaseName = (char*)"openzmeter";
char *Database::pHost         = (char*)"127.0.0.1";
int   Database::pPort         = 5432;
char *Database::pUser         = (char*)"openzmeter";
char *Database::pPassword     = (char*)"";
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
  if((pDBConn != NULL) && (PQstatus(pDBConn) == CONNECTION_BAD)) {
    PQfinish(pDBConn);
    pDBConn = NULL;
  } else if((pDBConn != NULL) && (PQstatus(pDBConn) == CONNECTION_OK)) {
    return true;
  }
  char ConnectionString[512];
  sprintf(ConnectionString, "host=%s port=%d user=%s password=%s dbname=%s", pHost, pPort, pUser, pPassword, pDatabaseName);
  pDBConn = PQconnectdb(ConnectionString);
  if(PQstatus(pDBConn) == CONNECTION_BAD) {
    char *Save;
    Tools::LogMsg("ERROR", "Cant open database (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQfinish(pDBConn);
    pDBConn = NULL;
    return false;
  }
  ExecSentence("SET synchronous_commit TO OFF");
  ExecSentence("SET client_min_messages = error");
  ExecSentence("CREATE SCHEMA IF NOT EXISTS " + BindID(pSchema));
  ExecSentence("SET search_path TO " +  Bind(pSchema) + ", public");
  return true;
}
Database::~Database() {
  if(pResult != NULL) PQclear(pResult);
  PQfinish(pDBConn);
}
bool Database::Config(json_object *config) {
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "HostName") == 0)) {
      pHost = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) {
      pPort = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Database") == 0)) {
      pDatabaseName = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "UserName") == 0)) {
      pUser = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) {
      pPassword = (char*)json_object_get_string(JSON_Val);
    }
  }
  Database DB;
  Tools::LogMsg("DEBUG", "Creating database tables\n");
  return DB.ExecResource("SQL/Database.sql");
}
bool Database::ExecResource(string file) {
  size_t Len;
  if(pResult != NULL) PQclear(pResult);
  char *Script = (char*)Tools::GetFile(file, &Len);
  Script[Len - 1] = 0;
  return ExecSentence(Script);
}
bool Database::ExecSentence(string sql) {
  if(pResult != NULL) PQclear(pResult);
  if(Connect() == false) return false;
  pResult = PQexec(pDBConn, sql.c_str());
  if((PQresultStatus(pResult) != PGRES_COMMAND_OK) && (PQresultStatus(pResult) != PGRES_TUPLES_OK)) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query \"%s\" (%s)\n", sql.c_str(), strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
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
//    Tools::LogMsg("ERROR", "Error in SQL query \"%s\" (%s)\n", sql, strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
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
    Result.append(to_string(params[i]));
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
  return json.parse(ResultString(row, name));
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
//  if(pParams.size() != 0) Tools::LogMsg("ERROR", "There are more placeholders than params!!\n");
//  return Result;
//}
//======================================================================================================================