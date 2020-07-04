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
#include <set>
#include "HTTPServer.h"
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemote : public Analyzer {
  private :
    typedef struct {
      uint64_t        Sequence;
      json            Request;
      json            Response;
      pthread_cond_t  Cond;
    } Request_t;
  private :
    struct Comp {
      using is_transparent = void;
      bool operator()(const Request_t *a1, const Request_t *a2) const {
        return a1->Sequence < a2->Sequence;
      }
      bool operator()(const uint64_t seq, const Request_t *a) const {
        return seq < a->Sequence;
      }
      bool operator()(const Request_t *a, const uint64_t seq) const {
        return a->Sequence < seq;
      }
    };
  private :
    volatile bool           pTerminate;
    pthread_mutex_t         pMutex;
    pthread_cond_t          pProducerCond;
    pthread_cond_t          pConsumerCond;
    Request_t              *pProducerRequest;
    bool                    pConsumerReady;
    set<Request_t*, Comp>   pRequestList;
    uint64_t                pSequence;
  private :
    void PeriodicTasks();
    void HandleRequest(const string &url, const json &params, Response &response);
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetSeries(const json &params, Response &response) override;
    void GetSeriesNow(const json &params, Response &response) override;
    void GetWaveStream(const json &params, Response &response) override;
    void GetSeriesRange(const json &params, Response &response) override;
    void DelSeriesRange(const json &params, Response &response) override;
    void GetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;
    void GetPrices(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void GetRemote(const json &params, Response &response) override;
    void SetRemote(const json &params, Response &response) override;
    bool HandleModbus(const uint8_t *buff, const uint16_t start, const uint8_t len);
  public :
    AnalyzerRemote(const string &id);
    ~AnalyzerRemote();
};
//----------------------------------------------------------------------------------------------------------------------