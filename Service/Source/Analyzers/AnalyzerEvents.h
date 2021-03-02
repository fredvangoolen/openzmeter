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
class AnalyzerEvents {
  protected :
    const string  pSerial;
  public :
    void GetEvents(const json &params, Response &response) const;
    void SetEvents(const json &params, Response &response) const;
    void SetEvent(const json &params, Response &response) const;
    void DelEvent(const json &params, Response &response) const;
    void GetEvent(const json &params, Response &response) const;
    void GetEventsRange(const json &params, Response &response) const;
    void DelEventsRange(const json &params, Response &response) const;
  public :
    AnalyzerEvents(const string &id);
    virtual ~AnalyzerEvents();
};
//----------------------------------------------------------------------------------------------------------------------