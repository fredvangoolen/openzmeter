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
#include <vector>
#include "Device.h"
#include "AnalyzerRemote.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class DeviceRemote final : public Device {
  private :
    static bool            pRegistered;
  private :
    RWLocks                pLocks;
    bool                   pEnabled;
  private :
    static bool            Register();
    static vector<Device*> GetInstances();
    static json            CheckConfig(const json &config);
  private :
    static void GetRemoteCallback(const json &params, Response &response);
    static void SetRemoteCallback(const json &params, Response &response);
  private :
    DeviceRemote();
    void Configure(const json &config);
    void GetDevice(const json &params, Response &response) override;
    void SetDevice(const json &params, Response &response) override;
  public :
    ~DeviceRemote();
};
//----------------------------------------------------------------------------------------------------------------------
