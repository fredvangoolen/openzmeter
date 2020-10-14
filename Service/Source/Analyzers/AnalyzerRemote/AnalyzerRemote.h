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
#include <map>
#include "HTTPServer.h"
#include "Analyzer.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemote : public Analyzer {
  private :
    typedef struct Request_t {
//      bool                     Terminate;
      int                      Pipe[2];
//      bool                     WriterTerminated;
      json                     ReqParams;
      string                   ResMime;
      uint16_t                 ResCode;
//      list<string>             ResData;
      pthread_cond_t           Cond;
//      pthread_mutex_t          Mutex;
      map<string, Request_t*> *Map;
    } Request_t;
  private :
    volatile bool               pTerminate;        //Termination signal
    pthread_mutex_t             pMutex;            //Control access to shared variables and maps
    pthread_cond_t              pCond;             //Wake getRemote as needed
    map<string, Request_t*>     pSendPending;      //Request awaiting to be send
    map<string, Request_t*>     pResponsePending;  //Request awaiting response



//    pthread_cond_t              pProducerCond;

//    Request_t                  *pProducerRequest;
//    bool                        pConsumerReady;
  private :
    void HandleRequest(const AnalyzerRights rights, const string &url, const json &params, Response &response);
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetStatus(const json &params, Response &response) override;
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
    void GetCosts(const json &params, Response &response) override;
    void GetBills(const json &params, Response &response) override;
    void GetAlarmsDef(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void AddAlarmDef(const json &params, Response &response) override;
    void GetAlarmDef(const json &params, Response &response) override;
    void SetAlarmDef(const json &params, Response &response) override;
    void DelAlarmDef(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void GetRates(const json &params, Response &response) override;
    void AddRate(const json &params, Response &response) override;
    void GetRate(const json &params, Response &response) override;
    void SetRate(const json &params, Response &response) override;
    void DelRate(const json &params, Response &response) override;
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    void GetRemote(const json &params, Response &response);
    void SetRemote(const map<string, string> &params, Response &response, const ContentReader &content_reader);
  public :
    AnalyzerRemote(const string &id);
    ~AnalyzerRemote();
};
//----------------------------------------------------------------------------------------------------------------------