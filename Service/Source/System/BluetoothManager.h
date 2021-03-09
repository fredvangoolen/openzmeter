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
#include "HTTPServer.h"
#ifdef OPENWRT
  #include <pthread.h>
  #include <iwlib.h>
  #include <dbus/dbus.h>
#endif
// ---------------------------------------------------------------------------------------------------------------------
class BluetoothManager final {
#ifdef OPENWRT
  private :
    static volatile bool    pTerminate;    //Terminate flag
    static pthread_mutex_t  pMutex;        //Shared variables lock
    static pthread_t        pThread;       //bluetooth manager thread
    static bool             pEnable;       //Enable or disable
    static bool             pEnabled;      //Current status
    static string           pServerName;   //bluetooth network name
    static string           pInterface;    //bluetooth bridge name
  private :
    static void     ToggleEnable();
    static string   GetMacBT(DBusConnection *bus);
    static bool     SendBoolBT(DBusConnection *bus, const char *property, bool value);
    static bool     SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value);
    static bool     SendStringBT(DBusConnection *bus, const char *property, const char *value);
    static void    *Thread(void *params);
#endif
  private :
    static void     GetBluetoothCallback(const json &params, Response &response);
    static void     SetBluetoothCallback(const json &params, Response &response);
  public :
    static bool Enabled();
  public :
    BluetoothManager();
    ~BluetoothManager();
};
//----------------------------------------------------------------------------------------------------------------------

// Module OpenAPI documentation ========================================================================================
static const auto DocBluetooth = R"({
  "tags": [ { "name": "Bluetooth", "description": "Manage bluetooth services, if avaliable" } ]
})"_json;
static const auto DocGetBluetooth = R"({
  "paths": {
    "/getBluetooth": {
      "post": {
        "summary": "Get bluetooth configuration",
        "tags": [ "Bluetooth" ],
        "requestBody": { "$ref": "#/components/requestBodies/EmptyObject" },
        "responses": {
          "200": {
            "description": "The method executed without error",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object",
                  "properties": {
                    "Enabled": { "type": "boolean" },
                    "Running": { "type": "boolean" },
                    "Interface": { "type": "string" },
                    "Name": { "type": "string"}
                  },
                  "example": {
                    "Enabled": true,
                    "Running": true,
                    "Interface": "br-bt",
                    "Name": "openZmeter_xxxx"
                  }
                }
              }
            }
          },
          "401": { "$ref": "#/components/responses/401Response" },
          "403": { "$ref": "#/components/responses/403Response" },
          "405": { "$ref": "#/components/responses/403Response" }
        }
      }
    }
  }
})"_json;
static const auto DocSetBluetooth = R"({
  "paths": {
    "/setBluetooth": {
      "post": {
        "summary": "Set bluetooth configuration",
        "tags": [
          "Bluetooth"
        ],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "required": [ "Enabled" ],
                "example": {
                  "Enabled": true
                },
                "properties": {
                  "Enabled": { "type": "boolean" }
                }
              }
            }
          }
        },
        "responses": {
          "200": { "$ref": "#/components/responses/200Response" },
          "401": { "$ref": "#/components/responses/401Response" },
          "403": { "$ref": "#/components/responses/403Response" },
          "405": { "$ref": "#/components/responses/403Response" }
        }
      }
    }
  }
})"_json;
//======================================================================================================================