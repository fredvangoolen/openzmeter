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
PIDFile::PIDFile(string name) {
  pFile = name;
  char PID_Number[16];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(!pFile.empty()) remove(pFile.c_str());
  int File = open(pFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) LOGFile::Instance()->Warning("Can not create PID file (%d)\n", errno);
  if(write(File, PID_Number, Len) <= 0) LOGFile::Instance()->Warning("Can not create PID file (%d)\n", errno);
  if(File > 0) close(File);
}
PIDFile::~PIDFile() {
  if(!pFile.empty()) remove(pFile.c_str());
}
//----------------------------------------------------------------------------------------------------------------------
LOGFile *LOGFile::pInstance = NULL;
LOGFile::LOGFile(string name, bool debug) {
  if(pInstance != NULL) return;
  pthread_mutex_init(&pMutex, NULL);
  pFile = fopen(name.c_str(), "at");
  if(!pFile) pFile = fopen(name.c_str(), "wt");
  if(pFile == NULL) LOGFile::Instance()->Warning("Can not create or open log file (%d)\n", errno);
  pDebug = debug;
  pConsole = true;
  pInstance = this;  
}
LOGFile::~LOGFile() {
  pthread_mutex_lock(&pMutex);
  pInstance = NULL;
  if(pFile) fclose(pFile);
  pthread_mutex_destroy(&pMutex);
}
LOGFile *LOGFile::Instance() {
  return pInstance;
}
void LOGFile::ConsoloeOutput(bool enabled) {
  pConsole = enabled;
}
void LOGFile::Debug(const char* msg, ...) {
  if(pDebug == false) return;
  va_list args;
  va_start(args, msg);
  Msg(DEBUG, msg, args);
  va_end(args);
}
void LOGFile::Info(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(INFO, msg, args);
  va_end(args);
}
void LOGFile::Warning(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(WARNING, msg, args);
  va_end(args);
}
void LOGFile::Error(const char* msg, ...) {
  va_list args;
  va_start(args, msg);
  Msg(ERROR, msg, args);
  va_end(args);
}
void LOGFile::Msg(Type type, const char *msg, ...) {
  va_list args;
  va_start(args, msg);
  time_t Now;
  struct tm *TimeStamp;
  char DateStr[80];
  pthread_mutex_lock(&pMutex);
  time(&Now);
  TimeStamp = localtime(&Now);
  strftime(DateStr, 80, "[%d/%m/%Y - %X] ", TimeStamp);
  if(pConsole == true) {
    if(type == ERROR) {
      printf("\033[1;31m%s\033[0m", DateStr);
    } else if(type == WARNING) {
      printf("\033[1;33m%s\033[0m", DateStr);
    } else if(type == DEBUG) {
      printf("\033[1;37m%s\033[0m", DateStr);
    } else if(type == INFO) {
      printf("\033[1;34m%s\033[0m", DateStr);
    } else {
      return;
    }
    vprintf(msg, args);
    fflush(stdout);
  }
  if(pFile != NULL) {
    va_start(args, msg);
    fprintf(pFile, "%s", DateStr);
    vfprintf(pFile, msg, args);
  }
  pthread_mutex_unlock(&pMutex);
  va_end(args);
}
//----------------------------------------------------------------------------------------------------------------------
extern char _binary_ResourceFiles_start[];
extern char _binary_ResourceFiles_end[];
ResourceLoader *ResourceLoader::pInstance = NULL;
ResourceLoader::ResourceLoader() {
  if(pInstance != NULL) return;
  pInstance = this;
  char *Ptr = _binary_ResourceFiles_start;
  uint32_t FileCount = atoi(Ptr);
  pFiles.reserve(FileCount);
  Ptr = &Ptr[strlen(Ptr) + 1];
  for(uint32_t i = 0; i < FileCount; i++) {
    File_t File;
    File.Name = Ptr;
    Ptr = &Ptr[strlen(Ptr) + 1];
    File.Size = atoi(Ptr);
    Ptr = &Ptr[strlen(Ptr) + 1];
    File.Ptr = (uint8_t*)Ptr;
    Ptr = &Ptr[File.Size];
    pFiles.push_back(File);
  }
}
ResourceLoader *ResourceLoader::Instance() {
  return pInstance;
}
uint8_t *ResourceLoader::GetFile(string name, size_t *size) {
  for(uint32_t i = 0; i < pFiles.size(); i++) {
    if(pFiles[i].Name.compare(name) == 0) {
      *size = pFiles[i].Size;
      return pFiles[i].Ptr;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------
ConfigManager *ConfigManager::pInstance = NULL;
ConfigManager::ConfigManager(string name) {
  if(pInstance != NULL) return;
  pInstance = this;
  pFile = name;

  if(file.empty()) {
//    LogMsg("ERROR", "No config file!\n");
    return false;
  }
  LogMsg("INFO", "Loading config file...\n");
  pthread_mutex_lock(&pConfigMutex);
  int File = open(file.c_str(), O_RDONLY);
  if(File == -1) {
    LogMsg("ERROR", "Can not open config file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  struct stat FileStatus;
  if(fstat(File, &FileStatus) != 0) {
    LOGFile::Instance()->Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    close(File);
    pthread_mutex_unlock(&pConfigMutex);
    return false;
  }
  char *ConfigText = (char*)mmap(NULL, FileStatus.st_size, PROT_READ, MAP_PRIVATE | MAP_POPULATE, File, 0);
  if(ConfigText == NULL) {
    LOGFile::Instance()->Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
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
  
  
  
  
  
  
  
  
  
  
  
  
  pFile = name;
    pthread_mutex_t pMutex;
    
}


//class ConfigManager {
//  private :
//    static ConfigManager *pInstance;
//  private :
//    string          pFile;
//    pthread_mutex_t pMutex;
//  public :
//    ConfigManager(string name);
//    json ReadConfig();
//    bool SaveConfig();
//};
//



//añadido para probar el envio
//  #include <arpa/inet.h>
//      #include <netinet/in.h>
//      #include <stdio.h>
//      #include <sys/types.h>
//      #include <sys/socket.h>
//      #include <unistd.h>

//----------------------------------------------------------------------------------------------------------------------
//bool            Tools::Debug        = false;
//bool            Tools::pDaemonized  = false;
//FILE           *Tools::pLogFile     = NULL;
//json            Tools::pConfig      = NULL;
//string          Tools::pPidFile;
//string          Tools::pConfigFile;
//pthread_mutex_t Tools::pLogMutex    = PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t Tools::pConfigMutex = PTHREAD_MUTEX_INITIALIZER;
//
//uint8_t Tools::pBatteryLevel = 0;
//bool Tools::pBatteryCharging = false;
//bool Tools::pExternalPower = false;
//----------------------------------------------------------------------------------------------------------------------


void Tools::RegisterHandlers(void (*function)(int)) {
  signal(SIGINT, function);
  signal(SIGTERM, function);
}

void Tools::Daemonize() {
//  Tools::LogMsg("INFO", "Move to background task\n");
  pid_t ProcID = fork();
  if(ProcID < 0) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  if(ProcID > 0) exit(0);
  umask(0);
  pid_t SID = setsid();
  if(SID < 0) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    exit(1);
  }
  pDaemonized = true;
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
}
bool Tools::SaveConfig() {
  int File = open(pConfigFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
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
void Tools::GetSysInfo(HTTPRequest &req) {
  struct sysinfo Info;
  struct statvfs Disk;
  if(sysinfo(&Info) != 0) {
    LOGFile::Instance()->Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    req.FromString("'System error", MHD_HTTP_BAD_REQUEST);
    return;
  }
  if(statvfs("/", &Disk) != 0) {
    LOGFile::Instance()->Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    req.FromString("'System error", MHD_HTTP_BAD_REQUEST);
    return;
  }
  uint64_t TotalRam, FreeRam;
  if(GetMemory(&TotalRam, &FreeRam) == false) {
    req.FromString("'System error", MHD_HTTP_BAD_REQUEST);  
    return;
  }
  json Response;
  Response["UpTime"] = Info.uptime;
  //  *writer = json_object_new_object();
//  json_object_object_add(*writer, "UpTime", json_object_new_int(Info.uptime));
//  json_object_object_add(*writer, "Swap_Total", json_object_new_int64(Info.totalswap * Info.mem_unit));
//  json_object_object_add(*writer, "Swap_Free", json_object_new_int64(Info.freeswap * Info.mem_unit));
//  json_object_object_add(*writer, "RAM_Total", json_object_new_int64(TotalRam));
//  json_object_object_add(*writer, "RAM_Free", json_object_new_int64(FreeRam));
//  json_object_object_add(*writer, "CPU_Usage", json_object_new_double(Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)));
//  json_object_object_add(*writer, "CPU_Count", json_object_new_int(get_nprocs()));
//  json_object_object_add(*writer, "Disk_Total", json_object_new_int64((uint64_t)Disk.f_blocks * Disk.f_frsize));
//  json_object_object_add(*writer, "Disk_Free", json_object_new_int64((uint64_t)Disk.f_bfree * Disk.f_frsize));
//  json_object_object_add(*writer, "Battery_Percent", json_object_new_int(pBatteryLevel));
//  json_object_object_add(*writer, "Battery_Charging", json_object_new_boolean(pBatteryCharging));
//  json_object_object_add(*writer, "External_Power", json_object_new_boolean(pExternalPower));
//  int TempFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
//  if(TempFile > 0) {
//    char Temp[16];
//    if(read(TempFile, Temp, sizeof(Temp)) > 0) json_object_object_add(*writer, "Temperature", json_object_new_int(atoi(Temp)));
//    close(TempFile);
//  }
//  *postdata = (char*)writer.dump().c_str();
//  *resultLen = strlen(*postdata);
  //*postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);
  req.FromJSON(Response);
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
//void Tools::SendUDP(json obj) {
//  struct sockaddr_in Client;
//  int Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//  if(Socket == -1) {
//    LOGFile::Instance()->Error("cant open socket\n");
//    return;
//  }
//  memset((char *)&Client, 0, sizeof(Client));
//  Client.sin_family = AF_INET;
//  Client.sin_port = htons(8888);
//  if(inet_aton("127.0.0.1", &Client.sin_addr) == 0) {
//    LOGFile::Instance()->Error("inet_aton() failed\n");
//    close(Socket);
//    return;
//  }
//  string Data = obj.dump();
//  if(sendto(Socket, Data.c_str(), Data.size(), 0, (sockaddr*)&Client, sizeof(Client)) == -1) {
//    LOGFile::Instance()->Error("sendto()\n");
//  }
//  close(Socket);
//}