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
bool Database::Initialized   = false;
char *Database::DatabaseName = (char*)"openzmeter";
char *Database::Host         = (char*)"127.0.0.1";;
int   Database::Port         = 5432;
char *Database::User         = (char*)"openzmeter";;
char *Database::Password     = (char*)"";;
//----------------------------------------------------------------------------------------------------------------------
Database::Database() {
  Params    = NULL;
  Result    = NULL;
  ParamsLen = 0;
}
Database *Database::GetInstance() {
  Database *DB = new Database();
  char ConnectionString[512];
  sprintf(ConnectionString, "host=%s port=%d user=%s password=%s dbname=%s", Host, Port, User, Password, DatabaseName);
  DB->DBConn = PQconnectdb(ConnectionString);
  if(PQstatus(DB->DBConn) == CONNECTION_BAD) {
    char *Save;
    if(Initialized == false) Tools::LogMsg("ERROR", "Cant open database (%s)\n", strtok_r(PQerrorMessage(DB->DBConn), "\r\n", &Save));
    delete DB;
    return NULL;
  }
  DB->ExecSentence("SET synchronous_commit TO OFF");
  return DB;
}
Database::~Database() {
  if(Result != NULL) PQclear(Result);
  PQfinish(DBConn);
}
bool Database::PrepareDB(json_object *config) {
  json_object_object_foreach(config, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "HostName") == 0)) {
      Host = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_int) && (strcmp(JSON_Key, "Port") == 0)) {
      Port = json_object_get_int(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Database") == 0)) {
      DatabaseName = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "UserName") == 0)) {
      User = (char*)json_object_get_string(JSON_Val);
    } else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0)) {
      Password = (char*)json_object_get_string(JSON_Val);
    }
  }
  Database *DB = GetInstance();
  if(DB == NULL) return false;
  
  
  
  
  
  //Empiezo a meter tablas si no estan y procedimientos
  Initialized = true;
  delete DB;
  return true;
}
bool Database::ExecSentence(const char* sql) {
  if(Result != NULL) PQclear(Result);
  Result = PQexec(DBConn, sql);
  if(PQresultStatus(Result) != PGRES_COMMAND_OK) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query (%s)\n", strtok_r(PQerrorMessage(DBConn), "\r\n", &Save));
    PQclear(Result);
    Result = NULL;
    return false;
  } else {
    PQclear(Result);
    Result = NULL;
    return true;  
  }
}
char* Database::ExecSentenceJSON(const char* sql, size_t *len) {
  if(Result != NULL) PQclear(Result);
  Result = PQexec(DBConn, sql);
  if(PQresultStatus(Result) != PGRES_TUPLES_OK) {
    char *Save;
    Tools::LogMsg("ERROR", "Error in SQL query (%s)\n", strtok_r(PQerrorMessage(DBConn), "\r\n", &Save));
    PQclear(Result);
    Result = NULL;
    return NULL;
  } else {
    *len = PQgetlength(Result, 0, 0);
    return PQgetvalue(Result, 0, 0);
  }
}
void Database::PrepareSentence(const char *sentence) {
  Sentence = sentence;
  if(ParamsAlloc != 0) {
    for(int i = 0; i < ParamsLen; i++) free(Params[i]);
    free(Params);
  }
  ParamsLen = 0;
  ParamsAlloc = 0;
  for(size_t i = 0; i < strlen(sentence); i++) {
    if(sentence[i] == '?') ParamsAlloc++;
  }
  if(ParamsAlloc != 0) Params = (char**)malloc(sizeof(char*) * ParamsAlloc);
}
void Database::BindSingleUInt32(uint32_t param) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(sizeof(char) * 11);
    sprintf(Params[ParamsLen], "%u", param);
    ParamsLen++;
  }
}
void Database::BindSingleUInt64(uint64_t param) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(sizeof(char) * 20);
    sprintf(Params[ParamsLen], "%" PRIu64, param);
    ParamsLen++;
  }
}
void Database::BindSingleFloat(float param) {
  if(ParamsLen < ParamsAlloc) {
    if(isnan(param)) {
      Params[ParamsLen] = strdup("NULL");
    } else {
      Params[ParamsLen] = (char*)malloc(sizeof(char) * 16);
      sprintf(Params[ParamsLen], "%g", param);
    }
    ParamsLen++;
  }
}
void Database::BindArrayArrayFloat(float **params, int lenA, int lenB) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(4 + (4 * sizeof(char) * 18 * lenB * lenA));
    int Pos = sprintf(Params[ParamsLen], "'{");
    for(int i = 0; i < lenA; i++) {
      Pos += sprintf(&Params[ParamsLen][Pos], "{");
      for(int n = 0; n < lenB; n++) Pos += sprintf(&Params[ParamsLen][Pos], isnan(params[i][n]) ? "NULL, " : "%g, ", params[i][n]);    
      if(lenB > 0) Pos -= 2;
      Pos += sprintf(&Params[ParamsLen][Pos], "}, ");
    }
    if(lenA > 0) Pos -= 2;
    sprintf(&Params[ParamsLen][Pos], "}'");
    ParamsLen++;    
  }
}
void Database::BindArrayUInt16(uint16_t *params, int len) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(4 + sizeof(char) * 5 * len);
    int Pos = sprintf(Params[ParamsLen], "'{");
    for(int i = 0; i < len; i++) Pos += sprintf(&Params[ParamsLen][Pos], "%u, ", params[i]);
    if(len > 0) Pos -= 2;
    sprintf(&Params[ParamsLen][Pos], "}'");
    ParamsLen++;
  }
}

