/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <stdarg.h>
#include <stdio.h>
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static bool  Daemonized;
    static FILE *LogFile;
    static char *PidFileName;
  public :
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
    static Electrical_t Electrical;
    static DB_t         DB;
  public :
    static bool Debug;
    static void LogMsg(const char *level, const char *msg, ...);
    static bool SetLogFile(const char *filename);
    static void Daemonize();
    static bool CreatePidFile();
    static bool RemovePidFile();
};
//----------------------------------------------------------------------------------------------------------------------