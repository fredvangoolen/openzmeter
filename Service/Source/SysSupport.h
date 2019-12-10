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
class SysSupport {
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
    static void GetNewsCallback(HTTPRequest &req);
    static void GetUsersCallback(HTTPRequest &req);
    static void GetUserCallback(HTTPRequest &req);
    static void SetUserCallback(HTTPRequest &req);
    static void AddUserCallback(HTTPRequest &req);
    static void DelUserCallback(HTTPRequest &req);
    #ifdef OPENWRT
      static void     GetBluetoothCallback(HTTPRequest &req);
      static void     SetBluetoothCallback(HTTPRequest &req);
      static void     GetWIFICallback(HTTPRequest &req);
      static void     SetWIFICallback(HTTPRequest &req);
      static void     ScanWIFICallback(HTTPRequest &req);
      static void     GetNTPCallback(HTTPRequest &req);
      static void     SetNTPCallback(HTTPRequest &req);
      static void     GetUpdatesCallback(HTTPRequest &req);
      static void     UpgradeCallback(HTTPRequest &req);
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
    virtual ~SysSupport();
};