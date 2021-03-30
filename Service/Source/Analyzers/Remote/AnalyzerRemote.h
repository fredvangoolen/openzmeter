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
#include <queue>
#include "HTTPRequest.h"
#include "Analyzer.h"
#include "AnalyzerAccess.h"
#include "AnalyzerSeries.h"
#include "AnalyzerRates.h"
#include "AnalyzerEvents.h"
#include "AnalyzerAlarms.h"
#include "AnalyzerRemoteNow.h"
#include "AnalyzerRemoteActions.h"
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerRemote : public Analyzer {
  private :
    pthread_rwlock_t           pLock;                    //Control access to shared variables
    float                      pSamplerate;              //Cached samplerate
    json                       pConfig;                  //Cached analyzer configuration
  private :
    AnalyzerAccess             pAccess;                  //Access control class
    AnalyzerRates              pRates;                   //Rate calculation from database stored data
    AnalyzerSeries             pSeries;                  //Series base class
    AnalyzerAlarms             pAlarms;                  //Alarms base class
    AnalyzerEvents             pEvents;                  //Events base class
    AnalyzerRemoteNow          pNow;                     //Manage temporal data
    AnalyzerRemoteActions      pActions;                 //Manage pending actions
  private :
    void GetAnalyzer(HTTPRequest &req) override;
    void SetAnalyzer(HTTPRequest &req) override;
    void GetStatus(HTTPRequest &req) override;
    void GetSeries(HTTPRequest &req) override;
    void SetSeries(HTTPRequest &req) override;
    void GetSeriesNow(HTTPRequest &req) override;
    void GetWaveStream(HTTPRequest &req) override;
    void GetSeriesRange(HTTPRequest &req) override;
    void DelSeriesRange(HTTPRequest &req) override;
    void GetEvents(HTTPRequest &req) override;
    void SetEvents(HTTPRequest &req) override;
    void GetEventsRange(HTTPRequest &req) override;
    void DelEventsRange(HTTPRequest &req) override;
    void GetEvent(HTTPRequest &req) override;
    void SetEvent(HTTPRequest &req) override;
    void DelEvent(HTTPRequest &req) override;
    void GetCosts(HTTPRequest &req) override;
    void GetBills(HTTPRequest &req) override;
    void TestAlarm(HTTPRequest &req) override;
    void GetAlarms(HTTPRequest &req) override;
    void SetAlarms(HTTPRequest &req) override;
    void GetAlarmsRange(HTTPRequest &req) override;
    void DelAlarm(HTTPRequest &req) override;
    void SetAlarm(HTTPRequest &req) override;
    void DelAlarmsRange(HTTPRequest &req) override;
    void SetRemote(HTTPRequest &req) override;
    void SetRemote(const map<string, string> &params, Response &response);
  private :
    void HandleAction(const string &name, const json &params);
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    AnalyzerRemote(const string &id, const AnalyzerMode mode, const string &user);
    ~AnalyzerRemote();
};
//----------------------------------------------------------------------------------------------------------------------