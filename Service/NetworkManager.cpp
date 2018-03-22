/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include <string.h>
#include "Tools.h"
#include "NetworkManager.h"
uint32_t NetworkManager::NM_DEVICE_TYPE_WIFI = 2;
NetworkManager::NetworkManager() {
  DBus = NULL;
  DBusError Error;
  dbus_error_init(&Error);
  DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    Tools::LogMsg("ERROR", "Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
  }
  if(DBus == NULL) return;
}
bool NetworkManager::SendGetDevices() {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDevices");
  if(Msg == NULL) return false;
  DBusError Error;
  dbus_error_init(&Error);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, &Error);
  if(dbus_error_is_set(&Error)) {
    dbus_error_free(&Error);
    return false;
  }
  return RecvGetDevices(Reply);
}
bool NetworkManager::RecvGetDevices(DBusMessage *reply) {
  DBusMessageIter IterA, IterB;
  if(!dbus_message_iter_init(reply, &IterA)) return false;
  if(dbus_message_iter_get_arg_type(&IterA) != DBUS_TYPE_ARRAY) return false;
  dbus_message_iter_recurse(&IterA, &IterB);
  char *DevicePath;
  do {
    if(dbus_message_iter_get_arg_type(&IterB) != DBUS_TYPE_OBJECT_PATH) return false;
    dbus_message_iter_get_basic(&IterB, &DevicePath);
    if(!DevicePath) return false;
    if(SendGetDeviceType(DevicePath) == false) return false;
  } while(dbus_message_iter_next(&IterB));

  return true;
}
bool NetworkManager::SendGetDeviceType(char *devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.DBus.Properties", "Get");
  if(Msg == NULL) return false;
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *ParamInterface = "org.freedesktop.NetworkManager.Device";
  if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface)) return false;
  const char *ParamDeviceType = "DeviceType";
  if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType)) return false;
  DBusError Error;
  dbus_error_init(&Error);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, &Error);
  if(dbus_error_is_set(&Error)) {
    dbus_error_free(&Error);
    return false;
  }
  return RecvGetDeviceType(Reply, devicePath);
}
bool NetworkManager::RecvGetDeviceType(DBusMessage *reply, char *devicePath) {
  DBusMessageIter Args;
  if(!dbus_message_iter_init(reply, &Args)) return false;
  if(dbus_message_iter_get_arg_type(&Args) != DBUS_TYPE_VARIANT) return false;
  DBusMessageIter VariantIter;
  dbus_message_iter_recurse(&Args, &VariantIter);
  if(dbus_message_iter_get_arg_type(&VariantIter) != DBUS_TYPE_UINT32) return false;
  uint32_t DeviceType;
  dbus_message_iter_get_basic(&VariantIter, &DeviceType);
  if(DeviceType == NM_DEVICE_TYPE_WIFI) return SendGetAccessPoints(devicePath);
  return true;
}
bool NetworkManager::SendGetAccessPoints(char* devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.NetworkManager.Device.Wireless", "GetAccessPoints");
  if(Msg == NULL) return false;
  DBusError Error;
  dbus_error_init(&Error);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, &Error);
  if(dbus_error_is_set(&Error)) {
    dbus_error_free(&Error);
    return false;
  }
  return RecvGetAccessPoints(Reply);
}
bool NetworkManager::RecvGetAccessPoints(DBusMessage* reply) {
  DBusMessageIter IterA, IterB;
  if(!dbus_message_iter_init(reply, &IterA)) return false;
  if(dbus_message_iter_get_arg_type(&IterA) != DBUS_TYPE_ARRAY) return false;
  dbus_message_iter_recurse(&IterA, &IterB);
  char *Path;
  do {
    if(dbus_message_iter_get_arg_type(&IterB) != DBUS_TYPE_OBJECT_PATH) return false;
    dbus_message_iter_get_basic(&IterB, &Path);
    if(!Path) return false;
    if(SendGetAPProperties(Path) == false) return false;
  } while(dbus_message_iter_next(&IterB));
  return true;
}
bool NetworkManager::SendGetAPProperties(char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  if(Msg == NULL) return false;
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *Param = "org.freedesktop.NetworkManager.AccessPoint";
  if(!dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Param)) return false;
  DBusError Error;
  dbus_error_init(&Error);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, &Error);
  if(dbus_error_is_set(&Error)) {
    dbus_error_free(&Error);
    return false;
  }
  return RecvGetAPProperties(Reply);
}
bool NetworkManager::RecvGetAPProperties(DBusMessage* reply) {
  DBusMessageIter IterA, IterB;
  if(!dbus_message_iter_init(reply, &IterA)) return false;
  if(dbus_message_iter_get_arg_type(&IterA) != DBUS_TYPE_ARRAY) return false;
  dbus_message_iter_recurse(&IterA, &IterB);
  AccessPoint_t AccessPoint;
  do {
    DBusMessageIter Dict;
    dbus_message_iter_recurse(&IterB, &Dict);
    do {
      const char *Key;
      dbus_message_iter_get_basic(&Dict, &Key);
      if(!Key) return false; 
      dbus_message_iter_next(&Dict);
      DBusMessageIter Variant;
      dbus_message_iter_recurse(&Dict, &Variant);
      if(!strcmp(Key, "Ssid")) {
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_ARRAY) return false;
        DBusMessageIter VariantMember;
        dbus_message_iter_recurse(&Variant, &VariantMember);
        memset(AccessPoint.Name, 0x00, sizeof(AccessPoint.Name));
        uint32_t i = 0;
        do {
          if(dbus_message_iter_get_arg_type(&VariantMember) != DBUS_TYPE_BYTE) return false;
          dbus_message_iter_get_basic(&VariantMember, &AccessPoint.Name[i]);
          i++;
        } while(dbus_message_iter_next(&VariantMember) && i < sizeof(AccessPoint.Name));
        break;
      }
      if(!strcmp(Key, "HwAddress")) {
        char *hwAddress;
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_STRING) return false;
        dbus_message_iter_get_basic(&Variant, &hwAddress);
        if(!hwAddress) return false;
        strcpy(AccessPoint.MAC, hwAddress);
        break;
      }
      if(!strcmp(Key, "WpaFlags")) {
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_UINT32) return false;
        dbus_message_iter_get_basic(&Variant, &AccessPoint.WpaFlags);
        break;
      }
      if(!strcmp(Key, "Mode")) {
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_UINT32) return false;
        dbus_message_iter_get_basic(&Variant, &AccessPoint.Mode);
        break;
      }
      if(!strcmp(Key, "Frequency")) {
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_UINT32) return false;
        dbus_message_iter_get_basic(&Variant, &AccessPoint.Freq);
        break;
      }
      if(!strcmp(Key, "Strength")) {
        if(dbus_message_iter_get_arg_type(&Variant) != DBUS_TYPE_BYTE) return false;
        dbus_message_iter_get_basic(&Variant, &AccessPoint.Power);
        break;
      }
    } while (dbus_message_iter_next(&Dict));
  } while(dbus_message_iter_next(&IterB));
  
  printf("SSID       : '%s'\n", AccessPoint.Name);
  printf("  BSID     : '%s'\n", AccessPoint.MAC);
  printf("  WPAMode  : 0x%08X\n", AccessPoint.WpaFlags);
  printf("  Mode     : 0x%08X\n", AccessPoint.Mode);
  printf("  Frequency: %dMHz\n", AccessPoint.Freq);
  printf("  Signal   : %d%%\n", AccessPoint.Power);
  return true; 
}