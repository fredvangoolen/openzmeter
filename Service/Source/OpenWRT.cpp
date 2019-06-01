#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "OpenWRT.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Bluetooth *Bluetooth::pInstance   = NULL;
bool       Bluetooth::pRegistered = false;
//----------------------------------------------------------------------------------------------------------------------
Bluetooth::Bluetooth() {
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  pMutex        = PTHREAD_MUTEX_INITIALIZER;
//  pBridgeSocket = -1;
  pBridgeName = "br-bt";
  pServerName = "Stopped";
  pFailState    = false;
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
  if(Config["Enabled"].is_boolean()) Enabled = Config["Enabled"];
  if(Config["Bridge"].is_string()) pBridgeName = Config["Bridge"];
  json Save;
  Save["Enabled"] = Enabled;
  Save["Bridge"] = pBridgeName;
  ConfigManager::SaveConfig("/Bluetooth", Save);
  if(Enabled) Start();
  if(pRegistered == false) {
    HTTPServer::RegisterCallback("/getBluetooth", GetBluetoothCallback);
    HTTPServer::RegisterCallback("/setBluetooth", SetBluetoothCallback);
    HTTPServer::RegisterCallback("/GetWiFiNetworks", GetWiFiNetworksCallback);
    pRegistered = true;
  }
  pInstance = this;
}
Bluetooth::~Bluetooth() {
  pInstance = NULL;
  Stop();
  if(pDBus) dbus_connection_unref(pDBus);
}
bool Bluetooth::Start() {
  pthread_mutex_lock(&pMutex);
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
  pFailState = true;
  if(SendBool("Powered", false) == false) {
    pthread_mutex_unlock(&pMutex);
    return false; //Disable adapter
  }
  string MAC = GetMac();
  if(MAC.empty() == true) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  MAC.erase(std::remove(MAC.begin(), MAC.end(), ':'), MAC.end());
  pServerName = "openZmeter_" + MAC.substr(8);
  if(SendString("Alias", pServerName.c_str()) == false) {
    pthread_mutex_unlock(&pMutex);
    return false; //Set adapter name 
  }
  if(SendBool("Powered", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false; //enable adapter
  }
  if(SendBool("Discoverable", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendUINT32("DiscoverableTimeout", 0) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendBool("Discoverable", true) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  if(SendUINT32("DiscoverableTimeout", 0) == false) {
    pthread_mutex_unlock(&pMutex);
    return false;
  }
  //pBridgeSocket = socket(AF_INET, SOCK_DGRAM, 0);
  //if(pBridgeSocket < 0) {
  //  LOGFile::Error("Can not open control socket for bluetooth interface (%s)\n", strerror(errno));
  //  pthread_mutex_unlock(&pMutex);
  //  Stop();
  //  return false;
  //}
  //char BridgeReq[IFNAMSIZ];
  //unsigned long arg[3] = { BRCTL_ADD_BRIDGE, (unsigned long) BridgeReq };
  //strncpy(BridgeReq, pBridgeName.c_str(), IFNAMSIZ);
  //int Ret = ioctl(pBridgeSocket, SIOCSIFBR, arg);
  //if((Ret != 0) && (errno != EEXIST)){
  //  LOGFile::Error("Can not create bridge for bluetooth interface (%s)\n", strerror(errno));
  //  pthread_mutex_unlock(&pMutex);
  //  Stop();
  //  return false;
  //}
  //Set IP on interface
//  ifreq       InterfaceRequest;
//  sockaddr_in Address;
//  memset(&InterfaceRequest, 0, sizeof(struct ifreq));
//  memset(&Address, 0, sizeof(struct sockaddr));
//  strncpy(InterfaceRequest.ifr_name, "br0", IFNAMSIZ);
//  Address.sin_addr.s_addr = inet_addr("10.0.0.1");
//  Address.sin_family = AF_INET;
//  memcpy(&InterfaceRequest.ifr_addr, &Address, sizeof(struct sockaddr));
//  Ret = ioctl(pBridgeSocket, SIOCSIFADDR, (char *)&InterfaceRequest);
//  if(Ret != 0) {
//    LOGFile::Error("Can not set IP for bluetooth interface (%s)\n", strerror(errno));
//    pthread_mutex_unlock(&pMutex);
//    Stop();
//    return false;
//  }
//  //Bring up interface
//  Ret = ioctl(pBridgeSocket, SIOCGIFFLAGS, &InterfaceRequest);
//  if(Ret != 0) {
//    LOGFile::Error("Can not set IP for bluetooth interface (%s)\n", strerror(errno));
//    pthread_mutex_unlock(&pMutex);
//    Stop();
//    return false;
//  }
//  InterfaceRequest.ifr_flags |= IFF_UP | IFF_RUNNING;
//  Ret = ioctl(pBridgeSocket, SIOCSIFFLAGS, &InterfaceRequest);
//  if(Ret != 0) {
//    LOGFile::Error("Can not set IP for bluetooth interface (%s)\n", strerror(errno));
//    pthread_mutex_unlock(&pMutex);
//    Stop();
//    return false;
//  }
  Msg = dbus_message_new_method_call("org.bluez", "/org/bluez/hci0", "org.bluez.NetworkServer1", "Register");
  dbus_message_iter_init_append(Msg, &ArgsIter);
  const char *UUID = "nap";
  const char *Bridge = pBridgeName.c_str();
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &UUID);
  dbus_message_iter_append_basic(&ArgsIter, DBUS_TYPE_STRING, &Bridge);
  Reply = dbus_connection_send_with_reply_and_block(pDBus, Msg, DBUS_TIMEOUT_USE_DEFAULT, NULL);
  if(Reply) dbus_message_unref(Reply);
  dbus_message_unref(Msg);
  //Start DNSMASQ
//  string CaptiveOpt = "--dhcp-option-force=160,http://10.0.0.1";
//  json Config = ConfigManager::ReadConfig("/HTTPServer");
//  uint16_t Port = 8080;
//  if(Config.is_object()) {
//    if(Config["Port"].is_number_unsigned()) Port = Config["Port"];
//  }
//  if(Port != 80) CaptiveOpt += ":" + to_string(Port);
//  CaptiveOpt += "/";
//  pDNSMasqPID = fork();
//  if(pDNSMasqPID == 0) {
//    if(execlp("dnsmasq", "dnsmasq", "--keep-in-foreground", "--interface=br0", "--bind-interfaces", "--dhcp-range=10.0.0.2,10.0.0.254,1h", "--address=/#/10.0.0.1", CaptiveOpt.c_str(), NULL) == -1) {
//      LOGFile::Error("Can not start 'DNSMASQ' (%s)\n", strerror(errno));
//      exit(1);
//    }
//  }
  pthread_mutex_unlock(&pMutex);
  LOGFile::Info("Bluetooth interface UP\n");
  pFailState = false;
  return true;
}
bool Bluetooth::Stop() {
  pthread_mutex_lock(&pMutex);
  pFailState = false;
//  if(pDNSMasqPID != 0) kill(pDNSMasqPID, SIGTERM);
//  waitpid(pDNSMasqPID, NULL, 0);
//  pDNSMasqPID = 0;
  pServerName = "Stopped";
  if(SendBool("Powered", false) == false) {
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
  //Bring down interface
//  if(pBridgeSocket > 0) {
//    ifreq       InterfaceRequest;
//    strncpy(InterfaceRequest.ifr_name, "br0", IFNAMSIZ);
//    int Ret = ioctl(pBridgeSocket, SIOCGIFFLAGS, &InterfaceRequest);
//    if(Ret != 0) {
//      LOGFile::Error("Can not disable bridge in bluetooth interface (%s)\n", strerror(errno));
//      pthread_mutex_unlock(&pMutex);
//      return false;
//    }
//    InterfaceRequest.ifr_flags &= ~(IFF_UP);
//    Ret = ioctl(pBridgeSocket, SIOCSIFFLAGS, &InterfaceRequest);
//    if(Ret != 0) {
//      LOGFile::Error("Can not disable bridge in bluetooth interface (%s)\n", strerror(errno));
//      pthread_mutex_unlock(&pMutex);
//      return false;
//    }
//    char BridgeReq[IFNAMSIZ];
//    unsigned long arg[3] = { BRCTL_DEL_BRIDGE, (unsigned long) BridgeReq };
//    strncpy(BridgeReq, pBridgeName.c_str(), IFNAMSIZ);
//    Ret = ioctl(pBridgeSocket, SIOCSIFBR, arg);
//    if(Ret != 0) {
//      LOGFile::Error("Can not remove bridge for bluetooth interface (%s)\n", strerror(errno));
//      pthread_mutex_unlock(&pMutex);
//      return false;
//    }
//    close(pBridgeSocket);
//    pBridgeSocket = 0;
//  }
  pthread_mutex_unlock(&pMutex);
  LOGFile::Info("Bluetooth interface DOWN\n");
  return true;
}
void Bluetooth::HandleAgent() {
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
bool Bluetooth::SendBool(const char *property, bool value) {
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
bool Bluetooth::SendUINT32(const char *property, uint32_t value) {
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
bool Bluetooth::SendString(const char *property, const char *value) {
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
string Bluetooth::GetMac() {
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
void Bluetooth::GetBluetoothCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  json Config = ConfigManager::ReadConfig("/Bluetooth");
  Config["Name"] = pInstance->pServerName;
  Config["Fail"] = pInstance->pFailState;
  req.FromJSON(Config);
}
void Bluetooth::SetBluetoothCallback(HTTPRequest &req) {
  json Params = req.GetParams();
  //only 'admin' user can get this information
  if(req.UserName() != "admin") return req.FromString("", MHD_HTTP_UNAUTHORIZED);
  if(pInstance == NULL) return req.FromString("", MHD_HTTP_INTERNAL_SERVER_ERROR);
  pInstance->Stop();
  bool   Enabled = true;
  if(Params["Enabled"].is_boolean()) Enabled = Params["Enabled"];
  json Save;
  Save["Enabled"] = Enabled;
  ConfigManager::SaveConfig("/Bluetooth", Save);
  json Response;
  Response["Status"] = true;
  if(Enabled) Response["Status"] = pInstance->Start();
  return req.FromJSON(Response); 
}
void Bluetooth::GetWiFiNetworksCallback(HTTPRequest& req) {

}