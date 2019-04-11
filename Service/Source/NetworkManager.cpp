/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tools.h"
#include "NetworkManager.h"
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
//----------------------------------------------------------------------------------------------------------------------
pthread_mutex_t NetworkManager::pMutex = PTHREAD_MUTEX_INITIALIZER;
const uint32_t  NetworkManager::NM_DEVICE_TYPE_WIFI = 2;
//----------------------------------------------------------------------------------------------------------------------
void NetworkManager::GetNetworksCallback(HTTPRequest &req) {
  DBusConnection *DBus = CreateConnection();
  if(DBus == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);  
  json Response;
  SendGetDevices(DBus, Response);
  SendGetActiveConnections(DBus, Response);
  SendGetSettingsPaths(DBus, Response);
//  dbus_connection_unref(DBus);
//  *postdata = (char*)json_object_to_json_string_length(*writer, JSON_C_TO_STRING_PLAIN, resultLen);  
  return req.FromJSON(Response);
}
DBusConnection *NetworkManager::CreateConnection() {
  DBusError Error;
  dbus_error_init(&Error);
  DBusConnection *DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    LOGFile::Instance()->Error("Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
    return NULL;
  }
  return DBus;
}
bool NetworkManager::SetJSONNetworkConfig(json_object *json) {
//  DBusConnection *DBus = CreateConnection();
//  if(DBus == NULL) return false;  
//  if(SendSaveSettings(DBus, json)) {
//    Reset();
//    dbus_connection_unref(DBus);
//    return true;
//  } else {
//    dbus_connection_unref(DBus);
    return false;
//  }
}
void NetworkManager::Reset() {
  DBusConnection *DBus = CreateConnection();
  if(DBus == NULL) return; 
  pthread_mutex_lock(&pMutex);
  SendEnable(DBus, 0);
  sleep(1);
  SendEnable(DBus, 1);
  sleep(1);
  pthread_mutex_unlock(&pMutex);
  dbus_connection_unref(DBus);
}
uint8_t NetworkManager::GetConnectivityStatus() {
  DBusConnection *DBus = CreateConnection();
  if(DBus == NULL) return 0;
  uint8_t Result = SendConnectivityState(DBus);
  dbus_connection_unref(DBus);
  return Result;
}
void NetworkManager::SendEnable(DBusConnection *dbus, int state) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "Enable");
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_BOOLEAN, &state);
  dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
}
uint8_t NetworkManager::SendConnectivityState(DBusConnection *dbus) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "CheckConnectivity");
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  uint32_t Result = 0;
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_UINT32)) {
//      DBusMessageIter Value;
//      dbus_message_iter_recurse(&Struct, &Value);
      dbus_message_iter_get_basic(&Struct, &Result);
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  return Result;
}
void NetworkManager::SendGetDevices(DBusConnection *dbus, json &resp) {
  json Response = json::array();
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDevices");
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
          char *DevicePath;
          dbus_message_iter_get_basic(&Array, &DevicePath);
          if(DevicePath) SendGetDeviceType(dbus, Response, DevicePath);
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  resp["AccessPoints"] = Response;
}
void NetworkManager::SendGetDeviceType(DBusConnection *dbus, json &json, char *devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.DBus.Properties", "Get");
  const char *ParamInterface = "org.freedesktop.NetworkManager.Device";
  const char *ParamDeviceType = "DeviceType";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
      DBusMessageIter VariantIter;
      dbus_message_iter_recurse(&Struct, &VariantIter);
      if(dbus_message_iter_get_arg_type(&VariantIter) == DBUS_TYPE_UINT32) {
        uint32_t DeviceType;
        dbus_message_iter_get_basic(&VariantIter, &DeviceType);
        if(DeviceType == NM_DEVICE_TYPE_WIFI) {
          SendRequestScan(dbus, devicePath);
          SendGetAccessPoints(dbus, json, devicePath);
        }
      }
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
void NetworkManager::SendRequestScan(DBusConnection *dbus, char* devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.NetworkManager.Device.Wireless", "RequestScan");
  DBusMessageIter ArgsIter, Array;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_ARRAY, "{sv}", &Array);
  dbus_message_iter_close_container(&ArgsIter, &Array);
  dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  sleep(5);
}
void NetworkManager::SendGetAccessPoints(DBusConnection *dbus, json &resp, char* devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.NetworkManager.Device.Wireless", "GetAccessPoints");
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
          char *Path;
          dbus_message_iter_get_basic(&Array, &Path);
          if(Path) {
            json AccessPoint;
            if(SendGetAPProperties(dbus, AccessPoint, Path)) resp.push_back(AccessPoint);
          }
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
bool NetworkManager::SendGetAPProperties(DBusConnection *dbus, json &resp, char *path) {
  bool IsAP = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *Param = "org.freedesktop.NetworkManager.AccessPoint";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Param);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
          DBusMessageIter Dict;
          dbus_message_iter_recurse(&Array, &Dict);
          do {
            if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
              const char *Key;
              dbus_message_iter_get_basic(&Dict, &Key);
              dbus_message_iter_next(&Dict);
              if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
                DBusMessageIter Variant;
                dbus_message_iter_recurse(&Dict, &Variant);
                if((strcmp(Key, "Ssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                  DBusMessageIter Array;
                  dbus_message_iter_recurse(&Variant, &Array);
                  char Name[33] = {0};
                  uint32_t i = 0;
                  do {
                    if (dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_BYTE) dbus_message_iter_get_basic(&Array, &Name[i++]);
                  } while (dbus_message_iter_next(&Array) && (i < sizeof (Name)));
                  if(i) resp["SSID"] = Name;
                  break;
                }
                if((strcmp(Key, "HwAddress") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                  char *hwAddress;
                  dbus_message_iter_get_basic(&Variant, &hwAddress);
                  if (hwAddress) resp["MAC"] = hwAddress;
                  break;
                }
                if((strcmp(Key, "RsnFlags") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                  uint32_t Flags;
                  dbus_message_iter_get_basic(&Variant, &Flags);
                  if(Flags == 0) resp["Mode"] = "Open";
                  if((Flags & 0x00000033) != 0) resp["Mode"] = "WEP";
                  if(((Flags & 0x00000044) != 0) || ((Flags & 0x00000088) != 0)) resp["Mode"] = "WPA";
                  if(Flags & 0x00000200) IsAP = false;
                  break;
                }
                if((strcmp(Key, "Mode") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                  uint32_t Mode;
                  dbus_message_iter_get_basic(&Variant, &Mode);
                  if(Mode != 2) IsAP = false;
                  break;
                }
                if((strcmp(Key, "Frequency") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                  uint32_t Freq;
                  dbus_message_iter_get_basic(&Variant, &Freq);
                  resp["Freq"] = Freq;
                  break;
                }
                if((strcmp(Key, "Strength") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_BYTE)) {
                  uint8_t Power;
                  dbus_message_iter_get_basic(&Variant, &Power);
                  resp["Power"] = Power;
                  break;
                }
              }
            }
          } while (dbus_message_iter_next(&Dict));
        }
      } while (dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  return IsAP;
}
void NetworkManager::SendGetActiveConnections(DBusConnection *dbus, json &resp) {
  json Response = json::array();
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.DBus.Properties", "Get");
  const char *ParamInterface = "org.freedesktop.NetworkManager";
  const char *ParamDeviceType = "ActiveConnections";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct, Variant;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
      dbus_message_iter_recurse(&Struct, &Variant);
      if(dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY) {
        DBusMessageIter Array;
        dbus_message_iter_recurse(&Variant, &Array);
        do {
          if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
            char *DevicePath;        
            dbus_message_iter_get_basic(&Array, &DevicePath);
            if(DevicePath) {
              json Connection;
              if(SendGetConnectionProperties(dbus, Connection, DevicePath)) Response.push_back(Connection);
            }    
          }
        } while(dbus_message_iter_next(&Array));
      }
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  resp["ActiveConnections"] = Response;
}
bool NetworkManager::SendGetConnectionProperties(DBusConnection *dbus, json &resp, char *path) {
  bool IsWifi = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *ParamInterface = "org.freedesktop.NetworkManager.Connection.Active";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
          DBusMessageIter Dict;
          dbus_message_iter_recurse(&Array, &Dict);
          do {
            if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
              const char *Key;
              dbus_message_iter_get_basic(&Dict, &Key);
              dbus_message_iter_next(&Dict);
              if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
                DBusMessageIter Variant;
                dbus_message_iter_recurse(&Dict, &Variant);
                if((strcmp(Key, "Type") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                  char *Type;
                  dbus_message_iter_get_basic(&Variant, &Type);
                  if(Type && (strcmp(Type, "802-11-wireless") != 0)) IsWifi = false;
                  break;
                }
                if((strcmp(Key, "SpecificObject") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                  char *AccessPoint;
                  dbus_message_iter_get_basic(&Variant, &AccessPoint);
                  if(AccessPoint && (SendGetAPProperties(dbus, resp, AccessPoint) == false)) IsWifi = false;
                  break;
                }
                if((strcmp(Key, "Ip4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                  char *IP4Config;
                  dbus_message_iter_get_basic(&Variant, &IP4Config);
                  if(IP4Config) SendGetIP4Properties(dbus, resp, IP4Config);
                  break;
                }
                if((strcmp(Key, "Dhcp4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                  char *DHCP4Config;
                  dbus_message_iter_get_basic(&Variant, &DHCP4Config);
                  if(DHCP4Config && (strcmp(DHCP4Config, "/") == 0)) {
                    resp["DHCP"] = false;
                  } else {
                    resp["DHCP"] = true;
                  }
                  break;
                }
              }
            }
          } while (dbus_message_iter_next(&Dict));
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  return IsWifi;
}
void NetworkManager::SendGetIP4Properties(DBusConnection *dbus, json &resp, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *ParamInterface = "org.freedesktop.NetworkManager.IP4Config";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
          DBusMessageIter Dict;
          dbus_message_iter_recurse(&Array, &Dict);
          do {
            if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
              const char *Key;
              dbus_message_iter_get_basic(&Dict, &Key);
              dbus_message_iter_next(&Dict);
              if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT)) {
                DBusMessageIter Variant;
                dbus_message_iter_recurse(&Dict, &Variant);
                if((strcmp(Key, "Nameservers") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                  DBusMessageIter VariantMember;
                  dbus_message_iter_recurse(&Variant, &VariantMember);
                  uint32_t i = 0;
                  do {
                    if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_UINT32) {
                      struct in_addr DNS;
                      dbus_message_iter_get_basic(&VariantMember, &DNS);
                      char DNSName[16];
                      snprintf(DNSName, sizeof(DNSName), "DNS%d", i++);
                      resp[DNSName] = inet_ntoa(DNS);
                    }
                  } while(dbus_message_iter_next(&VariantMember));
                  break;
                }
                if((strcmp(Key, "Addresses") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                  DBusMessageIter Array;
                  dbus_message_iter_recurse(&Variant, &Array);
                  if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_ARRAY) {
                    DBusMessageIter SubArray;
                    dbus_message_iter_recurse(&Array, &SubArray);
                    if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                      struct in_addr IP;
                      dbus_message_iter_get_basic(&SubArray, &IP);
                      resp["IP"] = inet_ntoa(IP);
                    }
                    dbus_message_iter_next(&SubArray);
                    if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                      uint32_t Mask;
                      dbus_message_iter_get_basic(&SubArray, &Mask);
                      Mask = 0xFFFFFFFF ^ ((1 << (32 - Mask)) - 1);
                      char IPMask[16];
                      snprintf(IPMask, sizeof(IPMask), "%d.%d.%d.%d", (Mask >> 24) & 0xFF, (Mask >> 16) & 0xFF, (Mask >> 8) & 0xFF, Mask & 0xFF);
                      resp["Mask"] = IPMask;
                    }
                    dbus_message_iter_next(&SubArray);
                    if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                      struct in_addr Gateway;
                      dbus_message_iter_get_basic(&SubArray, &Gateway);
                      resp["Gateway"] = inet_ntoa(Gateway);
                    }
                  }
                  break;
                }
              }
            }
          } while (dbus_message_iter_next(&Dict));
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
void NetworkManager::SendGetSettingsPaths(DBusConnection *dbus, json &resp) {
  json Response = json::array();
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", "ListConnections");
  if(Msg) {
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) {
      DBusMessageIter Struct;
      if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
        DBusMessageIter Array;
        dbus_message_iter_recurse(&Struct, &Array);
        do {
          if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
            char *Path;
            dbus_message_iter_get_basic(&Array, &Path);
            if(Path) {
              json Setting;
              if(SendGetSettings(dbus, Setting, Path)) Response.push_back(Setting);
            }
          }
        } while(dbus_message_iter_next(&Array));
      }
      dbus_message_unref(Reply);
    }
    dbus_message_unref(Msg);
  }
  resp["Settings"] = Response;
}
bool NetworkManager::SendGetSettings(DBusConnection *dbus, json &resp, char *path) {
  bool IsWifi = false, ValidAuth = true, IsAP = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
  if(Msg) {
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) {
      resp["ConfigPath"] = path;
      DBusMessageIter Struct;
      if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
        DBusMessageIter Array;
        dbus_message_iter_recurse(&Struct, &Array);
        do {
          if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
            DBusMessageIter Dict;
            dbus_message_iter_recurse(&Array, &Dict);
            do {
              if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
                const char *Key;
                dbus_message_iter_get_basic(&Dict, &Key);
                dbus_message_iter_next(&Dict);
                if(Key && (dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_ARRAY)) {
                  DBusMessageIter Array;
                  dbus_message_iter_recurse(&Dict, &Array);
                  if(strcmp(Key, "802-11-wireless") == 0) {
                    IsWifi = true;
                    do {
                      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                        DBusMessageIter Dict;
                        dbus_message_iter_recurse(&Array, &Dict);
                        do {
                          if(dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_STRING) {
                            const char *Key;
                            dbus_message_iter_get_basic(&Dict, &Key);
                            dbus_message_iter_next(&Dict);
                            if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                              DBusMessageIter Variant;
                              dbus_message_iter_recurse(&Dict, &Variant);
                              if((strcmp(Key, "mode") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                                char *Mode;
                                dbus_message_iter_get_basic(&Variant, &Mode);
                                if((Mode) && (strcmp(Mode, "infrastructure") != 0)) IsAP = false;
                                break;
                              }
                              if((strcmp(Key, "ssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                                DBusMessageIter VariantMember;
                                dbus_message_iter_recurse(&Variant, &VariantMember);
                                char Name[32] = { 0 };
                                uint32_t i = 0;
                                do {
                                  if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_BYTE) dbus_message_iter_get_basic(&VariantMember, &Name[i++]);
                                } while(dbus_message_iter_next(&VariantMember) && (i < sizeof(Name)));
                                if(i) resp["SSID"] = Name;
                                break;
                              }
                              if((strcmp(Key, "bssid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                                DBusMessageIter VariantMember;
                                dbus_message_iter_recurse(&Variant, &VariantMember);
                                char MAC[32];
                                uint32_t i = 0;
                                do {
                                  if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_BYTE) {
                                    uint8_t Byte;
                                    dbus_message_iter_get_basic(&VariantMember, &Byte);
                                    sprintf(&MAC[(i++) * 3], "%02X:", Byte);
                                  }
                                } while(dbus_message_iter_next(&VariantMember) && (i < 7));
                                MAC[17] = 0;
                                resp["MAC"] = MAC;
                                break;
                              }
                            }
                          }
                        } while (dbus_message_iter_next(&Dict));
                      }
                    } while (dbus_message_iter_next(&Array));
                    break;
                  }
                  if(!strcmp(Key, "connection")) {
                    do {
                      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                        DBusMessageIter Dict;
                        dbus_message_iter_recurse(&Array, &Dict);
                        do {
                          const char *Key;
                          dbus_message_iter_get_basic(&Dict, &Key);
                          dbus_message_iter_next(&Dict);
                          if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                            DBusMessageIter Variant;
                            dbus_message_iter_recurse(&Dict, &Variant);
                            if((strcmp(Key, "uuid") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                              char *UUID;
                              dbus_message_iter_get_basic(&Variant, &UUID);
                              resp["UUID"] = UUID;
                              break;
                            }
                          }
                        } while (dbus_message_iter_next(&Dict));
                      }
                    } while (dbus_message_iter_next(&Array));
                    break;
                  }                  
                  if(!strcmp(Key, "802-11-wireless-security")) {
                    do {
                      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                        DBusMessageIter Dict;
                        dbus_message_iter_recurse(&Array, &Dict);
                        do {
                          const char *Key;
                          dbus_message_iter_get_basic(&Dict, &Key);
                          dbus_message_iter_next(&Dict);
                          if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                            DBusMessageIter Variant;
                            dbus_message_iter_recurse(&Dict, &Variant);
                            if((strcmp(Key, "key-mgmt") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                              char *Key;
                              dbus_message_iter_get_basic(&Variant, &Key);
                              if(Key) {
                                if(strcmp(Key, "none") == 0)
                                  resp["Mode"] = "WEP";
                                else if(strcmp(Key, "wpa-psk") == 0)
                                  resp["Mode"] = "WPA";
                                else
                                  ValidAuth = false;
                              }
                              break;
                            }
                          }
                        } while (dbus_message_iter_next(&Dict));
                      }
                    } while (dbus_message_iter_next(&Array));
                    break;
                  }
                  if(!strcmp(Key, "ipv4")) {
                    do {
                      if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_DICT_ENTRY) {
                        DBusMessageIter Dict;
                        dbus_message_iter_recurse(&Array, &Dict);
                        do {
                          const char *Key;
                          dbus_message_iter_get_basic(&Dict, &Key);
                          dbus_message_iter_next(&Dict);
                          if(Key && dbus_message_iter_get_arg_type(&Dict) == DBUS_TYPE_VARIANT) {
                            DBusMessageIter Variant;
                            dbus_message_iter_recurse(&Dict, &Variant);
                            if((strcmp(Key, "method") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                              char *DHCP;
                              dbus_message_iter_get_basic(&Variant, &DHCP);
                              if(DHCP) resp["DHCP"] = (strcmp(DHCP, "auto") == 0);
                              break;
                            }
                            if((strcmp(Key, "dns") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                              DBusMessageIter VariantMember;
                              dbus_message_iter_recurse(&Variant, &VariantMember);
                              uint32_t i = 0;
                              do {
                                if(dbus_message_iter_get_arg_type(&VariantMember) == DBUS_TYPE_UINT32) {
                                  struct in_addr DNS;
                                  dbus_message_iter_get_basic(&VariantMember, &DNS);
                                  char DNSName[16];
                                  sprintf(DNSName, "DNS%d", i++);
                                  resp[DNSName] = inet_ntoa(DNS);
                                }
                              } while(dbus_message_iter_next(&VariantMember));
                              break;
                            }
                            if((strcmp(Key, "addresses") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_ARRAY)) {
                              DBusMessageIter Array;
                              dbus_message_iter_recurse(&Variant, &Array);
                              if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_ARRAY) {
                                DBusMessageIter SubArray;
                                dbus_message_iter_recurse(&Array, &SubArray);
                                if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                                  struct in_addr IP;
                                  dbus_message_iter_get_basic(&SubArray, &IP);
                                  resp["IP"] = inet_ntoa(IP);
                                }
                                dbus_message_iter_next(&SubArray);
                                if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                                  uint32_t Mask;
                                  dbus_message_iter_get_basic(&SubArray, &Mask);
                                  Mask = 0xFFFFFFFF ^ ((1 << (32 - Mask)) - 1);
                                  char IPMask[16];
                                  snprintf(IPMask, sizeof(IPMask), "%d.%d.%d.%d", (Mask >> 24) & 0xFF, (Mask >> 16) & 0xFF, (Mask >> 8) & 0xFF, Mask & 0xFF);
                                  resp["Mask"] = IPMask;
                                }
                                dbus_message_iter_next(&SubArray);
                                if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                                  struct in_addr Gateway;
                                  dbus_message_iter_get_basic(&SubArray, &Gateway);
                                  resp["Gateway"] = inet_ntoa(Gateway);
                                }
                              }
                              break;
                            }
                          }
                        } while (dbus_message_iter_next(&Dict));
                      }
                    } while (dbus_message_iter_next(&Array));
                    break;
                  }
                }
              }
            } while (dbus_message_iter_next(&Dict));
          }
        } while(dbus_message_iter_next(&Array));
      }
      dbus_message_unref(Reply);
    }
    dbus_message_unref(Msg);
  }
  return (IsWifi && ValidAuth && IsAP);
}
//bool NetworkManager::SendSaveSettings(DBusConnection *dbus, json_object *json) {
//  char     *SSID  = NULL;
//  char     *BSSID = NULL;
//  bool      DHCP = false;
//  char     *IP = NULL;
//  char     *DNS0 = NULL;
//  char     *DNS1 = NULL;
//  char     *Gateway = NULL;
//  char     *Mode = NULL;
//  char     *Password = NULL;
//  char     *ConfigPath = NULL;
//  char     *UUID = NULL;
//  json_object_object_foreach(json, JSON_Key, JSON_Val) {
//    enum json_type JSON_Type = json_object_get_type(JSON_Val);
//    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "SSID") == 0))
//      SSID = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "MAC") == 0))      
//      BSSID = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "DHCP") == 0))
//      DHCP = json_object_get_boolean(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "IP") == 0))
//      IP = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DNS0") == 0))
//      DNS0 = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DNS1") == 0))
//      DNS1 = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Gateway") == 0))
//      Gateway = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Mode") == 0))
//      Mode = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0))
//      Password = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "ConfigPath") == 0))
//      ConfigPath = (char*)json_object_get_string(JSON_Val);
//    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "UUID") == 0))
//      UUID = (char*)json_object_get_string(JSON_Val);
//    else
//      return false;
//  }
//  if((ConfigPath != NULL) && (UUID == NULL)) {
//    DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", ConfigPath, "org.freedesktop.NetworkManager.Settings.Connection", "Delete");  
//    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
//    return (Reply != NULL);
//  } else {
//  if((SSID == NULL) || (strlen(SSID) > 32)) return false;
//    int MAC[6];
//    if(BSSID != NULL) {
//      int Bytes = sscanf(BSSID, "%02x:%02x:%02x:%02x:%02x:%02x", &MAC[0], &MAC[1], &MAC[2], &MAC[3], &MAC[4], &MAC[5]);
//      if((Bytes != 6) || (strlen(BSSID) != 17)) return false;
//    }
//    if((Mode == NULL) || ((strcmp("Open", Mode) != 0) && (strcmp("WPA", Mode) != 0) && (strcmp("WEP", Mode) != 0))) return false;
//    if((strcmp("Open", Mode) == 0) && (Password != NULL)) return false;
//    uint32_t bDNS0, bDNS1, bGateway, bIP, bMask = 24, Reconnect = 0;
//    if((DHCP == false) && ((IP == NULL) || (DNS0 == NULL) || (Gateway == NULL))) return false;
//    if((IP != NULL) && (inet_aton(IP, (struct in_addr *)&bIP) == 0)) return false;
//    if((Gateway != NULL) && (inet_aton(Gateway, (struct in_addr *)&bGateway) == 0)) return false;
//    if((DNS0 != NULL) && (inet_aton(DNS0, (struct in_addr *)&bDNS0) == 0)) return false;
//    if((DNS1 != NULL) && (inet_aton(DNS1, (struct in_addr *)&bDNS1) == 0)) return false;
//    const char *Strings[] = { "Setup by openZmeter", "connection", "id", "type", "802-11-wireless", "ssid", "bssid", "ipv4", "method", "auto", "manual", "dns", "addresses", "802-11-wireless-security", "key-mgmt", "none", "wpa-psk", "wep-key0", "psk", "uuid", "autoconnect-retries" };
//    DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", (ConfigPath == NULL) ? "/org/freedesktop/NetworkManager/Settings" : ConfigPath, (ConfigPath == NULL) ? "org.freedesktop.NetworkManager.Settings" : "org.freedesktop.NetworkManager.Settings.Connection", (ConfigPath == NULL) ? "AddConnection" : "Update");
//    DBusMessageIter ArgsIter, Array, Dict, SubArray, SubDict, Variant, VariantArray, VariantSubArray;
//    dbus_message_iter_init_append(Msg, &ArgsIter);
//    dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_ARRAY, "{sa{sv}}", &Array);
//    //connection
//    dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
//    dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[1]);
//    dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
//    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[2]);
//    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[0]);
//    dbus_message_iter_close_container(&SubDict, &Variant);
//    dbus_message_iter_close_container(&SubArray, &SubDict);
//    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[3]);
//    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[4]);
//    dbus_message_iter_close_container(&SubDict, &Variant);
//    dbus_message_iter_close_container(&SubArray, &SubDict);
//    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[20]);
//    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_INT32_AS_STRING, &Variant);
//    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_INT32, &Reconnect);
//    dbus_message_iter_close_container(&SubDict, &Variant);
//    dbus_message_iter_close_container(&SubArray, &SubDict);
//    if(UUID != NULL) {
//      dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//      dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[19]);
//      dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//      dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &UUID);
//      dbus_message_iter_close_container(&SubDict, &Variant);
//      dbus_message_iter_close_container(&SubArray, &SubDict);
//    }  
//    dbus_message_iter_close_container(&Dict, &SubArray);
//    dbus_message_iter_close_container(&Array, &Dict);
//    //802-11-wireless
//    dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
//    dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[4]);
//    dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
//    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[5]);
//    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "ay", &Variant);
//    dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &VariantArray);
//    for(uint i = 0; i < strlen(SSID); i++) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_BYTE, &SSID[i]);
//    dbus_message_iter_close_container(&Variant, &VariantArray);
//    dbus_message_iter_close_container(&SubDict, &Variant);
//    dbus_message_iter_close_container(&SubArray, &SubDict);
//    if(BSSID) {
//      dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//      dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[6]);
//      dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "ay", &Variant);
//      dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &VariantArray);
//      for(int i = 0; i < 6; i++) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_BYTE, &MAC[i]);
//      dbus_message_iter_close_container(&Variant, &VariantArray);
//      dbus_message_iter_close_container(&SubDict, &Variant);
//      dbus_message_iter_close_container(&SubArray, &SubDict);
//    }
//    dbus_message_iter_close_container(&Dict, &SubArray);
//    dbus_message_iter_close_container(&Array, &Dict);
//    //ipv4
//    dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
//    dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[7]);
//    dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
//    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[8]);
//    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[DHCP ? 9 : 10]);
//    dbus_message_iter_close_container(&SubDict, &Variant);
//    dbus_message_iter_close_container(&SubArray, &SubDict);
//    if(!DHCP) {
//      dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//      dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[11]);
//      dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "au", &Variant);
//      dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantArray);
//      if(DNS0) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_UINT32, &bDNS0);
//      if(DNS1) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_UINT32, &bDNS1);
//      dbus_message_iter_close_container(&Variant, &VariantArray);
//      dbus_message_iter_close_container(&SubDict, &Variant);
//      dbus_message_iter_close_container(&SubArray, &SubDict);
//      dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//      dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[12]);
//      dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "aau", &Variant);
//      dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantArray);
//      dbus_message_iter_open_container(&VariantArray, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantSubArray);
//      dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &bIP);
//      dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &bMask);
//      dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &bGateway);
//      dbus_message_iter_close_container(&VariantArray, &VariantSubArray);
//      dbus_message_iter_close_container(&Variant, &VariantArray);
//      dbus_message_iter_close_container(&SubDict, &Variant);
//      dbus_message_iter_close_container(&SubArray, &SubDict);
//    }
//    dbus_message_iter_close_container(&Dict, &SubArray);
//    dbus_message_iter_close_container(&Array, &Dict);
//    //802-11-wireless-security
//    if(strcmp("Open", Mode) != 0) {
//      dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
//      dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[13]);
//      dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
//
//        dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//        dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[14]);
//        dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//        dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[(strcmp("WEP", Mode) == 0) ? 15 : 16]);
//        dbus_message_iter_close_container(&SubDict, &Variant);
//        dbus_message_iter_close_container(&SubArray, &SubDict);
//      if(Password != NULL) {
//        dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
//        dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[(strcmp("WEP", Mode) == 0) ? 17 : 18]);
//        dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
//        dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Password);
//        dbus_message_iter_close_container(&SubDict, &Variant);
//        dbus_message_iter_close_container(&SubArray, &SubDict);
//      }
//      dbus_message_iter_close_container(&Dict, &SubArray);
//      dbus_message_iter_close_container(&Array, &Dict);
//    }
//    dbus_message_iter_close_container(&ArgsIter, &Array);
//    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(dbus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
//    return (Reply != NULL);
//  }
//}