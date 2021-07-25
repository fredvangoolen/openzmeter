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
#include "HTTPRequest.h"
#ifdef OPENWRT
  #include <dbus/dbus.h>
  #include <sys/ioctl.h>
  #include <net/if.h>
#endif
// ---------------------------------------------------------------------------------------------------------------------
class BluetoothManager final {
#ifdef OPENWRT
  private :
    static volatile bool    pTerminate;    //Terminate flag
    static pthread_mutex_t  pMutex;        //Shared variables lock
    static pthread_t        pThread;       //bluetooth manager thread
    static string           pServerName;   //bluetooth network name
    static string           pInterface;    //bluetooth bridge name
    static in_addr          pMask;         //Interface netmask
    static in_addr          pIP;           //Interface IP
  private :
    static void     ToggleEnable();
    static string   GetMacBT(DBusConnection *bus);
    static bool     SendBoolBT(DBusConnection *bus, const char *property, bool value);
    static bool     SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value);
    static bool     SendStringBT(DBusConnection *bus, const char *property, const char *value);
    static void    *Thread(void *params);
#endif
  private :
    static void     GetBluetoothCallback(HTTPRequest &req);
    static void     SetBluetoothCallback(HTTPRequest &req);
  public :
    static bool Enabled();
    static bool InNetwork(const string &ip);
  public :
    BluetoothManager();
    ~BluetoothManager();
};
// ---------------------------------------------------------------------------------------------------------------------