/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "CaptureInterface.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <dbus/dbus.h>
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    typedef struct {
      char    *Name;
      uint32_t Size;
      uint8_t *Ptr;
    } File_t;
  private :
    static bool            Daemonized;
    static FILE           *LogFile;
    static File_t         *Files;
    static uint32_t        FileCount;
    static DBusConnection *DBus;
  public :
    typedef struct {
      uint16_t HTTPPort;
      char    *HTTPPath;
      char    *HTTPPass;
      char    *Log;
      char    *Pid;
    } Global_t;
    typedef struct {
      char             *Type;
      char             *Serial;
      char             *Name;
      float             VoltageGain;
      int               VoltageOffset;
      float             CurrentGain;
      int               CurrentOffset;
      CaptureInterface *Instance;
    } Device_t;
    typedef struct {
      Device_t *Items;
      uint32_t  Len;
    } Devices_t;
    typedef struct {
      int       NominalFrequency;
      float     NominalVoltage;
    } Electrical_t;
    typedef struct {
      char *Database;
      char *Host;
      int   Port;
      char *User;
      char *Password;
    } DB_t;
    static Global_t      Global;
    static Electrical_t  Electrical;
    static DB_t          DB;
    static Devices_t     Devices;
    static bool          Debug;    
  private :
    static char    *rstrip(char* s);
    static char    *lskip(const char* s);
    static bool     Config_Handler(char *section, char *param, char *value);
    static bool     Config_Handler(char *section);
    static uint64_t ParseProcMem(char *name, char *buff);
  public :
    static bool     ReadConfig(char *file);
    static void     LogMsg(const char *level, const char *msg, ...);
    static void     Daemonize();
    static bool     CreateLogFile();
    static bool     CreatePidFile();
    static bool     RemovePidFile();
    static bool     GetMemory(uint64_t *total, uint64_t *available);
    static uint64_t GetTime();
    static uint8_t *GetFile(const char *mame, uint32_t *size);
    static void     GetNetworks();
};
//----------------------------------------------------------------------------------------------------------------------