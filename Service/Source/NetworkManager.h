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
    void SendGetDeviceType(json_object *json, char *devicePath);
    void SendRequestScan(char *devicePath);
    void SendGetAccessPoints(json_object *json, char *devicePath);
    bool SendGetAPProperties(json_object *json, char *path);
    void SendGetActiveConnections(json_object *json);
    bool SendGetConnectionProperties(json_object *json, char *path);
    void SendGetIP4Properties(json_object *json, char *path);
    void SendGetSettingsPaths(json_object *json);
    void RecvGetSettingsPaths(json_object *json, DBusMessage* reply);
    bool SendGetSettings(json_object *json, char *path);
    bool SendSaveSettings(json_object *json);    
    void SendEnable(int state);
  private:
    NetworkManager();
    ~NetworkManager();    
  public :
    static bool SetJSONNetworkConfig(json_object *json);
    static json_object *GetJSONNetworks();
};

