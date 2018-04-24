/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tools.h"
#include "NetworkManager.h"
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
//----------------------------------------------------------------------------------------------------------------------
bool                Tools::Daemonized = false;
bool                Tools::Debug      = false;
FILE               *Tools::LogFile    = NULL;
Tools::Global_t     Tools::Global;
Tools::DB_t         Tools::DB;
Tools::Electrical_t Tools::Electrical;
Tools::Devices_t    Tools::Devices;
//----------------------------------------------------------------------------------------------------------------------
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
  ssize_t i = write(File, PID_Number, Len);
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
  Global.HTTPPort = 8080;
  Global.HTTPPath = (char*)"/usr/share/openZmeter/WEB/";
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