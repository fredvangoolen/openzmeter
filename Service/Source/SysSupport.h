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
#include <queue>
#include "HTTPServer.h"
#ifdef OPENWRT
  #include <iwlib.h>
  #include <dbus/dbus.h>
#else

#endif
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class SysSupport final {
  public :
    typedef struct {
      RWLocks            Lock;            //Lock before access struct
      bool               Updates;         //System have uninstalled updates
      bool               Events;          //Running events
      uint64_t           UpTime;          //Uptime in seconds
      uint64_t           Swap_Total;      //Total swap memory in bytes
      float              Swap_Usage;      //Swap usage in percentage
      float              CPU_Usage;       //CPU usage in percentage
      uint64_t           Disk_Total;      //Total disk space in bytes
      float              Disk_Usage;      //Disk usage in percentage
      uint64_t           RAM_Total;       //Total RAM memory in bytes
      float              RAM_Usage;       //RAM usage in percentage
      float              Temperature;     //System temperature
      string             WIFI_Network;    //Current WIFI network or empty for non connected
      float              WIFI_Power;      //Current WIFI signal if connected
      string             BT_Network;      //BT network or empty if disabled
      bool               Time_IsSet;      //If system time is set (manualy or with NTP comunication)
      float              Battery_Volt;    //Battery voltage
      float              Battery_Level;   //Battery percentage

    } SystemStatus_t;
  private :
    #ifdef OPENWRT
      typedef struct {
        string Package;
        string From;
        string To;
      } Update_t;
    #endif
  private :
    static SystemStatus_t     pSystemStatus;
    static queue<string>      pSystemActions;
    static RWLocks            pActionsLock;
    #ifdef OPENWRT
      static DBusConnection  *pDBus;
      static pthread_mutex_t  pBTMutex;
      static string           pBTBridgeName;
      static string           pBTServerName;
      static bool             pBTEnabled;
      static string           pInterfaceWIFI;
      static string           pNetworkWIFI;
      static string           pWirelessWIFI;
      static vector<Update_t> pUpdates;
      static RWLocks          pUpdatesLock;
      static uint64_t         pLastTask_1H;
      static uint64_t         pLastTask_5S;
      static uint64_t         pLastTask_1M;
    #endif
  private :
    static bool Register();
    static void GetNewsCallback(const json &params, Response &response);
    static void GetUsersCallback(const json &params, Response &response);
    static void GetUserCallback(const json &params, Response &response);
    static void SetUserPasswordCallback(const json &params, Response &response);
    static void SetUserCallback(const json &params, Response &response);
    static void AddUserCallback(const json &params, Response &response);
    static void DelUserCallback(const json &params, Response &response);
    #ifdef OPENWRT
      static void     GetBluetoothCallback(const json &params, Response &response);
      static void     SetBluetoothCallback(const json &params, Response &response);
      static void     GetWIFICallback(const json &params, Response &response);
      static void     SetWIFICallback(const json &params, Response &response);
      static void     ScanWIFICallback(const json &params, Response &response);
      static void     GetNTPCallback(const json &params, Response &response);
      static void     SetNTPCallback(const json &params, Response &response);
      static void     GetUpdatesCallback(const json &params, Response &response);
      static void     UpgradeCallback(const json &params, Response &response);
      static string   Run(string command, int *resultCode = NULL);
      static bool     StartBT();
      static bool     StopBT();
      static string   GetMacBT();
      static bool     SendBoolBT(const char *property, bool value);
      static bool     SendUINT32BT(const char *property, uint32_t value);
      static bool     SendStringBT(const char *property, const char *value);
      static void     HandleAgentBT();
      static uint64_t ParseProcMem(const char *name, const char *buff);
      static void     PeriodicTask_5S();
      static void     PeriodicTask_1M();
      static void     PeriodicTask_1H();
    #endif
  public :
    static SystemStatus_t *GetSystemStatus();
    static json            GetSystemStatusJSON();
    static void            SetSystemActions(const string &action);
    static void            PeriodicTask();
  public:
    SysSupport();
    ~SysSupport();
};