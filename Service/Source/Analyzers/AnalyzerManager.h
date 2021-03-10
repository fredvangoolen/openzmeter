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
          "AlarmID": { "type": "string" },
          "Name": { "type": "string" },
          "Priority": { "type": "number" },
          "Expression": { "type": "string" },
          "Aggreg": { "type": "string", "enum": [ "200MS", "3S", "1M", "10M", "15M", "1H" ] },
          "Samples": { "type": "number" },
          "Window": { "type": "number" }
        },
        "required": [ "Enabled", "AlarmID", "Name", "Priority", "Expression", "Aggreg", "Samples", "Window" ],
        "example": {
          "AlarmID": "00000000-0000-0000-0000-000000000000",
          "Enabled": true,
          "Name": "Alarm",
          "Priority": 0,
          "Expression": "Active_Power>100",
          "Aggreg": "3S",
          "Samples": 10,
          "Window": 20
        }
      },
      "MQTTTemplate": {
        "type": "object",
        "properties": {
          "Enabled": { "type": "boolean" },
          "Export10M": { "type": "boolean" },
          "Export15M": { "type": "boolean" },
          "Export1H": { "type": "boolean" },
          "Export1M": { "type": "boolean" },
          "Export200MS": { "type": "boolean" },
          "Export3S": { "type": "boolean" },
          "ExportAlarms": { "type": "boolean" },
          "ExportConfig": { "type": "boolean" },
          "Pass": { "type": "string" },
          "Prefix": { "type": "string" },
          "Server": { "type": "string" },
          "User": { "type": "string" }
        },
        "required": [ "Enabled", "Export10M", "Export15M", "Export1H", "Export1M", "Export200MS", "Export3S", "ExportAlarms", "ExportConfig", "Pass", "Prefix", "Server", "User" ],
        "example": {
          "Enabled": false,
          "Export10M": false,
          "Export15M": false,
          "Export1H": false,
          "Export1M": false,
          "Export200MS": false,
          "Export3S": true,
          "ExportAlarms": false,
          "ExportConfig": false,
          "Pass": "",
          "Prefix": "/OZM",
          "Server": "mqtt://test.mosquitto.org",
          "User": ""
        }
      },
      "PermissionTemplate": {
        "type": "object",
        "properties": {
          "Config": { "type": "boolean" },
          "Read": { "type": "boolean" },
          "User": { "type": "string" },
          "Write": { "type": "boolean" }
        },
        "required": [ "Config", "Read", "User", "Write" ],
        "example": {
          "Config": true,
          "Read": true,
          "User": "admin",
          "Write": true
        }
      },
      "TagsTemplate": {
        "type": "object",
        "additionalProperties": { "type": "string" },
        "example": {
          "foo": "bar"
        }
      },
      "SyncTemplate": {
        "type": "object",
        "properties": {
          "Enabled": { "type": "boolean" },
          "Pass": { "type": "string" },
          "Server": { "type": "string" },
          "User": { "type": "string" }
        },
        "required": [ "Enabled", "Pass", "Server", "User" ],
        "example": {
          "Enabled": false,
          "Pass": "admin",
          "Server": "192.168.0.1",
          "User": "admin"
        }
      }
    }
  }
})"_json;
static const auto DocGetAnalyzers = R"({
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
static const auto DocGetAnalyzer = R"({
  "paths": {
    "/getAnalyzer": {
      "post": {
        "summary": "Get list of avaliable analyzers",
        "tags": [ "Analyzers" ],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "example": {
                  "Analyzer": "VIRTUAL_3PN"
                },
                "properties": {
                  "Analyzer": { "type": "string" }
                },
                "required": [ "Analyzer" ]
              }
            }
          }
         },
        "responses": {
          "200": {
            "description": "The method executed without error",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object",
                  "properties": {
                    "AlarmsEnabled": { "type": "boolean" },
                    "EventsEnabled": { "type": "boolean" },
                    "MaxCurrent": { "type": "number" },
                    "MinCurrent": { "type": "number" },
                    "Mode": { "type": "string", "enum": ["1PHASE", "3PHASE", "3PHASE+N"] },
                    "Name": { "type": "string" },
                    "NominalFrequency": { "type": "number" },
                    "NominalVoltage": { "type": "number" },
                    "Online": { "type": "boolean" },
                    "OnlineTime": { "type": "number" },
                    "SampleRate": { "type": "number" },
                    "Save200MS": { "type": "boolean" },
                    "TimeZone": { "type": "boolean" },
                    "User": { "type": "string" },
                    "Alarms": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/AlarmTemplate" }
                    },
                    "Users": {
                      "type": "array",
                      "items": { "$ref": "#/components/schemas/PermissionTemplate" }
                    },
                    "MQTT": { "$ref": "#/components/schemas/MQTTTemplate" },
                    "Tags": { "$ref": "#/components/schemas/TagsTemplate" },
                    "PhaseNames": { "type": "array", "items": { "type": "string"} },
                    "Sync": { "$ref": "#/components/schemas/SyncTemplate" },
                    "PriceModules": { "$ref": "#/components/schemas/PriceModuleTemplate" },
                    "Rates": { "type": "array", "items": { "$ref": "#/components/schemas/RateTemplate" } }
                  },
                  "required": [ "AlarmsEnabled", "EventsEnabled", "MaxCurrent", "MinCurrent", "Mode", "Name", "NominalFrequency", "NominalVoltage", "Online", "OnlineTime", "SampleRate", "Save200MS", "TimeZone", "User", "Alarms", "MQTT", "Users", "Tags", "PhaseNames", "Sync", "PriceModules", "Rates" ],
                  "example": {
                    "AlarmsEnabled": true,
                    "EventsEnabled": true,
                    "MaxCurrent": 35,
                    "MinCurrent": 0.1,
                    "Mode": "3PHASE+N",
                    "Name": "Unnamed analyzer",
                    "NominalFrequency": 50,
                    "NominalVoltage": 230,
                    "Online": false,
                    "OnlineTime": 0,
                    "SampleRate": 100000,
                    "Save200MS": false,
                    "TimeZone": "Europe/Madrid",
                    "User": "admin",
                    "PhaseNames": [ "R", "S", "T", "N" ]
                  }
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


static const auto DocDelAlarm = R"({
  "paths": {
    "/delAlarm": {
      "post": {
        "summary": "Delete alarm",
        "tags": [ "Analyzers" ],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "example": {
                  "Analyzer": "VIRTUAL_3PN",
                  "AlarmID": "00000000-0000-0000-0000-000000000000",
                  "Trigger": 1600000000000
                },
                "properties": {
                  "Analyzer": { "type": "string" },
                  "AlarmID": { "type": "string" },
                  "Trigger": { "type": "number" }
                },
                "required": [ "Analyzer", "AlarmID", "Trigger" ]
              }
            }
          }
        },
        "responses": {
          "200": { "$ref": "#/components/responses/200Response" },
          "400": { "$ref": "#/components/responses/400Response" },
          "401": { "$ref": "#/components/responses/401Response" },
          "500": { "$ref": "#/components/responses/500Response" }
        }
      }
    }
  }
})"_json;

//    static void SetAlarmCallback(const json &params, Response &response);
//    static void DelAlarmsRangeCallback(const json &params, Response &response);
//    static void SetRemoteCallback(const json &params, Response &response);
//======================================================================================================================