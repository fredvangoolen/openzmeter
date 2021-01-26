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
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerManager final {
  private :
    static map<string, Analyzer*>  pAnalyzers;       //Initiated by analyzer
    static pthread_rwlock_t        pAnalyzersLock;   //Control access to critical sections
  private :
    static void CallAnalyzerFunction(const json &params, Response &response, void (Analyzer::*method)(const json &params, Response &response));
    static void GetAnalyzersCallback(const json &params, Response &response);
    static void GetAnalyzerCallback(const json &params, Response &response);
    static void SetAnalyzerCallback(const json &params, Response &response);
    static void GetStatusCallback(const json &params, Response &response);
    static void GetSeriesCallback(const json &params, Response &response);
    static void GetSeriesNowCallback(const json &params, Response &response);
    static void GetCostsCallback(const json &params, Response &response);
    static void GetBillsCallback(const json &params, Response &response);
    static void GetWaveStream(const json &params, Response &response);
    static void GetSeriesRangeCallback(const json &params, Response &response);
    static void DelSeriesRangeCallback(const json &params, Response &response);
    static void GetEventsCallback(const json &params, Response &response);
    static void GetEventsRangeCallback(const json &params, Response &response);
    static void DelEventsRangeCallback(const json &params, Response &response);
    static void GetEventCallback(const json &params, Response &response);
    static void SetEventCallback(const json &params, Response &response);
    static void DelEventCallback(const json &params, Response &response);
    static void TestAlarmCallback(const json &params, Response &response);
    static void GetAlarmsCallback(const json &params, Response &response);
    static void GetAlarmsRangeCallback(const json &params, Response &response);
    static void DelAlarmCallback(const json &params, Response &response);
    static void DelAlarmsRangeCallback(const json &params, Response &response);
//    static void GetRateCallback(const json &params, Response &response);
//    static void AddRateCallback(const json &params, Response &response);
//    static void SetRateCallback(const json &params, Response &response);
//    static void DelRateCallback(const json &params, Response &response);
//    static void GetPermissionsCallback(const json &params, Response &response);
//    static void AddPermissionCallback(const json &params, Response &response);
//    static void SetPermissionCallback(const json &params, Response &response);
//    static void DelPermissionCallback(const json &params, Response &response);
  public :
    static bool AddAnalyzer(Analyzer *analyzer, const string &user = "admin");
    static bool DelAnalyzer(const string &serial);
    static bool HandleModbus(uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len);
  public :
    AnalyzerManager();
    ~AnalyzerManager();
};
//----------------------------------------------------------------------------------------------------------------------