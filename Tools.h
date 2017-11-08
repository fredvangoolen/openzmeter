/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static bool  Daemonized;
    static FILE *LogFile;
  public :
    typedef struct {
      uint16_t HTTPPort;
      char    *Listen;
      char    *Log;
      char    *Pid;
    } Global_t;
    typedef struct {
      char    *Type;
      uint16_t PID;
      uint16_t VID;
      char    *Serial;
      uint32_t ID;
      char    *Name;
      float    VoltageGain;
      int      VoltageOffset;
      float    CurrentGain;
      int      CurrentOffset;
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
  private :
    static char *rstrip(char* s);
    static char *lskip(const char* s);
    static bool  Config_Handler(char *section, char *param, char *value);
    static bool  Config_Handler(char *section);
  public :
    static bool Debug;
    static bool ReadConfig(char *file);
    static void LogMsg(const char *level, const char *msg, ...);
    static void Daemonize();
    static bool CreateLogFile();
    static bool CreatePidFile();
    static bool RemovePidFile();
    static uint64_t GetTime();
};
//----------------------------------------------------------------------------------------------------------------------