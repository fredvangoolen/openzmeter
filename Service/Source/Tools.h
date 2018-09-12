/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <json-c/json.h>
#include "Database.h"
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static bool            pDaemonized;
    static FILE           *pLogFile;
    static pthread_mutex_t pLogMutex;
    static json_object    *pConfig;
    static char           *pPidFile;
  public :
    static bool          Debug;
  private :
    static uint64_t ParseProcMem(char *name, char *buff);
    static bool     GetMemory(uint64_t *total, uint64_t *available);
    static bool     CreatePidFile(char *name);
    static bool     RemovePidFile();
  public :
    static bool         ReadConfig(char *file);
    static void         LogMsg(const char *level, const char *msg, ...);
    static void         Daemonize();
    static bool         CreateLogFile(char *name);
    static uint64_t     GetTime();
    static bool         GetSysInfo(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen);
    static void         RegisterHandlers(void (*function)(int));
    static bool         JSONString(json_object *obj, const char *key, char **dst, bool copy);
    static bool         JSONFloat(json_object *obj, const char *key, float *dst);
    static bool         JSONUInt64(json_object *obj, const char *key, uint64_t *dst);
    static bool         JSONArrayFloat(json_object *obj, const char *key, float *dst, int len);
};
//----------------------------------------------------------------------------------------------------------------------