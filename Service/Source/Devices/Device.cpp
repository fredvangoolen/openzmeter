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
#include <string.h>
#include <math.h>
#include "Device.h"
#include "HTTPServer.h"
#include "Tariff.h"
//----------------------------------------------------------------------------------------------------------------------
Device::Device(const string &serial, const string &driver, const string &name) : pSerial(serial), pDriver(driver), pName(name) {
  pEnabled = false;
}
Device::~Device() {
}
json Device::GetJSONDescriptor() {
  json Device = json::object();
  Device["Driver"]    = pDriver;
  Device["Name"]      = pName;
  Device["Serial"]    = pSerial;
  Device["Enabled"]   = pEnabled;
  Device["Analyzers"] = AnalyzersCount();
  return Device;
}
string Device::Serial() const {
  return pSerial;
}
//----------------------------------------------------------------------------------------------------------------------