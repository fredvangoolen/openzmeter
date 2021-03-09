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
#include <vector>
#include <list>
#include <stdlib.h>
#include "nlohmann/json.hpp"
#include "cpp-httplib/httplib.h"
#include "Database.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace httplib;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
#define HTTP_OK                     200
#define HTTP_BAD_REQUEST            400
#define HTTP_UNAUTHORIZED           401
#define HTTP_FORBIDDEN              403
#define HTTP_NOT_FOUND              404
#define HTTP_METHOD_NOT_ALLOWED     405
#define HTTP_INTERNAL_SERVER_ERROR  500
#ifndef __BUILD__
#define __BUILD__ "UNKNOWN"
#endif
//----------------------------------------------------------------------------------------------------------------------
class HTTPServer final : private Server {
  private :
    typedef void (*Func1_t)(const json &params, Response &response);
  private :
    static pthread_rwlock_t             pLock;            //Control access to pCallbacks vector
    static map<string, json>            pComponents;      //Components openAPI descriptor
    static map<string, Func1_t>         pCallbacks;       //List of callbacks without reader
    static string                       pRelativePath;    //Relative path of server
    static string                       pLogoImg;         //Aditional logo image
  private:
    static uint16_t                     pPort;
    static pthread_t                    pThread;
  private :
    static void        PeriodicTasks();
    static string      ParseCookie(const string &cookie);
    static string      PatchFile(const string &file, const string &find, const string &replace);
    static string      PatchString(string &src, const string &find, const string &replace);
    static const char *MimeType(const string &file);
    static void        HandleGet(const Request &request, Response &response);
    static void        HandlePost(const Request &request, Response &response);
    static void        GetVersionCallback(const json &params, Response &response);
    static void        LogInCallback(const json &params, Response &response);
    static void        LogOutCallback(const Request &request, Response &response);
    static void        GetAPICallback(const json &params, Response &response);
  private :
    void *Thread();
  public :
    static void   RegisterCallback(const string &handleName, const Func1_t function, const json &doc = json::object());
    static void   UnregisterCallback(const string &handleName);
    static void   RegisterDoc(const string &name, const json &doc);
    static void   UnregisterDoc(const string &name);
    static void   FillResponse(Response &response, const string &body, const char *mime, const int status);
    static void   FillResponse(Response &response, const char *body, const size_t size, const char *mime, const int status);
  public :
    HTTPServer();
    ~HTTPServer();
};
//----------------------------------------------------------------------------------------------------------------------

// Module OpenAPI documentation ========================================================================================
static const auto DocBase = R"({
  "openapi": "3.0.1",
  "info": {
    "title": "openZmeter API",
    "description": "Full API definition of openZmeter daemon. Not all functions are avaliable on all systems, not for all users or not all time.",
    "license": {
      "name": "GNU General Public License v3.0",
      "url": "https://www.gnu.org/licenses/gpl-3.0.html"
    }
  },
  "servers": [],
  "components": {
    "requestBodies": {
      "EmptyObject": {
        "required": true,
        "content": {
          "application/json": {
            "schema": {
              "type": "object",
              "example": {}
            }
          }
        }
      }
    },
    "responses": {
      "200Response" : {
        "description": "The method executed without error",
        "content": {
          "text/plain": {
            "schema": { "type": "string", "example": " " }
          }
        }
      },
      "400Response" : {
        "description": "The request is malformed or have missing params",
        "content": {
          "text/plain": {
            "schema": { "type": "string", "example": "Malformed JSON params" }
          }
        }
      },
      "401Response": {
        "description": "The user is not logged in",
        "content": {
          "text/plain": {
            "schema": { "type": "string", "example": "Unauthorided access to API" }
          }
        }
      },
      "403Response": {
        "description": "The user is not allowed to exec this method",
        "content": {
          "text/plain" : {
            "schema": { "type": "string", "example": " " }
          }
        }
      },
      "405Response": {
        "description": "The method is not allowed in selected object",
        "content": {
          "text/plain": {
            "schema": { "type": "string", "example": " " }
          }
        }
      },
      "500Response": {
        "description": "The method fail to execute",
        "content": {
          "text/plain": {
            "schema": { "type": "string", "example": " " }
          }
        }
      }
    },
    "schemas": {}
  },
  "tags": [ { "name": "System", "description": "System management functions" } ],
  "paths": {
    "/logIn": {
      "post": {
        "summary": "Authorize to access the system",
        "tags": [ "System" ],
        "requestBody": {
          "required": true,
          "content": {
            "application/json": {
              "schema": {
                "required": [ "UserName", "Password" ],
                "example": {
                  "UserName": "admin",
                  "Password": "admin",
                  "UpdateLastLogin": true
                },
                "properties": {
                  "UserName": { "type": "string" },
                  "Password": { "type": "string" },
                  "UpdateLastLogin": { "type": "boolean" }
                }
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
                    "LastLogin": { "type": "number" }
                  },
                  "required": [ "LastLogin" ],
                  "example": {
                    "LastLogin": 1000000
                  }
                }
              }
            },
            "headers": {
              "Set-Cookie": {
                "schema": {
                  "type": "string",
                  "example": "00000000-0000-0000-0000-000000000000"
                }
              }
            }
          },
          "400": { "$ref": "#/components/responses/400Response" },
          "401": { "$ref": "#/components/responses/401Response" }
        }
      }
    },
    "/logOut": {
      "post": {
        "summary": "Close active user session",
        "tags": [ "System" ],
        "requestBody": { "$ref": "#/components/requestBodies/EmptyObject" },
        "responses": {
          "200": { "$ref": "#/components/responses/200Response" }
        }
      }
    }
   }
})"_json;
static const auto DocGetVersion = R"({
  "paths": {
    "/getVersion": {
      "post": {
        "summary": "Get service version and compiled in functions",
        "tags": [ "System" ],
        "requestBody": { "$ref": "#/components/requestBodies/EmptyObject" },
        "responses": {
          "200": {
            "description": "The method executed without error",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object",
                  "properties": {
                    "Version": { "type": "string" },
                    "Functions": { "type": "array", "items": { "type": "string" }}
                  },
                  "required": [ "Version", "Functions" ],
                  "example": {
                    "Version": "branch_yyyymmdd.hh.mm",
                    "Functions": [ "getVersion", "getSeries" ]
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
static const auto DocGetAPI = R"({
  "paths": {
    "/getAPI": {
      "post": {
        "summary": "Get implemented API methods documentation, returned data is in OpenAPI format",
        "tags": [ "System" ],
        "requestBody": { "$ref": "#/components/requestBodies/EmptyObject" },
        "responses": {
          "200": {
            "description": "The method executed without error",
            "content": {
              "application/json": {
                "schema": {
                  "type": "object",
                  "properties": {},
                  "example": {}
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
//======================================================================================================================