/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "CaptureInterface.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <json-c/json.h>
#include <dbus/dbus.h>
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static const uint32_t NM_DEVICE_TYPE_WIFI;
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
    static bool     GetMemory(uint64_t *total, uint64_t *available);
    static bool     SendGetDevices(DBusConnection *bus, json_object *json);
    static bool     RecvGetDevices(DBusConnection *bus, json_object *json, DBusMessage *reply);
    static bool     SendGetDeviceType(DBusConnection *bus, json_object *json, char *devicePath);
    static bool     RecvGetDeviceType(DBusConnection *bus, json_object *json, DBusMessage *reply, char *devicePath);
    static bool     SendGetAccessPoints(DBusConnection *bus, json_object *json, char *devicePath);
    static bool     RecvGetAccessPoints(DBusConnection *bus, json_object *json, DBusMessage *reply);
    static bool     SendGetAPProperties(DBusConnection *bus, json_object *json, char *path);
    static bool     RecvGetAPProperties(DBusConnection *bus, json_object *json, DBusMessage *reply);
    static bool     SendGetActiveConnections(DBusConnection *bus, json_object *json);
    static bool     RecvGetActiveConnections(DBusConnection *bus, json_object *json, DBusMessage* reply);
    static bool     SendGetConnectionProperties(DBusConnection *bus, json_object *json, char *path);
    static bool     RecvGetConnectionProperties(DBusConnection *bus, json_object *json, DBusMessage* reply, bool *isWifi);
    static bool     SendGetIP4Properties(DBusConnection *bus, json_object *json, char *path);
    static bool     RecvGetIP4Properties(DBusConnection *bus, json_object *json, DBusMessage* reply);
    static bool     SendGetSettingsPaths(DBusConnection *bus, json_object *json);
    static bool     RecvGetSettingsPaths(DBusConnection *bus, json_object *json, DBusMessage* reply);
    static bool     SendGetSettings(DBusConnection *bus, json_object *json, char *path);
    static bool     RecvGetSettings(DBusConnection *bus, json_object *json, DBusMessage* reply, bool *iswifi);
  public :
    static bool         ReadConfig(char *file);
    static void         LogMsg(const char *level, const char *msg, ...);
    static void         Daemonize();
    static bool         CreateLogFile();
    static bool         CreatePidFile();
    static bool         RemovePidFile();
    static uint64_t     GetTime();
    static uint8_t     *GetFile(const char *mame, uint32_t *size);
    static json_object *GetJSONSysInfo();
    static json_object *GetJSONNetworks();
};
//----------------------------------------------------------------------------------------------------------------------