/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <curl/curl.h>
#include "AnalyzerSoft.h"
class AnalyzerSoft::Sync final {
  class CurlBuff final {
    private :
      char  *pBuffer;
      size_t pSize;
    public :
      CurlBuff();
      ~CurlBuff();
      bool Append(void *data, uint32_t size);
      char *Data();
  };
  private :
    typedef struct {
      json                Params;
      string              Method;
      uint64_t            Sequence;
      AnalyzerSoft::Sync *Sync;
      AnalyzerSoft       *Analyzer;
    } Request_t;
  private :
    string          pServer;
    string          pUser;
    string          pPass;
    pthread_t       pThread;
    bool            pTerminate;
    bool            pLogged;
    RWLocks         pLocks;
    AnalyzerSoft   *pParent;
    CURLSH         *pCurl;                    //Shared object for curl lib
    curl_slist     *pCurlHeaders;             //Common headers
    pthread_mutex_t pCurlLocks;               //Curl access control
    uint32_t        pThreadsCount;            //Keep threads count
  private :
    static void    *Sync_Thread(void *args);
    static void    *Request_Thread(void *args);
    static size_t   CurlWriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
    static void     CurlLockCallback(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr);
    static void     CurlUnlockCallback(CURL *handle, curl_lock_data data, void *userptr);
  private :
    uint16_t CurlSendPost(const string &url, const string &contents, CurlBuff *buffer);
  public:
    Sync(AnalyzerSoft *parent, const string &server, const string &user, const string &pass);
    ~Sync();
    void Configure(const string &server, const string &user, const string &pass);
    bool Logged();
};