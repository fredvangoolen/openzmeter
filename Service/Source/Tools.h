/* ================================== zMeter ===========================================================================
 * File:     Tools.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class ConfigManager {
  private :
    static ConfigManager          *pInstance;
    static pthread_mutex_t         pMutex;
  private :
    int   pFile;
    json  pConfig;
  public :
    ConfigManager(string name);
    virtual ~ConfigManager();
  public :
    static json  ReadConfig(string path);
    static bool  WriteConfig(string path, json val);
};
//----------------------------------------------------------------------------------------------------------------------
class PIDFile {
  private :
    string pFile;
  public :
    PIDFile();
    virtual ~PIDFile();
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
class LOGFile {
  private :
    enum Type {ERROR = 1, WARNING = 2, INFO = 3, DEBUG = 4};
  private :
    static pthread_mutex_t  pMutex;
    static bool             pConsole;
    static LOGFile         *pInstance;
  private :
    static void Msg(Type type, const char *msg, va_list params);
  private :
    FILE   *pFile;
    Type    pLevel;
  public :
    LOGFile();
    virtual ~LOGFile();
  public :
    static void Info(const char *msg, ...);
    static void Warning(const char *msg, ...);
    static void Error(const char *msg, ...);
    static void Debug(const char *msg, ...);
  public :
};
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  public :
    static uint64_t  GetTime();
    static string    Run(string command);
    static void      SendUDP(json obj);
};
//----------------------------------------------------------------------------------------------------------------------