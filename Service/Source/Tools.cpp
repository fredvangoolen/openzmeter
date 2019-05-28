/* ================================== zMeter ===========================================================================
 * File:     Tools.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <fstream>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Tools.h"
#include "NetworkManager.h"
#include "HTTPServer.h"
#include "DeviceInterface.h"
#include "NTP.h"
//añadido para probar el envio
  #include <arpa/inet.h>
      #include <netinet/in.h>
      #include <stdio.h>
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <unistd.h>
//----------------------------------------------------------------------------------------------------------------------
PIDFile::PIDFile() {
  pFile = "/var/run/openZmeter.pid";
  json Config = ConfigManager::ReadConfig("/PidFile");
  if(Config.is_string()) pFile = Config;
  char PID_Number[16];
  int Len = sprintf(PID_Number, "%d", getpid());
  if(!pFile.empty()) remove(pFile.c_str());
  int File = open(pFile.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
  if(File < 0) LOGFile::Warning("Can not create PID file (%s)\n", strerror(errno));
  if(write(File, PID_Number, Len) <= 0) LOGFile::Warning("Can not create PID file (%s)\n", strerror(errno));
  if(File > 0) close(File);
}
PIDFile::~PIDFile() {
  if(!pFile.empty()) remove(pFile.c_str());
}
//----------------------------------------------------------------------------------------------------------------------
LOGFile        *LOGFile::pInstance = NULL;
bool            LOGFile::pConsole = true;
bool            LOGFile::pDebug = true;
pthread_mutex_t LOGFile::pMutex = PTHREAD_MUTEX_INITIALIZER;
LOGFile::LOGFile(bool debug, bool console) {
  string Name = "/var/log/openZmeter.log";
  json Config = ConfigManager::ReadConfig("/LogFile");
  if(Config.is_string()) Name = Config;
  pConsole = console;
  pDebug   = debug;
  pFile = NULL;
  if(pInstance != NULL) {
    delete pInstance;
    pInstance = NULL;
  }
  if(Name.empty() == false) {
    pFile = fopen(Name.c_str(), "at");
    if(!pFile) pFile = fopen(Name.c_str(), "wt");
    if(pFile == NULL) LOGFile::Warning("Can not create or open log file (%s)\n", strerror(errno));
  }
  pInstance = this;
}
LOGFile::~LOGFile() {
  pthread_mutex_lock(&pMutex);
  pInstance = NULL;
  pthread_mutex_unlock(&pMutex);
  if(pFile) fclose(pFile);
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
void LOGFile::Msg(Type type, const char *msg, va_list params) {
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
    vprintf(msg, params);
    fflush(stdout);
  }
  if((pInstance != NULL) && (pInstance->pFile != NULL)) {
    fprintf(pInstance->pFile, "%s", DateStr);
    vfprintf(pInstance->pFile, msg, params);
  }
  pthread_mutex_unlock(&pMutex);
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
ResourceLoader::~ResourceLoader() {
  pInstance = NULL;
}
uint8_t *ResourceLoader::GetFile(string name, size_t *size) {
  if(pInstance == NULL) return NULL;
  for(uint32_t i = 0; i < pInstance->pFiles.size(); i++) {
    if(pInstance->pFiles[i].Name != name) continue;
    *size = pInstance->pFiles[i].Size;
    return pInstance->pFiles[i].Ptr;
  }
  return NULL;
}
//----------------------------------------------------------------------------------------------------------------------
ConfigManager   *ConfigManager::pInstance = NULL;
pthread_mutex_t  ConfigManager::pMutex = PTHREAD_MUTEX_INITIALIZER;
ConfigManager::ConfigManager(string name) {
  if(pInstance != NULL) {
    delete pInstance;
    pInstance = NULL;
  }
  pthread_mutex_lock(&pMutex);
  pFile = open(name.c_str(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(pFile < 0) {
    LOGFile::Error("Can not open config file (%s)\n", strerror(errno));
    return;
  }
  string Read;
  char Buff[256];
  ssize_t Len;
  do {
    Len = read(pFile, Buff, sizeof(Buff));
    Read += string(Buff, Len);
  } while(Len > 0);
  pConfig = json::parse(Read, NULL, false);
  pInstance = this;
  pthread_mutex_unlock(&pMutex);
}
ConfigManager::~ConfigManager() {
  pthread_mutex_lock(&pMutex);
  close(pFile);
  pInstance = NULL;
  pthread_mutex_unlock(&pMutex);
}
bool ConfigManager::SaveConfig(string path, json val) {
  pthread_mutex_lock(&pMutex);
  if(pInstance == NULL) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pInstance->pConfig[json::json_pointer(path)].merge_patch(val);
  if(pInstance->pFile < 0) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  lseek(pInstance->pFile, 0, SEEK_SET);
  string Write = pInstance->pConfig.dump(2);
  if(write(pInstance->pFile, Write.c_str(), Write.size()) < 0) {
    LOGFile::Error("Can not write configuration to file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(ftruncate(pInstance->pFile, Write.size()) < 0 ) {
    LOGFile::Error("Can not write configuration to file (%s)\n", strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  pthread_mutex_unlock(&pMutex);
  return true;
}
json ConfigManager::ReadConfig(string path) {
  if(pInstance == NULL) return nullptr;
//  printf("Config Path %s -> %s\n", path.c_str(), pInstance->pConfig[json::json_pointer(path)].dump().c_str());
//  fflush(stdout);
  return pInstance->pConfig[json::json_pointer(path)];
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

uint64_t Tools::GetTime() {
  struct timeval Time;
  if(gettimeofday(&Time, NULL) != 0) {
//    LogMsg("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return 0;
  }
  return (uint64_t)Time.tv_sec * 1000 + Time.tv_usec / 1000;
}
uint64_t Tools::ParseProcMem(const char *name, const char *buff) {
  const char *hit = strstr(buff, name);
  if(hit == NULL) return 0;
  errno = 0;
  long Val = strtol(hit + strlen(name), NULL, 10);
  if(errno != 0) return 0;
  return Val * 1024;
}
json Tools::GetSysInfo() {
  struct sysinfo Info;
  struct statvfs Disk;
  json Result;
  if(sysinfo(&Info) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return Result;
  }
  Result["UpTime"] = Info.uptime;
  Result["Swap_Total"] = (uint64_t)Info.totalswap * Info.mem_unit;
  Result["Swap_Usage"] = ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap;
  Result["CPU_Usage"] = ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0;
  if(statvfs("/", &Disk) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return Result;
  }
  Result["Disk_Total"] = (uint64_t)Disk.f_blocks * Disk.f_frsize;
  Result["Disk_Usage"] = ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks;
  char Buffer[8192];
  int TmpFile = open("/proc/meminfo", O_RDONLY);
  if(TmpFile > 0) {
    size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
    close(TmpFile);
    if(Len == 0) return Result;
    Buffer[Len] = 0;
    Result["RAM_Total"] = ParseProcMem("MemTotal:", Buffer);
    Result["RAM_Usage"] = ((ParseProcMem("MemTotal:", Buffer) -  ParseProcMem("MemFree:", Buffer) - ParseProcMem("Cached:", Buffer) - ParseProcMem("Buffers:", Buffer)) * 100.0) / + ParseProcMem("MemTotal:", Buffer);
  }
  TmpFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if(TmpFile > 0) {
    size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
    close(TmpFile);
    Buffer[Len] = 0;
    Result["Temperature"] = atoi(Buffer) / 1000.0;
  }

  
  //  json_object_object_add(*writer, "Battery_Percent", json_object_new_int(pBatteryLevel));
//  json_object_object_add(*writer, "Battery_Charging", json_object_new_boolean(pBatteryCharging));
//  json_object_object_add(*writer, "External_Power", json_object_new_boolean(pExternalPower));
  return Result;
}


void Tools::SendUDP(json obj) {
  struct sockaddr_in Client;
  int Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(Socket == -1) {
    LOGFile::Error("cant open socket\n");
    return;
  }
  memset((char *)&Client, 0, sizeof(Client));
  Client.sin_family = AF_INET;
  Client.sin_port = htons(8888);
  if(inet_aton("127.0.0.1", &Client.sin_addr) == 0) {
    LOGFile::Error("inet_aton() failed\n");
    close(Socket);
    return;
  }
  string Data = obj.dump();
  if(sendto(Socket, Data.c_str(), Data.size(), 0, (sockaddr*)&Client, sizeof(Client)) == -1) {
    LOGFile::Error("sendto()\n");
  }
  close(Socket);
}