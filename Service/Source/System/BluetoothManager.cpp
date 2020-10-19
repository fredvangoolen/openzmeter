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
#ifdef OPENWRT // ======================================================================================================
#include "ConfigFile.h"
#include "LogFile.h"
#include "MessageLoop.h"
#include "Status.h"
// ---------------------------------------------------------------------------------------------------------------------
volatile bool    BluetoothManager::pTerminate   = false;
bool             BluetoothManager::pEnable      = false;
string           BluetoothManager::pInterface   = "bt";
string           BluetoothManager::pServerName  = "";
pthread_t        BluetoothManager::pThread      = 0;
pthread_mutex_t  BluetoothManager::pMutex       = PTHREAD_MUTEX_INITIALIZER;
// ---------------------------------------------------------------------------------------------------------------------
BluetoothManager::BluetoothManager() {
  LogFile::Info("Starting Bluetooth manager");
  HTTPServer::RegisterCallback("getBluetooth", GetBluetoothCallback);
  HTTPServer::RegisterCallback("setBluetooth", SetBluetoothCallback);
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
    pthread_setname_np(pThread, "Bluetooth");
  }
  MessageLoop::RegisterListener("Bluetooth_Toogle", ToggleEnable);
}
BluetoothManager::~BluetoothManager() {
  LogFile::Info("Stopping bluetooth manager...");
  MessageLoop::UnregisterListener("Bluetooth_Toogle");
  HTTPServer::UnregisterCallback("getBluetooth");
  HTTPServer::UnregisterCallback("setBluetooth");
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pThread = 0;
  LogFile::Info("Bluetooth manager stoped");
}
void BluetoothManager::ToggleEnable() {
  pthread_mutex_lock(&pMutex);
  json Config = json::object();
  Config["Enabled"] = !pEnable;
  ConfigFile::WriteConfig("/Bluetooth", Config);
  pthread_mutex_unlock(&pMutex);
}
void *BluetoothManager::Thread(void *params) {
  DBusConnection *DBus = NULL;
  nice(5);
  bool Enabled = false;
  Status::SetValue("/Bluetooth_Enabled", Enabled);
//  chmod("/dev/watchdog", S_IWUSR | S_IWGRP | S_IWOTH);
//  int WatchdogFD = open("/dev/watchdog", O_WRONLY);
//  if(WatchdogFD == -1) {
//    LogFile::Error("Watchdog is not avaliable (%s)", strerror(errno));
//    WatchdogFD = 0;
//  } else {
//    int Timeout = 20;
//    ioctl(WatchdogFD, WDIOC_SETTIMEOUT, &Timeout);
//  }
//    if(WatchdogFD != 0) write(WatchdogFD, "\0", 1);
  while((pTerminate == false) || (Enabled == true)) {
    bool Enable = (pTerminate == false) ? pEnable : false;
    if(DBus == NULL) {
      DBusError Error;
      dbus_error_init(&Error);
      DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
      if(dbus_error_is_set(&Error)) {
        LogFile::Error("Can not open DBus connection (%s)", Error.message);
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
        LogFile::Info("Authorized BT client: '%s'", Param);
        DBusMessage *Reply = dbus_message_new_method_return(Msg);
        dbus_connection_send(DBus, Reply, &Serial);
        dbus_connection_flush(DBus);
        dbus_message_unref(Reply);
      }
      dbus_message_unref(Msg);
    } else if((Enabled == false) && (Enable == true)) {
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
      LogFile::Info("Bluetooth interface UP");
      pthread_mutex_lock(&pMutex);
      pServerName = NetName;
      pthread_mutex_unlock(&pMutex);
      Enabled = true;
      Status::SetValue("/Bluetooth_Enabled", Enabled);
    } else if((Enabled == true) && (Enable == false)) {
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
      LogFile::Info("Bluetooth interface DOWN");
      pthread_mutex_lock(&pMutex);
      pServerName = "";
      pthread_mutex_unlock(&pMutex);
      Enabled = false;
      Status::SetValue("/Bluetooth_Enabled", Enabled);
    }
  }
  if(DBus != NULL) dbus_connection_unref(DBus);
//  if(WatchdogFD != 0) {
//    write(WatchdogFD, "V", 1);
//    close(WatchdogFD);
//  }
//  LogFile::Debug("Watchdog stopped");
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
void BluetoothManager::GetBluetoothCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Bluetooth");
  pthread_mutex_lock(&pMutex);
  Config["Name"] = pServerName;
  pthread_mutex_unlock(&pMutex);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void BluetoothManager::SetBluetoothCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json Config = json::object();
  pthread_mutex_lock(&pMutex);
  pEnable = params["Enabled"];
  pthread_mutex_unlock(&pMutex);
  Config["Enabled"] = pEnable;
  Config["Interface"] = pInterface;
  ConfigFile::WriteConfig("/Bluetooth", Config);
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
#else // ===============================================================================================================
BluetoothManager::BluetoothManager() {
}
BluetoothManager::~BluetoothManager() {
}
#endif // ==============================================================================================================