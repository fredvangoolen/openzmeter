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
      HTTPRequest    *Request;

      ProviderLocks   Locks;
    } Request_t;
  private :
    pthread_t        pThread;
    uint64_t         pQuerySequence;
    list<Query_t*>   pQueries;
    ProviderLocks    *pRequestLocks;
  private :
    void  HandleRequest(string url, const json &params, Response &response);
    void  GetAnalyzer(const json &params, Response &response);
    void  SetAnalyzer(const json &params, Response &response);
    void  GetSeries(const json &params, Response &response);
    void  GetSeriesNow(const json &params, Response &response);
    void  GetSeriesRange(const json &params, Response &response);
    void  GetEvents(const json &params, Response &response);
    void  GetEvent(const json &params, Response &response);
    void  SetEvent(const json &params, Response &response);
    void  DelEvent(const json &params, Response &response);
    void  GetRates(const json &params, Response &response);
    void  GetRate(const json &params, Response &response);
    void  SetRate(const json &params, Response &response);
    void  DelRate(const json &params, Response &response);
    void  AddRate(const json &params, Response &response);
    void  GetPrices(const json &params, Response &response);
    void  TestAlarm(const json &params, Response &response);
    void  GetAlarms(const json &params, Response &response);
    void  GetAlarm(const json &params, Response &response);
    void  SetAlarm(const json &params, Response &response);
    void  AddAlarm(const json &params, Response &response);
    void  DelAlarm(const json &params, Response &response);
  public :
    AnalyzerRemote(string id);
    ~AnalyzerRemote();
    void  SetRemote(const json &params, Response &response);
    void  GetRemote(const json &params, Response &response);
};