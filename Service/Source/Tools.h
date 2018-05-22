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
    static bool            Daemonized;
    static int             LogFile;
    static json_object    *Config;
    static char           *PidFile;
  public :
//    typedef struct {
//      uint16_t HTTPPort;
//      char    *HTTPPath;
//      char    *HTTPPass;
////      char    *Log;
////      char    *Pid;
//    } Global_t;
    typedef struct {
      char             *Type;
      char             *Serial;
      char             *Name;
      float             VoltageGain;
      int               VoltageOffset;
      float             CurrentGain;
      int               CurrentOffset;
      Analyzer *Instance;
    } Device_t;
    typedef struct {
      Device_t *Items;
      uint32_t  Len;
    } Devices_t;
    typedef struct {
      int       NominalFrequency;
      float     NominalVoltage;
    } Electrical_t;
    static Electrical_t  Electrical;
    static Devices_t     Devices;
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