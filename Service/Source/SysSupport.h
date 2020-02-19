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
  private :  //System actions variables
    static queue<string>      pSystemActions;
    static pthread_mutex_t    pSystemActionMutex;
    static pthread_cond_t     pSystemActionCond;
    static pthread_t          pSystemActionThread;
  private :  //System status variables
    static bool     pTerminate;
    static RWLocks  pSysStatLock;
    static bool     pBT_Enabled;        //BT network enabled/disabled
    static uint64_t pUpTime;            //Uptime in seconds
    static uint64_t pSwap_Total;        //Total swap memory in bytes
    static float    pSwap_Usage;        //Swap usage in percentage
    static float    pCPU_Usage;         //CPU usage in percentage
    static uint64_t pDisk_Total;        //Total disk space in bytes
    static float    pDisk_Usage;        //Disk usage in percentage
    static uint64_t pRAM_Total;         //Total RAM memory in bytes
    static float    pRAM_Usage;         //RAM usage in percentage
    static float    pTemperature;       //System temperature
    static string   pWIFI_Network;      //Current WIFI network or empty for non connected
    static float    pWIFI_Power;        //Current WIFI signal if connected
    static bool     pTimeSet;           //If system time is set (manualy or with NTP comunication)

//    bool               Updates;         //System have uninstalled updates
//    bool               Events;          //Running events


//    float              Battery_Volt;    //Battery voltage
//    float              Battery_Level;   //Battery percentage
  #ifdef OPENWRT
  private :
    static bool        pBTEnable;       //Enable or disable
    static string      pBTServerName;   //bluetooth network name
    static string      pBTBridgeName;   //bluetooth bridge name
    static pthread_t   pBTThread;       //bluetooth manager thread
    static pthread_t   pStatsThread;    //sys mem, cpu, etc thread
//      static DBusConnection  *pDBus;
//      static pthread_mutex_t  pBTMutex;

//      static bool             pBTEnabled;
      static string           pInterfaceWIFI;
      static string           pNetworkWIFI;
      static string           pWirelessWIFI;

//      static vector<Update_t> pUpdates;
//      static RWLocks          pUpdatesLock;

//      static uint64_t         pLastTask_1H;
//      static uint64_t         pLastTask_5S;
//      static uint64_t         pLastTask_1M;
  #endif
  private :
    static void GetVersionCallback(const json &params, Response &response);
    static void GetUsersCallback(const json &params, Response &response);
    static void GetUserCallback(const json &params, Response &response);
    static void SetUserPasswordCallback(const json &params, Response &response);
    static void SetUserCallback(const json &params, Response &response);
    static void AddUserCallback(const json &params, Response &response);
    static void DelUserCallback(const json &params, Response &response);
  #ifdef OPENWRT
  private :
    static string   GetMacBT(DBusConnection *bus);
    static string   Run(string command, int *resultCode = NULL);
    static uint64_t ParseProcMem(const char *name, const char *buff);
    static bool     SendBoolBT(DBusConnection *bus, const char *property, bool value);
    static bool     SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value);
    static bool     SendStringBT(DBusConnection *bus, const char *property, const char *value);
    static void     Upgrade(const string &package);
    static void     GetUpdates();
    static void    *Bluetooth_Thread(void *params);
    static void    *SysStat_Thread(void *params);
    static void     GetBluetoothCallback(const json &params, Response &response);
    static void     SetBluetoothCallback(const json &params, Response &response);
    static void     GetWIFICallback(const json &params, Response &response);
    static void     SetWIFICallback(const json &params, Response &response);
    static void     ScanWIFICallback(const json &params, Response &response);
    static void     GetNTPCallback(const json &params, Response &response);
    static void     SetNTPCallback(const json &params, Response &response);
    static void     GetUpdatesCallback(const json &params, Response &response);
    static void     SetUpgradeCallback(const json &params, Response &response);
//      static void     PeriodicTask_5S();
//      static void     PeriodicTask_1M();
//      static void     PeriodicTask_1H();
    #endif
  private :
    static void *Actions_Thread(void *params);
  public :
    static json GetSystemStatusJSON();
    static void AddSystemAction(const string &action);
  public:
    SysSupport();
    ~SysSupport();
};