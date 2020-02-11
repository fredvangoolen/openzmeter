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
#include <list>
#include "HTTPServer.h"
#include "Analyzer.h"

class AnalyzerRemote : public Analyzer {
  private :
    typedef struct {
      bool          Sended;
      const string *RequestMethod;
      const json   *RequestParams;
      Response     *Resp;
      ProviderLocks Locks;
    } Request_t;

//    typedef struct {
//      HTTPRequest    *Request;

//      ProviderLocks   Locks;
//    } Request_t;
  private :
    map<uint64_t, Request_t*> pPendingRequest;
    map<uint64_t, Request_t*> pSendedRequest;
    pthread_mutex_t           pLocks;
    uint64_t                  pSequence;
  private :
    void  SetRequester();
    void  ClearRequester();
    void  HandleRequest(const string &url, const json &params, Response &response);
    void  SetRemote(const json &params, Response &response);
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
};