/* ================================== zMeter ===========================================================================
 * File:     NetworkManager.cpp
 * Author:   Eduardo Viciana (12 de abril de 2017)
 * -------------------------------------------------------------------------------------------------------------------*/
#include "Tools.h"
#include "NetworkManager.h"
#include <string.h>
#include <arpa/inet.h>
//----------------------------------------------------------------------------------------------------------------------
const uint32_t NetworkManager::NM_DEVICE_TYPE_WIFI = 2;
//----------------------------------------------------------------------------------------------------------------------
NetworkManager::NetworkManager() {
  DBusError Error;
  dbus_error_init(&Error);
  DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    Tools::LogMsg("ERROR", "Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
  }
}
NetworkManager::~NetworkManager() {
  dbus_connection_unref(DBus);
}
bool NetworkManager::SetJSONNetworkConfig(json_object *json) {
  NetworkManager NM;
  if(NM.DBus == NULL) return NULL;  

  json_object *Result = json_object_new_object();
  NM.SendAddSettings(json);
  return NULL;
  //DBusConnection *DBus = GetDBUSConnection();
  //if(DBus == NULL) return NULL;
//  return Result;
}
json_object *NetworkManager::GetJSONNetworks() {
  NetworkManager NM;
  if(NM.DBus == NULL) return NULL;  
  json_object *Result = json_object_new_object();
  json_object *AccessPoints = json_object_new_array();
  NM.SendGetDevices(AccessPoints);
  json_object_object_add(Result, "AccessPoints", AccessPoints);
  json_object *ActiveConnections = json_object_new_array();
  NM.SendGetActiveConnections(ActiveConnections);
  json_object_object_add(Result, "ActiveConnections", ActiveConnections);
  json_object *Settings = json_object_new_array();
  NM.SendGetSettingsPaths(Settings);
  json_object_object_add(Result, "Settings", Settings);
  size_t Size;
  char *Tmp = (char*)json_object_to_json_string_length(Result, JSON_C_TO_STRING_PRETTY, &Size);
  printf("%s\n", Tmp);
  return Result;
}
void NetworkManager::SendGetDevices(json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", "GetDevices");
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_ARRAY)) {
      DBusMessageIter Array;
      dbus_message_iter_recurse(&Struct, &Array);
      do {
        if(dbus_message_iter_get_arg_type(&Array) == DBUS_TYPE_OBJECT_PATH) {
          char *DevicePath;
          dbus_message_iter_get_basic(&Array, &DevicePath);
          if(DevicePath) SendGetDeviceType(json, DevicePath);
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
void NetworkManager::SendGetDeviceType(json_object *json, char *devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.DBus.Properties", "Get");
  const char *ParamInterface = "org.freedesktop.NetworkManager.Device";
  const char *ParamDeviceType = "DeviceType";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
      DBusMessageIter VariantIter;
      dbus_message_iter_recurse(&Struct, &VariantIter);
      if(dbus_message_iter_get_arg_type(&VariantIter) == DBUS_TYPE_UINT32) {
        uint32_t DeviceType;
        dbus_message_iter_get_basic(&VariantIter, &DeviceType);
        if(DeviceType == NM_DEVICE_TYPE_WIFI) SendGetAccessPoints(json, devicePath);
      }
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
void NetworkManager::SendGetAccessPoints(json_object *json, char* devicePath) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", devicePath, "org.freedesktop.NetworkManager.Device.Wireless", "GetAccessPoints");
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
            json_object *AccessPoint = json_object_new_object();
            if(SendGetAPProperties(AccessPoint, Path)) {
              json_object_array_add(json, AccessPoint);
            } else {
              json_object_put(AccessPoint);
            }
          }
        }
      } while(dbus_message_iter_next(&Array));
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
bool NetworkManager::SendGetAPProperties(json_object *json, char *path) {
  bool IsAP = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *Param = "org.freedesktop.NetworkManager.AccessPoint";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Param);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
                  if(i) json_object_object_add(json, "SSID", json_object_new_string(Name));
                  break;
                }
                if((strcmp(Key, "HwAddress") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_STRING)) {
                  char *hwAddress;
                  dbus_message_iter_get_basic(&Variant, &hwAddress);
                  if (hwAddress) json_object_object_add(json, "MAC", json_object_new_string(hwAddress));
                  break;
                }
                if((strcmp(Key, "WpaFlags") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_UINT32)) {
                  uint32_t Flags;
                  dbus_message_iter_get_basic(&Variant, &Flags);
                  if(Flags == 0) json_object_object_add(json, "Mode", json_object_new_string("Open"));
                  if((Flags & 0x00000033) != 0) json_object_object_add(json, "Mode", json_object_new_string("WEP"));
                  if(((Flags & 0x00000044) != 0) || ((Flags & 0x00000088) != 0)) json_object_object_add(json, "Mode", json_object_new_string("WPA"));
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
                  json_object_object_add(json, "Freq", json_object_new_int(Freq));
                  break;
                }
                if((strcmp(Key, "Strength") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_BYTE)) {
                  uint8_t Power;
                  dbus_message_iter_get_basic(&Variant, &Power);
                  json_object_object_add(json, "Power", json_object_new_int(Power));
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
void NetworkManager::SendGetActiveConnections(json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.DBus.Properties", "Get");
  const char *ParamInterface = "org.freedesktop.NetworkManager";
  const char *ParamDeviceType = "ActiveConnections";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamDeviceType);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
              json_object *Connection = json_object_new_object();  
              if(SendGetConnectionProperties(Connection, DevicePath)) {
                json_object_array_add(json, Connection);      
              } else {
                json_object_put(Connection);
              }
            }    
          }
        } while(dbus_message_iter_next(&Array));
      }
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
}
bool NetworkManager::SendGetConnectionProperties(json_object *json, char *path) {
  bool IsWifi = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *ParamInterface = "org.freedesktop.NetworkManager.Connection.Active";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
                  if(AccessPoint && (SendGetAPProperties(json, AccessPoint) == false)) IsWifi = false;
                  break;
                }
                if((strcmp(Key, "Ip4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                  char *IP4Config;
                  dbus_message_iter_get_basic(&Variant, &IP4Config);
                  if(IP4Config) SendGetIP4Properties(json, IP4Config);
                  break;
                }
                if((strcmp(Key, "Dhcp4Config") == 0) && (dbus_message_iter_get_arg_type(&Variant) == DBUS_TYPE_OBJECT_PATH)) {
                  char *DHCP4Config;
                  dbus_message_iter_get_basic(&Variant, &DHCP4Config);
                  if(DHCP4Config && (strcmp(DHCP4Config, "/") == 0)) {
                    json_object_object_add(json, "DHCP", json_object_new_boolean(false));
                  } else {
                    json_object_object_add(json, "DHCP", json_object_new_boolean(true));
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
void NetworkManager::SendGetIP4Properties(json_object *json, char *path) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.DBus.Properties", "GetAll");
  const char *ParamInterface = "org.freedesktop.NetworkManager.IP4Config";
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &ParamInterface);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
                      i++;
                      char DNSName[16];
                      sprintf(DNSName, "DNS%d", i);
                      json_object_object_add(json, DNSName, json_object_new_string(inet_ntoa(DNS)));
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
                      json_object_object_add(json, "IP", json_object_new_string(inet_ntoa(IP)));
                    }
                    dbus_message_iter_next(&SubArray);
                    if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                      uint32_t Mask;
                      dbus_message_iter_get_basic(&SubArray, &Mask);
                      json_object_object_add(json, "Mask", json_object_new_int(Mask));
                    }
                    dbus_message_iter_next(&SubArray);
                    if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                      struct in_addr Gateway;
                      dbus_message_iter_get_basic(&SubArray, &Gateway);
                      json_object_object_add(json, "Gateway", json_object_new_string(inet_ntoa(Gateway)));
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
void NetworkManager::SendGetSettingsPaths(json_object *json) {
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", "ListConnections");
  if(Msg) {
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
              json_object *Setting = json_object_new_object();
              if(SendGetSettings(Setting, Path)) {
                json_object_array_add(json, Setting);
              } else {
                json_object_put(Setting);
              }
            }
          }
        } while(dbus_message_iter_next(&Array));
      }
      dbus_message_unref(Reply);
    }
    dbus_message_unref(Msg);
  }
}
bool NetworkManager::SendGetSettings(json_object *json, char *path) {
  bool IsWifi = false, ValidAuth = true;
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", path, "org.freedesktop.NetworkManager.Settings.Connection", "GetSettings");
  if(Msg) {
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) {
      json_object_object_add(json, "ConfigPath", json_object_new_string(path));
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
                                if((Mode) && (strcmp(Mode, "infrastructure") == 0)) IsWifi = true;
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
                                if(i) json_object_object_add(json, "SSID", json_object_new_string(Name));
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
                                json_object_object_add(json, "MAC", json_object_new_string(MAC));
                                break;
                              }
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
                                  json_object_object_add(json, "Mode", json_object_new_string("WEP"));
                                else if(strcmp(Key, "wpa-psk") == 0)
                                  json_object_object_add(json, "Mode", json_object_new_string("WPA"));
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
                              if(DHCP) json_object_object_add(json, "DHCP", json_object_new_boolean(strcmp(DHCP, "auto") == 0));
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
                                  json_object_object_add(json, DNSName, json_object_new_string(inet_ntoa(DNS)));
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
                                  json_object_object_add(json, "IP", json_object_new_string(inet_ntoa(IP)));
                                }
                                dbus_message_iter_next(&SubArray);
                                if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                                  uint32_t Mask;
                                  dbus_message_iter_get_basic(&SubArray, &Mask);
                                  json_object_object_add(json, "Mask", json_object_new_int(Mask));
                                }
                                dbus_message_iter_next(&SubArray);
                                if(dbus_message_iter_get_arg_type(&SubArray) == DBUS_TYPE_UINT32) {
                                  struct in_addr Gateway;
                                  dbus_message_iter_get_basic(&SubArray, &Gateway);
                                  json_object_object_add(json, "Gateway", json_object_new_string(inet_ntoa(Gateway)));
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
  return (IsWifi && ValidAuth);
}
bool NetworkManager::SendAddSettings(json_object *json) {
  char     *SSID  = NULL;
  char     *BSSID = NULL;
  bool      DHCP = false;
  char     *IP = NULL;
  char     *DNS0 = NULL;
  char     *DNS1 = NULL;
  char     *Gateway = NULL;
  char     *Mode = NULL;
  char     *Password = NULL;
  bool      Error = false;
  json_object_object_foreach(json, JSON_Key, JSON_Val) {
    enum json_type JSON_Type = json_object_get_type(JSON_Val);
    if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "SSID") == 0))
      SSID = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "BSSID") == 0))      
      BSSID = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_boolean) && (strcmp(JSON_Key, "DHCP") == 0))
      DHCP = json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "IP") == 0))
      IP = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DNS0") == 0))
      DNS0 = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "DNS1") == 0))
      DNS1 = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Gateway") == 0))
      Gateway = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Mode") == 0))
      Mode = (char*)json_object_get_string(JSON_Val);
    else if((JSON_Type == json_type_string) && (strcmp(JSON_Key, "Password") == 0))
      Password = (char*)json_object_get_string(JSON_Val);
    else 
      Error = true;
  }
  uint8_t MAC[6];
  if(BSSID != NULL) {
    sscanf(macStr, "%x:%x:%x:%x:%x:%x", &Mac[0], &Mac[1], &Mac[2], &Mac[3], &iMac[4], &iMac[5]);
    
    if(strlen(BSSID) != 17) {
      Error = true;
    } else {
      
    }
  }
  if(DHCP == true) {
  
  
  }








  const char *Strings[] = { "Setup by openZmeter", "connection", "id", "type", "802-11-wireless", "ssid", "bssid", "ipv4", "method", "auto", "manual", "dns", "addresses", "802-11-wireless-security", "key-mgmt", "none", "wpa-psk", "wep-key0", "psk" };
  DBusMessage *Msg = dbus_message_new_method_call("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings", "org.freedesktop.NetworkManager.Settings", "AddConnection");
  DBusMessageIter ArgsIter, Array, Dict, SubArray, SubDict, Variant, VariantArray, VariantSubArray;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_ARRAY, "{sa{sv}}", &Array);
  //connection
  dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
  dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[1]);
  dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
  dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
  dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[2]);
  dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
  dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[0]);
  dbus_message_iter_close_container(&SubDict, &Variant);
  dbus_message_iter_close_container(&SubArray, &SubDict);
  dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
  dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[3]);
  dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
  dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[4]);
  dbus_message_iter_close_container(&SubDict, &Variant);
  dbus_message_iter_close_container(&SubArray, &SubDict);
  dbus_message_iter_close_container(&Dict, &SubArray);
  dbus_message_iter_close_container(&Array, &Dict);
  //802-11-wireless
  dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
  dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[4]);
  dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
  dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
  dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[5]);
  dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "ay", &Variant);
  dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &VariantArray);
  for(int i = 0; i < strlen(NetworkSSID); i++) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_BYTE, &NetworkSSID[i]);
  dbus_message_iter_close_container(&Variant, &VariantArray);
  dbus_message_iter_close_container(&SubDict, &Variant);
  dbus_message_iter_close_container(&SubArray, &SubDict);
  if(NetworkBSSID) {
    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[6]);
    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "ay", &Variant);
    dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE_AS_STRING, &VariantArray);
    for(int i = 0; i < 6; i++) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_BYTE, &NetworkBSSID[i]);
    dbus_message_iter_close_container(&Variant, &VariantArray);
    dbus_message_iter_close_container(&SubDict, &Variant);
    dbus_message_iter_close_container(&SubArray, &SubDict);
  }
  dbus_message_iter_close_container(&Dict, &SubArray);
  dbus_message_iter_close_container(&Array, &Dict);
  //ipv4
  dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
  dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[7]);
  dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
  dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
  dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[8]);
  dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
  dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[DHCP ? 9 : 10]);
  dbus_message_iter_close_container(&SubDict, &Variant);
  dbus_message_iter_close_container(&SubArray, &SubDict);
  if(!DHCP) {
    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[11]);
    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "au", &Variant);
    dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantArray);
    for(int i = 0; i < 2; i++) dbus_message_iter_append_basic(&VariantArray, DBUS_TYPE_UINT32, &DNS[i]);
    dbus_message_iter_close_container(&Variant, &VariantArray);
    dbus_message_iter_close_container(&SubDict, &Variant);
    dbus_message_iter_close_container(&SubArray, &SubDict);
    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[12]);
    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, "aau", &Variant);
    dbus_message_iter_open_container(&Variant, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantArray);
    dbus_message_iter_open_container(&VariantArray, DBUS_TYPE_ARRAY, DBUS_TYPE_UINT32_AS_STRING, &VariantSubArray);
    dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &IP);
    dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &Mask);
    dbus_message_iter_append_basic(&VariantSubArray, DBUS_TYPE_UINT32, &Gateway);
    dbus_message_iter_close_container(&VariantArray, &VariantSubArray);
    dbus_message_iter_close_container(&Variant, &VariantArray);
    dbus_message_iter_close_container(&SubDict, &Variant);
    dbus_message_iter_close_container(&SubArray, &SubDict);
  }
  dbus_message_iter_close_container(&Dict, &SubArray);
  dbus_message_iter_close_container(&Array, &Dict);
  //802-11-wireless-security
  dbus_message_iter_open_container(&Array, DBUS_TYPE_DICT_ENTRY, NULL, &Dict);
  dbus_message_iter_append_basic(&Dict, DBUS_TYPE_STRING, &Strings[13]);
  if(strcmp("Open", KeyType) != 0) {
    dbus_message_iter_open_container(&Dict, DBUS_TYPE_ARRAY, "{sv}", &SubArray);
    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[14]);
    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Strings[(strcmp("WEP", KeyType) == 0) ? 15 : 16]);
    dbus_message_iter_close_container(&SubDict, &Variant);
    dbus_message_iter_close_container(&SubArray, &SubDict);
    dbus_message_iter_open_container(&SubArray, DBUS_TYPE_DICT_ENTRY, NULL, &SubDict);
    dbus_message_iter_append_basic(&SubDict, DBUS_TYPE_STRING, &Strings[(strcmp("WEP", KeyType) == 0) ? 17 : 18]);
    dbus_message_iter_open_container(&SubDict, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &Variant);
    dbus_message_iter_append_basic(&Variant, DBUS_TYPE_STRING, &Password);
    dbus_message_iter_close_container(&SubDict, &Variant);
    dbus_message_iter_close_container(&SubArray, &SubDict);
  }
  dbus_message_iter_close_container(&Dict, &SubArray);
  dbus_message_iter_close_container(&Array, &Dict);

  dbus_message_iter_close_container(&ArgsIter, &Array);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
//    json_object *Setting = json_object_new_object();
//    if(RecvGetSettings(Setting, Msg) == false) {
//      json_object_put(Setting);
//    } else {
//      json_object_array_add(json, Setting);
//    }
  }
}