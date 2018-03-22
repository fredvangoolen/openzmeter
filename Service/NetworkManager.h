/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.h
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#pragma once
#include <dbus/dbus.h>
//----------------------------------------------------------------------------------------------------------------------
class NetworkManager {
  private :
    static uint32_t NM_DEVICE_TYPE_WIFI;
  private :
    DBusConnection *DBus;
    bool RecvGetDevices(DBusMessage *reply);
    bool SendGetDeviceType(char *devicePath);
    bool RecvGetDeviceType(DBusMessage *reply, char *devicePath);
    bool SendGetAccessPoints(char *devicePath);
    bool RecvGetAccessPoints(DBusMessage *reply);
    bool SendGetAPProperties(char *path);
    bool RecvGetAPProperties(DBusMessage *reply);
  public :
    typedef struct {
      char     Name[32];
      char     MAC[18];
      uint32_t WpaFlags;
      uint32_t Mode;
      uint32_t Freq;
      uint8_t  Power;
    } AccessPoint_t;
    
  public : 
    NetworkManager();
    bool SendGetDevices();
};
