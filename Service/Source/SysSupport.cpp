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
#ifdef OPENWRT
  #include <sys/sysinfo.h>
  #include <sys/statvfs.h>
#endif
#ifndef __BUILD__
  #define __BUILD__ "UNKNOWN"
#endif
#include "SysSupport.h"
#include "HTTPServer.h"
//----------------------------------------------------------------------------------------------------------------------
SysSupport::SystemStatus_t     SysSupport::pSystemStatus;
queue<string>                  SysSupport::pSystemActions;
RWLocks                        SysSupport::pActionsLock;
#ifdef OPENWRT
  DBusConnection              *SysSupport::pDBus         = NULL;
  pthread_mutex_t              SysSupport::pBTMutex      = PTHREAD_MUTEX_INITIALIZER;
  string                       SysSupport::pBTBridgeName = "br-bt";
  string                       SysSupport::pBTServerName;
  bool                         SysSupport::pBTEnabled    = true;
  string                       SysSupport::pInterfaceWIFI;
  string                       SysSupport::pNetworkWIFI;
  string                       SysSupport::pWirelessWIFI;
  vector<SysSupport::Update_t> SysSupport::pUpdates;
  RWLocks                      SysSupport::pUpdatesLock;
  uint64_t                     SysSupport::pLastTask_1H = 0;
  uint64_t                     SysSupport::pLastTask_1M = 0;
  uint64_t                     SysSupport::pLastTask_5S = 0;
#endif
//----------------------------------------------------------------------------------------------------------------------
SysSupport::SysSupport() {
  HTTPServer::RegisterCallback("getNews",         GetNewsCallback);
  HTTPServer::RegisterCallback("getUsers",        GetUsersCallback);
  HTTPServer::RegisterCallback("getUser",         GetUserCallback);
  HTTPServer::RegisterCallback("setUser",         SetUserCallback);
  HTTPServer::RegisterCallback("setUserPassword", SetUserPasswordCallback);
  HTTPServer::RegisterCallback("addUser",         AddUserCallback);
  HTTPServer::RegisterCallback("delUser",         DelUserCallback);
  pTerminate = false;

  #ifdef OPENWRT
    HTTPServer::RegisterCallback("getBluetooth", GetBluetoothCallback);
    HTTPServer::RegisterCallback("setBluetooth", SetBluetoothCallback);
    HTTPServer::RegisterCallback("scanWIFI",     ScanWIFICallback);
    HTTPServer::RegisterCallback("getWIFI",      GetWIFICallback);
    HTTPServer::RegisterCallback("setWIFI",      SetWIFICallback);
    HTTPServer::RegisterCallback("getNTP",       GetNTPCallback);
    HTTPServer::RegisterCallback("setNTP",       SetNTPCallback);
    HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
    HTTPServer::RegisterCallback("upgrade",      UpgradeCallback);

    json Config = ConfigManager::ReadConfig("/Bluetooth");
    DBusError Error;
    dbus_error_init(&Error);
    pDBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
    if(dbus_error_is_set(&Error)) {
      LOGFile::Error("Can not open DBus connection (%s)\n", Error.message);
      dbus_error_free(&Error);
      pDBus = NULL;
      return;
    }
    if(Config.is_object() == true) {
      if(Config["Enabled"].is_boolean()) pBluetooth_Enabled = Config["Enabled"];
      if(Config["Bridge"].is_string()) pBTBridgeName = Config["Bridge"];
    }
    Config = json::object();
    Config["Enabled"] = pBTEnabled;
    Config["Bridge"] = pBTBridgeName;
    ConfigManager::WriteConfig("/Bluetooth", Config);
    Config = ConfigManager::ReadConfig("/WIFI");
    pInterfaceWIFI = "wlan0";
    pNetworkWIFI = "lan";
    pWirelessWIFI = "default_radio";
    if(Config.is_object() == true) {
      if(Config["Interface"].is_string()) pInterfaceWIFI = Config["Interface"];
      if(Config["Network"].is_string()) pNetworkWIFI = Config["Network"];
      if(Config["Config"].is_string()) pWirelessWIFI = Config["Config"];
    }
    Config = json::object();
    Config["Interface"] = pInterfaceWIFI;
    Config["Network"] = pNetworkWIFI;
    Config["Config"] = pWirelessWIFI;
    ConfigManager::WriteConfig("/WIFI", Config);
    if(pBTEnabled) StartBT();

  #endif
}
SysSupport::~SysSupport() {
  HTTPServer::UnregisterCallback("getNews");
  HTTPServer::UnregisterCallback("getUsers");
  HTTPServer::UnregisterCallback("getUser");
  HTTPServer::UnregisterCallback("setUser");
  HTTPServer::UnregisterCallback("setUserPassword");
  HTTPServer::UnregisterCallback("addUser");
  HTTPServer::UnregisterCallback("delUser");
  #ifdef OPENWRT
    HTTPServer::UnregisterCallback("getBluetooth");
    HTTPServer::UnregisterCallback("setBluetooth");
    HTTPServer::UnregisterCallback("scanWIFI");
    HTTPServer::UnregisterCallback("getWIFI");
    HTTPServer::UnregisterCallback("setWIFI");
    HTTPServer::UnregisterCallback("getNTP");
    HTTPServer::UnregisterCallback("setNTP");
    HTTPServer::UnregisterCallback("getUpdates");
    HTTPServer::UnregisterCallback("upgrade");
    StopBT();
    if(pDBus) dbus_connection_unref(pDBus);
  #endif
}
//----------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
void *SysSupport::Bluetooth_Thread(void *params) {
  
}


