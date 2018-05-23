/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "Analyzer.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <json-c/json.h>
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static bool          Daemonized;
    static int           LogFile;
    static json_object  *Config;
    static char         *PidFile;
  public :
    static bool          Debug;
    static bool          Terminate;
  private :
//    static char    *rstrip(char* s);
//    static char    *lskip(const char* s);
//    static bool     Config_Handler(char *section, char *param, char *value);
//    static bool     Config_Handler(char *section);
    static uint64_t ParseProcMem(char *name, char *buff);
    static bool     GetMemory(uint64_t *total, uint64_t *available);
    static bool     CreatePidFile(char *name);
    static bool     RemovePidFile();
    static void     SignalHandler(int signal);
  public :
    static bool         ReadConfig(char *file);
    static void         LogMsg(const char *level, const char *msg, ...);
    static void         Daemonize();
    static bool         CreateLogFile();
    static uint64_t     GetTime();
    static json_object *GetJSONSysInfo();
    static void         RegisterHandlers();
    static void         AddDevice();
};
//----------------------------------------------------------------------------------------------------------------------