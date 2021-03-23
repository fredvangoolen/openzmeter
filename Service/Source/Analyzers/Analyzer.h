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
#include <complex>
#include "nlohmann/json.hpp"
#include "Tools.h"
#include "Database.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Analyzer {
  public :
    enum AnalyzerMode { PHASE1, PHASE3, PHASE3N };
    enum AnalyzerState { UNCONFIGURED, OFFLINE, SYNCING, ONLINE };
  private :
    pthread_rwlock_t   pMembersLocks;            //Locks for configurations
    AnalyzerState      pState;                   //Timestamp of last save or communication
    string             pName;                    //Analyzer description
    uint64_t           pLastTime;                //Last state change timestamp
  protected :
    const string       pSerial;                  //Serial number
    const string       pOwner;                   //Analyzer owner user
    const AnalyzerMode pMode;                    //Analyzer mode
  protected :
    Analyzer(const string &id, const AnalyzerMode mode, const string &user = "admin");
  public :
    AnalyzerState State();
    void          State(const AnalyzerState conf);
    string        Name();
    void          Name(const string &name);
  public :
    virtual void GetAnalyzer(HTTPRequest *req) = 0;
    virtual void SetAnalyzer(HTTPRequest *req) = 0;
    virtual void GetStatus(HTTPRequest *req) = 0;
    virtual void GetSeries(HTTPRequest *req) = 0;
    virtual void SetSeries(HTTPRequest *req) = 0;
    virtual void GetSeriesNow(HTTPRequest *req) = 0;
    virtual void GetCosts(HTTPRequest *req) = 0;
    virtual void GetBills(HTTPRequest *req) = 0;
    virtual void GetWaveStream(HTTPRequest *req) = 0;
    virtual void GetSeriesRange(HTTPRequest *req) = 0;
    virtual void DelSeriesRange(HTTPRequest *req) = 0;
    virtual void GetEvents(HTTPRequest *req) = 0;
    virtual void SetEvents(HTTPRequest *req) = 0;
    virtual void GetEventsRange(HTTPRequest *req) = 0;
    virtual void DelEventsRange(HTTPRequest *req) = 0;
    virtual void GetEvent(HTTPRequest *req) = 0;
    virtual void SetEvent(HTTPRequest *req) = 0;
    virtual void DelEvent(HTTPRequest *req) = 0;
    virtual void TestAlarm(HTTPRequest *req) = 0;
    virtual void GetAlarms(HTTPRequest *req) = 0;
    virtual void SetAlarms(HTTPRequest *req) = 0;
    virtual void GetAlarmsRange(HTTPRequest *req) = 0;
    virtual void DelAlarm(HTTPRequest *req) = 0;
    virtual void SetAlarm(HTTPRequest *req) = 0;
    virtual void DelAlarmsRange(HTTPRequest *req) = 0;
    virtual void SetRemote(HTTPRequest *req) = 0;
    virtual bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) = 0;
  public :
    virtual ~Analyzer();
    string   Serial();
    json     GetJSONDescriptor();
};
//----------------------------------------------------------------------------------------------------------------------