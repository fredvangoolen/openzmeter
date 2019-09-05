#pragma once
#include <iwlib.h>
#include <string>
#include <dbus/dbus.h>
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class System {
  private :
    typedef struct {
      string Package;
      string From;
      string To;
    } Update_t;
  private :
    static System          *pInstance;
    static pthread_mutex_t  pMutex;
    static bool             pRegistered;
    static vector<Update_t> pUpdates;
    static uint64_t         pLastUpdateCheck;
  private :
    DBusConnection  *pDBus;
    string           pBTBridgeName;
    string           pBTServerName;
    bool             pBTStarted;
    string           pInterfaceWIFI;
    string           pNetworkWIFI;
    string           pWirelessWIFI;
  private:
    bool   StartBT();
    bool   StopBT();
    string GetMacBT();
    bool   SendBoolBT(const char *property, bool value);
    bool   SendUINT32BT(const char *property, uint32_t value);
    bool   SendStringBT(const char *property, const char *value);
    void   HandleAgentBT();
    void   CheckUpdates();
  private :
    static string   Run(string command, int *resultCode = NULL);
    static uint64_t ParseProcMem(const char *name, const char *buff);
    static void GetBluetoothCallback(HTTPRequest &req);
    static void SetBluetoothCallback(HTTPRequest &req);
    static void GetWIFICallback(HTTPRequest &req);
    static void SetWIFICallback(HTTPRequest &req);
    static void ScanWIFICallback(HTTPRequest &req);
    static void GetNTPCallback(HTTPRequest &req);
    static void SetNTPCallback(HTTPRequest &req);
    static void GetUpdatesCallback(HTTPRequest &req);
    static void UpdateCallback(HTTPRequest &req);
  public :
    static json GetSystemDetails();  
  public:
    System();
    void PeriodicTask();
    virtual ~System();
};
//----------------------------------------------------------------------------------------------------------------------