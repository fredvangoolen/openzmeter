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
#include "HTTPRequest.h"
#include "nlohmann/json.hpp"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class AnalyzerEvents {
  protected :
    const string  pSerial;
  public :
    void GetEvents(HTTPRequest *req) const;
    void SetEvents(HTTPRequest *req) const;
    void SetEvent(HTTPRequest *req) const;
    void DelEvent(HTTPRequest *req) const;
    void GetEvent(HTTPRequest *req) const;
    void GetEventsRange(HTTPRequest *req) const;
    void DelEventsRange(HTTPRequest *req) const;
  public :
    AnalyzerEvents(const string &id);
    virtual ~AnalyzerEvents();
};
//----------------------------------------------------------------------------------------------------------------------