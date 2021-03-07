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
#include "HTTPServer.h"
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
    void GetAnalyzer(const json &params, Response &response) override;
    void SetAnalyzer(const json &params, Response &response) override;
    void GetStatus(const json &params, Response &response) override;
    void GetSeries(const json &params, Response &response) override;
    void SetSeries(const json &params, Response &response) override;
    void GetSeriesNow(const json &params, Response &response) override;
    void GetWaveStream(const json &params, Response &response) override;
    void GetSeriesRange(const json &params, Response &response) override;
    void DelSeriesRange(const json &params, Response &response) override;
    void GetEvents(const json &params, Response &response) override;
    void SetEvents(const json &params, Response &response) override;
    void GetEventsRange(const json &params, Response &response) override;
    void DelEventsRange(const json &params, Response &response) override;
    void GetEvent(const json &params, Response &response) override;
    void SetEvent(const json &params, Response &response) override;
    void DelEvent(const json &params, Response &response) override;
    void GetCosts(const json &params, Response &response) override;
    void GetBills(const json &params, Response &response) override;
    void TestAlarm(const json &params, Response &response) override;
    void GetAlarms(const json &params, Response &response) override;
    void SetAlarms(const json &params, Response &response) override;
    void GetAlarmsRange(const json &params, Response &response) override;
    void DelAlarm(const json &params, Response &response) override;
    void SetAlarm(const json &params, Response &response) override;
    void DelAlarmsRange(const json &params, Response &response) override;
    void SetRemote(const json &params, Response &response) override;
    void SetRemote(const map<string, string> &params, Response &response);
  private :
    void HandleAction(const string &name, const json &params);
    bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) override;
  public :
    AnalyzerRemote(const string &id, const AnalyzerMode mode, const string &user);
    ~AnalyzerRemote();
};
//----------------------------------------------------------------------------------------------------------------------