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
Database::Database(const char *schema) {
  pSchema    = (schema == NULL) ? NULL : strdup(schema);
  pParams    = NULL;
  pResult    = NULL;
  pParamsLen = 0;
  pDBConn    = NULL;
}
const char *Database::GetSchema() {
  return (pSchema) ? pSchema : "public";
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
  if(pSchema) {
    PrepareSentence("CREATE SCHEMA IF NOT EXISTS ?");
    BindSingleString(pSchema, true);
    ExecSentence();
    PrepareSentence("SET search_path TO ?");
    BindSingleString(pSchema);
    ExecSentence();
  }
  return true;
}
Database::~Database() {
  if(pSchema) free((char*)pSchema);
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
  Database DB(NULL);
  size_t Len;
  char *Script = (char*)Tools::GetFile("SQL/Database.sql", &Len);
  Script[Len - 1] = 0;
  Tools::LogMsg("DEBUG", "Creating database tables\n");
  return DB.ExecSentence(Script);
}
bool Database::ExecSentence(const char* sql) {
  if(pResult != NULL) PQclear(pResult);
  if(Connect() == false) return false;
  pResult = PQexec(pDBConn, sql);
  if((PQresultStatus(pResult) != PGRES_COMMAND_OK) && (PQresultStatus(pResult) != PGRES_TUPLES_OK)) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query \"%s\" (%s)\n", sql, strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return false;
  } else {
    return true;  
  }
}
char *Database::ExecSentenceJSON(const char* sql, size_t *len) {
  if(pResult != NULL) PQclear(pResult);
  if(Connect() == false) return NULL;
  pResult = PQexec(pDBConn, sql);
  if(PQresultStatus(pResult) != PGRES_TUPLES_OK) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query \"%s\" (%s)\n", sql, strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    return NULL;
  } else {
    if(PQntuples(pResult) == 0) return NULL;
    *len = PQgetlength(pResult, 0, 0);
    return (PQgetisnull(pResult, 0, 0) == true) ? NULL : PQgetvalue(pResult, 0, 0);
  }
}
void Database::PrepareSentence(const char *sentence) {
  pSentence = sentence;
  if(pParamsAlloc != 0) {
    for(int i = 0; i < pParamsLen; i++) free(pParams[i]);
    free(pParams);
  }
  pParamsLen = 0;
  pParamsAlloc = 0;
  for(size_t i = 0; i < strlen(sentence); i++) {
    if(sentence[i] == '?') pParamsAlloc++;
  }
  if(pParamsAlloc != 0) pParams = (char**)malloc(sizeof(char*) * pParamsAlloc);
}
void Database::BindSingleUInt32(uint32_t param) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(sizeof(char) * 11);
    sprintf(pParams[pParamsLen], "%u", param);
    pParamsLen++;
  }
}
void Database::BindSingleUInt64(uint64_t param) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(sizeof(char) * 20);
    sprintf(pParams[pParamsLen], "%" PRIu64, param);
    pParamsLen++;
  }
}
void Database::BindSingleFloat(float param) {
  if(pParamsLen < pParamsAlloc) {
    if(isnan(param)) {
      pParams[pParamsLen] = strdup("NULL");
    } else {
      pParams[pParamsLen] = (char*)malloc(sizeof(char) * 16);
      sprintf(pParams[pParamsLen], "%g", param);
    }
    pParamsLen++;
  }
}
void Database::BindArrayArrayFloat(const vector< vector<float> > &params) {
  if(pParamsLen < pParamsAlloc) {
    stringstream Stream;
    Stream <<  "'{";
    for(uint i = 0; i < params.size(); i++) {
      Stream <<  "{";
      for(uint n = 0; n < params[i].size(); n++) {
        if(isnan(params[i][n])) 
          Stream << "NULL";
        else
          Stream << params[i][n];
        if(n < (params[i].size() - 1))
          Stream << ", ";
        else
          Stream << "}";
      }
      if(i < (params.size() - 1)) Stream << ", ";
    }
    Stream <<  "}'";
    //sprintf(&pParams[pParamsLen][Pos], "}'");
    pParams[pParamsLen] = strdup(Stream.str().c_str());
    pParamsLen++;   
  }
}
void Database::BindArrayUInt16(const vector<uint16_t> &params) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(4 + sizeof(char) * 5 * params.size());
    int Pos = sprintf(pParams[pParamsLen], "'{");
    for(uint i = 0; i < params.size(); i++) Pos += sprintf(&pParams[pParamsLen][Pos], "%u, ", params[i]);
    if(params.size() > 0) Pos -= 2;
    sprintf(&pParams[pParamsLen][Pos], "}'");
    pParamsLen++;
  }
}
void Database::BindArrayFloat(const vector<float> &params) {
  if(pParamsLen < pParamsAlloc) {
    stringstream Stream;
    Stream <<  "'{";
    for(uint i = 0; i < params.size(); i++) {
      if(isnan(params[i])) 
        Stream << "NULL";
      else
        Stream << params[i];
      if(i < (params.size() - 1)) Stream << ", ";
    //    pParams[pParamsLen] = (char*)malloc(4 + sizeof(char) * 18 * len);
//    int Pos = sprintf(pParams[pParamsLen], "'{");

            
//            for(int i = 0; i < len; i++) Pos += sprintf(&pParams[pParamsLen][Pos], isnan(params[i]) ? "NULL, " : "%g, ", params[i]);
//    if(len > 0) Pos -= 2;
    }
    Stream <<  "}'";
    
//    sprintf(&pParams[pParamsLen][Pos], "}'");
    pParams[pParamsLen] = strdup(Stream.str().c_str());
    pParamsLen++;
  }
}
void Database::BindSingleBool(bool param) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = param ? strdup("true") : strdup("false");
    pParamsLen++;
  }
}
void Database::BindSingleString(const char *string, bool identifier) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(sizeof(char) * strlen(string) + 3);
    if(identifier == false)
      sprintf(pParams[pParamsLen], "'%s'", string);
    else
      sprintf(pParams[pParamsLen], "\"%s\"", string);
    pParamsLen++;
  }  
}
uint32_t Database::ResultRows() {
  if(pResult == NULL) return 0;
  return PQntuples(pResult);
}
const char *Database::ResultValue(uint32_t row, const char *name) {
  if(pResult == NULL) return NULL;
  if(PQntuples(pResult) < 1) return NULL;
  int32_t Col = PQfnumber(pResult, name);
  if(Col == -1) return NULL;
  if(PQgetisnull(pResult, row, Col) == 1) return NULL;
  return PQgetvalue(pResult, row, Col);
}
bool Database::ExecSentence() {
  bool Return = false;
  if(pParamsAlloc == pParamsLen) {
    int maxLenght = strlen(pSentence);
    for(int i = 0; i < pParamsLen; i++) maxLenght = maxLenght + strlen(pParams[i]);
    char *Buffer = (char*)malloc(sizeof(char) * maxLenght);
    char *Read   = (char*)pSentence;
    char *Write  = Buffer;
    int   Param  = 0;
    while(true) {
      if(*Read == '?') {
        int Len = strlen(pParams[Param]);
        memcpy(Write, pParams[Param], Len);
        Write = Write + Len;
        Param++;
      } else {
        *Write++ = *Read;
        if(*Read == 0) break;
      }
      Read++;
    }
    Return = ExecSentence(Buffer);
    free(Buffer);
  } else {
    Tools::LogMsg("ERROR", "There are more placeholders than params!!\n");
  }
  if(pParamsAlloc != 0) {
    for(int i = 0; i < pParamsLen; i++) free(pParams[i]);
    free(pParams);
    pParamsAlloc = 0;
  }
  return Return;
}
//======================================================================================================================