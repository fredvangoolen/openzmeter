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
#include <pthread.h>
#include "nlohmann/json.h"
#include "Analyzer.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Device {
  private :
    struct Comp {
      using is_transparent = void;
      bool operator()(const Analyzer *a1, const Analyzer *a2) const {
        return a1->Serial < a2->Serial;
      }
      bool operator()(const string &serial, const Analyzer *a) const {
        return serial < a->Serial;
      }
      bool operator()(const Analyzer *a, const string &serial) const {
        return a->Serial < serial;
      }
    };
  protected :
    bool                   pEnabled;           //Enable flag
    RWLocks                pAnalyzersLock;     //Analyzer list lock
    set<Analyzer*, Comp>   pAnalyzers;         //Analyzers list
  public :
    const string           Serial;             //Device serial
    const string           Driver;             //Driver name
    const string           Name;               //Driver description
  private :
    bool CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response));
  protected :
    Device(const string &serial, const string &driver, const string &name);
    void AddAnalyzer(Analyzer *analyzer);
    void ClearAnalyzers();
  public :
    json GetJSONDescriptor();
    json GetJSONAnalyzers(const string &user);
    bool GetAnalyzer(const json &params, Response &response);
    bool SetAnalyzer(const json &params, Response &response);
    bool GetSeries(const json &params, Response &response);
    bool GetSeriesNow(const json &params, Response &response);
    bool GetWaveStream(const json &params, Response &response);
    bool GetSeriesRange(const json &params, Response &response);
    bool DelSeriesRange(const json &params, Response &response);
    bool GetEvents(const json &params, Response &response);
    bool GetEventsRange(const json &params, Response &response);
    bool DelEventsRange(const json &params, Response &response);
    bool GetEvent(const json &params, Response &response);
    bool SetEvent(const json &params, Response &response);
    bool DelEvent(const json &params, Response &response);
    bool GetPrices(const json &params, Response &response);
    bool TestAlarm(const json &params, Response &response);
    bool GetAlarms(const json &params, Response &response);
    bool GetAlarmsRange(const json &params, Response &response);
    bool DelAlarmsRange(const json &params, Response &response);
    bool DelAlarm(const json &params, Response &response);
    bool GetPermissions(const json &params, Response &response);
    bool AddPermission(const json &params, Response &response);
    bool SetPermission(const json &params, Response &response);
    bool DelPermission(const json &params, Response &response);
    bool HandleModbus(const uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len);
  public :
    virtual ~Device();
    virtual void PeriodicTasks();
    virtual void GetDevice(const json &params, Response &response) = 0;
    virtual void SetDevice(const json &params, Response &response) = 0;
    virtual void GetRemote(const json &params, Response &response);
    virtual void SetRemote(const json &params, Response &response);

};
