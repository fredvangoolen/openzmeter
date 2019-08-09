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
class RWLocks {
  //Multiple read access allowed, exclusive access for writing
  private : 
    pthread_mutex_t  pMutex;
    pthread_cond_t   pReadersCondition;
    pthread_cond_t   pWritersCondition;
    uint32_t         pReaders;
    uint32_t         pWriters;
    bool             pActiveWriter;
  public :
    RWLocks();
    void ReadLock();
    void ReadUnlock();
    void WriteLock();
    void WriteUnlock();
};
//----------------------------------------------------------------------------------------------------------------------
class InstanceLocks {
  //Hold destructor until no references to object
  private : 
    pthread_mutex_t  pMutex;
    pthread_cond_t   pCond;
    uint32_t         pReferences;
  public :
    InstanceLocks();
    void WaitInstances();
    void Reference();
    void Dereference();
};
//----------------------------------------------------------------------------------------------------------------------
class WorkerLocks {
  //Worker locks until signal to wakeup, caller can wait worker to ends
  private :
    bool            pWorking;
    bool            pTerminate;
    pthread_mutex_t pMutex;
    pthread_cond_t  pCond;
  public :
    WorkerLocks();
    void Abort();
    bool WorkerWaitTask();
    void WorkerEndsTask();
    void WakeWorker();
    void WaitWorker();
    bool IsWorking();
};
//----------------------------------------------------------------------------------------------------------------------
class ProviderLocks {
  //Provider wait until all consumers ends, consumers wait until provider write
  private :
    uint32_t         pWaitingThreads;
    pthread_mutex_t  pWaitingThreadsMutex;
    pthread_cond_t   pWaitingThreadsCond;
    bool             pDataReady;
    bool             pTerminate;
    pthread_mutex_t  pDataReadyMutex;
    pthread_cond_t   pDataReadyCond;
  public :
    ProviderLocks();
    void Abort();
    void ProviderWrite();
    void ProviderWait();
    bool ConsumerWait(uint32_t timeoutMS);
    void ConsumerEnds();
};
//----------------------------------------------------------------------------------------------------------------------
class ConfigManager {
  private :
    static ConfigManager    *pInstance;
    static pthread_mutex_t   pMutex;
  private :
    int   pFile;
    json  pConfig;
  public :
    ConfigManager(string name);
    virtual ~ConfigManager();
  public :
    static json ReadConfig(const string &path);
    static bool WriteConfig(const string &path, const json &val);
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
    static uint8_t *GetFile(const string name, size_t *size);
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
    static void      SendUDP(const json &obj);
};
//----------------------------------------------------------------------------------------------------------------------