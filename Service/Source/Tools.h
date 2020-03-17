// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#pragma once
#include <termios.h>
#include <string>
#include <vector>
#include <pthread.h>
#include <nlohmann/json.hpp>
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Worker final {
  private :
    void           *pCreationParam;
    void           *pRunParam;
    bool            pWorking;
    volatile bool   pTerminate;
    pthread_t       pThread;
    pthread_mutex_t pMutex;
    pthread_cond_t  pCond;
    void          (*pCallback)(void*, void*);
  private :
    static void *Thread(void *args);
  public :
    Worker(const string &name, void *param, void (*callback)(void*, void*));
    ~Worker();
    bool IsWorking();
    void Run(void *param);
    void Wait();
};
class RWLocks final {
  //Multiple read access allowed, exclusive access for writing
  private :
    pthread_mutex_t  pMutex;
    pthread_cond_t   pReadersCondition;
    pthread_cond_t   pWritersCondition;
    uint32_t         pReaders;
    uint32_t         pWriters;
    bool             pWriter;
  public :
    RWLocks();
    ~RWLocks();
    void ReadLock();
    bool TryReadLock();
    void WriteLock();
    void Unlock();
};
//----------------------------------------------------------------------------------------------------------------------
class ProviderLocks {
  //Provider wait until all consumers ends, consumers wait until provider write
  private :
    volatile bool    pTerminate;
    bool             pDataReady;
    uint32_t         pConsumers;
    pthread_mutex_t  pMutex;
    pthread_cond_t   pConsumersCond;
    pthread_cond_t   pProducerCond;
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
    static pthread_mutex_t   pMutex;
    static int               pFile;
    static json              pConfig;
  public :
    ConfigManager(const string &name);
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
    enum Type {ERROR = 0, WARNING = 1, INFO = 2, DEBUG = 3};
  private :
    static pthread_mutex_t  pMutex;
    static bool             pConsole;
    static FILE            *pFile;
    static Type             pLevel;
  private :
    static void Msg(Type type, const char *msg, va_list params);
  public :
    LOGFile();
    virtual ~LOGFile();
  public :
    static void CloseConsole();
    static void Info(const char *msg, ...);
    static void Warning(const char *msg, ...);
    static void Error(const char *msg, ...);
    static void Debug(const char *msg, ...);
  public :
};
//----------------------------------------------------------------------------------------------------------------------
class Tools {
  private :
    static termios pConsoleAttr;
  private :
    static void ResetConsole();
  public :
    static void      DisableEcho();
    static uint64_t  GetTime();
    static void      SendUDP(const json &obj);
};
//----------------------------------------------------------------------------------------------------------------------