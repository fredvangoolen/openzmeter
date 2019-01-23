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
#include "NTP.h"
//----------------------------------------------------------------------------------------------------------------------
bool            Tools::Debug        = false;
bool            Tools::pDaemonized  = false;
FILE           *Tools::pLogFile     = NULL;
json_object    *Tools::pConfig      = NULL;
const char     *Tools::pPidFile     = NULL;
const char     *Tools::pConfigFile  = NULL;
pthread_mutex_t Tools::pLogMutex    = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Tools::pConfigMutex = PTHREAD_MUTEX_INITIALIZER;

uint8_t Tools::pBatteryLevel = 0;
bool Tools::pBatteryCharging = false;
bool Tools::pExternalPower = false;
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
vector<Tools::File_t> Tools::pFiles;
uint8_t *Tools::GetFile(const char* name, size_t* size) {
  if(pFiles.size() == 0) {
    char *Ptr   = _binary_ResourceFiles_start;
    uint32_t FileCount = atoi(Ptr);
    pFiles.reserve(FileCount);
    Ptr = &Ptr[strlen(Ptr) + 1];
    for(uint32_t i = 0; i < FileCount; i++) {
      File_t File;
      File.Name = strdup(Ptr);
      Ptr = &Ptr[strlen(Ptr) + 1];
      File.Size = atoi(Ptr);
      Ptr = &Ptr[strlen(Ptr) + 1];
      File.Ptr = (uint8_t*)Ptr;
      Ptr = &Ptr[File.Size];
      pFiles.push_back(File);
    }
  }
  for(uint32_t i = 0; i < pFiles.size(); i++) {
    if(strcmp(name, pFiles[i].Name) == 0) {
      *size = pFiles[i].Size;
      return pFiles[i].Ptr;
    }
  }
  return NULL;
}
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
bool Tools::CreateLogFile(const char *name) {
  pLogFile = fopen(name, "at");
  if(!pLogFile) pLogFile = fopen(name, "wt");
  if(pLogFile < 0) {
    Tools::LogMsg("WARN", "Can not create or open log file (%d)\n", errno);
    return true;
  }
  return true;
}
bool Tools::CreatePidFile(const char *name) {
  pPidFile = name;
  char PID_Number[64];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(pPidFile) remove(pPidFile);
  int File = open(pPidFile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogMsg("WARN", "Can not create PID file (%d)\n", errno);
    return true;
  }
  if(write(File, PID_Number, Len) <= 0) LogMsg("WARN", "Can not create PID file (%d)\n", errno);
  close(File);
  return true;
}
bool Tools::RemovePidFile() {
  if(pPidFile) return remove(pPidFile);
  return true;
}
void Tools::Daemonize() {
  Tools::LogMsg("INFO", "Move to background task\n");
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
bool Tools::ReadConfig(const char *file) {
  pConfigFile = file;
  if(file == NULL) {
    LogMsg("ERROR", "No config file!\n");
    return false;
  }
  LogMsg("INFO", "Loading config file...\n");
  pthread_mutex_lock(&pConfigMutex);
  int File = open(file, O_RDONLY);
  if(File == -1) {
    LogMsg("ERROR", "Can not open config file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  struct stat FileStatus;
  if(fstat(File, &FileStatus) != 0) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(File);
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  char *ConfigText = (char*)mmap(NULL, FileStatus.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, File, 0);
  if(ConfigText == NULL) {
    Tools::LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(File);
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  pConfig = json_tokener_parse(ConfigText);
  munmap(ConfigText, FileStatus.st_size);
  close(File);
  if(pConfig == NULL) {
    LogMsg("ERROR", "Can not open config file (Invalid JSON format)\n");
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  json_object *Find;
  if((json_object_object_get_ex(pConfig, "LogFile", &Find) == true) && (json_object_get_type(Find) == json_type_string)) CreateLogFile((char*)json_object_get_string(Find));
  if((json_object_object_get_ex(pConfig, "PidFile", &Find) == true) && (json_object_get_type(Find) == json_type_string)) CreatePidFile((char*)json_object_get_string(Find));
  if((json_object_object_get_ex(pConfig, "Database", &Find) != true) || (json_object_get_type(Find) != json_type_object) || (Database::Config(Find) == false)) {
    pthread_mutex_unlock(&pConfigMutex);
    LogMsg("ERROR", "Can not connect to database\n");
    return false;
  }
  if((json_object_object_get_ex(pConfig, "HTTPServer", &Find) != true) || (json_object_get_type(Find) != json_type_object) || (HTTPServer::Config(Find) == false)) {
    pthread_mutex_unlock(&pConfigMutex);
    LogMsg("ERROR", "Can not create HTTP server\n");
    return false;
  }
  if((json_object_object_get_ex(pConfig, "NTP", &Find) != true) || (json_object_get_type(Find) != json_type_object)) json_object_object_add(pConfig, "NTP", json_object_new_object());
  if((json_object_object_get_ex(pConfig, "NTP", &Find) != true) || (json_object_get_type(Find) != json_type_object) || (NTP::Config(Find) == false)) {
    pthread_mutex_unlock(&pConfigMutex);
    LogMsg("ERROR", "Can not create NTP client\n");
    return false;
  }
  if((json_object_object_get_ex(pConfig, "Devices", &Find) != true) || (json_object_get_type(Find) != json_type_object)) json_object_object_add(pConfig, "Devices", json_object_new_object());
  if((json_object_object_get_ex(pConfig, "Devices", &Find) != true) || (json_object_get_type(Find) != json_type_object) || (DeviceInterface::Config(Find) == false)) {
    pthread_mutex_unlock(&pConfigMutex);
    LogMsg("ERROR", "Can not create device manager\n");
    return false;
  }
  pthread_mutex_unlock(&pConfigMutex);
  return true;
}
bool Tools::SaveConfig() {
  int File = open(pConfigFile, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogMsg("ERROR", "Can not write configuration to file (%d)\n", errno);
    return false;
  }
  size_t Len = 0;
  char *ConfigText = (char*)json_object_to_json_string_length(pConfig, JSON_C_TO_STRING_PRETTY | JSON_C_TO_STRING_SPACED, &Len);
  if(write(File, ConfigText, Len) <= 0) {
    LogMsg("ERROR", "Can not write configuration to file (%d)\n", errno);
    return false;
  }
  close(File);
  return true;
}
json_object *Tools::GetConfigObject() {
  return pConfig;
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
  json_object_object_add(*writer, "Battery_Percent", json_object_new_int(pBatteryLevel));
  json_object_object_add(*writer, "Battery_Charging", json_object_new_boolean(pBatteryCharging));
  json_object_object_add(*writer, "External_Power", json_object_new_boolean(pExternalPower));
  int TempFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if(TempFile > 0) {
    char Temp[16];
    if(read(TempFile, Temp, sizeof(Temp)) > 0) json_object_object_add(*writer, "Temperature", json_object_new_int(atoi(Temp)));
    close(TempFile);
  }
  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  return true;
}
bool Tools::JSONString(const json_object *obj, const char *key, char **dst, bool copy) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_string)) *dst = (copy ? strdup((char*)json_object_get_string(Find)) : (char*)json_object_get_string(Find));
  return (Find != NULL);
}
bool Tools::JSONFloat(const json_object *obj, const char *key, float *dst) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && ((json_object_get_type(Find) == json_type_double) || (json_object_get_type(Find) == json_type_int))) *dst = json_object_get_double(Find);
  return (Find != NULL);
}
bool Tools::JSONUInt64(const json_object *obj, const char *key, uint64_t *dst) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_int)) *dst = json_object_get_int64(Find);
  return (Find != NULL);
}
bool Tools::JSONUInt32(const json_object *obj, const char *key, uint32_t *dst) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_int)) *dst = json_object_get_int(Find);
  return (Find != NULL);
}
bool Tools::JSONArrayFloat(const json_object* obj, const char* key, float* dst, uint len) {
  json_object *Find;
  if((json_object_object_get_ex(obj, key, &Find) == true) && (json_object_get_type(Find) == json_type_array)) {
    if(json_object_array_length(Find) != len) return false;
    for(uint i = 0; i < len; i++) {
      json_object *Item = json_object_array_get_idx(Find, i);
      json_type Type = json_object_get_type(Item);
      if((Type != json_type_double) && (Type != json_type_int)) return false;
      dst[i] = json_object_get_double(json_object_array_get_idx(Find, i));
    }
    return true;
  }
  return false;
}
char *Tools::trim(char *str) {
  if(str == NULL) return NULL;
  while((str[0] == 0) && (str[0] == ' ')) str++;
  char *Start = str;
  while((str[0] != 0) && (str[0] != ' ')) str++;
  str[0] = 0;
  return Start;
}