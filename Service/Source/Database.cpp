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
bool Database::pInitialized   = false;
char *Database::pDatabaseName = (char*)"openzmeter";
char *Database::pHost         = (char*)"127.0.0.1";;
int   Database::pPort         = 5432;
char *Database::pUser         = (char*)"openzmeter";;
char *Database::pPassword     = (char*)"";;
//----------------------------------------------------------------------------------------------------------------------
Database::Database(char *schema) {
  pSchema    = (schema == NULL) ? (char*)"public" : schema;
  pParams    = NULL;
  pResult    = NULL;
  pParamsLen = 0;
  pDBConn    = NULL;
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
    PQfinish(pDBConn);
    if(pInitialized == false) Tools::LogMsg("ERROR", "Cant open database (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    pDBConn = NULL;
    return false;
  }
  ExecSentence("SET synchronous_commit TO OFF");
  PrepareSentence("SET search_path TO ?");
  BindSingleString(pSchema);
  ExecSentence();
  return true;
}
Database::~Database() {
  if(pResult != NULL) PQclear(pResult);
  PQfinish(pDBConn);
}
void Database::Config(json_object *config) {
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
  if(DB.Connect() == true) {
    //Empiezo a meter tablas si no estan y procedimientos
    pInitialized = true;
  }
}
bool Database::ExecSentence(const char* sql) {
  if(pResult != NULL) PQclear(pResult);
  if(Connect() == false) return false;
  pResult = PQexec(pDBConn, sql);
  if(PQresultStatus(pResult) != PGRES_COMMAND_OK) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQclear(pResult);
    pResult = NULL;
    return false;
  } else {
    PQclear(pResult);
    pResult = NULL;
    return true;  
  }
}
char *Database::ExecSentenceJSON(const char* sql, size_t *len) {
  if(pResult != NULL) PQclear(pResult);
  if(Connect() == false) return NULL;
  pResult = PQexec(pDBConn, sql);
  if(PQresultStatus(pResult) != PGRES_TUPLES_OK) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query (%s)\n", strtok_r(PQerrorMessage(pDBConn), "\r\n", &Save));
    PQclear(pResult);
    pResult = NULL;
    return NULL;
  } else {
    *len = PQgetlength(pResult, 0, 0);
    return PQgetvalue(pResult, 0, 0);
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
void Database::BindArrayArrayFloat(float *params, int lenA, int lenB) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(4 + (4 * sizeof(char) * 18 * lenB * lenA));
    int Pos = sprintf(pParams[pParamsLen], "'{");
    for(int i = 0; i < lenA; i++) {
      Pos += sprintf(&pParams[pParamsLen][Pos], "{");
      for(int n = 0; n < lenB; n++) Pos += sprintf(&pParams[pParamsLen][Pos], isnan(params[i * lenB + n]) ? "NULL, " : "%g, ", params[i * lenB + n]);    
      if(lenB > 0) Pos -= 2;
      Pos += sprintf(&pParams[pParamsLen][Pos], "}, ");
    }
    if(lenA > 0) Pos -= 2;
    sprintf(&pParams[pParamsLen][Pos], "}'");
    pParamsLen++;    
  }
}
void Database::BindArrayUInt16(uint16_t *params, int len) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(4 + sizeof(char) * 5 * len);
    int Pos = sprintf(pParams[pParamsLen], "'{");
    for(int i = 0; i < len; i++) Pos += sprintf(&pParams[pParamsLen][Pos], "%u, ", params[i]);
    if(len > 0) Pos -= 2;
    sprintf(&pParams[pParamsLen][Pos], "}'");
    pParamsLen++;
  }
}
void Database::BindArrayFloat(float *params, int len) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(4 + sizeof(char) * 18 * len);
    int Pos = sprintf(pParams[pParamsLen], "'{");
    for(int i = 0; i < len; i++) Pos += sprintf(&pParams[pParamsLen][Pos], isnan(params[i]) ? "NULL, " : "%g, ", params[i]);
    if(len > 0) Pos -= 2;
    sprintf(&pParams[pParamsLen][Pos], "}'");
    pParamsLen++;
  }
}
void Database::BindSingleBool(bool param) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = param ? strdup("true") : strdup("false");
    pParamsLen++;
  }
}
void Database::BindSingleString(const char *string) {
  if(pParamsLen < pParamsAlloc) {
    pParams[pParamsLen] = (char*)malloc(sizeof(char) * strlen(string) + 3);
    sprintf(pParams[pParamsLen], "'%s'", string);
    pParamsLen++;
  }  
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