void SysSupport::GetNewsCallback(const json &params, Response &response) {
  Database DB;
  if(params["NewsTime"].is_number()) {
    uint64_t Time = params["NewsTime"];
    DB.ExecSentenceAsyncResult("SELECT json_build_object('NewsTime', newstime, 'Header', header, 'Contents', contents) as result FROM news WHERE newstime = " + DB.Bind(Time));
  } else {
    DB.ExecSentenceAsyncResult("SELECT json_build_object('Version', '" __BUILD__ "', 'News', COALESCE(array_agg(newstime ORDER BY newstime ASC), '{}')) AS result from news");
  }
  if(DB.FecthRow() == false) {
    response.set_content("", "text/plain");
    response.status = HTTP_INTERNAL_SERVER_ERROR;
  } else {
    response.set_content(DB.ResultString("result"), "application/json");
    response.status = HTTP_OK;
  }
}
void SysSupport::GetUsersCallback(const json &params, Response &response) {
  //only 'admin' user can get this information
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Filter") == false) || (params["Filter"].is_string() == false)) return HTTPServer::FillResponse(response, "'Filter' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Limit") == false) || (params["Limit"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'Limit' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Offset") == false) || (params["Offset"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'Offset' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string   Filter = params["Filter"];
  uint32_t Limit  = params["Limit"];
  uint32_t Offset  = params["Offset"];
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT COALESCE(to_json(array_agg(t1.user)), '[]') AS result FROM (SELECT json_build_object('User', username, 'Name', name, 'Email', email, 'TimeZone', time_zone) AS user FROM users WHERE position(" + DB.Bind(Filter) + " in username) > 0 ORDER BY username ASC LIMIT " + DB.Bind(Limit) + " OFFSET " + DB.Bind(Offset) + ") t1");
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void SysSupport::GetUserCallback(const json &params, Response &response) {
  string UserName = params["UserName"];
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string User = params["User"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  DB.ExecSentenceAsyncResult("SELECT json_build_object('User', username, 'Name', name, 'Address', address, 'Company', company, 'Photo', photo, 'Email', email, 'Telegram', telegram, 'TimeZone', time_zone) AS result FROM users WHERE username = " + DB.Bind(User));
  if(DB.FecthRow() == false) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  HTTPServer::FillResponse(response, DB.ResultString("result"), "application/json", HTTP_OK);
}
void SysSupport::SetUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Address") == false) || (params["Address"].is_string() == false)) return HTTPServer::FillResponse(response, "'Address' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Company") == false) || (params["Company"].is_string() == false)) return HTTPServer::FillResponse(response, "'Company' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Photo") == false) || (params["Photo"].is_string() == false)) return HTTPServer::FillResponse(response, "'Photo' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Email") == false) || (params["Email"].is_string() == false)) return HTTPServer::FillResponse(response, "'Email' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Telegram") == false) || (params["Telegram"].is_string() == false)) return HTTPServer::FillResponse(response, "'Telegram' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  string Name = params["Name"];
  string Address = params["Address"];
  string Company = params["Company"];
  string Photo = params["Photo"];
  string Email = params["Email"];
  string Telegram = params["Telegram"];
  string TimeZone = params["TimeZone"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("UPDATE users SET name = " + DB.Bind(Name) + ", address = " + DB.Bind(Address) + ", company = " + DB.Bind(Company) +  ", photo = " + DB.Bind(Photo) + ", email = " + DB.Bind(Email) + ", telegram = " + DB.Bind(Telegram) + ", time_zone = " + DB.Bind(TimeZone) + " WHERE username = " + DB.Bind(User));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void SysSupport::SetUserPasswordCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return HTTPServer::FillResponse(response, "'Password' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  string Password = params["Password"];
  if((UserName != "admin") && (UserName != User)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  if(UserName == User) {
    if((params.contains("PrevPassword") == false) || (params["PrevPassword"].is_string() == false)) return HTTPServer::FillResponse(response, "'PrevPassword' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    string PrevPassword = params["PrevPassword"];
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + " AND password = MD5(" + DB.Bind(PrevPassword) + ") RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  } else {
    DB.ExecSentenceAsyncResult("UPDATE users SET password = MD5(" + DB.Bind(Password) + ") WHERE username = " + DB.Bind(User) + ") RETURNING username");
    return HTTPServer::FillResponse(response, "", "text/plain", (DB.FecthRow() == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
  }
}
void SysSupport::AddUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Name") == false) || (params["Name"].is_string() == false)) return HTTPServer::FillResponse(response, "'Name' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Address") == false) || (params["Address"].is_string() == false)) return HTTPServer::FillResponse(response, "'Address' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Company") == false) || (params["Company"].is_string() == false)) return HTTPServer::FillResponse(response, "'Company' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Photo") == false) || (params["Photo"].is_string() == false)) return HTTPServer::FillResponse(response, "'Photo' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Email") == false) || (params["Email"].is_string() == false)) return HTTPServer::FillResponse(response, "'Email' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Telegram") == false) || (params["Telegram"].is_string() == false)) return HTTPServer::FillResponse(response, "'Telegram' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("TimeZone") == false) || (params["TimeZone"].is_string() == false)) return HTTPServer::FillResponse(response, "'TimeZone' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Password") == false) || (params["Password"].is_string() == false)) return HTTPServer::FillResponse(response, "'Password' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  string Password = params["Password"];
  string Name = params["Name"];
  string Address = params["Address"];
  string Company = params["Company"];
  string Photo = params["Photo"];
  string Email = params["Email"];
  string Telegram = params["Telegram"];
  string TimeZone = params["TimeZone"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("INSERT INTO users(username, password, lastlogin, name, address, company, photo, email, telegram, time_zone) VALUES(" + DB.Bind(User) + ", MD5(" + DB.Bind(Password) + "), 0, " + DB.Bind(Name) + ", " + DB.Bind(Address) + ", " + DB.Bind(Company) +  ", " + DB.Bind(Photo) + ", " + DB.Bind(Email) + ", " + DB.Bind(Telegram) + ", " + DB.Bind(TimeZone) + ")");
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void SysSupport::DelUserCallback(const json &params, Response &response) {
  if((params.contains("User") == false) || (params["User"].is_string() == false)) return HTTPServer::FillResponse(response, "'User' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string UserName = params["UserName"];
  string User = params["User"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Database DB;
  bool Result = DB.ExecSentenceNoResult("DELETE FROM users WHERE name = " + DB.Bind(User));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
SysSupport::SystemStatus_t *SysSupport::GetSystemStatus() {
  return &pSystemStatus;
}
json SysSupport::GetSystemStatusJSON() {
  json Result = json::object();
  pSystemStatus.Lock.ReadLock();
  Result["Updates"] = pSystemStatus.Updates;
  Result["Events"] = pSystemStatus.Events;
  Result["UpTime"] = pSystemStatus.UpTime;
  Result["Swap_Total"] = pSystemStatus.Swap_Total;
  Result["Swap_Usage"] = pSystemStatus.Swap_Usage;
  Result["CPU_Usage"] = pSystemStatus.CPU_Usage;
  Result["Disk_Total"] = pSystemStatus.Disk_Total;
  Result["Disk_Usage"] = pSystemStatus.Disk_Usage;
  Result["RAM_Total"] = pSystemStatus.RAM_Total;
  Result["RAM_Usage"] = pSystemStatus.RAM_Usage;
  Result["Temperature"] = pSystemStatus.Temperature;
  Result["WIFI_Network"] = pSystemStatus.WIFI_Network;
  Result["WIFI_Power"] = pSystemStatus.WIFI_Power;
  Result["BT_Network"] = pSystemStatus.BT_Network;
  Result["Time_IsSet"] = pSystemStatus.Time_IsSet;
  Result["Battery_Volt"] = pSystemStatus.Battery_Volt;
  Result["Battery_Level"] = pSystemStatus.Battery_Level;
  pSystemStatus.Lock.Unlock();
  return Result;
}
void SysSupport::SetSystemActions(const string &action) {
  pActionsLock.WriteLock();
  pSystemActions.push(action);
  pActionsLock.Unlock();
}
#ifdef OPENWRT
  string SysSupport::Run(string command, int *resultCode) {
    char Buffer[128];
    string Result = "";
    FILE *Pipe = popen(command.c_str(), "r");
    if(!Pipe) {
      LOGFile::Error("Can not exec '%s' (%s)\n", command.c_str(), strerror(errno));
      if(resultCode != NULL) *resultCode = -1;
      return "";
    }
    while(fgets(Buffer, sizeof(Buffer), Pipe) != NULL) Result += Buffer;
    int Code = WEXITSTATUS(pclose(Pipe));
    if(resultCode != NULL) *resultCode = Code;
    return Result;
  }
  bool SysSupport::StartBT() {
    pthread_mutex_lock(&pBTMutex);
    //Register agent
    DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RegisterAgent");
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *Path = "/oZm_BT/agent1";
    const char *Capabilities = "NoInputNoOutput";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Capabilities);
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    //Set as default agent
    Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "RequestDefaultAgent");
    dbus_message_iter_init_append(Msg, &ArgsIter);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
    Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    if(SendBoolBT("Powered", false) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    string MAC = GetMacBT();
    if(MAC.empty() == true) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    MAC.erase(std::remove(MAC.begin(), MAC.end(), ':'), MAC.end());
    pBTServerName = "openZmeter_" + MAC.substr(8);
    if(SendStringBT("Alias", pBTServerName.c_str()) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    if(SendBoolBT("Powered", true) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    if(SendBoolBT("Discoverable", true) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    if(SendUINT32BT("DiscoverableTimeout", 0) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    if(SendBoolBT("Discoverable", true) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    if(SendUINT32BT("DiscoverableTimeout", 0) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.bluez.NetworkServer1", "Register");
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *UUID = "nap";
    const char *Bridge = pBTBridgeName.c_str();
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Bridge);
    Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    pthread_mutex_unlock(&pBTMutex);
    LOGFile::Info("Bluetooth interface UP\n");
    pBTEnabled = true;
    return true;
  }
  bool SysSupport::StopBT() {
    pthread_mutex_lock(&pBTMutex);
    pBTServerName = "";
    if(SendBoolBT("Powered", false) == false) {
      pthread_mutex_unlock(&pBTMutex);
      return false;
    }
    //Unregister agent
    DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez", "org.bluez.AgentManager1", "UnregisterAgent");
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *Path = "/oZm_BT/agent1";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_OBJECT_PATH, &Path);
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.bluez.NetworkServer1", "Unregister");
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *UUID = "nap";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
    Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    pthread_mutex_unlock(&pBTMutex);
    LOGFile::Info("Bluetooth interface DOWN\n");
    pBTEnabled = false;
    return true;
  }
  void SysSupport::HandleAgentBT() {
    dbus_connection_read_write(pDBus, 0);
    DBusMessage *Msg = dbus_connection_pop_message(pDBus);
    if(Msg == NULL) return;
    if(dbus_message_is_method_call(Msg, "org.bluez.Agent1", "AuthorizeService")) {
      dbus_uint32_t Serial = 0;
      char *Param = (char*)"";
      DBusMessageIter Args;
      dbus_message_iter_init(Msg, &Args);
      dbus_message_iter_get_basic(&Args, &Param);
      LOGFile::Info("Authorized BT client: '%s'\n", Param);
      DBusMessage *Reply = dbus_message_new_method_return(Msg);
      dbus_connection_send(pDBus, Reply, &Serial);
      dbus_connection_flush(pDBus);
      dbus_message_unref(Reply);
    }
    dbus_message_unref(Msg);
  }
  string SysSupport::GetMacBT() {
    string Result = "";
    DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Get");
    DBusMessageIter ArgsIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *Path = "org.bluez.Adapter1";
    const char *Property = "Address";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Property);
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
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
  bool SysSupport::SendBoolBT(const char *property, bool value) {
    if(pDBus == NULL) return false;
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
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    return true;
  }
  bool SysSupport::SendUINT32BT(const char *property, uint32_t value) {
    if(pDBus == NULL) return false;
    DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Set");
    DBusMessageIter ArgsIter, SubIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *Path = "org.bluez.Adapter1";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &property);
    dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_VARIANT, DBUS_TYPE_UINT32_AS_STRING, &SubIter);
    dbus_message_iter_append_basic(&SubIter, DBUS_TYPE_UINT32, &value);
    dbus_message_iter_close_container(&ArgsIter, &SubIter);
    DBusMessage *Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    if(Reply) dbus_message_unref(Reply);
    dbus_message_unref(Msg);
    return true;
  }
  bool SysSupport::SendStringBT(const char *property, const char *value) {
    if(pDBus == NULL) return false;
    DBusMessage *Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.freedesktop.DBus.Properties", "Set");
    DBusMessageIter ArgsIter, SubIter;
    dbus_message_iter_init_append(Msg, &ArgsIter);
    const char *Path = "org.bluez.Adapter1";
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Path);
    dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &property);
    dbus_message_iter_open_container(&ArgsIter, DBUS_TYPE_VARIANT, DBUS_TYPE_STRING_AS_STRING, &SubIter);
    dbus_message_iter_append_basic(&SubIter, DBUS_TYPE_STRING, &value);
    dbus_message_iter_close_container(&ArgsIter, &SubIter);
    dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
    return true;
  }
  uint64_t SysSupport::ParseProcMem(const char *name, const char *buff) {
    const char *hit = strstr(buff, name);
    if(hit == NULL) return 0;
    errno = 0;
    long Val = strtol(hit + strlen(name), NULL, 10);
    if(errno != 0) return 0;
    return Val * 1024;
  }
  void SysSupport::PeriodicTask_5S() {
    struct sysinfo Info;
    struct statvfs Disk;
    pLastTask_5S = Tools::GetTime();
    if(sysinfo(&Info) != 0) {
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.UpTime = 0;
      pSystemStatus.Swap_Total = 0;
      pSystemStatus.Swap_Usage = NAN;
      pSystemStatus.CPU_Usage = NAN;
      pSystemStatus.Lock.Unlock();
    } else {
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.UpTime = Info.uptime;
      pSystemStatus.Swap_Total = (uint64_t)Info.totalswap * Info.mem_unit;
      pSystemStatus.Swap_Usage = ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap;
      pSystemStatus.CPU_Usage = ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0;
      pSystemStatus.Lock.Unlock();
    }
    if(statvfs("/", &Disk) != 0) {
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.Disk_Total = 0;
      pSystemStatus.Disk_Usage = NAN;
      pSystemStatus.Lock.Unlock();
    } else {
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.Disk_Total = (uint64_t)Disk.f_blocks * Disk.f_frsize;
      pSystemStatus.Disk_Usage = ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks;
      pSystemStatus.Lock.Unlock();
    }
    char Buffer[8192];
    int TmpFile = open("/proc/meminfo", O_RDONLY);
    if(TmpFile > 0) {
      size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
      close(TmpFile);
      Buffer[Len] = 0;
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.RAM_Total = ParseProcMem("MemTotal:", Buffer);
      pSystemStatus.RAM_Usage = ((ParseProcMem("MemTotal:", Buffer) -  ParseProcMem("MemFree:", Buffer) - ParseProcMem("Cached:", Buffer) - ParseProcMem("Buffers:", Buffer)) * 100.0) / + ParseProcMem("MemTotal:", Buffer);
      pSystemStatus.Lock.Unlock();
    } else {
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.RAM_Total = 0;
      pSystemStatus.RAM_Usage = NAN;
      pSystemStatus.Lock.Unlock();
    }
    TmpFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
    if(TmpFile > 0) {
      size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
      close(TmpFile);
      Buffer[Len] = 0;
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.Temperature = atoi(Buffer) / 1000.0;
      pSystemStatus.Lock.Unlock();
    } else {
      pSystemStatus.Lock.WriteLock();
      pSystemStatus.Temperature = NAN;
      pSystemStatus.Lock.Unlock();
    }
    string CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInterfaceWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pSystemStatus.Lock.WriteLock();
        pSystemStatus.WIFI_Network = "";
        pSystemStatus.WIFI_Power = NAN;
        pSystemStatus.Lock.Unlock();
      } else if(Tmp.is_object() && Tmp["ssid"].is_string() && Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
        pSystemStatus.Lock.WriteLock();
        pSystemStatus.WIFI_Network = Tmp["ssid"];
        float Current = Tmp["quality"], Max = Tmp["quality_max"];
        pSystemStatus.WIFI_Power = Current * 100.0 / Max;
        pSystemStatus.Lock.Unlock();
      }
    }
    pSystemStatus.Lock.WriteLock();
    pSystemStatus.BT_Network = (pBTEnabled == true) ? pBTServerName : "";
    pSystemStatus.Lock.Unlock();
  }
  void SysSupport::PeriodicTask_1M() {
    pLastTask_1M = Tools::GetTime();
    string CommandStr = Run("ubus call uci get '{\"config\":\"system\", \"section\":\"ntp\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if((Tmp.is_discarded() == false) && Tmp.is_object() && Tmp["values"].is_object() && Tmp["values"]["enabled"].is_string() && Tmp["values"]["server"].is_string()) {
        string Value = Tmp["values"]["enabled"];
        if(Value == "1") {
          string Server = Tmp["values"]["server"];
          int Code;
          CommandStr = Run("ntpd -n -q -p " + Server, &Code);
          pSystemStatus.Lock.WriteLock();
          pSystemStatus.Time_IsSet = (Code == 0);
          pSystemStatus.Lock.Unlock();
        } else {
          pSystemStatus.Lock.WriteLock();
          pSystemStatus.Time_IsSet = (Tools::GetTime() > 946684800000);
          pSystemStatus.Lock.Unlock();
        }
        return;
      }
    }
    pSystemStatus.Lock.WriteLock();
    pSystemStatus.Time_IsSet = false;
    pSystemStatus.Lock.Unlock();
  }
  void SysSupport::PeriodicTask_1H() {
    pLastTask_1H = Tools::GetTime();
    pUpdatesLock.WriteLock();
    pUpdates.clear();
    pUpdatesLock.Unlock();
    int ResultCode = 0;
    string Result = Run("opkg update", &ResultCode);
    if(ResultCode != 0) return;
    Result = Run("opkg list-upgradable", &ResultCode);
    if(ResultCode != 0) return;
    pUpdatesLock.WriteLock();
    size_t Pos = 0;
    while(Pos < Result.size()) {
      Update_t Update;
      size_t Pos2 = Result.find(" - ", Pos);
      Update.Package = Result.substr(Pos, Pos2 - Pos);
      Pos = Pos2 + 3;
      Pos2 = Result.find(" - ", Pos);
      Update.From = Result.substr(Pos, Pos2 - Pos);
      Pos = Pos2 + 3;
      Pos2 = Result.find("\n", Pos);
      if(Pos2 == string::npos) Pos2 = Result.size();
      Update.To = Result.substr(Pos, Pos2 - Pos);
      Pos = Pos2 + 1;
      pUpdates.push_back(Update);
    }
    pSystemStatus.Lock.WriteLock();
    pSystemStatus.Updates = (pUpdates.size() > 0);
    pSystemStatus.Lock.Unlock();
    pUpdatesLock.Unlock();
  }
  void SysSupport::GetUpdatesCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    json Response = json::object();
    Response["LastCheck"] = pLastTask_1H;
    Response["Updates"] = json::array();
    pUpdatesLock.ReadLock();
    for(uint32_t n = 0; n < pUpdates.size(); n++) {
      json Update = json::object();
      Update["Package"] = pUpdates[n].Package;
      Update["From"] = pUpdates[n].From;
      Update["To"] = pUpdates[n].To;
      Response["Updates"].push_back(Update);
    }
    pUpdatesLock.Unlock();
    HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::UpgradeCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    json Response = json::object();
    int ResultCode = 0;
    string Result = Run("opkg list-upgradable | cut -f 1 -d ' ' | xargs opkg upgrade --force-depends", &ResultCode);
    Response["Result"] = (ResultCode == 0);
    PeriodicTask_1H();
    //req.FromJSON(Response);
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
  }
  void SysSupport::GetBluetoothCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    json Config = ConfigManager::ReadConfig("/Bluetooth");
    pthread_mutex_lock(&pBTMutex);
    Config["Name"] = pBTServerName;
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::SetBluetoothCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    if(params["Enabled"].is_boolean()) pBTEnabled = params["Enabled"];
    json Config;
    pthread_mutex_lock(&pBTMutex);
    Config["Enabled"] = pBTEnabled;
    Config["Bridge"] = pBTBridgeName;
    pthread_mutex_unlock(&pBTMutex);
    ConfigManager::WriteConfig("/Bluetooth", Config);
    json Response;
    Response["Status"] = true;
    if(pBTEnabled == true) Response["Status"] = StartBT();
    if(pBTEnabled == false) Response["Status"] = StopBT();
    HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::ScanWIFICallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    pthread_mutex_lock(&pBTMutex);
    json Result = json::array();
    string CommandStr = Run("ubus call iwinfo scan '{\"device\":\"" + pInterfaceWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pBTMutex);
        return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
      }
      if(Tmp.is_object() && Tmp["results"].is_array()) {
        for(uint Pos = 0; Pos < Tmp["results"].size(); Pos++) {
          json AP = json::object();
          if(Tmp["results"][Pos]["ssid"].is_string()) AP["ESSID"] = Tmp["results"][Pos]["ssid"];
          if(Tmp["results"][Pos]["channel"].is_number()) AP["Channel"] = Tmp["results"][Pos]["channel"];
          if(Tmp["results"][Pos]["quality"].is_number() && Tmp["results"][Pos]["quality_max"].is_number()) {
            float Current = Tmp["results"][Pos]["quality"], Max = Tmp["results"][Pos]["quality_max"];
            AP["Quality"] = (int)(Current * 100.0 / Max);
          }
          Result.push_back(AP);
        }
      }
    }
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::GetWIFICallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    if(!params["Type"].is_string() || ((params["Type"] != "CURRENT") && (params["Type"] != "CONFIG"))) return HTTPServer::FillResponse(response, "'Type' param is requiered to be 'CURRENT' or 'CONFIG'", "text/plain", HTTP_BAD_REQUEST);
    pthread_mutex_lock(&pBTMutex);
    json Result = json::object();
    if(params["Type"] == "CURRENT") {
      string CommandStr = Run("ubus call network.device status '{\"name\":\"" + pInterfaceWIFI + "\"}'");
      if(CommandStr.empty() == false) {
        json Tmp = json::parse(CommandStr, NULL, false);
        if(Tmp.is_discarded()) {
          pthread_mutex_unlock(&pBTMutex);
          return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
        }
        if(Tmp.is_object() && Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];
        if(Result["Enabled"] == false) {
          pthread_mutex_unlock(&pBTMutex);
//          return req.FromJSON(Result);
        }
      }
      CommandStr = Run("ubus call network.interface." + pNetworkWIFI + " status");
      if(CommandStr.empty() == false) {
        json Tmp = json::parse(CommandStr, NULL, false);
        if (Tmp.is_discarded()) {
          pthread_mutex_unlock(&pBTMutex);
          return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
        }
        if(Tmp.is_object()) {
          if(Tmp["up"].is_boolean()) Result["Connected"] = Tmp["up"];
          if(Result["Connected"] == false) {
            pthread_mutex_unlock(&pBTMutex);
//            return req.FromJSON(Result);
          }
          if(Tmp["dns-server"].is_array()) Result["DNS"] = Tmp["dns-server"];
          if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["IP"] = Tmp["ipv4-address"][0]["address"];
          if(Tmp["proto"].is_string()) Result["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
          if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Gateway"] = Tmp["route"][0]["nexthop"];
        }
      }
      CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInterfaceWIFI + "\"}'");
      if(CommandStr.empty() == false) {
        json Tmp = json::parse(CommandStr, NULL, false);
        if(Tmp.is_discarded()) {
          pthread_mutex_unlock(&pBTMutex);
          return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
        }
        if(Tmp.is_object()) {
          if(Tmp["ssid"].is_string()) Result["ESSID"] = Tmp["ssid"];
          if(Tmp["channel"].is_number()) Result["Channel"] = Tmp["channel"];
          if(Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
            float Current = Tmp["quality"], Max = Tmp["quality_max"];
            Result["Quality"] = (int) (Current * 100.0 / Max);
          }
        }
      }
    } else {
      string CommandStr = Run("ubus call uci get '{\"config\":\"network\", \"section\":\"" + pNetworkWIFI + "\"}'");
      if(CommandStr.empty() == false) {
        json Tmp = json::parse(CommandStr, NULL, false);
        if(Tmp.is_discarded()) {
          pthread_mutex_unlock(&pBTMutex);
          return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
        }
        if(Tmp.is_object()) {
          if(Tmp["values"].is_object()) {
            if(Tmp["values"]["proto"].is_string()) Result["IPMode"] = (Tmp["values"]["proto"] == "static") ? "STATIC" : "DHCP";
            if(Result["IPMode"] != "DHCP") {
              if(Tmp["values"]["ipaddr"].is_string()) Result["IP"] = Tmp["values"]["ipaddr"];
              if(Tmp["values"]["netmask"].is_string()) Result["Netmask"] = Tmp["values"]["netmask"];
              if(Tmp["values"]["gateway"].is_string()) Result["Gateway"] = Tmp["values"]["gateway"];
              if(Tmp["values"]["dns"].is_array()) Result["DNS"] = Tmp["values"]["dns"];
            }
          }
        }
      }
      CommandStr = Run("ubus call uci get '{\"config\":\"wireless\", \"section\":\"" + pWirelessWIFI + "\"}'");
      if(CommandStr.empty() == false) {
        json Tmp = json::parse(CommandStr, NULL, false);
        if(Tmp.is_discarded()) {
          pthread_mutex_unlock(&pBTMutex);
          return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
        }
        if(Tmp.is_object()) {
          if(Tmp["values"].is_object()) {
            if(Tmp["values"]["ssid"].is_string()) Result["ESSID"] = Tmp["values"]["ssid"];
            if(Tmp["values"]["encryption"].is_string()) Result["KeyMode"] = (Tmp["values"]["encryption"] == "none") ? "NONE" : (Tmp["values"]["encryption"] == "wep") ? "WEP" : (Tmp["values"]["encryption"] == "psk") ? "WPA" : "WPA2";
            if(Result["KeyMode"] != "NONE") {
              if(Tmp["values"]["key"].is_string()) Result["Key"] = Tmp["values"]["key"];
            }
          }
        }
      }
    }
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::SetWIFICallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    pthread_mutex_lock(&pBTMutex);
    json Result;
    Result["Status"] = true;
    int ErrorCode = 0;
    if(params["Enabled"].is_boolean()) {
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"enabled\":\"" + (params["Enabled"] ? "1" : "0") + "\"}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["IPMode"].is_string() && ((params["IPMode"] == "DHCP") || (params["IPMode"] == "STATIC"))) {
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"proto\":\"" + ((params["IPMode"] == "DHCP") ? "dhcp" : "static") + "\"}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["IP"].is_string()) {
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"ipaddr\":" + params["IP"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["Netmask"].is_string()) {
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"netmask\":" + params["Netmask"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["Gateway"].is_string()) {
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"gateway\":" + params["Gateway"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["DNS"].is_array()) {
//      if(params["DNS"].size() == 0) params["DNS"] = "";
      Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"dns\":" + params["DNS"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["ESSID"].is_string()) {
      Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"ssid\":" + params["ESSID"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["KeyMode"].is_string() && ((params["KeyMode"] == "NONE") || (params["KeyMode"] == "WEP") || (params["KeyMode"] == "WPA") || (params["KeyMode"] == "WPA2"))) {
      Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"encryption\":\"" + ((params["KeyMode"] == "NONE") ? "none" : (params["KeyMode"] == "WEP") ? "wep" : (params["KeyMode"] == "WPA") ? "psk" : "psk2") + "\"}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    if(params["Key"].is_string()) {
      Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"key\":" + params["Key"].dump() + "}}'", &ErrorCode);
      if(ErrorCode != 0) Result["Status"] = false;
    }
    Run("uci commit && ubus call network reload", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::GetNTPCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    pthread_mutex_lock(&pBTMutex);
    json Result = json::object();
    string CommandStr = Run("ubus call uci get '{\"config\":\"system\", \"section\":\"ntp\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pBTMutex);
        return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
      }
      if(Tmp.is_object()) {
        if(Tmp["values"].is_object()) {
          if(Tmp["values"]["enabled"].is_string()) {
            string Value = Tmp["values"]["enabled"];
            Result["Enabled"] = (Value == "1");
          }
          if(Tmp["values"]["server"].is_string()) Result["Server"] = Tmp["values"]["server"];
        }
      }
    }
    Result["Time"] = Tools::GetTime();
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  void SysSupport::SetNTPCallback(const json &params, Response &response) {
    //only 'admin' user can get this information
    if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
    pthread_mutex_lock(&pBTMutex);
    json Result;
    int ErrorCode = 0;
    if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    if((params.contains("Server") == false) || (params["Server"].is_string() == false)) return HTTPServer::FillResponse(response, "'Server' param is requiered", "text/plain", HTTP_BAD_REQUEST);
    bool Enabled  = params["Enabled"];
    string Server = params["Server"];
    Run("ubus call uci set '{\"config\":\"system\", \"section\":\"ntp\", \"values\":{\"enabled\":\"" + string((Enabled == true) ? "1" : "0") + "\", \"server\":\"" + Server + "\"}}'", &ErrorCode);
    if(ErrorCode == 0) Run("uci commit", &ErrorCode);
    if(ErrorCode == 0) Run("reload_config", &ErrorCode);
    if((params["Enabled"] == false) && params["Time"].is_number()) {
      uint64_t Time = params["Time"];
      timeval TimeVal;
      TimeVal.tv_sec  = Time / 1000;
      TimeVal.tv_usec = Time % 1000;
      if(settimeofday(&TimeVal, NULL) != 0) {
        Result["Status"] = false;
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      }
    }
    pthread_mutex_unlock(&pBTMutex);
    HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
  }
#endif
void SysSupport::PeriodicTask() {
  string Action = "";
  pActionsLock.WriteLock();
  if(pSystemActions.size() > 0) {
    Action = pSystemActions.front();
    pSystemActions.pop();
  }
  pActionsLock.Unlock();
  if(Action.empty() == false) LOGFile::Debug("Action '%s' picked from list\n", Action.c_str());
  #ifdef OPENWRT
    HandleAgentBT();
    uint64_t Now = Tools::GetTime();
    if((Now > 5000) && (pLastTask_5S < (Now - 5000))) PeriodicTask_5S();
    if((Now > 60000) && (pLastTask_1M < (Now - 60000))) PeriodicTask_1M();
    if((Now > 86400000) && (pLastTask_1H < (Now - 86400000))) PeriodicTask_1H();
    if(Action == "PowerOff") {
      Run("poweroff");
    } else if(Action == "ToogleBluetooth") {
      pthread_mutex_lock(&pBTMutex);
      json Config = json::object();
      Config["Enabled"] = !pBTEnabled;
      pthread_mutex_unlock(&pBTMutex);
      ConfigManager::WriteConfig("/Bluetooth", Config);
      if(pBTEnabled == true) {
        StopBT();
      } else {
        StartBT();
      }
    }
  #else
    if(Action.empty() == false) LOGFile::Info("Received '%s' action\n", Action.c_str());
  #endif
}
//----------------------------------------------------------------------------------------------------------------------
