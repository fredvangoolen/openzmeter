// This file is part of openZmeter.
//
// openZmeter is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// openZmeter is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with openZmeter.  If not, see <https://www.gnu.org/licenses/>
// ---------------------------------------------------------------------------------------------------------------------
#include "BluetoothManager.h"
#include "ResourceFile.h"
#include "ConfigFile.h"
#include "LogFile.h"
#include "Worker.h"
#include "MessageLoop.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
volatile bool    BluetoothManager::pTerminate   = false;
bool             BluetoothManager::pEnable      = true;
bool             BluetoothManager::pEnabled     = false;
string           BluetoothManager::pInterface   = "bt";
string           BluetoothManager::pServerName  = "";
pthread_t        BluetoothManager::pThread      = 0;
pthread_mutex_t  BluetoothManager::pMutex       = PTHREAD_MUTEX_INITIALIZER;
#endif
// ---------------------------------------------------------------------------------------------------------------------
BluetoothManager::BluetoothManager() {
  LogFile::Info("Starting BluetoothManager...");
  HTTPServer::RegisterDoc("Bluetooth", "API/Bluetooth/Bluetooth.json");
  HTTPServer::RegisterCallback("getBluetooth", GetBluetoothCallback, "API/Bluetooth/GetBluetooth.json");
  HTTPServer::RegisterCallback("setBluetooth", SetBluetoothCallback, "API/Bluetooth/SetBluetooth.json");
#ifdef OPENWRT
  json Config = ConfigFile::ReadConfig("/Bluetooth");
  if(Config.is_object() == true) {
    if(Config["Enabled"].is_boolean()) pEnable = Config["Enabled"];
    if(Config["Interface"].is_string()) pInterface = Config["Interface"];
  }
  pTerminate = false;
  Config = json::object();
  Config["Enabled"] = pEnable;
  Config["Interface"] = pInterface;
  ConfigFile::WriteConfig("/Bluetooth", Config);
  if(pthread_create(&pThread, NULL, Thread, NULL) != 0) {
    pThread = 0;
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  } else {
    Worker::SetThreadProperties(pThread, "Bluetooth", Worker::LOW);
  }
  MessageLoop::RegisterListener("Bluetooth_Toogle", ToggleEnable);
#endif
  LogFile::Info("BluetoothManager started");
}
BluetoothManager::~BluetoothManager() {
  LogFile::Info("Stopping BluetoothManager...");
  MessageLoop::UnregisterListener("Bluetooth_Toogle");
  HTTPServer::UnregisterCallback("getBluetooth");
  HTTPServer::UnregisterCallback("setBluetooth");
  HTTPServer::UnregisterDoc("Bluetooth");
#ifdef OPENWRT
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pThread = 0;
#endif
  LogFile::Info("BluetoothManager stoped");
}
bool BluetoothManager::Enabled() {
#ifdef OPENWRT
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  bool Result = pEnabled;
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  return Result;
#else
  return false;
#endif
}
void BluetoothManager::GetBluetoothCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
#ifdef OPENWRT
  json Config = ConfigFile::ReadConfig("/Bluetooth");
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  Config["Name"] = pServerName;
  Config["Running"] = pEnabled;
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  req.Success(Config);
#else
  req.Error("", HTTP_METHOD_NOT_ALLOWED);
#endif
}
void BluetoothManager::SetBluetoothCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
#ifdef OPENWRT
  if(!req.CheckParamBool("Enabled")) return;
  json Config = json::object();
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  pEnable = req.Param("Enabled");
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  Config["Enabled"] = pEnable;
  Config["Interface"] = pInterface;
  ConfigFile::WriteConfig("/Bluetooth", Config);
  req.Success(json::object());
#else
  req.Error("", HTTP_METHOD_NOT_ALLOWED);
