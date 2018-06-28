/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <dbus/dbus.h>
#include <json-c/json.h>
class NetworkManager {
  private :
    static const uint32_t NM_DEVICE_TYPE_WIFI;
  private :
    static DBusConnection *CreateConnection();
    static void    SendGetDevices(DBusConnection *dbus, json_object *json);
    static void    SendGetDeviceType(DBusConnection *dbus, json_object *json, char *devicePath);
    static void    SendRequestScan(DBusConnection *dbus, char *devicePath);
    static void    SendGetAccessPoints(DBusConnection *dbus, json_object *json, char *devicePath);
    static bool    SendGetAPProperties(DBusConnection *dbus, json_object *json, char *path);
    static void    SendGetActiveConnections(DBusConnection *dbus, json_object *json);
    static bool    SendGetConnectionProperties(DBusConnection *dbus, json_object *json, char *path);
    static void    SendGetIP4Properties(DBusConnection *dbus, json_object *json, char *path);
    static void    SendGetSettingsPaths(DBusConnection *dbus, json_object *json);
    static void    RecvGetSettingsPaths(DBusConnection *dbus, json_object *json, DBusMessage* reply);
    static bool    SendGetSettings(DBusConnection *dbus, json_object *json, char *path);
    static bool    SendSaveSettings(DBusConnection *dbus, json_object *json);    
    static void    SendEnable(DBusConnection *dbus, int state);
    static uint8_t SendConnectivityState(DBusConnection *dbus);
  public :
    static uint8_t      GetConnectivityStatus();
    static bool         SetJSONNetworkConfig(json_object *json);
    static json_object *GetJSONNetworks();
};

