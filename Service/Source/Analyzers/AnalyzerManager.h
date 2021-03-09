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
    static void SetAlarmCallback(const json &params, Response &response);
    static void DelAlarmsRangeCallback(const json &params, Response &response);
    static void SetRemoteCallback(const json &params, Response &response);
  public :
    static bool AddAnalyzer(Analyzer *analyzer);
    static bool DelAnalyzer(const string &serial);
    static bool HandleModbus(uint8_t *buff, const string &serial, const uint16_t start, const uint8_t len);
  public :
    AnalyzerManager();
    ~AnalyzerManager();
};
//----------------------------------------------------------------------------------------------------------------------

// Module OpenAPI documentation ========================================================================================
static const auto DocAnalyzer = R"({
  "tags": [ { "name": "Analyzers", "description": "Analyzer information queries" } ],
  "components": {
    "schemas": {
      "AnalyzerListTemplate": {
        "type": "object",
        "properties": {
          "Analyzer": { "type": "string" },
          "Config": { "type": "boolean" },
          "Name": { "type": "string" },
          "Online": { "type": "boolean" },
          "Read": { "type": "boolean" },
          "User": { "type": "string" },
          "Write": { "type": "boolean" }
        },
        "required": [ "Analyzer", "Config", "Name", "Online", "Read", "User", "Write" ],
        "example": {
          "Analyzer": "VIRTUAL_3PN",
          "Config": true,
          "Name": "Unnamed analyzer",
          "Online": false,
          "Read": true,
          "User": "admin",
          "Write": true
        }
      },
      "AlarmTemplate": {
        "type": "object",
        "properties": {
          "Enabled": { "type": "boolean" },
          "Name": { "type": "string" },
          "Priority": { "type": "number" },
          "Expression": { "type": "string" },
          "Aggreg": { "type": "string", "enum": [ "200MS", "3S", "1M", "10M", "15M", "1H" ] },
          "Samples": { "type": "number" },
          "Window": { "type": "number" }
        },
        "example": {
          "Enabled": true,
          "Name": "Alarm1",
          "Priority": 0,
          "Expression": "Active_Power>100",
          "Aggreg": "3S",
          "Samples": 10,
          "Window": 20
        }
      }
    }
  }
})"_json;
static const auto GetAnalyzers = R"({
  "paths": {
    "/getAnalyzers": {
      "post": {
        "summary": "Get list of avaliable analyzers",
        "tags": [ "Analyzers" ],
        "requestBody": { "$ref": "#/components/requestBodies/EmptyObject" },
        "responses": {
          "200": {
            "description": "The method executed without error",
            "content": {
              "application/json": {
                "schema": {
                  "type": "array",
                  "items" : { "$ref": "#/components/schemas/AnalyzerListTemplate" }
                }
              }
            }
          },
          "401": { "$ref": "#/components/responses/401Response" }
        }
      }
    }
  }
})"_json;

//    static void GetAnalyzerCallback(const json &params, Response &response);
//    static void SetAnalyzerCallback(const json &params, Response &response);
//    static void GetStatusCallback(const json &params, Response &response);
//    static void GetSeriesCallback(const json &params, Response &response);
//    static void GetSeriesNowCallback(const json &params, Response &response);
//    static void GetCostsCallback(const json &params, Response &response);
//    static void GetBillsCallback(const json &params, Response &response);
//    static void GetWaveStream(const json &params, Response &response);
//    static void GetSeriesRangeCallback(const json &params, Response &response);
//    static void DelSeriesRangeCallback(const json &params, Response &response);
//    static void GetEventsCallback(const json &params, Response &response);
//    static void GetEventsRangeCallback(const json &params, Response &response);
//    static void DelEventsRangeCallback(const json &params, Response &response);
//    static void GetEventCallback(const json &params, Response &response);
//    static void SetEventCallback(const json &params, Response &response);
//    static void DelEventCallback(const json &params, Response &response);
//    static void TestAlarmCallback(const json &params, Response &response);
//    static void GetAlarmsCallback(const json &params, Response &response);
//    static void GetAlarmsRangeCallback(const json &params, Response &response);
//    static void DelAlarmCallback(const json &params, Response &response);
//    static void SetAlarmCallback(const json &params, Response &response);
//    static void DelAlarmsRangeCallback(const json &params, Response &response);
//    static void SetRemoteCallback(const json &params, Response &response);
//======================================================================================================================