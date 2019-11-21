/* ================================== zMeter ===========================================================================
 * File:     AnalyzerRemote.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <list>
#include "HTTPServer.h"
#include "Analyzer.h"
class AnalyzerRemote : public Analyzer {
  private :
    typedef struct {
      uint64_t        Sequence;
      string          Method;
      HTTPRequest    *Request;
      ProviderLocks   Locks;
    } Query_t;
    typedef struct {
      HTTPRequest    *Request;htop
              
      ProviderLocks   Locks;    
    } Request_t;
  private :
    pthread_t        pThread;
    uint64_t         pQuerySequence;
    list<Query_t*>   pQueries;
    ProviderLocks    *pRequestLocks;
  private :
    void  HandleRequest(string url, HTTPRequest &req);
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
    void  TestAlarm(HTTPRequest &req);
    void  GetAlarms(HTTPRequest &req);
    void  GetAlarm(HTTPRequest &req);
    void  SetAlarm(HTTPRequest &req);
    void  AddAlarm(HTTPRequest &req);
    void  DelAlarm(HTTPRequest &req);
  public :
    AnalyzerRemote(string id);
    ~AnalyzerRemote();
    void  SetRemote(HTTPRequest &req);
    void  GetRemote(HTTPRequest &req);
};