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
#include "nlohmann/json.hpp"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerAccess {
  public :
    enum Access { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
  private :
    const string pSerial;
    const string pOwner;
  public :
    static json CheckConfig(const json &config, const string &owner = "admin");
  public :
    AnalyzerAccess(const string &serial, const string &owner = "admin");
    virtual ~AnalyzerAccess();
    void Config(const json &config) const;
    json Users() const;
    json GetStatus(const vector<string> &series, const string &user = "admin") const;
    bool TestUser(const Access neededAccess, const string &user = "admin") const;
};
// ---------------------------------------------------------------------------------------------------------------------