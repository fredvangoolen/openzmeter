/* ================================== zMeter ===========================================================================
 * File:     Adquisition.c
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <postgresql/libpq-fe.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <sys/param.h>
#include "Main.h"
/* Almaceno datos en la DB con SQLite ==================================================================================
 * La cantidad de datos a guardar no es demasiado grande y esta bien ordenada, por lo que deberia de accederse rapido 
 * En total la DB contiene:
 *  - Eventos    Ilimitado
 *  - Agreg_10C  (4 horas de vida)       72000 filas
 *  - Agreg_3S   (60 horas de vida)      72000 filas
 *  - Agreg_1M   (50 dias de vida)       72000 filas  *No contemplado en el standar, pero util para graficar
 *  - Agreg_10M  (500 dias de vida)      72000 filas
 *  - Agreg_2H   (6000 dias de vida)     72000 filas
 * A este modulo entran los bloques procesados, y la gestion de las agregaciones se realiza durante la insercion de 
 * estos bloques en un hilo distinto. Al ser el ultimo paso del proceso libero la estructura trabajo 
 * 
 * Se sigue la clase S, en la que no se producen solapes de los periodos. Como ademas de la calidad medimos consumos,
 * esto hace que la medida sea mas precisa.
 */

// Tipos privados ======================================================================================================
typedef struct {
  enum DB_Type Type;
  bool SaveAndFree;
  union {
    Params_t *Params;
    Event_t  *Event;
//    Price_t  *Prices;
    void     *Data;
  } Data;
} DBSave_t;
typedef struct {
  DBSave_t        *Buffer;               //Donde almaceno trabajos pendientes (RingBuffer)
  uint32_t         ReadPos;              //Posicion de lectura del buffer
  uint32_t         WritePos;             //Posicion de escritura del buffer
  uint32_t         Size;                 //Tamaño total del buffer
  uint32_t         Used;                 //Muestras en uso del buffer  
  pthread_mutex_t  Mutex;                //Mutex para bloquear acceso al buffer
  pthread_t        Thread;               //Descriptor del hilo de captura
  bool             Terminate;            //Señal para detener el hilo de captura
  PGconn          *Database;             //Conexion a la base de datos
  uint64_t         Timestamp;            //Marca de tiempo del anterior segmento
} DB_t;

