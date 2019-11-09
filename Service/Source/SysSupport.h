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
  private :
    #ifdef OPENWRT
      typedef struct {
        string Package;
        string From;
        string To;
      } Update_t;
    #endif
  private :
    static json               pSystemDetails;
    static RWLocks            pDetailsLock;
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
      static uint64_t         pLastUpdateCheck;
      static uint64_t         pLastDestailsCheck;
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
      static void     CheckUpdates();
      static uint64_t ParseProcMem(const char *name, const char *buff);
      static void     UpdateSystemDetails();
    #endif
  public :
    static json GetSystemDetails();
    static void SetSystemDetails(const json &details);
    static bool PendingSystemActions();
    static void SetSystemActions(const string &action);
    static void PeriodicTask();
  public:
    SysSupport();
    virtual ~SysSupport();
};