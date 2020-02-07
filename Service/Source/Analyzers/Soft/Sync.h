/* ================================== zMeter ===========================================================================
 * File:     CaptureInterface.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include "AnalyzerSoft.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
#define CPPHTTPLIB_READ_TIMEOUT_SECOND 30
#include "cpp-httplib/httplib.h"
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