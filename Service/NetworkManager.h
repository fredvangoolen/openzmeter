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
    DBusConnection *DBus;
    void SendGetDevices(json_object *json);
    void RecvGetDevices(json_object *json, DBusMessage *reply);
    void SendGetDeviceType(json_object *json, char *devicePath);
    void RecvGetDeviceType(json_object *json, DBusMessage *reply, char *devicePath);
    void SendGetAccessPoints(json_object *json, char *devicePath);
    void RecvGetAccessPoints(json_object *json, DBusMessage *reply);
    void SendGetAPProperties(json_object *json, char *path);
    bool RecvGetAPProperties(json_object *json, DBusMessage *reply);
    void SendGetActiveConnections(json_object *json);
    void RecvGetActiveConnections(json_object *json, DBusMessage* reply);
    void SendGetConnectionProperties(json_object *json, char *path);
    bool RecvGetConnectionProperties(json_object *json, DBusMessage* reply);
    void SendGetIP4Properties(json_object *json, char *path);
    void RecvGetIP4Properties(json_object *json, DBusMessage* reply);
    void SendGetSettingsPaths(json_object *json);
    void RecvGetSettingsPaths(json_object *json, DBusMessage* reply);
    void SendGetSettings(json_object *json, char *path);
    bool RecvGetSettings(json_object *json, DBusMessage* reply);
    bool SendAddSettings(json_object *json);
//    static bool     RecvGetSettings(DBusConnection *bus, json_object *json, DBusMessage* reply);
public:
  NetworkManager();
  virtual ~NetworkManager();
private:

};

