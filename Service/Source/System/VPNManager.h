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
#include "HTTPServer.h"
#ifdef OPENWRT // ======================================================================================================
class VPNManager final {
  private :
    static constexpr char *pFileName = (char*)"/etc/openvpn/openZmeter.conf";
  private :
    static void     GetVPNCallback(const json &params, Response &response);
    static void     SetVPNCallback(const json &params, Response &response);
  public :
    VPNManager();
    ~VPNManager();
};
#else // ===============================================================================================================
class VPNManager final {
  public :
    VPNManager();
    ~VPNManager();
};
#endif // ==============================================================================================================
