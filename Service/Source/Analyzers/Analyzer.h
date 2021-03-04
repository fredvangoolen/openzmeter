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
  private :
    pthread_rwlock_t   pMembersLocks;            //Locks for configurations
    uint64_t           pLastSave;                //Timestamp of last save or communication
    bool               pConfigured;              //Control if analyzer is configured
    string             pName;                    //Analyzer description
  protected :
    const string       pSerial;                  //Serial number
    const string       pOwner;                   //Analyzer owner user
    const AnalyzerMode pMode;                    //Analyzer mode
  protected :
    Analyzer(const string &id, const AnalyzerMode mode, const string &user = "admin");
  public :
    bool     Configured();
    void     Configured(const bool conf);
    string   Name();
    void     Name(const string &name);
    void     Online(const bool online);
    bool     Online();
    uint64_t OnlineTime();
  public :
    virtual void GetAnalyzer(const json &params, Response &response) = 0;
    virtual void SetAnalyzer(const json &params, Response &response) = 0;
    virtual void GetStatus(const json &params, Response &response) = 0;
    virtual void GetSeries(const json &params, Response &response) = 0;
    virtual void SetSeries(const json &params, Response &response) = 0;
    virtual void GetSeriesNow(const json &params, Response &response) = 0;
    virtual void GetCosts(const json &params, Response &response) = 0;
    virtual void GetBills(const json &params, Response &response) = 0;
    virtual void GetWaveStream(const json &params, Response &response) = 0;
    virtual void GetSeriesRange(const json &params, Response &response) = 0;
    virtual void DelSeriesRange(const json &params, Response &response) = 0;
    virtual void GetEvents(const json &params, Response &response) = 0;
    virtual void SetEvents(const json &params, Response &response) = 0;
    virtual void GetEventsRange(const json &params, Response &response) = 0;
    virtual void DelEventsRange(const json &params, Response &response) = 0;
    virtual void GetEvent(const json &params, Response &response) = 0;
    virtual void SetEvent(const json &params, Response &response) = 0;
    virtual void DelEvent(const json &params, Response &response) = 0;
    virtual void TestAlarm(const json &params, Response &response) = 0;
    virtual void GetAlarms(const json &params, Response &response) = 0;
    virtual void SetAlarms(const json &params, Response &response) = 0;
    virtual void GetAlarmsRange(const json &params, Response &response) = 0;
    virtual void DelAlarm(const json &params, Response &response) = 0;
    virtual void SetAlarm(const json &params, Response &response) = 0;
    virtual void DelAlarmsRange(const json &params, Response &response) = 0;
    virtual void SetRemote(const json &params, Response &response) = 0;
    virtual bool HandleModbus(uint8_t *buff, const uint16_t start, const uint8_t len) = 0;
  public :
    virtual ~Analyzer();
    string   Serial();
    json     GetJSONDescriptor();
};
//----------------------------------------------------------------------------------------------------------------------