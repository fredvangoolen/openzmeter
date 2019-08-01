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
      string          ResponseData;
      uint16_t        ResponseCode;
      ProviderLocks   Locks;
    } Query_t;
    typedef struct {
      HTTPRequest   Request;
      ProviderLocks Locks;
    } Request_t;
  private :
    pthread_t        pThread;
    uint64_t         pQuerySequence;
    RWLocks          pQueriesLock;
    list<Query_t*>   pQueries;
    
    RWLocks          pRequestLocks;
    list<Request_t*> pRequest;
    
    
    //Los getRemote duermen 20 segundos, hasta que se despiertan, si hay peticiones cojen las que puedan y las mandan
    
  private :
    void  HandleRequest(string url, HTTPRequest &req);
    void  SendQuery();
    
    
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