#endif
}
#ifdef OPENWRT
void BluetoothManager::ToggleEnable() {
  if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  json Config = json::object();
  pEnable = !pEnable;
  Config["Enabled"] = pEnable;
  ConfigFile::WriteConfig("/Bluetooth", Config);
  if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void *BluetoothManager::Thread(void *params) {
  DBusConnection *DBus = NULL;
  while((pTerminate == false) || (pEnabled == true)) {
    bool Enable = (pTerminate == false) ? pEnable : false;
    if(DBus == NULL) {
      DBusError Error;
      dbus_error_init(&Error);
      DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
      if(dbus_error_is_set(&Error)) {
        LogFile::Error("%s:%d -> %s", Error.message);
        dbus_error_free(&Error);
        usleep(1000000);
        continue;
      }
    }
    if(dbus_connection_read_write(DBus, 100) == false) {
      dbus_connection_unref(DBus);
      DBus = NULL;
    }
    DBusMessage *Msg = dbus_connection_pop_message(DBus);
    if(Msg != NULL) {
      if(dbus_message_is_method_call(Msg, "org.bluez.Agent1", "AuthorizeService")) {
        dbus_uint32_t Serial = 0;
        char *Param = (char*)"";
        DBusMessageIter Args;
        dbus_message_iter_init(Msg, &Args);
        dbus_message_iter_get_basic(&Args, &Param);
        LogFile::Info("Authorized BT connection for client: '%s'", Param);
        DBusMessage *Reply = dbus_message_new_method_return(Msg);
        dbus_connection_send(DBus, Reply, &Serial);
        dbus_connection_flush(DBus);
        dbus_message_unref(Reply);
      } else if(dbus_message_is_method_call(Msg, "org.bluez.Agent1", "RequestAuthorization")) {
        dbus_uint32_t Serial = 0;
        char *Param = (char*)"";
        DBusMessageIter Args;
        dbus_message_iter_init(Msg, &Args);
        dbus_message_iter_get_basic(&Args, &Param);
        LogFile::Info("Authorized BT pairing: '%s'", Param);
        DBusMessage *Reply = dbus_message_new_method_return(Msg);
        dbus_connection_send(DBus, Reply, &Serial);
        dbus_connection_flush(DBus);
        dbus_message_unref(Reply);
      }
      dbus_message_unref(Msg);
    } else if((pEnabled == false) && (Enable == true)) {
      DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RegisterAgent");
      DBusMessageIter ArgsIter;
      dbus_message_iter_init_append(Msg, &ArgsIter);
      const char *Path = "/oZm_BT/agent1";
      const char *Capabilities = "NoInputNoOutput";
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Capabilities);
      DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RequestDefaultAgent");
      dbus_message_iter_init_append(Msg, &ArgsIter);
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
      Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      if(SendBoolBT(DBus, "Powered", false) == false) continue;
      string MAC = GetMacBT(DBus);
      if(MAC.empty() == true) continue;
      MAC.erase(std::remove(MAC.begin(), MAC.end(), ':'), MAC.end());
      string NetName = "openZmeter_" + MAC.substr(8);
      if(SendStringBT(DBus, "Alias", NetName.c_str()) == false) continue;
      if(SendBoolBT(DBus, "Powered", true) == false) continue;
      if(SendBoolBT(DBus, "Discoverable", true) == false) continue;
      if(SendUINT32BT(DBus, "DiscoverableTimeout", 0) == false) continue;
      if(SendBoolBT(DBus, "Discoverable", true) == false) continue;
      if(SendUINT32BT(DBus, "DiscoverableTimeout", 0) == false) continue;
      Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.bluez.NetworkServer1", "Register");
      dbus_message_iter_init_append(Msg, &ArgsIter);
      const char *UUID = "nap";
      const char *Bridge = ("br-" + pInterface).c_str();
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Bridge);
      Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      LogFile::Info("BluetoothManager interface UP");
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pServerName = NetName;
      pEnabled = true;
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else if((pEnabled == true) && (Enable == false)) {
      if(SendBoolBT(DBus, "Powered", false) == false) continue;
      DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "UnregisterAgent");
      DBusMessageIter ArgsIter;
      dbus_message_iter_init_append(Msg, &ArgsIter);
      const char *Path = "/oZm_BT/agent1";
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
      DBusMessage *Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.bluez.NetworkServer1", "Unregister");
      dbus_message_iter_init_append(Msg, &ArgsIter);
      const char *UUID = "nap";
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
      Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      LogFile::Info("BluetoothManager interface DOWN");
      if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pServerName = "";
      pEnabled = false;
      if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    }
  }
  if(DBus != NULL) dbus_connection_unref(DBus);
  return NULL;
}
string BluetoothManager::GetMacBT(DBusConnection *bus) {
  string Result = "";
  DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Get");
  DBusMessageIter ArgsIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *Path = "org.bluez.Adapter1";
  const char *Property = "Address";
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Property);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) {
    DBusMessageIter Struct;
    if(dbus_message_iter_init(Reply, &Struct) && (dbus_message_iter_get_arg_type(&Struct) == DBUS_TYPE_VARIANT)) {
      DBusMessageIter VariantIter;
      dbus_message_iter_recurse(&Struct, &VariantIter);
      if(dbus_message_iter_get_arg_type(&VariantIter) == DBUS_TYPE_STRING) {
        char *Address;
        dbus_message_iter_get_basic(&VariantIter, &Address);
        Result = Address;
      }
    }
    dbus_message_unref(Reply);
  }
  dbus_message_unref(Msg);
  return Result;
}
bool BluetoothManager::SendBoolBT(DBusConnection *bus, const char *property, bool value) {
  DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Set");
  DBusMessageIter ArgsIter, SubIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *Path = "org.bluez.Adapter1";
  uint32_t Value = (value == true) ? 1 : 0;
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &property);
  dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_VARIANT, DBUS_TYPE_BOOLEAN_AS_STRING, &SubIter);
  dbus_message_iter_append_basic(&SubIter, DBUS_TYPE_BOOLEAN, &Value);
  dbus_message_iter_close_container(&ArgsIter, &SubIter);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) dbus_message_unref(Reply);
  dbus_message_unref(Msg);
  return true;
}
bool BluetoothManager::SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value) {
  DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Set");
  DBusMessageIter ArgsIter, SubIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *Path = "org.bluez.Adapter1";
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &property);
  dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_VARIANT, DBUS_TYPE_UINT32_AS_STRING, &SubIter);
  dbus_message_iter_append_basic(&SubIter, DBUS_TYPE_UINT32, &value);
  dbus_message_iter_close_container(&ArgsIter, &SubIter);
  DBusMessage *Reply = dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) dbus_message_unref(Reply);
  dbus_message_unref(Msg);
  return true;
}
bool BluetoothManager::SendStringBT(DBusConnection *bus, const char *property, const char *value) {
  DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Set");
  DBusMessageIter ArgsIter, SubIter;
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *Path = "org.bluez.Adapter1";
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &property);
  dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &SubIter);
  dbus_message_iter_append_basic(&SubIter, DBUS_TYPE_STRING, &value);
  dbus_message_iter_close_container(&ArgsIter, &SubIter);
  dbus_connection_send_with_reply_and_block(bus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  return true;
}
#endif
//----------------------------------------------------------------------------------------------------------------------