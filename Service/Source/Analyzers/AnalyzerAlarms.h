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
//----------------------------------------------------------------------------------------------------------------------
using namespace httplib;
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerAlarms {
  protected :
    const string  pSerial;
  public :
    void GetAlarms(HTTPRequest *req) const;
    void SetAlarms(HTTPRequest *req) const;
    void DelAlarm(HTTPRequest *req) const;
    void SetAlarm(HTTPRequest *req) const;
    void GetAlarmsRange(HTTPRequest *req) const;
    void DelAlarmsRange(HTTPRequest *req) const;
  public :
    AnalyzerAlarms(const string &id);
    virtual ~AnalyzerAlarms();
};
//----------------------------------------------------------------------------------------------------------------------