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
#ifdef OPENWRT // ======================================================================================================
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class System final {
  private :
    static void     PeriodicTasks();
    static void     PowerOff();
    static uint64_t ParseProcMem(const string &name, const string &buff);
  public :
    static string Exec(const string &command, int *resultCode = NULL);
    static json   uBusCall(const string &path, const string &method, const json &params = nullptr);
  public :
    System();
    ~System();
};
#else // ===============================================================================================================
class System final {
  public :
    System();
    ~System();
};
#endif // ==============================================================================================================