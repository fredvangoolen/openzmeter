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
#include "RWLocks.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class StatusManager final {
  private :  //System actions variables
//    static queue<string>      pSystemActions;
//    static pthread_mutex_t    pSystemActionMutex;
  private :
    static json     pStatus;
    static RWLocks  pLock;


//    static bool     pBT_Enabled;        //BT network enabled/disabled
//    static uint64_t pUpTime;            //Uptime in seconds
//    static uint64_t pSwap_Total;        //Total swap memory in bytes
//    static float    pSwap_Usage;        //Swap usage in percentage
//    static float    pCPU_Usage;         //CPU usage in percentage
//    static uint64_t pDisk_Total;        //Total disk space in bytes
//    static float    pDisk_Usage;        //Disk usage in percentage
//    static uint64_t pRAM_Total;         //Total RAM memory in bytes
//    static float    pRAM_Usage;         //RAM usage in percentage
//    static float    pTemperature;       //System temperature
//    static string   pWIFI_Network;      //Current WIFI network or empty for non connected
//    static float    pWIFI_Power;        //Current WIFI signal if connected
//    static bool     pTimeSet;           //If system time is set (manualy or with NTP comunication)
//    bool               Events;          //Running events


//    float              Battery_Volt;    //Battery voltage
//    float              Battery_Level;   //Battery percentage
  public :
    static json GetJSON();
    static void SetValue(const string &path, const json &value);
    static json GetValue(const string &path);
  public:
    StatusManager();
    ~StatusManager();
};