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
#include "cpp-httplib/httplib.h"
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace httplib;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
#define HTTP_OK                     200
#define HTTP_BAD_REQUEST            400
#define HTTP_UNAUTHORIZED           401
#define HTTP_FORBIDDEN              403
#define HTTP_NOT_FOUND              404
#define HTTP_METHOD_NOT_ALLOWED     405
#define HTTP_INTERNAL_SERVER_ERROR  500
// ---------------------------------------------------------------------------------------------------------------------
class HTTPRequest final {
  private :
    Response &pResponse;
    string    pAccept;
    json      pParams;
    string    pUserName;
    string    pCookie;
  public :
    bool CheckParamNumber(const string &name);
    bool CheckParamString(const string &name);
    bool CheckParamBool(const string &name);
    bool CheckParamObject(const string &name);
    bool CheckParamArray(const string &name);
    bool CheckParamEnum(const string &name, const vector<string> &values);
    bool CheckParamSet(const string &name, const vector<string> &values, const bool needAll = false);
    bool CheckParamArrayString(const string &name);
  public :
    void    Success(const json &body);
    void    SuccessStreamArray(function<void(function<bool(const json &)>)> func);
    void    Error(const string &body, const int status);
  public :
    void    SetHeader(const string &name, const string &value);
  public :
    json    Params() const;
    json    Param(const string &param) const;
    string  Cookie() const;
    string  UserName() const;
    int     Status() const;
  public :
    HTTPRequest(const Request &request, Response &response);
    ~HTTPRequest();
};
// ---------------------------------------------------------------------------------------------------------------------