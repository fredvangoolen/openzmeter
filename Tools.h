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
      int   sample_rate;
      int   nominal_frequency;
      float nominal_voltage;
      float VoltageGain;
      int   VoltageOffset;
      float CurrentGain;
      int   CurrentOffset;
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
  public :
    static bool Debug;
    static bool ReadConfig(char *file);
    static void LogMsg(const char *level, const char *msg, ...);
    static void Daemonize();
    static bool CreateLogFile();
    static bool CreatePidFile();
    static bool RemovePidFile();
};
//----------------------------------------------------------------------------------------------------------------------