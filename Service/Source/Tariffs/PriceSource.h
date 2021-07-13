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
#include <map>
#include "nlohmann/json.hpp"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class PriceSource {
  protected :
    typedef struct {
      PriceSource* (*Instance)();
      json         (*VarsInfo)();
      void         (*VarsFill)(map<string, float> &params);
    } Module_t;
  protected :
    static map<string, Module_t> pModules;
  public :
    static PriceSource *Instance(const string &name, map<string, float> &params);
    static bool         CheckConfig(const string &name, map<string, float> &params);
    static json         VarsInfo();
  public :
    virtual ~PriceSource();
    virtual void GetSample(const uint64_t sampletime, map<string, float> &vars) = 0;
};
// ---------------------------------------------------------------------------------------------------------------------