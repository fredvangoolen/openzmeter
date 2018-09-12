/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include "Tools.h"
#include "NetworkManager.h"
#include "HTTPServer.h"
#include "DeviceInterface.h"
//----------------------------------------------------------------------------------------------------------------------
bool         Tools::Debug        = false;
bool         Tools::pDaemonized  = false;
FILE        *Tools::pLogFile     = NULL;
json_object *Tools::pConfig      = NULL;
char        *Tools::pPidFile     = NULL;
pthread_mutex_t Tools::pLogMutex = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
void Tools::RegisterHandlers(void (*function)(int)) {
  signal(SIGINT, function);
  signal(SIGTERM, function);
}
void Tools::LogMsg(const char *level, const char *msg, ...) {
  if((Debug == false) && (strcmp(level, "DEBUG") == 0)) return;
  va_list args;
  va_start(args, msg);
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[80];
  pthread_mutex_lock(&pLogMutex);
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X] ", TimeStamp);
  if(pDaemonized == false) {
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
  }
  if(pLogFile) {
    va_start(args, msg);
    fprintf(pLogFile, "%s", DateStr);
    vfprintf(pLogFile, msg, args);
  }
  pthread_mutex_unlock(&pLogMutex);
  va_end(args);
}
bool Tools::CreateLogFile(char *name) {
  pLogFile = fopen(name, "at");
  if(!pLogFile) pLogFile = fopen(name, "wt");
  if(pLogFile < 0) {
    Tools::LogMsg("WARN", "Can not create or open log file (%d)\n", errno);
    return true;
  }
  return true;
}
bool Tools::CreatePidFile(char *name) {
  pPidFile = name;
  char PID_Number[64];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(pPidFile) remove(pPidFile);
  int File = open(pPidFile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogMsg("WARN", "Can not create PID file (%d)\n", errno);
    return true;
  }
  ssize_t i = write(File, PID_Number, Len);
  close(File);
  return true;
}
bool Tools::RemovePidFile() {
  if(pPidFile) return remove(pPidFile);
  return true;
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
  pDaemonized = true;
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
bool Tools::ReadConfig(char *file) {
  if(file == NULL) return true;
  LogMsg("INFO", "Loading config file...\n");
  int File = open(file, O_RDONLY);
  if(File == -1) {
    LogMsg("ERROR", "Can not open config file (%s)\n", strerror(errno));
    return false;
  }
  struct stat FileStatus;
  if(fstat(File, &FileStatus) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(File);
    return false;
  }
  char *ConfigText = (char*)malloc(FileStatus.st_size + 1);
  if(ConfigText == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(File);
    return false;
  }
  FileStatus.st_size = read(File, ConfigText, FileStatus.st_size);
  ConfigText[FileStatus.st_size] = 0;
  json_object *Config = json_tokener_parse(ConfigText);
  close(File);
  free(ConfigText);
  if(Config == NULL) {
    LogMsg("ERROR", "Can not open config file (Invalid JSON format)\n");
    return false;
  }
  json_object *Find;  
  if((json_object_object_get_ex(Config, "LogFile", &Find) == true) && (json_object_get_type(Find) == json_type_string)) CreateLogFile((char*)json_object_get_string(Find));
  if((json_object_object_get_ex(Config, "PidFile", &Find) == true) && (json_object_get_type(Find) == json_type_string)) CreatePidFile((char*)json_object_get_string(Find));
  if((json_object_object_get_ex(Config, "Database", &Find) == true) && (json_object_get_type(Find) == json_type_object)) {
    if(Database::Config(Find) == false) return false;
  }
  if((json_object_object_get_ex(Config, "HTTPServer", &Find) == true) && (json_object_get_type(Find) == json_type_object)) {
    if(HTTPServer::Config(Find) == false) return false;
  }
  if((json_object_object_get_ex(Config, "Devices", &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    if(DeviceInterface::Config(Find) == false) return false;
  } else {
    if(DeviceInterface::Config(NULL)  == false) return false;
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
bool Tools::GetSysInfo(char *user, json_object *reader, Database *db, json_object **writer, char **postdata, size_t *resultLen) {
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
  if(GetMemory(&TotalRam, &FreeRam) == false) return false;
  *writer = json_object_new_object();
  json_object_object_add(*writer, "UpTime", json_object_new_int(Info.uptime));
  json_object_object_add(*writer, "Swap_Total", json_object_new_int64(Info.totalswap * Info.mem_unit));
  json_object_object_add(*writer, "Swap_Free", json_object_new_int64(Info.freeswap * Info.mem_unit));
  json_object_object_add(*writer, "RAM_Total", json_object_new_int64(TotalRam));
  json_object_object_add(*writer, "RAM_Free", json_object_new_int64(FreeRam));
  json_object_object_add(*writer, "CPU_Usage", json_object_new_double(Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)));
  json_object_object_add(*writer, "CPU_Count", json_object_new_int(get_nprocs()));
  json_object_object_add(*writer, "Disk_Total", json_object_new_int64((uint64_t)Disk.f_blocks * Disk.f_frsize));
  json_object_object_add(*writer, "Disk_Free", json_object_new_int64((uint64_t)Disk.f_bfree * Disk.f_frsize));
  json_object_object_add(*writer, "Battery_Percent", json_object_new_int(100));
  int TempFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if(TempFile > 0) {
    char Temp[16];
    if(read(TempFile, Temp, sizeof(Temp)) > 0) json_object_object_add(*writer, "Temperature", json_object_new_int(atoi(Temp)));
    close(TempFile);
  }
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  return true;
}
bool Tools::JSONString(json_object *obj, const char *key, char **dst, bool copy) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_string)) *dst = (copy ? strdup((char*)json_object_get_string(Find)) : (char*)json_object_get_string(Find));
  return (Find != NULL);
}
bool Tools::JSONFloat(json_object *obj, const char *key, float *dst) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && ((json_object_get_type(Find) == json_type_double) || (json_object_get_type(Find) == json_type_int))) *dst = json_object_get_double(Find);
  return (Find != NULL);
}
bool Tools::JSONUInt64(json_object *obj, const char *key, uint64_t *dst) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_int)) *dst = json_object_get_int64(Find);
  return (Find != NULL);
}
bool Tools::JSONArrayFloat(json_object* obj, const char* key, float* dst, int len) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    if(json_object_array_length(Find) != len) return false;
    for(int i = 0; i < len; i++) {
      json_object *Item = json_object_array_get_idx(Find, i);
      json_type Type = json_object_get_type(Item);
      if((Type != json_type_double) && (Type != json_type_int)) return false;
      dst[i] = json_object_get_double(json_object_array_get_idx(Find, i));
    }
    return true;
  }
  return false;
}