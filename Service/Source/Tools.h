/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <vector>
#include <json-c/json.h>
#include <json.hpp>
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class Tools {
  private :
    typedef struct {
      string   Name;
      uint32_t Size;
      uint8_t *Ptr;
    } File_t;
  public :
    static uint8_t pBatteryLevel;
    static bool    pBatteryCharging;
    static bool    pExternalPower;
  private :      
    static bool            pDaemonized;
    static FILE           *pLogFile;
    static string          pConfigFile;
    static pthread_mutex_t pLogMutex;
    static pthread_mutex_t pConfigMutex;
    static json_object    *pConfig;
    static string          pPidFile;
    static vector<File_t>  pFiles;
  public :
    static bool          Debug;
  private :
    static uint64_t ParseProcMem(char *name, char *buff);
    static bool     GetMemory(uint64_t *total, uint64_t *available);
    static bool     CreatePidFile(const char *name);
    static bool     RemovePidFile();
  public :
    static bool         ReadConfig(string file);
    static bool         SaveConfig();
    static void         LogMsg(const char *level, const char *msg, ...);
    static void         Daemonize();
    static bool         CreateLogFile(const char *name);
    static uint64_t     GetTime();
    static bool         GetSysInfo(HTTPRequest &req);
    static void         RegisterHandlers(void (*function)(int));
    static bool         JSONString(const json_object *obj, const char *key, char **dst, bool copy = false);
    static bool         JSONFloat(const json_object *obj, const char *key, float *dst);
    static bool         JSONUInt64(const json_object *obj, const char *key, uint64_t *dst);
    static bool         JSONUInt32(const json_object *obj, const char *key, uint32_t *dst);
    static bool         JSONArrayFloat(const json_object *obj, const char *key, float *dst, uint len);
    static uint8_t     *GetFile(string mame, size_t *size);
    static json_object *GetConfigObject();
    static char        *trim(char *str);
};
//----------------------------------------------------------------------------------------------------------------------