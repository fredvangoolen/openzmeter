/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <json-c/json.h>
#include "HTTPServer.h"
#include "Capture.h"
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static bool          pDaemonized;
    static int           pLogFile;
    static json_object  *pConfig;
    static char         *pPidFile;
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
    static bool         CreateLogFile();
    static uint64_t     GetTime();
    static json_object *GetJSONSysInfo();
    static void         RegisterHandlers(void (*function)(int));
};
//----------------------------------------------------------------------------------------------------------------------