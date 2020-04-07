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
queue<string>    SysSupport::pSystemActions;
pthread_mutex_t  SysSupport::pSystemActionMutex  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t   SysSupport::pSystemActionCond   = PTHREAD_COND_INITIALIZER;
pthread_t        SysSupport::pSystemActionThread = 0;
bool             SysSupport::pTerminate          = false;
RWLocks          SysSupport::pSysStatLock        = RWLocks();
bool             SysSupport::pBT_Enabled         = false;
uint64_t         SysSupport::pUpTime             = 0;
uint64_t         SysSupport::pSwap_Total         = 0;
float            SysSupport::pSwap_Usage         = NAN;
float            SysSupport::pCPU_Usage          = NAN;
uint64_t         SysSupport::pDisk_Total         = 0;
float            SysSupport::pDisk_Usage         = NAN;
uint64_t         SysSupport::pRAM_Total          = 0;
float            SysSupport::pRAM_Usage          = NAN;
float            SysSupport::pTemperature        = NAN;
string           SysSupport::pWIFI_Network       = "";
float            SysSupport::pWIFI_Power         = NAN;
bool             SysSupport::pTimeSet            = false;
#ifdef OPENWRT
bool             SysSupport::pBTEnable      = false;
string           SysSupport::pBTBridgeName  = "br-bt";
string           SysSupport::pBTServerName  = "";
pthread_t        SysSupport::pBTThread      = 0;
pthread_t        SysSupport::pStatsThread   = 0;
string           SysSupport::pInterfaceWIFI = "wlan0";;
string           SysSupport::pNetworkWIFI   = "lan";
string           SysSupport::pWirelessWIFI  = "default_radio";

