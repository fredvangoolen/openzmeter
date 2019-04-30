/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <dbus/dbus.h>
#include <pthread.h>
#include "json.hpp"
//----------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
class NetworkManager {
  private :
    static const uint32_t NM_DEVICE_TYPE_WIFI;
    static pthread_mutex_t pMutex;
  private :
    
    
  private :
    static DBusConnection *CreateConnection();
    static void    SendGetDevices(DBusConnection *dbus, json &resp);
    static void    SendGetDeviceType(DBusConnection *dbus, json &resp, char *devicePath);
    static void    SendRequestScan(DBusConnection *dbus, char *devicePath);
    static void    SendGetAccessPoints(DBusConnection *dbus, json &resp, char *devicePath);
    static bool    SendGetAPProperties(DBusConnection *dbus, json &resp, char *path);
    static void    SendGetActiveConnections(DBusConnection *dbus, json &resp);
    static bool    SendGetConnectionProperties(DBusConnection *dbus, json &resp, char *path);
    static void    SendGetIP4Properties(DBusConnection *dbus, json &resp, char *path);
    static void    SendGetSettingsPaths(DBusConnection *dbus, json &resp);
    static void    RecvGetSettingsPaths(DBusConnection *dbus, json &resp, DBusMessage* reply);
    static bool    SendGetSettings(DBusConnection *dbus, json &resp, char *path);
    static bool    SendSaveSettings(DBusConnection *dbus, json &resp);    
    static void    SendEnable(DBusConnection *dbus, int state);
    static uint8_t SendConnectivityState(DBusConnection *dbus);
  public :
    static void     Reset();
    static uint8_t  GetConnectivityStatus();
    static void     GetNetworksCallback(HTTPRequest &req);
    static bool     SetJSONNetworkConfig(json json);    
};

