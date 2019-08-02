/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <list>
#include "HTTPServer.h"
#include "Analyzers.h"
class AnalyzerRemote : public Analyzer {
  private :
    typedef struct {
      uint64_t        Sequence;
      json            RequestContents;
      HTTPRequest    *Request;
      SleepLocks      Locks;
    } Query_t;
  private :
    pthread_t        pThread;
    uint64_t         pQuerySequence;
//    pthread_mutex_t  pQueriesMutex;
    list<Query_t*>   pQueries;
    SleepLocks      *pRequestLocks;
    pthread_mutex_t  pMutex;
  private :
    void  HandleRequest(string url, HTTPRequest &req);
    void  PushQuery();
    
    
    void  GetAnalyzer(HTTPRequest &req);
    void  SetAnalyzer(HTTPRequest &req);
    void  GetSeries(HTTPRequest &req);
    void  GetSeriesNow(HTTPRequest &req);
    void  GetSeriesRange(HTTPRequest &req);
    void  GetEvents(HTTPRequest &req);
    void  GetEvent(HTTPRequest &req);
    void  SetEvent(HTTPRequest &req);
    void  DelEvent(HTTPRequest &req);
    void  GetRates(HTTPRequest &req);
    void  GetRate(HTTPRequest &req);
    void  SetRate(HTTPRequest &req);
    void  DelRate(HTTPRequest &req);
    void  AddRate(HTTPRequest &req);
    void  GetPrices(HTTPRequest &req);
  public :
    AnalyzerRemote(string id);
    ~AnalyzerRemote();
    void  SetRemote(HTTPRequest &req);
    void  GetRemote(HTTPRequest &req);
};