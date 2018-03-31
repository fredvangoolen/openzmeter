/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tools.h"
#include <string.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <arpa/inet.h>
//----------------------------------------------------------------------------------------------------------------------
const uint32_t      Tools::NM_DEVICE_TYPE_WIFI = 2;
bool                Tools::Daemonized = false;
bool                Tools::Debug      = false;
FILE               *Tools::LogFile    = NULL;
Tools::Global_t     Tools::Global;
Tools::DB_t         Tools::DB;
Tools::Electrical_t Tools::Electrical;
Tools::Devices_t    Tools::Devices;
//----------------------------------------------------------------------------------------------------------------------
#ifdef EMBED_RESOURCES
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
Tools::File_t *Tools::Files = NULL;
uint32_t Tools::FileCount = 0;
uint8_t *Tools::GetFile(const char* name, uint32_t* size) {
  if(Files == NULL) {
    char *Ptr   = _binary_ResourceFiles_start;
    FileCount = atoi(Ptr);
    Files = (File_t*)malloc(sizeof(File_t) * FileCount);
    Ptr = &Ptr[strlen(Ptr) + 1];
    for(uint32_t i = 0; i < FileCount; i++) {
      Files[i].Name = strdup(Ptr);
      Ptr = &Ptr[strlen(Ptr) + 1];
      Files[i].Size = atoi(Ptr);
      Ptr = &Ptr[strlen(Ptr) + 1];
      Files[i].Ptr = (uint8_t*)Ptr;
      Ptr = &Ptr[Files[i].Size];
    }
  }
  for(uint32_t i = 0; i < FileCount; i++) {
    if(strcmp(name, Files[i].Name) == 0) {
      *size = Files[i].Size;
      return Files[i].Ptr;
    }
  }
  return NULL;
}
#endif
void Tools::LogMsg(const char *level, const char *msg, ...) {
  if((Debug == false) && (strcmp(level, "DEBUG") == 0)) return;
  va_list args;
  va_start(args, msg);
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[80];
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X] ", TimeStamp);
  if(Daemonized == false) {
    flockfile(stdout);
    if(strcmp(level, "ERROR") == 0) {
      printf("\033[1;31m%s\033[0m", DateStr);
    } else if(strcmp(level, "WARN") == 0) {
      printf("\033[1;33m%s\033[0m", DateStr);
    } else if(strcmp(level, "DEBUG") == 0) {
      printf("\033[1;37m%s\033[0m", DateStr);
    } else if(strcmp(level, "INFO") == 0) {
      printf("\033[1;34m%s\033[0m", DateStr);
    } else {
      return;
    }
    vprintf(msg, args);
    fflush(stdout);
    funlockfile(stdout);
  }

  if(LogFile) {
    va_start(args, msg);
    flockfile(LogFile);
    fprintf(LogFile, "%s", DateStr);
    vfprintf(LogFile, msg, args);
    fflush(LogFile);
    funlockfile(LogFile);
  }
  va_end(args);
}
bool Tools::CreateLogFile() {
  LogFile = fopen(Global.Log, "at");
  if(!LogFile) LogFile = fopen(Global.Log, "wt");
  if(!LogFile) {
    Tools::LogMsg("WARN", "Can not create or open log file (%d)\n", errno);
    return true;
  }
  return true;
}
bool Tools::CreatePidFile() {
  char PID_Number[64];
  int Len = sprintf(PID_Number, "%d", getpid());
  remove(Global.Pid);
  int File = open(Global.Pid, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogMsg("WARN", "Can not create PID file (%d)\n", errno);
    return true;
  }
  write(File, PID_Number, Len);
  close(File);
  return true;
}
bool Tools::RemovePidFile() {
 return remove(Global.Pid);
}
void Tools::Daemonize() {
  pid_t ProcID = fork();
  if(ProcID < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  Daemonized = true;
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
bool Tools::ReadConfig(char *file) {
  Global.Pid      = (char*)"/var/run/openZmeter.pid";
  Global.Log      = (char*)"/var/log/openZmeter.log";
  Global.HTTPPort = 80;
  Global.HTTPPath = (char*)"./WEB/";
  Global.HTTPPass = (char*)"admin";
  DB.Database     = (char*)"openzmeter";
  DB.Host         = (char*)"127.0.0.1";
  DB.Port         = 5432;
  DB.User         = (char*)"openzmeter";
  DB.Password     = (char*)"";
  Devices.Items   = NULL;
  Devices.Len     = 0;
  Electrical.NominalFrequency = 50;
  Electrical.NominalVoltage   = 230.0;
  
  if(file == NULL) return true;
  LogMsg("INFO", "Loading config file...\n");
  FILE *File = fopen(file, "r");
  if(!File) {
    LogMsg("WARN", "Can not open config file (%d)\n", errno);
    return true;
  }
  char Line[256];
  char Section[256] = "";
  int  LineNumber   = 0;
  bool Error        = false;
  while(fgets(Line, sizeof(Line), File) != NULL) {
    LineNumber++;
    char *Start = lskip(rstrip(Line));
    if((Start[0] == ';') || (Start[0] == '#') || (strlen(Start) == 0)) {
      continue;
    } else if(Start[0] == '[') {
      char *End = strchr(&Start[1], ']');
      if((End == NULL) || (&Start[1] == End)) {
        LogMsg("ERROR", "Reading config file (unmached ']' at line %d\n", LineNumber);
        Error = true;
        break;
      }
      End[0] = 0;
      strcpy(Section, &Start[1]);
      Config_Handler(Section);
    } else {
      char *End = strchr(&Start[1], '=');
      if(End == NULL) {
        LogMsg("ERROR", "Expected '=' at line %d\n", LineNumber);
        Error = true;
        break;
      }
      End[0] = 0;
      char *Param = rstrip(Start);
      char *Value = lskip(&End[1]);
      if(Config_Handler(Section, Param, Value) == false) {
        Error = true;
        break;
      }
    }
  }
  fclose(File);
  return !Error;
}
char *Tools::rstrip(char* s) {
  char *p = s + strlen(s);
  while(p > s && isspace((unsigned char)(*--p))) *p = '\0';
  return s;
}
char *Tools::lskip(const char* s) {
  while (*s && isspace((unsigned char)(*s))) s++;
  return (char*)s;
}
bool Tools::Config_Handler(char* section) {
  if(strcmp(section, "device") == 0) {
    Tools::Devices.Len++;
    Devices.Items = (Device_t*)realloc(Devices.Items, Devices.Len * sizeof(Device_t));
    Devices.Items[Devices.Len - 1].VoltageGain   =       0.01200000;
    Devices.Items[Devices.Len - 1].VoltageOffset =     292.00000000;
    Devices.Items[Devices.Len - 1].CurrentGain   =      -0.00078125;
    Devices.Items[Devices.Len - 1].CurrentOffset =       0.00000000;
    Devices.Items[Devices.Len - 1].Name          = (char*)"Default";
    Devices.Items[Devices.Len - 1].Serial        =             NULL;
    Devices.Items[Devices.Len - 1].Type          =             NULL;
  }
  return true;
}
bool Tools::Config_Handler(char *section, char *param, char *value) {
  if(strcmp(section, "global") == 0) {
    if(strcmp(param, "log") == 0) {
      Global.Log = strdup(value);
    } else if(strcmp(param, "pid") == 0) {
      Global.Pid = strdup(value);
    } else if(strcmp(param, "httpPort") == 0) {
      Global.HTTPPort = atoi(value);
    } else if(strcmp(param, "httpPath") == 0) {
      Global.HTTPPath = strdup(value);
    } else if(strcmp(param, "httpPass") == 0) {
      Global.HTTPPass = strdup(value);
    }
  } else if(strcmp(section, "database") == 0) {
    if(strcmp(param, "port") == 0) {
      DB.Port = atoi(value);
    } else if(strcmp(param, "database") == 0) {
      DB.Database = strdup(value);
    } else if(strcmp(param, "host") == 0) {
      DB.Host = strdup(value);
    } else if(strcmp(param, "user") == 0) {
      DB.User = strdup(value);
    } else if(strcmp(param, "pass") == 0) {
      DB.Password = strdup(value);
    } else {
      LogMsg("ERROR", "Unknown param '%s' in section '%s'\n", param, section);
      return false;
    }
  } else if(strcmp(section, "electrical") == 0) {
    if(strcmp(param, "nominal_frequency") == 0) {
      Electrical.NominalFrequency = atoi(value);
      if((Electrical.NominalFrequency != 50) && (Electrical.NominalFrequency != 60)) Electrical.NominalFrequency = 50;
    } else if(strcmp(param, "nominal_voltage") == 0) {
      Electrical.NominalVoltage = atof(value);
    } else {
      LogMsg("ERROR", "Unknown param '%s' in section '%s'\n", param, section);
      return false;
    }
  } else if(strcmp(section, "device") == 0) {  
    if(strcmp(param, "voltage_gain") == 0) {
      Devices.Items[Devices.Len - 1].VoltageGain = atof(value);
    } else if(strcmp(param, "voltage_offset") == 0) {
      Devices.Items[Devices.Len - 1].VoltageOffset = atoi(value);
    } else if(strcmp(param, "current_gain") == 0) {
      Devices.Items[Devices.Len - 1].CurrentGain = atof(value);
    } else if(strcmp(param, "current_offset") == 0) {
      Devices.Items[Devices.Len - 1].CurrentOffset = atoi(value);
    } else if(strcmp(param, "name") == 0) {
      Devices.Items[Devices.Len - 1].Name = strdup(value);
    } else if(strcmp(param, "serial") == 0) {
      Devices.Items[Devices.Len - 1].Serial = strdup(value);
    } else if(strcmp(param, "type") == 0) {
      Devices.Items[Devices.Len - 1].Type = strdup(value);      
    } else {
      LogMsg("INFO", "Unknown config param '%s/%s\n", section, param);
    }
  } else if(strcmp(section, "budget")) {
    
    if(strcmp(param, "fixed_price") == 0) {
    
    if(strcmp(value, "false") == 0) {
//      pconfig->budget.fixed_price = -1;
    } else {
//      pconfig->budget.fixed_price = atof(value);
    }
//  } else if(MATCH("budget", "country")) {
//    pconfig->budget.country = strdup(value);
//    pconfig->budget.country_config = NULL;
    }
  } else {
//    if(pconfig->budget.country != NULL && strcmp(pconfig->budget.country, "spain") == 0) {
//      budget_spain_configuration * countryConfig;
//      if(pconfig->budget.country_config == NULL) {
//        pconfig->budget.country_config = malloc(sizeof(budget_spain_configuration));
//      }
//      countryConfig = (budget_spain_configuration*) (pconfig->budget.country_config);
//      if(MATCH("budget-spain", "contracted_power")) {
//        countryConfig->contracted_power = atoi(value);
//      } else if(MATCH("budget-spain", "cpc")) {
//        countryConfig->cpc = atof(value);
//      } else if(MATCH("budget-spain", "rate_type")) {
//        countryConfig->rate_type = strdup(value);
//      } else if(MATCH("budget-spain", "vat")) {
//        countryConfig->vat = atof(value);
//      } else if(MATCH("budget-spain", "discount_percentage")) {
//        countryConfig->discount_percentage = atof(value);
//      } else if(MATCH("budget-spain", "electricity_tax_percentage")) {
//        countryConfig->electricity_tax_percentage = atof(value);
//      } else if(MATCH("budget-spain", "meter_rating")) {
//        countryConfig->meter_rating = atof(value);
//      } else {
//        return 0;
//      }
//    } else {
//      return 0; /* unknown section/name, error */
//    }
  }
  return true;
}
uint64_t Tools::GetTime() {
  struct timeval Time;
  if(gettimeofday(&Time, NULL) != 0) {
    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000;
}
uint64_t Tools::ParseProcMem(char *name, char *buff) {
  char *hit = strstr(buff, name);
  if(hit == NULL) return 0;
  errno = 0;
  long Val = strtol(hit + strlen(name), NULL, 10);
  if(errno != 0) return 0;
  return Val * 1024;
}
bool Tools::GetMemory(uint64_t *total, uint64_t *available) {
  char Buffer[8192];
  FILE *FD = fopen("/proc/meminfo", "r");
  if(FD == NULL) return false;
  size_t Len = fread(Buffer, 1, sizeof(Buffer), FD);
  fclose(FD);
  if(Len == 0) return false;
  Buffer[Len] = 0;
  *total = ParseProcMem((char*)"MemTotal:", Buffer);
  *available = ParseProcMem((char*)"MemFree:", Buffer) + ParseProcMem((char*)"Cached:", Buffer) + ParseProcMem((char*)"Buffers:", Buffer);
  return ((*total != 0) && (*available != 0));
}
json_object *Tools::GetJSONSysInfo() {
  struct sysinfo Info;
  struct statvfs Disk;
  if(sysinfo(&Info) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  if(statvfs("/", &Disk) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  uint64_t TotalRam, FreeRam;
  if(GetMemory(&TotalRam, &FreeRam) == false) return NULL;
  json_object *Result = json_object_new_object();
  json_object_object_add(Result, "UpTime", json_object_new_int(Info.uptime));
  json_object_object_add(Result, "Swap_Total", json_object_new_int64(Info.totalswap * Info.mem_unit));
  json_object_object_add(Result, "Swap_Free", json_object_new_int64(Info.freeswap * Info.mem_unit));
  json_object_object_add(Result, "RAM_Total", json_object_new_int64(TotalRam));
  json_object_object_add(Result, "RAM_Free", json_object_new_int64(FreeRam));
  json_object_object_add(Result, "CPU_Usage", json_object_new_double(Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)));
  json_object_object_add(Result, "CPU_Count", json_object_new_int(get_nprocs()));
  json_object_object_add(Result, "Disk_Total", json_object_new_int64((uint64_t)Disk.f_blocks * Disk.f_frsize));
  json_object_object_add(Result, "Disk_Free", json_object_new_int64((uint64_t)Disk.f_bfree * Disk.f_frsize));
  json_object_object_add(Result, "Battery_Percent", json_object_new_int(100));
  return Result;
}
void Tools::SendGetDevices(DBusConnection *bus, json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDevices");
  if(Msg) Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Msg) RecvGetDevices(bus, json, Msg);
}
void Tools::RecvGetDevices(DBusConnection *bus, json_object *json, DBusMessage *reply) {
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
        char *DevicePath;
        dbus_message_iter_get_basic(&Array, &DevicePath);
        if(DevicePath) SendGetDeviceType(bus, json, DevicePath);
      }
    } while(dbus_message_iter_next(&Array));
  }
}
void Tools::SendGetDeviceType(DBusConnection *bus, json_object *json, char *devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.DBus.Properties", "Get");
  if(Msg) {
    const char *ParamInterface = "org.freedesktop.NetworkManager.Device";
    const char *ParamDeviceType = "DeviceType";
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface)) return;
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType)) return;
    Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  }
  if(Msg) RecvGetDeviceType(bus, json, Msg, devicePath);
}
void Tools::RecvGetDeviceType(DBusConnection *bus, json_object *json, DBusMessage *reply, char *devicePath) {
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
    DBusMessageIter VariantIter;
    dbus_message_iter_recurse(&Struct, &VariantIter);
    if(dbus_message_iter_get_arg_type(&VariantIter) == DBUS_TYPE_UINT32) {
      uint32_t DeviceType;
      dbus_message_iter_get_basic(&VariantIter, &DeviceType);
      if(DeviceType == NM_DEVICE_TYPE_WIFI) SendGetAccessPoints(bus, json, devicePath);
    }
  }
}
void Tools::SendGetAccessPoints(DBusConnection *bus, json_object *json, char* devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.NetworkManager.Device.Wireless", "GetAccessPoints");
  if(Msg) Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Msg) RecvGetAccessPoints(bus, json, Msg);
}
void Tools::RecvGetAccessPoints(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
        char *Path;
        dbus_message_iter_get_basic(&Array, &Path);
        if(Path) SendGetAPProperties(bus, json, Path);
      }
    } while(dbus_message_iter_next(&Array));
  }
}
void Tools::SendGetAPProperties(DBusConnection *bus, json_object *json, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  if(Msg) {
    const char *Param = "org.freedesktop.NetworkManager.AccessPoint";
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Param)) return;
    Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  }
  if(Msg) {
    json_object *AccessPoint = json_object_new_object();
    if(RecvGetAPProperties(bus, json, Msg) == false) {
      json_object_put(AccessPoint);
    } else {
      json_object_array_add(json, AccessPoint);
    }    
  }
}
bool Tools::RecvGetAPProperties(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  bool IsAP = true;
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter Dict;
        dbus_message_iter_recurse(&Array, &Dict);
        do {
          if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
            const char *Key;
            dbus_message_iter_get_basic(&Dict, &Key);
            dbus_message_iter_next(&Dict);
            if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
              DBusMessageIter Variant;
              dbus_message_iter_recurse(&Dict, &Variant);
              if((strcmp(Key, "Ssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                DBusMessageIter Array;
                dbus_message_iter_recurse(&Variant, &Array);
                char Name[33] = {0};
                uint32_t i = 0;
                do {
                  if (dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_BYTE) dbus_message_iter_get_basic(&Array, &Name[i++]);
                } while (dbus_message_iter_next(&Array) && (i < sizeof (Name)));
                if(i) json_object_object_add(json, "SSID", json_object_new_string(Name));
                break;
              }
              if((strcmp(Key, "HwAddress") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                char *hwAddress;
                dbus_message_iter_get_basic(&Variant, &hwAddress);
                if (hwAddress) json_object_object_add(json, "MAC", json_object_new_string(hwAddress));
                break;
              }
              if((strcmp(Key, "WpaFlags") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                uint32_t Flags;
                dbus_message_iter_get_basic(&Variant, &Flags);
                json_object_object_add(json, "Flags", json_object_new_int(Flags));
                if(Flags & 0x00000200) IsAP = false;
                break;
              }
              if((strcmp(Key, "Mode") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                uint32_t Mode;
                dbus_message_iter_get_basic(&Variant, &Mode);
                if(Mode != 2) IsAP = false;
                break;
              }
              if((strcmp(Key, "Frequency") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                uint32_t Freq;
                dbus_message_iter_get_basic(&Variant, &Freq);
                json_object_object_add(json, "Freq", json_object_new_int(Freq));
                break;
              }
              if((strcmp(Key, "Strength") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_BYTE)) {
                uint8_t Power;
                dbus_message_iter_get_basic(&Variant, &Power);
                json_object_object_add(json, "Power", json_object_new_int(Power));
                break;
              }
            }
          }
        } while (dbus_message_iter_next(&Dict));
      }
    } while (dbus_message_iter_next(&Array));
  }
  return IsAP;
}
void Tools::SendGetActiveConnections(DBusConnection *bus, json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.DBus.Properties", "Get");
  if(Msg) {
    const char *ParamInterface = "org.freedesktop.NetworkManager";
    const char *ParamDeviceType = "ActiveConnections";
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface)) return;
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType)) return;
    Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  }
  if(Msg) RecvGetActiveConnections(bus, json, Msg);
}
void Tools::RecvGetActiveConnections(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  DBusMessageIter Struct, Variant;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
    dbus_message_iter_recurse(&Struct, &Variant);
    if(dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Variant, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
          char *DevicePath;        
          dbus_message_iter_get_basic(&Array, &DevicePath);
          if(DevicePath) SendGetConnectionProperties(bus, json, DevicePath);
        }
      } while(dbus_message_iter_next(&Array));
    }
  }
}
void Tools::SendGetConnectionProperties(DBusConnection *bus, json_object *json, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  if(Msg) {
    const char *ParamInterface = "org.freedesktop.NetworkManager.Connection.Active";
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface)) return;
    Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  }
  if(Msg) {
    json_object *Connection = json_object_new_object();
    if(RecvGetConnectionProperties(bus, Connection, Msg) == false) {
      json_object_put(Connection);
    } else {
      json_object_array_add(json, Connection);
    }
  }
}
bool Tools::RecvGetConnectionProperties(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  bool IsWifi = false;
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter Dict;
        dbus_message_iter_recurse(&Array, &Dict);
        do {
          if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
            const char *Key;
            dbus_message_iter_get_basic(&Dict, &Key);
            dbus_message_iter_next(&Dict);
            if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
              DBusMessageIter Variant;
              dbus_message_iter_recurse(&Dict, &Variant);
              if((strcmp(Key, "Type") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                char *Type;
                dbus_message_iter_get_basic(&Variant, &Type);
                if(Type) IsWifi = (strcmp(Type, "802-11-wireless") == 0);
                break;
              }
              if((strcmp(Key, "SpecificObject") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                char *AccessPoint;
                dbus_message_iter_get_basic(&Variant, &AccessPoint);
                if(AccessPoint) SendGetAPProperties(bus, json, AccessPoint);
                break;
              }
              if((strcmp(Key, "Ip4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                char *IP4Config;
                dbus_message_iter_get_basic(&Variant, &IP4Config);
                if(IP4Config) SendGetIP4Properties(bus, json, IP4Config);
                break;
              }
              if((strcmp(Key, "Dhcp4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                char *DHCP4Config;
                dbus_message_iter_get_basic(&Variant, &DHCP4Config);
                if(DHCP4Config && (strcmp(DHCP4Config, "/") == 0)) {
                  json_object_object_add(json, "DHCP", json_object_new_boolean(false));
                } else {
                  json_object_object_add(json, "DHCP", json_object_new_boolean(true));
                }
                break;
              }
            }
          }
        } while (dbus_message_iter_next(&Dict));
      }
    } while(dbus_message_iter_next(&Array));
  }
  return IsWifi;
}
void Tools::SendGetIP4Properties(DBusConnection *bus, json_object *json, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  if(Msg) {
    const char *ParamInterface = "org.freedesktop.NetworkManager.IP4Config";
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface)) return;
    Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  }
  if(Msg) RecvGetIP4Properties(bus, json, Msg);
}
void Tools::RecvGetIP4Properties(DBusConnection* bus, json_object* json, DBusMessage* reply) {
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter Dict;
        dbus_message_iter_recurse(&Array, &Dict);
        do {
          if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
            const char *Key;
            dbus_message_iter_get_basic(&Dict, &Key);
            dbus_message_iter_next(&Dict);
            if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
              DBusMessageIter Variant;
              dbus_message_iter_recurse(&Dict, &Variant);
              if((strcmp(Key, "Nameservers") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                DBusMessageIter VariantMember;
                dbus_message_iter_recurse(&Variant, &VariantMember);
                uint32_t i = 0;
                do {
                  if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_UINT32) {
                    struct in_addr DNS;
                    dbus_message_iter_get_basic(&VariantMember, &DNS);
                    i++;
                    char DNSName[16];
                    sprintf(DNSName, "DNS%d", i);
                    json_object_object_add(json, DNSName, json_object_new_string(inet_ntoa(DNS)));
                  }
                } while(dbus_message_iter_next(&VariantMember));
                break;
              }
              if((strcmp(Key, "Addresses") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                DBusMessageIter Array;
                dbus_message_iter_recurse(&Variant, &Array);
                if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_ARRAY) {
                  DBusMessageIter SubArray;
                  dbus_message_iter_recurse(&Array, &SubArray);
                  if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                    struct in_addr IP;
                    dbus_message_iter_get_basic(&SubArray, &IP);
                    json_object_object_add(json, "IP", json_object_new_string(inet_ntoa(IP)));
                  }
                  dbus_message_iter_next(&SubArray);
                  if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                    uint32_t Mask;
                    dbus_message_iter_get_basic(&SubArray, &Mask);
                    json_object_object_add(json, "Mask", json_object_new_int(Mask));
                  }
                  dbus_message_iter_next(&SubArray);
                  if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                    struct in_addr Gateway;
                    dbus_message_iter_get_basic(&SubArray, &Gateway);
                    json_object_object_add(json, "Gateway", json_object_new_string(inet_ntoa(Gateway)));
                  }
                }
                break;
              }
            }
          }
        } while (dbus_message_iter_next(&Dict));
      }
    } while(dbus_message_iter_next(&Array));
  }
}
void Tools::SendGetSettingsPaths(DBusConnection *bus, json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", "ListConnections");
  if(Msg) Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Msg) RecvGetSettingsPaths(bus, json, Msg);
}
void Tools::RecvGetSettingsPaths(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
        char *Path;
        dbus_message_iter_get_basic(&Array, &Path);
        if(Path) SendGetSettings(bus, json, Path);
      }
    } while(dbus_message_iter_next(&Array));
  }
}
void Tools::SendGetSettings(DBusConnection *bus, json_object *json, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
  if(Msg) Msg = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Msg) {
    json_object *Setting = json_object_new_object();
    json_object_object_add(Setting, "ConfigPath", json_object_new_string(path));
    if(RecvGetSettings(bus, Setting, Msg) == false) {
      json_object_put(Setting);
    } else {
      json_object_array_add(json, Setting);
    }
  }
}
bool Tools::RecvGetSettings(DBusConnection *bus, json_object *json, DBusMessage* reply) {
  bool IsWifi = false;
  DBusMessageIter Struct;
  if(dbus_message_iter_init(reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
    DBusMessageIter Array;
    dbus_message_iter_recurse(&Struct, &Array);
    do {
      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
        DBusMessageIter Dict;
        dbus_message_iter_recurse(&Array, &Dict);
        do {
          if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
            const char *Key;
            dbus_message_iter_get_basic(&Dict, &Key);
            dbus_message_iter_next(&Dict);
            if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_ARRAY)) {
              DBusMessageIter Array;
              dbus_message_iter_recurse(&Dict, &Array);
              if(strcmp(Key, "802-11-wireless") == 0) {
                do {
                  if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                    DBusMessageIter Dict;
                    dbus_message_iter_recurse(&Array, &Dict);
                    do {
                      if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
                        const char *Key;
                        dbus_message_iter_get_basic(&Dict, &Key);
                        dbus_message_iter_next(&Dict);
                        if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                          DBusMessageIter Variant;
                          dbus_message_iter_recurse(&Dict, &Variant);
                          if((strcmp(Key, "mode") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                            char *Mode;
                            dbus_message_iter_get_basic(&Variant, &Mode);
                            if(Mode) IsWifi = (strcmp(Mode, "infrastructure") == 0);
                            break;
                          }
                          if((strcmp(Key, "ssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                            DBusMessageIter VariantMember;
                            dbus_message_iter_recurse(&Variant, &VariantMember);
                            char Name[32] = { 0 };
                            uint32_t i = 0;
                            do {
                              if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_BYTE) dbus_message_iter_get_basic(&VariantMember, &Name[i++]);
                            } while(dbus_message_iter_next(&VariantMember) && (i < sizeof(Name)));
                            if(i) json_object_object_add(json, "SSID", json_object_new_string(Name));
                            break;
                          }
                          if((strcmp(Key, "bssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                            DBusMessageIter VariantMember;
                            dbus_message_iter_recurse(&Variant, &VariantMember);
                            char MAC[32];
                            uint32_t i = 0;
                            do {
                              if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_BYTE) {
                                uint8_t Byte;
                                dbus_message_iter_get_basic(&VariantMember, &Byte);
                                sprintf(&MAC[(i++) * 3], "%02X:", Byte);
                              }
                            } while(dbus_message_iter_next(&VariantMember) && (i < 7));
                            MAC[17] = 0;
                            json_object_object_add(json, "MAC", json_object_new_string(MAC));
                            break;
                          }
                        }
                      }
                    } while (dbus_message_iter_next(&Dict));
                  }
                } while (dbus_message_iter_next(&Array));
                break;
              }
              if(!strcmp(Key, "ipv4")) {
                do {
                  if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                    DBusMessageIter Dict;
                    dbus_message_iter_recurse(&Array, &Dict);
                    do {
                      const char *Key;
                      dbus_message_iter_get_basic(&Dict, &Key);
                      dbus_message_iter_next(&Dict);
                      if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                        DBusMessageIter Variant;
                        dbus_message_iter_recurse(&Dict, &Variant);
                        if((strcmp(Key, "method") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                          char *DHCP;
                          dbus_message_iter_get_basic(&Variant, &DHCP);
                          if(DHCP) json_object_object_add(json, "DHCP", json_object_new_boolean(strcmp(DHCP, "auto") == 0));
                          break;
                        }
                        if((strcmp(Key, "dns") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                          DBusMessageIter VariantMember;
                          dbus_message_iter_recurse(&Variant, &VariantMember);
                          uint32_t i = 0;
                          do {
                            if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_UINT32) {
                              struct in_addr DNS;
                              dbus_message_iter_get_basic(&VariantMember, &DNS);
                              char DNSName[16];
                              sprintf(DNSName, "DNS%d", i++);
                              json_object_object_add(json, DNSName, json_object_new_string(inet_ntoa(DNS)));
                            }
                          } while(dbus_message_iter_next(&VariantMember));
                          break;
                        }
                        if((strcmp(Key, "addresses") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                          DBusMessageIter Array;
                          dbus_message_iter_recurse(&Variant, &Array);
                          if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_ARRAY) {
                            DBusMessageIter SubArray;
                            dbus_message_iter_recurse(&Array, &SubArray);
                            if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                              struct in_addr IP;
                              dbus_message_iter_get_basic(&SubArray, &IP);
                              json_object_object_add(json, "IP", json_object_new_string(inet_ntoa(IP)));
                            }
                            dbus_message_iter_next(&SubArray);
                            if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                              uint32_t Mask;
                              dbus_message_iter_get_basic(&SubArray, &Mask);
                              json_object_object_add(json, "Mask", json_object_new_int(Mask));
                            }
                            dbus_message_iter_next(&SubArray);
                            if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                              struct in_addr Gateway;
                              dbus_message_iter_get_basic(&SubArray, &Gateway);
                              json_object_object_add(json, "Gateway", json_object_new_string(inet_ntoa(Gateway)));
                            }
                          }
                          break;
                        }
                      }
                    } while (dbus_message_iter_next(&Dict));
                  }
                } while (dbus_message_iter_next(&Array));
                break;
              }
            }
          }
        } while (dbus_message_iter_next(&Dict));
      }
    } while(dbus_message_iter_next(&Array));
  }
  return IsWifi;
}
json_object *Tools::GetJSONNetworks() {
  DBusConnection *DBus;
  DBusError Error;
  dbus_error_init(&Error);
  DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    Tools::LogMsg("ERROR", "Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
  }
  if(DBus == NULL) return NULL;
  json_object *Result = json_object_new_object();
  json_object *AccessPoints = json_object_new_array();
  SendGetDevices(DBus, AccessPoints);
  json_object_object_add(Result, "AccessPoints", AccessPoints);
  json_object *ActiveConnections = json_object_new_array();
  SendGetActiveConnections(DBus, ActiveConnections);
  json_object_object_add(Result, "ActiveConnections", ActiveConnections);
  json_object *Settings = json_object_new_array();
  SendGetSettingsPaths(DBus, Settings);
  json_object_object_add(Result, "Settings", Settings);
  dbus_connection_unref(DBus);
  size_t Size;
  char *Tmp = (char*)json_object_to_json_string_length(Result, JSON_C_TO_STRING_PRETTY, &Size);
  printf("%s\n", Tmp);
  return Result;
}

//954