/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "cpp-httplib/httplib.h"
#include "AnalyzerSoft.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerSoft::Sync final {
  private :
    typedef struct {
      json                Params;
      string              Method;
      uint64_t            Sequence;
      AnalyzerSoft::Sync *Sync;
      AnalyzerSoft       *Analyzer;
    } Request_t;
  private :
    AnalyzerSoft   *pParent;
    string          pServer;
    string          pPath;
    uint16_t        pPort;
    string          pUser;
    string          pPass;
    pthread_t       pThread;
    bool            pTerminate;
    string          pCookie;
    RWLocks         pLocks;
    uint32_t        pThreadsCount;            //Keep threads count
  private :
    static void    *Sync_Thread(void *args);
    static void    *Request_Thread(void *args);
  public:
    Sync(AnalyzerSoft *parent);
    ~Sync();
    void Configure(const string &server, const string &user, const string &pass);
    bool Logged();
};