void Database::BindArrayFloat(float *params, int len) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(4 + sizeof(char) * 18 * len);
    int Pos = sprintf(Params[ParamsLen], "'{");
    for(int i = 0; i < len; i++) Pos += sprintf(&Params[ParamsLen][Pos], isnan(params[i]) ? "NULL, " : "%g, ", params[i]);
    if(len > 0) Pos -= 2;
    sprintf(&Params[ParamsLen][Pos], "}'");
    ParamsLen++;
  }
}
void Database::BindSingleBool(bool param) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = param ? strdup("true") : strdup("false");
    ParamsLen++;
  }
}
void Database::BindSingleString(const char *string) {
  if(ParamsLen < ParamsAlloc) {
    Params[ParamsLen] = (char*)malloc(sizeof(char) * strlen(string) + 3);
    sprintf(Params[ParamsLen], "'%s'", string);
    ParamsLen++;
  }  
}
bool Database::ExecSentence() {
  bool Return = false;
  if(ParamsAlloc == ParamsLen) {
    int maxLenght = strlen(Sentence);
    for(int i = 0; i < ParamsLen; i++) maxLenght = maxLenght + strlen(Params[i]);
    char *Buffer = (char*)malloc(sizeof(char) * maxLenght);
    char *Read   = (char*)Sentence;
    char *Write  = Buffer;
    int   Param  = 0;
    while(true) {
      if(*Read == '?') {
        int Len = strlen(Params[Param]);
        memcpy(Write, Params[Param], Len);
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
  if(ParamsAlloc != 0) {
    for(int i = 0; i < ParamsLen; i++) free(Params[i]);
    free(Params);
    ParamsAlloc = 0;
  }
  return Return;
}


//
//// Tipos privados ======================================================================================================
//typedef struct {
//  enum DB_Type Type;
//  bool         SaveAndFree;
//  uint32_t     Node;
//  uint64_t     Timestamp;
//  union {
//    Params_t *Params;
//    Event_t  *Event;
//    float    *Prices;
//    void     *Data;
//  } Data;
//} DBSave_t;
//// Variables privadas ==================================================================================================
//static DBSave_t        *Buffer;               //Donde almaceno trabajos pendientes (RingBuffer)
//static uint32_t         ReadPos;              //Posicion de lectura del buffer
//static uint32_t         WritePos;             //Posicion de escritura del buffer
//static uint32_t         Size;                 //Tamaño total del buffer
//static uint32_t         Used;                 //Muestras en uso del buffer  
//static pthread_mutex_t  Mutex;                //Mutex para bloquear acceso al buffer
//static pthread_t        Thread;               //Descriptor del hilo de captura
//static bool             Terminate;            //Señal para detener el hilo de captura
//static PGconn          *Database;             //Conexion a la base de datos
//// Funciones ===========================================================================================================
//int DB_AppendArrayVal(char *str, int len, float *val) {
//  int Len = 0;
//  if(len == 0) return sprintf(&str[Len], "'{}'");
//  Len += sprintf(&str[Len], "'{");
//  for(int i = 0; i < len; i++) Len += isnan(val[i]) ? sprintf(&str[Len], "NULL, ") : sprintf(&str[Len], "%e, ", val[i]);
//  Len -= 2;
//  Len += sprintf(&str[Len], "}'");
//  return Len;  
//}
//void *DB_Thread(void *args) {
//  char Buff[65536];
//  pthread_setname_np(Thread, "Data Store");
//  while(!Terminate) {
//    int   Len = 0;
//    do {
//      pthread_mutex_lock(&Mutex);
//      if(Used > 0) {
//        pthread_mutex_unlock(&Mutex);
//        break;
//      }
//      pthread_mutex_unlock(&Mutex);
//      usleep(10000);
//    } while(1);
//    if(Buffer[ReadPos].Type == PARAMS_10C) {
//      Params_t *Params = Buffer[ReadPos].Data.Params;
//      Len  = sprintf(&Buff[Len], "SELECT CREATE_10C(%u, %"PRIu64", ", Buffer[ReadPos].Node, Buffer[ReadPos].Timestamp);
//      Len += isnan(Params->RMS_V) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->RMS_V);
//      Len += isnan(Params->RMS_I) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->RMS_I);
//      Len += isnan(Params->Active_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Active_Power);
//      Len += isnan(Params->Apparent_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Apparent_Power);
//      Len += isnan(Params->Power_Factor) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Power_Factor);
//      Len += isnan(Params->Active_Energy) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Active_Energy);
//      Len += isnan(Params->Fundamental_Reactive_Energy) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Reactive_Energy);
//      Len += isnan(Params->Fundamental_Active_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Active_Power);
//      Len += isnan(Params->Fundamental_Reactive_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Reactive_Power);
//      Len += isnan(Params->Fundamental_Apparent_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Apparent_Power);
//      Len += isnan(Params->Fundamental_Power_Factor) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Power_Factor);
//      Len += isnan(Params->THD_V) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->THD_V);
//      Len += isnan(Params->THD_I) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->THD_I);
//      Len += isnan(Params->Frequency) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Frequency);
//      Len += isnan(Params->Phi) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Phi);
//      Len += sprintf(&Buff[Len], "%s, ", Params->Flag ? "true" : "false");
//      Len += DB_AppendArrayVal(&Buff[Len], 50, Params->VoltageHarmonics);
//      Len += sprintf(&Buff[Len], ", ");
//      Len += DB_AppendArrayVal(&Buff[Len], 50, Params->CurrentHarmonics);        
//      Len += sprintf(&Buff[Len], ");");
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query %s\n (%s)\n", Buff, PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == PARAMS_3S) {
////      LogMsg("DEBUG", "Generando agregacion en tabla 3S\n");
//      sprintf(Buff, "SELECT AGREGATE_3S(%"PRIu64", %u)", Buffer[ReadPos].Timestamp, Buffer[ReadPos].Node);
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == PARAMS_1M) {
////      LogMsg("DEBUG", "Generando agregacion en tabla 1M\n");
//      sprintf(Buff, "SELECT AGREGATE_1M(%"PRIu64", %u)", Buffer[ReadPos].Timestamp, Buffer[ReadPos].Node);
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == PARAMS_10M) {
////      LogMsg("DEBUG", "Generando agregacion en tabla 10M\n");
//      sprintf(Buff, "SELECT AGREGATE_10M(%"PRIu64", %u)", Buffer[ReadPos].Timestamp, Buffer[ReadPos].Node);
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == PARAMS_1H) {
////      LogMsg("DEBUG", "Generando agregacion en tabla 1H\n");
//      sprintf(Buff, "SELECT AGREGATE_1H(%"PRIu64", %u)", Buffer[ReadPos].Timestamp, Buffer[ReadPos].Node);
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == EVENT) {
//      Event_t *Event = Buffer[ReadPos].Data.Event;
//      uint32_t NumSamples = MIN(EVENT_PREINIT + EVENT_POSTINIT + EVENT_PREEND + EVENT_POSTEND, EVENT_PREINIT + Event->Duration + EVENT_POSTEND);
//      Len = sprintf(Buff, "SELECT create_or_update_event(%"PRIu64", %e, ", Event->Start, Event->Duration / ADQUISITION_SAMPLEFREQ);
////      Len += sprintf(&Buff[Len], "'%s', ", EventType_ToString(Event->Type));
//      Len += sprintf(&Buff[Len], "%e, ", Event->Voltage);
//      Len += sprintf(&Buff[Len], "%e, ", Event->Change);
//      printf("Guardando %u muestras\n", NumSamples);
//      Len += DB_AppendArrayVal(&Buff[Len], (Buffer[ReadPos].SaveAndFree) ? NumSamples : 0, Event->Samples);
//      Len += sprintf(&Buff[Len], ");");
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    } else if(Buffer[ReadPos].Type == PRICES) {
//      float *Prices = Buffer[ReadPos].Data.Prices;
//      Len = sprintf(Buff, "SELECT INSERT_PRICES(%"PRIu64", ", Buffer[ReadPos].Timestamp);
//      Len += DB_AppendArrayVal(&Buff[Len], 24, Prices);
//      Len += sprintf(&Buff[Len], ");");
//      PGresult *Result = PQexec(Database, Buff);
////      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//    }
//    if(Buffer[ReadPos].SaveAndFree) free(Buffer[ReadPos].Data.Data);
//    pthread_mutex_lock(&Mutex);
//    ReadPos = (ReadPos + 1) % Size;
//    Used--;
//    pthread_mutex_unlock(&Mutex);
//  }
//  return NULL;
//}
////void DB_Stop() {
////  Terminate = true;
////  Tools::LogMsg("INFO", "Waiting for DB thread ends...");
////  pthread_join(Thread, NULL);
////  PQfinish(Database);
////}
//bool DB_Start(uint32_t *lastprice) {
//  struct timeval Time;
//  if(gettimeofday(&Time, NULL) != 0) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return false;
//  }
//  Tools::LogMsg("INFO", "Starting data save thread...\n");
//  if(pthread_mutex_init(&Mutex, NULL) != 0) {                                  
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    return false;                                                                  
//  }
//  Buffer = (DBSave_t*)malloc(sizeof(DBSave_t) * Config.DB.QueueLen);
//  if(Buffer == NULL) {
//    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
//    pthread_mutex_destroy(&Mutex);
//    return false;
//  }
//  ReadPos = 0;
//  WritePos = 0;
//  Used = 0;
//  char ConnectionString[512];
//  sprintf(ConnectionString, "host=%s port=%d user=%s password=%s dbname=%s", Config.DB.Host, Config.DB.Port, Config.DB.User, Config.DB.Password, Config.DB.Database);
//  Database = PQconnectdb(ConnectionString);
//  if(PQstatus(Database) == CONNECTION_BAD) {
////    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(Database));
//    pthread_mutex_destroy(&Mutex);
//    free(Buffer);
//    PQfinish(Database);
//    Database = NULL;
//    return false;
//  }
//  if(pthread_create(&Thread, NULL, DB_Thread, NULL) != 0) {
////    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(Database));
//    pthread_mutex_destroy(&Mutex);
//    free(Buffer);
//    PQfinish(Database);
//    Database = NULL;
//    return false;
//  }
//  PGresult *Result = PQexec(Database, "SET synchronous_commit TO OFF");
////  if(PQresultStatus(Result) != PGRES_COMMAND_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//  PQclear(Result);
//  char Tmp[128];
//  Tools::LogMsg("INFO", "Resuming from last execution...\n");
//  sprintf(Tmp, "SELECT STARTUP(%"PRIu64")", ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000));
//  Result = PQexec(Database, Tmp);
////  if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//  *lastprice = (PQgetisnull(Result, 0, 0)) ? (uint32_t)time(NULL) : atol(PQgetvalue(Result, 0, 0));
//  PQclear(Result);
//  return true;
//}
//void DB_Save(enum DB_Type type, uint32_t node, uint64_t timestamp, void *data, bool free) {
//  do {
//    pthread_mutex_lock(&Mutex);
//    if(Used < Size) {
//      Buffer[WritePos].Type        = type;
//      Buffer[WritePos].Node        = node;
//      Buffer[WritePos].Timestamp   = timestamp;
//      Buffer[WritePos].Data.Data   = data;
//      Buffer[WritePos].SaveAndFree = free;
//      WritePos = (WritePos + 1) % Size;
//      Used++;
//      pthread_mutex_unlock(&Mutex);
//      break;
//    } else {
//      pthread_mutex_unlock(&Mutex);
//      Tools::LogMsg("ERROR", "DB buffer overflow!\n");
//      usleep(10000);
//    }
//  } while(1);
//}
////======================================================================================================================