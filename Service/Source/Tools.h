/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class PIDFile {
  private :
    string pFile;
  public :
    PIDFile();
    virtual ~PIDFile();
};
//----------------------------------------------------------------------------------------------------------------------
class LOGFile {
  private :
    enum Type {WARNING, ERROR, DEBUG, INFO};
  private :
//    static LOGFile         *pInstance;
    static bool             pConfigured;
    static pthread_mutex_t  pMutex;
//    static bool             pDebug;
    static bool             pConsole;
    static fstream          pFile;
private :
    static void Msg(Type type, const char *msg, va_list params);
    static void WriteFile();
  public :
    static void Info(const char *msg, ...);
    static void Warning(const char *msg, ...);
    static void Error(const char *msg, ...);
    static void Debug(const char *msg, ...);
//    static void EnableConsole(bool enable);
//    static void EnableDe
  public :
//    LOGFile(bool debug, bool console);
//    virtual ~LOGFile();
};
//----------------------------------------------------------------------------------------------------------------------
class ResourceLoader {
  private :
    typedef struct {
      string   Name;
      uint32_t Size;
      uint8_t *Ptr;
    } File_t;
  private :
    static vector<File_t> pFiles;
  public :
    static uint8_t *GetFile(string name, size_t *size);
};
//----------------------------------------------------------------------------------------------------------------------
class ConfigManager {
  private :
    static ConfigManager  *pInstance;
    static pthread_mutex_t pMutex;
  private :
    int   pFile;
    json  pConfig;
  public :
    ConfigManager(string name);
    virtual ~ConfigManager();
  public :
    static void  SetFile(string name);
    static json  ReadConfig(string path);
    static bool  WriteConfig(string path, json val);
};
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  public :
    static uint8_t pBatteryLevel;
    static bool    pBatteryCharging;
    static bool    pExternalPower;
  private :
    static uint64_t ParseProcMem(const char *name, const char *buff);
//    static bool     GetMemory(uint64_t *total, uint64_t *available);
  public :
    static uint64_t     GetTime();
    static json         GetSysInfo();
    static void         SendUDP(json obj);
};
//----------------------------------------------------------------------------------------------------------------------