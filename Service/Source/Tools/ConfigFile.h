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
#include <pthread.h>
#include <stdlib.h>
#include "nlohmann/json.h"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class ConfigFile {
  private :
    static pthread_mutex_t   pMutex;
    static int               pFile;
    static json              pConfig;
  public :
    ConfigFile(const string &name);
    virtual ~ConfigFile();
  public :
    static json ReadConfig(const string &path);
    static bool WriteConfig(const string &path, const json &val);
};
//----------------------------------------------------------------------------------------------------------------------