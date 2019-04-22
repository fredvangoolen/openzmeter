/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <vector>
#include <json.hpp>
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
class PIDFile {
  string pFile;
  public :
    PIDFile(string name);
    virtual ~PIDFile();
};
//----------------------------------------------------------------------------------------------------------------------
class LOGFile {
  private :
    enum Type {WARNING, ERROR, DEBUG, INFO};
  private :
    static LOGFile *pInstance;
  private :
    void Msg(Type type, const char *msg, va_list params);
  private :
    FILE            *pFile;
    pthread_mutex_t  pMutex;
    bool             pDebug;
    bool             pConsole;
  public :
    static LOGFile *Instance();
  public :
    void Info(const char *msg, ...);
    void Warning(const char *msg, ...);
    void Error(const char *msg, ...);
    void Debug(const char *msg, ...);
  public :
    LOGFile(string name, bool debug, bool console);
    virtual ~LOGFile();
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
    static ResourceLoader *pInstance;
  private :
    vector<File_t> pFiles;
  public :
    static ResourceLoader *Instance();
  public :
    ResourceLoader();
    virtual ~ResourceLoader();
    uint8_t *GetFile(string name, size_t *size);
};
//----------------------------------------------------------------------------------------------------------------------
class ConfigManager {
  private :
    static ConfigManager *pInstance;
  private :
    string          pFile;
    json            pConfig;
    pthread_mutex_t pMutex;
  public :
    static ConfigManager *Instance();
  public :
    ConfigManager(string name);
    virtual ~ConfigManager();
    json &ReadConfig();
    bool  SaveConfig();
};
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  public :
    static uint8_t pBatteryLevel;
    static bool    pBatteryCharging;
    static bool    pExternalPower;
  private :      
    static bool            pDaemonized;

    

//    static json           *pConfig;

    
  public :
//    static bool          Debug;
  private :
    static uint64_t ParseProcMem(char *name, char *buff);
    static bool     GetMemory(uint64_t *total, uint64_t *available);

  public :

    static uint64_t     GetTime();
    static void         GetSysInfo(HTTPRequest &req);
    static void         SendUDP(json obj);
};
//----------------------------------------------------------------------------------------------------------------------