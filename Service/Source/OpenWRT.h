#pragma once
#include <string>
#include <dbus/dbus.h>
#include <uci.h>
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
using namespace std;
//----------------------------------------------------------------------------------------------------------------------
class Bluetooth {
  private :
    static Bluetooth *pInstance;
    static bool       pRegistered;
  private :
    DBusConnection  *pDBus;
    string           pBridgeName;

//    int              pBridgeSocket;
    string           pServerName;
    pthread_mutex_t  pMutex;
//    int              pDNSMasqPID;
    bool             pFailState;
  private:
    bool   Start();
    bool   Stop();
    string GetMac();
    bool   SendBool(const char *property, bool value);
    bool   SendUINT32(const char *property, uint32_t value);
    bool   SendString(const char *property, const char *value);
  public :
    static void GetBluetoothCallback(HTTPRequest &req);
    static void SetBluetoothCallback(HTTPRequest &req);
    static void GetWiFiNetworksCallback(HTTPRequest &req);
  public:
    Bluetooth();
    void HandleAgent();
    virtual ~Bluetooth();
};