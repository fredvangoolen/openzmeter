#include <sys/sysinfo.h>
#include <sys/statvfs.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <sys/socket.h>
//#include <linux/if.h>
//#include <linux/if_bridge.h>
//#include <sys/ioctl.h>
//#include <sys/types.h>
//#include <sys/wait.h>
#include "OpenWRT.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
bool            System::pRegistered      = false;
System         *System::pInstance        = NULL;
pthread_mutex_t System::pMutex           = PTHREAD_MUTEX_INITIALIZER;
//----------------------------------------------------------------------------------------------------------------------
System::System() {
  pthread_mutex_lock(&pMutex);
  if(pInstance != NULL) delete pInstance;
  pthread_mutex_unlock(&pMutex);
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  pBTBridgeName    = "br-bt";
  pBTServerName    = "Stopped";
  pBTStarted       = false;
  pLastUpdateCheck = 0;
  DBusError Error;
  dbus_error_init(&Error);
  pDBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    LOGFile::Error("Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
    pDBus = NULL;
    return;
  }
  bool Enabled = true;
  if(Config.is_object() == true) {
    if(Config["Enabled"].is_boolean()) Enabled = Config["Enabled"];
    if(Config["Bridge"].is_string()) pBTBridgeName = Config["Bridge"];
  }
  Config = json::object();
  Config["Enabled"] = Enabled;
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
  if(Enabled) StartBT();
  if(pRegistered == false) {
    HTTPServer::RegisterCallback("getBluetooth", GetBluetoothCallback);
    HTTPServer::RegisterCallback("setBluetooth", SetBluetoothCallback);
    HTTPServer::RegisterCallback("scanWIFI", ScanWIFICallback);
    HTTPServer::RegisterCallback("getWIFI",  GetWIFICallback);
    HTTPServer::RegisterCallback("setWIFI",  SetWIFICallback);
    HTTPServer::RegisterCallback("getNTP", GetNTPCallback);
    HTTPServer::RegisterCallback("setNTP", SetNTPCallback);
    HTTPServer::RegisterCallback("getUpdates", GetUpdatesCallback);
    HTTPServer::RegisterCallback("SetUpdates", UpdateCallback);
    pRegistered = true;
  }
  pthread_mutex_lock(&pMutex);
  pInstance = this;
  pthread_mutex_unlock(&pMutex);
}
System::~System() {
  pInstance = NULL;
  StopBT();
  if(pDBus) dbus_connection_unref(pDBus);
}
string System::Run(string command, int *resultCode) {
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
bool System::StartBT() {
  pthread_mutex_lock(&pMutex);
  if(pBTStarted == true) return true;
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
    pthread_mutex_unlock(&pMutex);
    return false; //Disable adapter
  }
  string MAC = GetMacBT();
  if(MAC.empty() == true) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  MAC.erase(std::remove(MAC.begin(), MAC.end(), ':'), MAC.end());
  pBTServerName = "openZmeter_" + MAC.substr(8);
  if(SendStringBT("Alias", pBTServerName.c_str()) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendBoolBT("Powered", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false; //enable adapter
  }
  if(SendBoolBT("Discoverable", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendUINT32BT("DiscoverableTimeout", 0) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendBoolBT("Discoverable", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendUINT32BT("DiscoverableTimeout", 0) == false) {
    pthread_mutex_unlock(&pMutex);
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
  pthread_mutex_unlock(&pMutex);
  LOGFile::Info("Bluetooth interface UP\n");
  pBTStarted = true;
  return true;
}
bool System::StopBT() {
  pthread_mutex_lock(&pMutex);
  if(pBTStarted == false) return true;
  pBTServerName = "Stopped";
  if(SendBoolBT("Powered", false) == false) {
    pthread_mutex_unlock(&pMutex);
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
  pthread_mutex_unlock(&pMutex);
  LOGFile::Info("Bluetooth interface DOWN\n");
  pBTStarted = false;
  return true;
}
void System::HandleAgentBT() {
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
void System::CheckUpdates() {
  int ResultCode = 0;
  string Result = Run("opkg update", &ResultCode);
  if(ResultCode != 0) return;
  Result = Run("opkg list-upgradable", &ResultCode);
  if(ResultCode != 0) return;
  pthread_mutex_lock(&pMutex);
  pUpdates.clear();
  size_t Pos = 0;
  while(Pos < Result.size()) {
    Update_t Update;
    size_t Pos2 = Result.find(" - ", Pos);
    Update.Package = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find(" - ", Pos);
    Update.Package = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find("\n", Pos);
    if(Pos2 == string::npos) Pos2 = Result.size();
    Update.To = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 1;
    pUpdates.push_back(Update);
  };  
  pthread_mutex_unlock(&pMutex);
}
void System::GetUpdatesCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  json Response = json::object();
  Response["LastCheck"] = pInstance->pLastUpdateCheck;
  Response["Updates"] = json::array();
  pthread_mutex_lock(&pMutex);
  for(uint32_t n = 0; n < pInstance->pUpdates.size(); n++) {
    json Update = json::object();
    Update["Package"] = pInstance->pUpdates[n].Package;
    Update["From"] = pInstance->pUpdates[n].From;
    Update["To"] = pInstance->pUpdates[n].To;
    Response["Updates"].push_back(Update);
  }
  req.FromJSON(Response);
}
void System::UpdateCallback(HTTPRequest &req) {

}
bool System::SendBoolBT(const char *property, bool value) {
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
bool System::SendUINT32BT(const char *property, uint32_t value) {
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
bool System::SendStringBT(const char *property, const char *value) {
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
string System::GetMacBT() {
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
void System::GetBluetoothCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  Config["Name"] = pInstance->pBTServerName;
  Config["Started"] = pInstance->pBTStarted;
  req.FromJSON(Config);
}
void System::SetBluetoothCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  bool Enabled = true;
  if(Params["Enabled"].is_boolean()) Enabled = Params["Enabled"];
  json Config;
  Config["Enabled"] = Enabled;
  Config["Bridge"] = pInstance->pBTBridgeName;
  ConfigManager::WriteConfig("/Bluetooth", Config);
  json Response;
  Response["Status"] = true;
  if(Enabled == true) Response["Status"] = pInstance->StartBT();
  if(Enabled == false) Response["Status"] = pInstance->StopBT();
  return req.FromJSON(Response); 
}
void System::ScanWIFICallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pMutex);
  json Result = json::array();
  string CommandStr = Run("ubus call iwinfo scan '{\"device\":\"" + pInstance->pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) {
      pthread_mutex_unlock(&pMutex);
      return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
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
  pthread_mutex_unlock(&pMutex);
  return req.FromJSON(Result);
}
void System::GetWIFICallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  if(!Params["Type"].is_string() || ((Params["Type"] != "CURRENT") && (Params["Type"] != "CONFIG"))) {
    return req.FromString("'Type' param is requiered to be 'CURRENT' or 'CONFIG'", MHD_HTTP_BAD_REQUEST);
  }
  pthread_mutex_lock(&pMutex);
  json Result = json::object();
  if(Params["Type"] == "CURRENT") {
    string CommandStr = Run("ubus call network.device status '{\"name\":\"" + pInstance->pInterfaceWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pMutex);
        return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
      }
      if(Tmp.is_object() && Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];
      if(Result["Enabled"] == false) {
        pthread_mutex_unlock(&pMutex);
        return req.FromJSON(Result);
      }
    }
    CommandStr = Run("ubus call network.interface." + pInstance->pNetworkWIFI + " status");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if (Tmp.is_discarded()) {
        pthread_mutex_unlock(&pMutex);
        return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
      }
      if(Tmp.is_object()) {
        if(Tmp["up"].is_boolean()) Result["Connected"] = Tmp["up"];
        if(Result["Connected"] == false) {
          pthread_mutex_unlock(&pMutex);
          return req.FromJSON(Result);
        }
        if(Tmp["dns-server"].is_array()) Result["DNS"] = Tmp["dns-server"];
        if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["IP"] = Tmp["ipv4-address"][0]["address"];
        if(Tmp["proto"].is_string()) Result["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
        if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Gateway"] = Tmp["route"][0]["nexthop"];
      }
    }
    CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInstance->pInterfaceWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pMutex);
        return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
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
    string CommandStr = Run("ubus call uci get '{\"config\":\"network\", \"section\":\"" + pInstance->pNetworkWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pMutex);
        return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
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
    CommandStr = Run("ubus call uci get '{\"config\":\"wireless\", \"section\":\"" + pInstance->pWirelessWIFI + "\"}'");
    if(CommandStr.empty() == false) {
      json Tmp = json::parse(CommandStr, NULL, false);
      if(Tmp.is_discarded()) {
        pthread_mutex_unlock(&pMutex);
        return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
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
  pthread_mutex_unlock(&pMutex);
  return req.FromJSON(Result);
}
void System::SetWIFICallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pMutex);
  json Result;
  Result["Status"] = true;
  int ErrorCode = 0;
  if(Params["Enabled"].is_boolean()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"enabled\":\"" + (Params["Enabled"] ? "1" : "0") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["IPMode"].is_string() && ((Params["IPMode"] == "DHCP") || (Params["IPMode"] == "STATIC"))) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"proto\":\"" + ((Params["IPMode"] == "DHCP") ? "dhcp" : "static") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["IP"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"ipaddr\":" + Params["IP"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["Netmask"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"netmask\":" + Params["Netmask"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["Gateway"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"gateway\":" + Params["Gateway"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["DNS"].is_array()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pInstance->pNetworkWIFI + "\", \"values\": {\"dns\":" + Params["DNS"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["ESSID"].is_string()) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pInstance->pWirelessWIFI + "\", \"values\": {\"ssid\":" + Params["ESSID"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["KeyMode"].is_string() && ((Params["KeyMode"] == "NONE") || (Params["KeyMode"] == "WEP") || (Params["KeyMode"] == "WPA") || (Params["KeyMode"] == "WPA2"))) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pInstance->pWirelessWIFI + "\", \"values\": {\"encryption\":\"" + ((Params["KeyMode"] == "NONE") ? "none" : (Params["KeyMode"] == "WEP") ? "wep" : (Params["KeyMode"] == "WPA") ? "psk" : "psk2") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  if(Params["Key"].is_string()) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pInstance->pWirelessWIFI + "\", \"values\": {\"key\":" + Params["Key"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
  }
  Run("uci commit && ubus call network reload", &ErrorCode);
  if(ErrorCode != 0) Result["Status"] = false;
  pthread_mutex_unlock(&pMutex);
  return req.FromJSON(Result);
}
void System::GetNTPCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pMutex);
  json Result = json::object();
  string CommandStr = Run("ubus call uci get '{\"config\":\"system\", \"section\":\"ntp\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) {
      pthread_mutex_unlock(&pMutex);
      return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
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
  pthread_mutex_unlock(&pMutex);
  return req.FromJSON(Result);
}
void System::SetNTPCallback(HTTPRequest& req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  pthread_mutex_lock(&pMutex);
  json Result;
  Result["Status"] = true;
  int ErrorCode = 0;
  if(Params["NTP_Enabled"].is_boolean() && Params["NTP_Server"].is_string()) {
    Run("ubus call uci set '{\"config\":\"system\", \"section\":\"ntp\", \"values\":{\"enabled\":\"" + string((Params["NTP_Enabled"] == true) ? "1" : "0") + "\", \"server\":" + Params["NTP_Server"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
    Run("uci commit", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
    Run("reload_config", &ErrorCode);
    if(ErrorCode != 0) Result["Status"] = false;
    if((Params["NTP_Enabled"] == false) && Params["Time"].is_number_unsigned()) {
      uint64_t Time = Params["Time"];
      timeval TimeVal;
      TimeVal.tv_sec  = Time / 1000;
      TimeVal.tv_usec = Time % 1000;
      if(settimeofday(&TimeVal, NULL) != 0) {
        Result["Status"] = false;
        LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
      }
    } 
  }
  pthread_mutex_unlock(&pMutex);
  return req.FromJSON(Result);
}
uint64_t System::ParseProcMem(const char *name, const char *buff) {
  const char *hit = strstr(buff, name);
  if(hit == NULL) return 0;
  errno = 0;
  long Val = strtol(hit + strlen(name), NULL, 10);
  if(errno != 0) return 0;
  return Val * 1024;
}
json System::GetSystemDetails() {
  struct sysinfo Info;
  struct statvfs Disk;
  pthread_mutex_lock(&pMutex);
  json Result = json::object();
  if(sysinfo(&Info) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return Result;
  }
  Result["UpTime"] = Info.uptime;
  Result["Swap_Total"] = (uint64_t)Info.totalswap * Info.mem_unit;
  Result["Swap_Usage"] = ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap;
  Result["CPU_Usage"] = ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0;
  if(statvfs("/", &Disk) != 0) {
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pthread_mutex_unlock(&pMutex);
    return Result;
  }
  Result["Disk_Total"] = (uint64_t)Disk.f_blocks * Disk.f_frsize;
  Result["Disk_Usage"] = ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks;
  char Buffer[8192];
  int TmpFile = open("/proc/meminfo", O_RDONLY);
  if(TmpFile > 0) {
    size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
    close(TmpFile);
    if(Len != 0) {
      Buffer[Len] = 0;
      Result["RAM_Total"] = ParseProcMem("MemTotal:", Buffer);
      Result["RAM_Usage"] = ((ParseProcMem("MemTotal:", Buffer) -  ParseProcMem("MemFree:", Buffer) - ParseProcMem("Cached:", Buffer) - ParseProcMem("Buffers:", Buffer)) * 100.0) / + ParseProcMem("MemTotal:", Buffer);
    }
  }
  TmpFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
  if(TmpFile > 0) {
    size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
    close(TmpFile);
    Buffer[Len] = 0;
    Result["Temperature"] = atoi(Buffer) / 1000.0;
  }
  string CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInstance->pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) {
      pthread_mutex_unlock(&pMutex);
      return Result;
    }
    if(Tmp.is_object()) {
      if(Tmp["ssid"].is_string()) Result["WIFI_Network"] = Tmp["ssid"];
      if(Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
        float Current = Tmp["quality"], Max = Tmp["quality_max"];
        Result["WIFI_Power"] = (int)(Current * 100.0 / Max);
      }
    }
  }
  pthread_mutex_unlock(&pMutex);
  return Result;
}
void System::PeriodicTask() {
  System::HandleAgentBT();
  if(pLastUpdateCheck < (Tools::GetTime() - 86400000)) {
    pLastUpdateCheck = Tools::GetTime();
    CheckUpdates();
  }
  //Comprobar actualizaciones
}
//----------------------------------------------------------------------------------------------------------------------