// Variables privadas ==================================================================================================
DB_t DB;
// Funciones ===========================================================================================================
int DB_AppendArrayVal(char *str, int len, float *val) {
  int Len = 0;
  if(len == 0) return sprintf(&str[Len], "'{}'");
  Len += sprintf(&str[Len], "'{");
  for(int i = 0; i < len; i++) Len += isnan(val[i]) ? sprintf(&str[Len], "NULL, ") : sprintf(&str[Len], "%e, ", val[i]);
  Len -= 2;
  Len += sprintf(&str[Len], "}'");
  return Len;  
}
void *DB_Thread(void *args) {
  char Buff[65536];
  pthread_setname_np(DB.Thread, "Data Store");
  while(!DB.Terminate) {
    int   Len = 0;
    do {
      pthread_mutex_lock(&DB.Mutex);
      if(DB.Used > 0) {
        pthread_mutex_unlock(&DB.Mutex);
        break;
      }
      pthread_mutex_unlock(&DB.Mutex);
      usleep(10000);
    } while(1);
    if(DB.Buffer[DB.ReadPos].Type == PARAMS_10C) {
      Params_t *Params = DB.Buffer[DB.ReadPos].Data.Params;
      Len  = sprintf(&Buff[Len], "SELECT CREATE_10C(%llu, ", Params->Timestamp);
      Len += isnan(Params->RMS_V) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->RMS_V);
      Len += isnan(Params->RMS_I) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->RMS_I);
      Len += isnan(Params->Active_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Active_Power);
      Len += isnan(Params->Reactive_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Reactive_Power);
      Len += isnan(Params->Apparent_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Apparent_Power);
      Len += isnan(Params->Power_Factor) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Power_Factor);
      Len += isnan(Params->Active_Energy) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Active_Energy);
      Len += isnan(Params->Reactive_Energy) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Reactive_Energy);
      Len += isnan(Params->Fundamental_Active_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Active_Power);
      Len += isnan(Params->Fundamental_Reactive_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Reactive_Power);
      Len += isnan(Params->Fundamental_Apparent_Power) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Apparent_Power);
      Len += isnan(Params->Fundamental_Power_Factor) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Fundamental_Power_Factor);
      Len += isnan(Params->THD_V) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->THD_V);
      Len += isnan(Params->THD_I) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->THD_I);
      Len += isnan(Params->Frequency) ? sprintf(&Buff[Len], "NULL, ") : sprintf(&Buff[Len], "%e, ", Params->Frequency);
      Len += sprintf(&Buff[Len], "%s, ", Params->Flag ? "true" : "false");
      Len += DB_AppendArrayVal(&Buff[Len], 50, Params->VoltageHarmonics);
      Len += sprintf(&Buff[Len], ", ");
      Len += DB_AppendArrayVal(&Buff[Len], 50, Params->CurrentHarmonics);        
      Len += sprintf(&Buff[Len], ");");
      PGresult *Result = PQexec(DB.Database, Buff);
      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query %s\n (%s)\n", Buff, PQresultErrorMessage(Result));
      PQclear(Result);
      if((Params->SequenceID % 15) == 0) {
        LogMsg("DEBUG", "Generando agregacion en tabla 3S\n");
        sprintf(Buff, "SELECT CREATE_3S(%llu)", DB.Timestamp);
        Result = PQexec(DB.Database, Buff);
        if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
        PQclear(Result);
      }
      if(((Params->Timestamp / 60000) * 60000) != ((DB.Timestamp / 60000) * 60000)) {
        LogMsg("DEBUG", "Generando agregacion en tabla 1M\n");
        sprintf(Buff, "SELECT CREATE_1M(%llu)", DB.Timestamp);
        Result = PQexec(DB.Database, Buff);
        if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
        PQclear(Result);
        if(((Params->Timestamp / 600000) * 600000) != ((DB.Timestamp / 600000) * 600000)) {
          LogMsg("DEBUG", "Generando agregacion en tabla 10M\n");
          sprintf(Buff, "SELECT CREATE_10M(%llu)", DB.Timestamp);
          Result = PQexec(DB.Database, Buff);
          if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
          PQclear(Result);
          if(((Params->Timestamp / 7200000) * 7200000) != ((DB.Timestamp / 7200000) * 7200000)) {
            LogMsg("DEBUG", "Generando agregacion en tabla 2H\n");
            sprintf(Buff, "SELECT CREATE_2H(%llu)", DB.Timestamp);
            Result = PQexec(DB.Database, Buff);
            if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
            PQclear(Result);
          }
        }
      }
      DB.Timestamp = Params->Timestamp;
      if(DB.Buffer[DB.ReadPos].SaveAndFree) free(Params);
    } else if(DB.Buffer[DB.ReadPos].Type == EVENT) {
      Event_t *Event = DB.Buffer[DB.ReadPos].Data.Event;
      uint32_t NumSamples = MIN(EVENT_PREINIT + EVENT_POSTINIT + EVENT_PREEND + EVENT_POSTEND, EVENT_PREINIT + Event->Duration + EVENT_POSTEND);
      Len = sprintf(Buff, "SELECT create_or_update_event(%llu, %e, ", Event->Start, Event->Duration / ADQUISITION_SAMPLEFREQ);
      Len += sprintf(&Buff[Len], "'%s', ", EventType_ToString(Event->Type));
      Len += sprintf(&Buff[Len], "%e, ", Event->Voltage);
      Len += sprintf(&Buff[Len], "%e, ", Event->Change);
      printf("Guardando %d muestras\n", NumSamples);
      Len += DB_AppendArrayVal(&Buff[Len], (DB.Buffer[DB.ReadPos].SaveAndFree) ? NumSamples : 0, Event->Samples);
      Len += sprintf(&Buff[Len], ");");
      PGresult *Result = PQexec(DB.Database, Buff);
      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
      PQclear(Result);
      if(DB.Buffer[DB.ReadPos].SaveAndFree) free(Event);
    } else if(DB.Buffer[DB.ReadPos].Type == PRICES) {
//      Prices_t *Prices = DB.Buffer[DB.ReadPos].Data.Prices;
//      Len = sprintf(Buff, "SELECT update_prices(%u, ", Prices->Timestamp);
//      Len += DB_AppendArrayVal(&Buff[Len], 24, Prices->HourPrice);
//      Len += sprintf(&Buff[Len], ");");
//      PGresult *Result = PQexec(DB.Database, Buff);
//      if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
//      PQclear(Result);
//      if(DB.Buffer[DB.ReadPos].SaveAndFree) free(Prices);
    }
    pthread_mutex_lock(&DB.Mutex);
    DB.ReadPos = (DB.ReadPos + 1) % DB.Size;
    DB.Used--;
    pthread_mutex_unlock(&DB.Mutex);
  }
  return NULL;
}
void DB_Stop() {
  DB.Terminate = true;
  LogMsg("INFO", "Waiting for DB thread ends...");
  pthread_join(DB.Thread, NULL);
  PQfinish(DB.Database);
}
uint32_t DB_Start(char *connString, uint16_t bufferLen) {
  struct timeval Time;
  if(gettimeofday(&Time, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  DB.Timestamp = ((uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000);
  LogMsg("INFO", "Starting data save thread...\n");
  if(pthread_mutex_init(&DB.Mutex, NULL) != 0) {                                  
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;                                                                  
  }
  DB.Buffer = (DBSave_t*)malloc(sizeof(DBSave_t) * bufferLen);
  if(DB.Buffer == NULL) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_mutex_destroy(&DB.Mutex);
    return 0;
  }
  DB.ReadPos = 0;
  DB.WritePos = 0;
  DB.Size = bufferLen;
  DB.Used = 0;
  DB.Database = PQconnectdb(connString);
  if(PQstatus(DB.Database) == CONNECTION_BAD) {
    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(DB.Database));
    pthread_mutex_destroy(&DB.Mutex);
    free(DB.Buffer);
    PQfinish(DB.Database);
    DB.Database = NULL;
    return 0;
  }
  if(pthread_create(&DB.Thread, NULL, DB_Thread, NULL) != 0) {
    LogMsg("ERROR", "No se pudo abrir la base de datos (%s)\n", PQerrorMessage(DB.Database));
    pthread_mutex_destroy(&DB.Mutex);
    free(DB.Buffer);
    PQfinish(DB.Database);
    DB.Database = NULL;
    return 0;
  }
  PGresult *Result = PQexec(DB.Database, "SET synchronous_commit TO OFF");
  if(PQresultStatus(Result) != PGRES_COMMAND_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
  PQclear(Result);
  char Tmp[128];
  LogMsg("INFO", "Resuming from last execution...\n");
  sprintf(Tmp, "SELECT STARTUP(%llu)", DB.Timestamp);
  Result = PQexec(DB.Database, Tmp);
  if(PQresultStatus(Result) != PGRES_TUPLES_OK) LogMsg("ERROR", "Error in SQL query (%s)\n", PQresultErrorMessage(Result));
  uint32_t LastPrice = (PQgetisnull(Result, 0, 0)) ? (uint32_t)time(NULL) : atol(PQgetvalue(Result, 0, 0));
  PQclear(Result);
  return LastPrice;
}
void DB_Save(enum DB_Type type, void *data, bool free) {
  do {
    pthread_mutex_lock(&DB.Mutex);
    if(DB.Used < DB.Size) {
      DB.Buffer[DB.WritePos].Type = type;
      DB.Buffer[DB.WritePos].Data.Data = data;
      DB.Buffer[DB.WritePos].SaveAndFree = free;
      DB.WritePos = (DB.WritePos + 1) % DB.Size;
      DB.Used++;
      pthread_mutex_unlock(&DB.Mutex);
      break;
    } else {
      pthread_mutex_unlock(&DB.Mutex);
      LogMsg("ERROR", "DB buffer overflow!\n");
      usleep(10000);
    }
  } while(1);
}
//======================================================================================================================