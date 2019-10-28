#pragma once
#include <string>
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
    static SysSupport      *pInstance;
    static pthread_mutex_t  pMutex;
  private :
    json               pSystemStatus;
    #ifdef OPENWRT
      DBusConnection  *pDBus;
      string           pBTBridgeName;
      string           pBTServerName;
      bool             pBTStarted;
      string           pInterfaceWIFI;
      string           pNetworkWIFI;
      string           pWirelessWIFI;
      vector<Update_t> pUpdates;
      uint64_t         pLastUpdateCheck;
      uint64_t         pLastDestailsCheck;
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
      static void   GetBluetoothCallback(HTTPRequest &req);
      static void   SetBluetoothCallback(HTTPRequest &req);
      static void   GetWIFICallback(HTTPRequest &req);
      static void   SetWIFICallback(HTTPRequest &req);
      static void   ScanWIFICallback(HTTPRequest &req);
      static void   GetNTPCallback(HTTPRequest &req);
      static void   SetNTPCallback(HTTPRequest &req);
      static void   GetUpdatesCallback(HTTPRequest &req);
      static void   UpgradeCallback(HTTPRequest &req);
      static string Run(string command, int *resultCode = NULL);
    #endif
  private:
    #ifdef OPENWRT
      bool     StartBT();
      bool     StopBT();
      string   GetMacBT();
      bool     SendBoolBT(const char *property, bool value);
      bool     SendUINT32BT(const char *property, uint32_t value);
      bool     SendStringBT(const char *property, const char *value);
      void     HandleAgentBT();
      void     CheckUpdates();
      uint64_t ParseProcMem(const char *name, const char *buff);
      void     UpdateSystemDetails();
    #endif
  public :
    static json GetSystemDetails();
    static void SetSystemDetails(const json &details);
  public:
    SysSupport();
    void PeriodicTask();
    virtual ~SysSupport();
};