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
    void Config(const json &config) const;
    json Users() const;
    void SetTime(const string &user, const uint64_t &alarm, const uint64_t &event) const;
    json GetTime(const string &user) const;
    bool TestUser(const string &user, const Access neededAccess) const;
  public :
    AnalyzerAccess(const string &serial, const string &owner = "admin");
    virtual ~AnalyzerAccess();
};
// ---------------------------------------------------------------------------------------------------------------------