//  vector<SysSupport::Update_t> SysSupport::pUpdates;
//  RWLocks                      SysSupport::pUpdatesLock;
//  uint64_t                     SysSupport::pLastTask_1H = 0;
//  uint64_t                     SysSupport::pLastTask_1M = 0;
//  uint64_t                     SysSupport::pLastTask_5S = 0;
#endif
//----------------------------------------------------------------------------------------------------------------------
SysSupport::SysSupport() {
  HTTPServer::RegisterCallback("getVersion",      GetVersionCallback);
  HTTPServer::RegisterCallback("getUsers",        GetUsersCallback);
  HTTPServer::RegisterCallback("getUser",         GetUserCallback);
  HTTPServer::RegisterCallback("setUser",         SetUserCallback);
  HTTPServer::RegisterCallback("setUserPassword", SetUserPasswordCallback);
  HTTPServer::RegisterCallback("addUser",         AddUserCallback);
  HTTPServer::RegisterCallback("delUser",         DelUserCallback);
  pTerminate = false;
  if(pthread_create(&pSystemActionThread, NULL, Actions_Thread, NULL) != 0) {
    pSystemActionThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pSystemActionThread, "SysActions");
  }
  #ifdef OPENWRT
  if(pthread_create(&pStatsThread, NULL, SysStat_Thread, NULL) != 0) {
    pStatsThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pStatsThread, "SysStats");
  }
  HTTPServer::RegisterCallback("getBluetooth", GetBluetoothCallback);
  HTTPServer::RegisterCallback("setBluetooth", SetBluetoothCallback);
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  if(Config.is_object() == true) {
    if(Config["Enabled"].is_boolean()) pBTEnable = Config["Enabled"];
    if(Config["Bridge"].is_string()) pBTBridgeName = Config["Bridge"];
  }
  Config = json::object();
  Config["Enabled"] = pBTEnable;
  Config["Bridge"] = pBTBridgeName;
  ConfigManager::WriteConfig("/Bluetooth", Config);
  if(pthread_create(&pBTThread, NULL, Bluetooth_Thread, NULL) != 0) {
    pBTThread = 0;
    LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
  } else {
    pthread_setname_np(pBTThread, "Bluetooth");
  }
  HTTPServer::RegisterCallback("scanWIFI",     ScanWIFICallback);
  HTTPServer::RegisterCallback("getWIFI",      GetWIFICallback);
  HTTPServer::RegisterCallback("setWIFI",      SetWIFICallback);
  HTTPServer::RegisterCallback("getNTP",       GetNTPCallback);
  HTTPServer::RegisterCallback("setNTP",       SetNTPCallback);
  Config = ConfigManager::ReadConfig("/WIFI");
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
  HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
  HTTPServer::RegisterCallback("setUpgrade",   SetUpgradeCallback);
  #endif
}
SysSupport::~SysSupport() {
  LOGFile::Debug("Stopping SysSupport\n");
  HTTPServer::UnregisterCallback("getVersion");
  HTTPServer::UnregisterCallback("getUsers");
  HTTPServer::UnregisterCallback("getUser");
  HTTPServer::UnregisterCallback("setUser");
  HTTPServer::UnregisterCallback("setUserPassword");
  HTTPServer::UnregisterCallback("addUser");
  HTTPServer::UnregisterCallback("delUser");
  pTerminate = true;
  pthread_cond_signal(&pSystemActionCond);
  if(pSystemActionThread != 0) pthread_join(pSystemActionThread, NULL);
  pSystemActionThread = 0;
  #ifdef OPENWRT
  HTTPServer::UnregisterCallback("getBluetooth");
  HTTPServer::UnregisterCallback("setBluetooth");
  HTTPServer::UnregisterCallback("scanWIFI");
  HTTPServer::UnregisterCallback("getWIFI");
  HTTPServer::UnregisterCallback("setWIFI");
  HTTPServer::UnregisterCallback("getNTP");
  HTTPServer::UnregisterCallback("setNTP");
  HTTPServer::UnregisterCallback("getUpdates");
  HTTPServer::UnregisterCallback("setUpgrade");
  if(pStatsThread != 0) pthread_join(pStatsThread, NULL);
  if(pBTThread != 0) pthread_join(pBTThread, NULL);
  pStatsThread = 0;
  pBTThread = 0;
  #endif
  LOGFile::Debug("SysSupport stopped\n");
}
json SysSupport::GetSystemStatusJSON() {
  json Result = json::object();
  pSysStatLock.ReadLock();
  Result["UpTime"] = pUpTime;
  Result["Swap_Total"] = pSwap_Total;
  Result["Swap_Usage"] = pSwap_Usage;
  Result["CPU_Usage"] = pCPU_Usage;
  Result["Disk_Total"] = pDisk_Total;
  Result["Disk_Usage"] = pDisk_Usage;
  Result["RAM_Total"] = pRAM_Total;
  Result["RAM_Usage"] = pRAM_Usage;
  Result["Temperature"] = pTemperature;
  Result["WIFI_Network"] = pWIFI_Network;
  Result["WIFI_Power"] = pWIFI_Power;
  Result["BT_Enabled"] = pBT_Enabled;
  Result["TimeSet"] = pTimeSet;
//  Result["Battery_Volt"] = pSystemStatus.Battery_Volt;
//  Result["Battery_Level"] = pSystemStatus.Battery_Level;
//  Result["Updates"] = pSystemStatus.Updates;
//  Result["Events"] = pSystemStatus.Events;
  pSysStatLock.Unlock();
  return Result;
}
void SysSupport::AddSystemAction(const string &action) {
  pthread_mutex_lock(&pSystemActionMutex);
  pSystemActions.push(action);
  pthread_cond_signal(&pSystemActionCond);
  pthread_mutex_unlock(&pSystemActionMutex);
}
void *SysSupport::Actions_Thread(void *params) {
  nice(5);
  while(true) {
    pthread_mutex_lock(&pSystemActionMutex);
    if(pSystemActions.size() == 0) pthread_cond_wait(&pSystemActionCond, &pSystemActionMutex);
    if(pTerminate == true) {
      pthread_mutex_unlock(&pSystemActionMutex);
      return NULL;
    }
    string Action = "";
    if(pSystemActions.size() > 0) {
      Action = pSystemActions.front();
      pSystemActions.pop();
    }
    pthread_mutex_unlock(&pSystemActionMutex);
    #ifdef OPENWRT
    if(Action != "") LOGFile::Info("Received '%s' action\n", Action.c_str());
    if(Action == "PowerOff") {
      Run("poweroff");
    } else if(Action == "ToogleBluetooth") {
      json Config = json::object();
      pSysStatLock.WriteLock();
      pBTEnable = !pBTEnable;
      Config["Enabled"] = pBTEnable;
      Config["Bridge"] = pBTBridgeName;
      pSysStatLock.Unlock();
      ConfigManager::WriteConfig("/Bluetooth", Config);
    }
    #endif
  }
}
#ifdef OPENWRT
void *SysSupport::Bluetooth_Thread(void *params) {
  DBusConnection *DBus = NULL;
  nice(5);
  while(true) {
    pSysStatLock.ReadLock();
    bool   PrevEnable = pBT_Enabled;
    bool   Enable     = (pTerminate == false) ? pBTEnable : false;
    pSysStatLock.Unlock();
    if((pTerminate == true) && (PrevEnable == false)) break;
    if(DBus == NULL) {
      DBusError Error;
      dbus_error_init(&Error);
      DBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
      if(dbus_error_is_set(&Error)) {
        LOGFile::Error("Can not open DBus connection (%s)\n", Error.message);
        dbus_error_free(&Error);
        usleep(1000000);
        continue;
      }
    }
    if(dbus_connection_read_write(DBus, 0) == false) {
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
        LOGFile::Info("Authorized BT client: '%s'\n", Param);
        DBusMessage *Reply = dbus_message_new_method_return(Msg);
        dbus_connection_send(DBus, Reply, &Serial);
        dbus_connection_flush(DBus);
        dbus_message_unref(Reply);
      }
      dbus_message_unref(Msg);
    } else if((PrevEnable == false) && (Enable == true)) {
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
      const char *Bridge = pBTBridgeName.c_str();
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
      dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Bridge);
      Reply = dbus_connection_send_with_reply_and_block(DBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
      if(Reply) dbus_message_unref(Reply);
      dbus_message_unref(Msg);
      LOGFile::Info("Bluetooth interface UP\n");
      pSysStatLock.WriteLock();
      pBTServerName = NetName;
      pBT_Enabled = true;
      pSysStatLock.Unlock();
    } else if((PrevEnable == true) && (Enable == false)) {
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
      LOGFile::Info("Bluetooth interface DOWN\n");
      pSysStatLock.WriteLock();
      pBTServerName = "";
      pBT_Enabled = false;
      pSysStatLock.Unlock();
    } else {
      usleep(100000);
    }
  }
  if(DBus != NULL) dbus_connection_unref(DBus);
  return NULL;
}
void *SysSupport::SysStat_Thread(void *params) {
  nice(15);
  uint64_t NextUpdate = Tools::GetTime() + 5000;
  while(pTerminate == false) {
    usleep(100000);
    if(Tools::GetTime() < NextUpdate) continue;
    NextUpdate = Tools::GetTime() + 5000;
    struct sysinfo Info;
    struct statvfs Disk;
    int Error1 = sysinfo(&Info);
    if(Error1 != 0) LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    int Error2 = statvfs("/", &Disk);
    if(Error2 != 0) LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    pSysStatLock.WriteLock();
    pUpTime = (Error1 != 0) ? 0 : Info.uptime;
    pSwap_Total = (Error1 != 0) ? 0 : (uint64_t)Info.totalswap * Info.mem_unit;
    pSwap_Usage = (Error1 != 0) ? NAN : ((Info.totalswap - Info.freeswap) * 100.0) / Info.totalswap;
    pCPU_Usage = (Error1 != 0) ? NAN : ((Info.loads[0] / (double)(1 << SI_LOAD_SHIFT)) / get_nprocs()) * 100.0;
    pDisk_Total = (Error2 != 0) ? 0 : (uint64_t)Disk.f_blocks * Disk.f_frsize;
    pDisk_Usage = (Error2 != 0) ? NAN : ((Disk.f_blocks - Disk.f_bfree) * 100.0) / Disk.f_blocks;
    pSysStatLock.Unlock();
    char Buffer[8192];
    int TmpFile = open("/proc/meminfo", O_RDONLY);
    Buffer[0] = 0;
    if(TmpFile > 0) {
      size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
      close(TmpFile);
      Buffer[Len] = 0;
    }
    pSysStatLock.WriteLock();
    pRAM_Total = ParseProcMem("MemTotal:", Buffer);
    pRAM_Usage = ((ParseProcMem("MemTotal:", Buffer) -  ParseProcMem("MemFree:", Buffer) - ParseProcMem("Cached:", Buffer) - ParseProcMem("Buffers:", Buffer)) * 100.0) / ParseProcMem("MemTotal:", Buffer);
    pSysStatLock.Unlock();

    TmpFile = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
    Buffer[0] = 0;
    if(TmpFile > 0) {
      size_t Len = read(TmpFile, Buffer, sizeof(Buffer));
      close(TmpFile);
      Buffer[Len] = 0;
    }
    pSysStatLock.WriteLock();
    pTemperature = atoi(Buffer) / 1000.0;
    pSysStatLock.Unlock();

    string CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInterfaceWIFI + "\"}'");
    json Tmp = nullptr;
    if(CommandStr.empty() == false) {
      Tmp = json::parse(CommandStr, NULL, false);
      if((Tmp.is_discarded() == true) || (Tmp.is_object() == false) || (Tmp["ssid"].is_string() == false) || (Tmp["quality"].is_number() == false) || (Tmp["quality_max"].is_number() == false)) Tmp = nullptr;
    }
    pSysStatLock.WriteLock();
    if(Tmp == nullptr) {
      pWIFI_Network = "";
      pWIFI_Power = NAN;
    } else {
      pWIFI_Network =  Tmp["ssid"];
      float Current = Tmp["quality"], Max = Tmp["quality_max"];
      pWIFI_Power = Current * 100.0 / Max;
    }
    pTimeSet = (Tools::GetTime() > 946684800000);
    pSysStatLock.Unlock();
  }
  return NULL;
}
string SysSupport::GetMacBT(DBusConnection *bus) {
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
bool SysSupport::SendBoolBT(DBusConnection *bus, const char *property, bool value) {
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
bool SysSupport::SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value) {
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
bool SysSupport::SendStringBT(DBusConnection *bus, const char *property, const char *value) {
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
uint64_t SysSupport::ParseProcMem(const char *name, const char *buff) {
  const char *hit = strstr(buff, name);
  if(hit == NULL) return 0;
  errno = 0;
  long Val = strtol(hit + strlen(name), NULL, 10);
  if(errno != 0) return 0;
  return Val * 1024;
}
#endif
//----------------------------------------------------------------------------------------------------------------------
void SysSupport::GetVersionCallback(const json &params, Response &response) {
  json Result = json::object();
  Result["Version"] = __BUILD__;
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
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
  bool Result = DB.ExecSentenceNoResult("DELETE FROM users WHERE username = " + DB.Bind(User));
  HTTPServer::FillResponse(response, "", "text/plain", (Result == false) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
#ifdef OPENWRT
void SysSupport::GetUpdatesCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  int ResultCode = 0;
  if(params.contains("Force") && params["Force"].is_boolean() && (params["Force"] == true)) {
    Run("opkg update", &ResultCode);
    if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  string Result = Run("opkg list-upgradable", &ResultCode);
  if(ResultCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  json Response = json::array();
  size_t Pos = 0;
  while(Pos < Result.size()) {
    json Update = json::object();
    size_t Pos2 = Result.find(" - ", Pos);
    Update["Package"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find(" - ", Pos);
    Update["From"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 3;
    Pos2 = Result.find("\n", Pos);
    if(Pos2 == string::npos) Pos2 = Result.size();
    Update["To"] = Result.substr(Pos, Pos2 - Pos);
    Pos = Pos2 + 1;
    Response.push_back(Update);
  }
  HTTPServer::FillResponse(response, Response.dump(), "application/json", HTTP_OK);
}
void SysSupport::SetUpgradeCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Package") == false) || (params["Package"].is_string() == false)) return HTTPServer::FillResponse(response, "'Package' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Package = params["Package"];
  int ResultCode = 0;
  string Result = Run("opkg upgrade --force-depends " + Package, &ResultCode);
  HTTPServer::FillResponse(response, "", "text/plain", (ResultCode == 0) ? HTTP_OK : HTTP_INTERNAL_SERVER_ERROR);
}
void SysSupport::GetBluetoothCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  pSysStatLock.ReadLock();
  Config["Enabled"] = pBTEnable;
  Config["Name"] = pBTServerName;
  pSysStatLock.Unlock();
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void SysSupport::SetBluetoothCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  json Config = json::object();
  pSysStatLock.WriteLock();
  pBTEnable = params["Enabled"];
  Config["Enabled"] = pBTEnable;
  Config["Bridge"] = pBTBridgeName;
  pSysStatLock.Unlock();
  ConfigManager::WriteConfig("/Bluetooth", Config);
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void SysSupport::ScanWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::array();
  string CommandStr = Run("ubus call iwinfo scan '{\"device\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if((Tmp.is_discarded() == true) || (Tmp.is_object() == false) || (Tmp["results"].is_array() == false)) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
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
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void SysSupport::GetWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  string CommandStr = Run("ubus call network.device status '{\"name\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object() && Tmp["up"].is_boolean()) Result["Enabled"] = Tmp["up"];
    if(Result["Enabled"] == false) return HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
  }
  CommandStr = Run("ubus call uci get '{\"config\":\"network\", \"section\":\"" + pNetworkWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["values"].is_object()) {
        Result["IPMode"] = "DHCP";
        if(Tmp["values"]["proto"].is_string()) Result["IPMode"] = (Tmp["values"]["proto"] == "static") ? "STATIC" : "DHCP";
        Result["IP"] = "";
        if(Tmp["values"]["ipaddr"].is_string()) Result["IP"] = Tmp["values"]["ipaddr"];
        Result["Netmask"] = "";
        if(Tmp["values"]["netmask"].is_string()) Result["Netmask"] = Tmp["values"]["netmask"];
        Result["Gateway"] = "";
        if(Tmp["values"]["gateway"].is_string()) Result["Gateway"] = Tmp["values"]["gateway"];
        Result["DNS"] = json::array();
        if(Tmp["values"]["dns"].is_array()) Result["DNS"] = Tmp["values"]["dns"];
      }
    }
  }
  CommandStr = Run("ubus call uci get '{\"config\":\"wireless\", \"section\":\"" + pWirelessWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
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
  Result["Status"] = json::object();
  CommandStr = Run("ubus call network.interface." + pNetworkWIFI + " status");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      Result["Status"]["Connected"] = false;
      if(Tmp["up"].is_boolean()) Result["Status"]["Connected"] = Tmp["up"];
      if(Result["Status"]["Connected"] == true) {
        if(Tmp["dns-server"].is_array()) Result["Status"]["DNS"] = Tmp["dns-server"];
        if(Tmp["ipv4-address"].is_array() && Tmp["ipv4-address"][0].is_object() && Tmp["ipv4-address"][0]["address"].is_string()) Result["Status"]["IP"] = Tmp["ipv4-address"][0]["address"];
        if(Tmp["proto"].is_string()) Result["Status"]["IPMode"] = (Tmp["proto"] == "dhcp") ? "DHCP" : "STATIC";
        if(Tmp["route"].is_array() && Tmp["route"][0].is_object() && Tmp["route"][0]["nexthop"].is_string()) Result["Status"]["Gateway"] = Tmp["route"][0]["nexthop"];
      }
    }
  }
  CommandStr = Run("ubus call iwinfo info '{\"device\":\"" + pInterfaceWIFI + "\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
    if(Tmp.is_object()) {
      if(Tmp["ssid"].is_string()) Result["Status"]["ESSID"] = Tmp["ssid"];
      if(Tmp["channel"].is_number()) Result["Status"]["Channel"] = Tmp["channel"];
      if(Tmp["quality"].is_number() && Tmp["quality_max"].is_number()) {
        float Current = Tmp["quality"], Max = Tmp["quality_max"];
        Result["Status"]["Quality"] = (int) (Current * 100.0 / Max);
      }
    }
  }
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void SysSupport::SetWIFICallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  int ErrorCode = 0;
  if(params.contains("Enabled") && params["Enabled"].is_boolean()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"enabled\":\"" + (params["Enabled"] ? "1" : "0") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("IPMode") && params["IPMode"].is_string() && ((params["IPMode"] == "DHCP") || (params["IPMode"] == "STATIC"))) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"proto\":\"" + ((params["IPMode"] == "DHCP") ? "dhcp" : "static") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("IP") && params["IP"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"ipaddr\":" + params["IP"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Netmask") && params["Netmask"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"netmask\":" + params["Netmask"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Gateway") && params["Gateway"].is_string()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"gateway\":" + params["Gateway"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("DNS") && params["DNS"].is_array()) {
    Run("ubus call uci set '{\"config\":\"network\",\"section\":\"" + pNetworkWIFI + "\", \"values\": {\"dns\":" + params["DNS"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("ESSID") && params["ESSID"].is_string()) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"ssid\":" + params["ESSID"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("KeyMode") && params["KeyMode"].is_string() && ((params["KeyMode"] == "NONE") || (params["KeyMode"] == "WEP") || (params["KeyMode"] == "WPA") || (params["KeyMode"] == "WPA2"))) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"encryption\":\"" + ((params["KeyMode"] == "NONE") ? "none" : (params["KeyMode"] == "WEP") ? "wep" : (params["KeyMode"] == "WPA") ? "psk" : "psk2") + "\"}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  if(params.contains("Key") && params["Key"].is_string()) {
    Run("ubus call uci set '{\"config\":\"wireless\",\"section\":\"" + pWirelessWIFI + "\", \"values\": {\"key\":" + params["Key"].dump() + "}}'", &ErrorCode);
    if(ErrorCode != 0) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
  }
  Run("uci commit && ubus call network reload", &ErrorCode);
  return HTTPServer::FillResponse(response, "", "text/plain", (ErrorCode != 0) ? HTTP_INTERNAL_SERVER_ERROR : HTTP_OK);
}
void SysSupport::GetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Result = json::object();
  string CommandStr = Run("ubus call uci get '{\"config\":\"system\", \"section\":\"ntp\"}'");
  if(CommandStr.empty() == false) {
    json Tmp = json::parse(CommandStr, NULL, false);
    if(Tmp.is_discarded()) return HTTPServer::FillResponse(response, "", "text/plain", HTTP_INTERNAL_SERVER_ERROR);
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
  HTTPServer::FillResponse(response, Result.dump(), "application/json", HTTP_OK);
}
void SysSupport::SetNTPCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
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
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
#endif
//void SysSupport::PeriodicTask() {
//  string Action = "";
//  pActionsLock.WriteLock();
//  if(pSystemActions.size() > 0) {
//    Action = pSystemActions.front();
//    pSystemActions.pop();
//  }
//  pActionsLock.Unlock();
//  if(Action.empty() == false) LOGFile::Debug("Action '%s' picked from list\n", Action.c_str());
//  #ifdef OPENWRT
//    HandleAgentBT();
//    uint64_t Now = Tools::GetTime();
//    if((Now > 60000) && (pLastTask_1M < (Now - 60000))) PeriodicTask_1M();
//    if((Now > 86400000) && (pLastTask_1H < (Now - 86400000))) PeriodicTask_1H();
//    if(Action == "PowerOff") {
//      Run("poweroff");
//    } else if(Action == "ToogleBluetooth") {
//      pthread_mutex_lock(&pBTMutex);
//      json Config = json::object();
//      Config["Enabled"] = !pBTEnabled;
//      pthread_mutex_unlock(&pBTMutex);
//      ConfigManager::WriteConfig("/Bluetooth", Config);
//      if(pBTEnabled == true) {
//        StopBT();
//      } else {
//        StartBT();
//      }
//    }
//  #else
//    if(Action.empty() == false) LOGFile::Info("Received '%s' action\n", Action.c_str());
//  #endif
//}
//----------------------------------------------------------------------------------------------------------------------
