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
#include <set>
#include <pthread.h>
#include "nlohmann/json.hpp"
#include "Analyzer.h"
#include "HTTPServer.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
using json = nlohmann::json;
//----------------------------------------------------------------------------------------------------------------------
class Device {
  protected :
    bool                   pEnabled;            //Enable flag
    const string           pSerial;             //Device serial
    const string           pDriver;             //Driver name
    const string           pName;               //Driver description
  protected :
    Device(const string &serial, const string &driver, const string &name);
  protected :
    virtual uint32_t AnalyzersCount() = 0;
  public :
    json   GetJSONDescriptor();
    string Serial() const;
  public :
    virtual ~Device();
    virtual void GetDevice(const json &params, Response &response) = 0;
    virtual void SetDevice(const json &params, Response &response) = 0;
};
//----------------------------------------------------------------------------------------------------------------------