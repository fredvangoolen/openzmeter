/* ================================== zMeter ===========================================================================
 * File:     AnalyzerInterface.h
 * Author:   Eduardo Viciana (2018)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <vector>
#include <complex>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "Tools.h"
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Analyzer {
  public :
    enum AnalyzerMode { PHASE1, PHASE3, PHASE3N };
    enum AnalyzerRights { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
  private :
    typedef struct {
      char  *Buffer;
      size_t Size;
      size_t MaxSize;
    } CurlWriteBuff_t;
    typedef struct {
//      pthread_t Thread;
      json      Params;
      string    Method;
      uint64_t  Sequence;
      Analyzer *Parent;
    } RemoteRequest_t;
  protected :
    uint64_t        pLastSave;                //Timestamp of last save or communication
    string          pSerial;                  //Serial number
    string          pName;                    //Analyzer description
  private :
    CURLSH         *pCurl;                    //Shared object for curl lib
    curl_slist     *pCurlHeaders;             //Common headers
    pthread_mutex_t pCurlLocks;               //Curl access control
    bool            pTerminateSync;           //Signal to terminate sync thread
    pthread_t       pSyncThread;              //Synchronize thread
    string          pSyncServer;              //Sync server
    string          pSyncUser;                //Sync username
    string          pSyncPass;                //Sync password
  protected :
    bool CheckRights(HTTPRequest &req, const uint neededAccess) const;
    bool EnableSync(const string &server, const string &user, const string &pass);
    json GetJSONSyncConfig();
    void DisableSync();
  private :
    static size_t CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static void   CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr);
    static void   CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr);
    static void  *Sync_Thread(void *args);
    static void  *Request_Thread(void *args);
  public :
    virtual void GetAnalyzer(HTTPRequest &req) = 0;
    virtual void SetAnalyzer(HTTPRequest &req) = 0;
    virtual void GetSeries(HTTPRequest &req) = 0;
    virtual void GetSeriesNow(HTTPRequest &req) = 0;
    virtual void GetSeriesRange(HTTPRequest &req) = 0;
    virtual void GetEvents(HTTPRequest &req) = 0;
    virtual void GetEvent(HTTPRequest &req) = 0;
    virtual void SetEvent(HTTPRequest &req) = 0;
    virtual void DelEvent(HTTPRequest &req) = 0;
    virtual void GetRates(HTTPRequest &req) = 0;
    virtual void GetRate(HTTPRequest &req) = 0;
    virtual void SetRate(HTTPRequest &req) = 0;
    virtual void DelRate(HTTPRequest &req) = 0;
    virtual void AddRate(HTTPRequest &req) = 0;
    virtual void GetPrices(HTTPRequest &req) = 0;
  public :
    Analyzer(const string &id);
    virtual ~Analyzer();
    string Serial() const;
    json   GetJSONShortDescriptor() const;
};