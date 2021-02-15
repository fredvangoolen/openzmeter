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
#include <string>
#include <map>
#include <pthread.h>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerAccess final {
  public :
    enum Access { READ = 0x01, WRITE = 0x02, CONFIG = 0x04 };
  private :
    string pSerial;
  public :
    static json CheckUsers(const json &config);
  public :
    AnalyzerAccess(const string &serial);
    ~AnalyzerAccess();
    void SetUsers(const json &config);
    json GetUsers();
    void SetTime(const string &user, const uint64_t &alarm, const uint64_t &event);
    json GetTime(const string &user);
    bool TestUser(const string &user, const Access neededAccess);
};
// ---------------------------------------------------------------------------------------------------------------------