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
#include "Analyzer.h"
#include "HTTPRequest.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerManager final {
  private :
    static map<string, Analyzer*>  pAnalyzers;       //Initiated by analyzer
    static pthread_rwlock_t        pAnalyzersLock;   //Control access to critical sections
  private :
    static void CallAnalyzerFunction(HTTPRequest &req, void (Analyzer::*method)(HTTPRequest &req));
    static void GetAnalyzersCallback(HTTPRequest &req);
    static void GetAnalyzerCallback(HTTPRequest &req);
    static void SetAnalyzerCallback(HTTPRequest &req);
    static void GetStatusCallback(HTTPRequest &req);
    static void GetSeriesCallback(HTTPRequest &req);
    static void GetSeriesNowCallback(HTTPRequest &req);
    static void GetCostsCallback(HTTPRequest &req);
    static void GetBillsCallback(HTTPRequest &req);
    static void GetWaveStream(HTTPRequest &req);
    static void GetSeriesRangeCallback(HTTPRequest &req);
    static void DelSeriesRangeCallback(HTTPRequest &req);
    static void GetEventsCallback(HTTPRequest &req);
    static void GetEventsRangeCallback(HTTPRequest &req);
    static void DelEventsRangeCallback(HTTPRequest &req);
    static void GetEventCallback(HTTPRequest &req);
    static void SetEventCallback(HTTPRequest &req);
    static void DelEventCallback(HTTPRequest &req);
    static void TestAlarmCallback(HTTPRequest &req);
    static void GetAlarmsCallback(HTTPRequest &req);
    static void GetAlarmsRangeCallback(HTTPRequest &req);
    static void DelAlarmCallback(HTTPRequest &req);
    static void SetAlarmCallback(HTTPRequest &req);
    static void DelAlarmsRangeCallback(HTTPRequest &req);
    static void SetRemoteCallback(HTTPRequest &req);
  public :
    static bool AddAnalyzer(Analyzer *analyzer);
    static bool DelAnalyzer(const string &serial);
    static bool HandleModbus(uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len);
  public :
    AnalyzerManager();
    ~AnalyzerManager();
};
//----------------------------------------------------------------------------------------------------------------------



//    static void SetAnalyzerCallback(const json &params, Response &response);
//    static void GetStatusCallback(const json &params, Response &response);
//    static void GetSeriesCallback(const json &params, Response &response);
//    static void GetSeriesNowCallback(const json &params, Response &response);
//    static void GetCostsCallback(const json &params, Response &response);
//    static void GetBillsCallback(const json &params, Response &response);
//    static void GetWaveStream(const json &params, Response &response);
//    static void GetSeriesRangeCallback(const json &params, Response &response);
//    static void GetEventsCallback(const json &params, Response &response);
//    static void GetEventsRangeCallback(const json &params, Response &response);
//    static void GetEventCallback(const json &params, Response &response);
//    static void SetEventCallback(const json &params, Response &response);
//    static void DelEventCallback(const json &params, Response &response);
//    static void TestAlarmCallback(const json &params, Response &response);
//    static void GetAlarmsCallback(const json &params, Response &response);
//    static void GetAlarmsRangeCallback(const json &params, Response &response);

//    static void SetRemoteCallback(const json &params, Response &response);
//======================================================================================================================