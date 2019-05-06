#include <sys/socket.h>
#include <linux/if.h>
#include <linux/if_bridge.h>
#include <sys/ioctl.h>
#include "Bluetooth.h"
#include "Tools.h"
//----------------------------------------------------------------------------------------------------------------------
Bluetooth::Bluetooth() {
  pDBus = NULL;
  DBusError Error;
  dbus_error_init(&Error);
  pDBus = dbus_bus_get(DBUS_BUS_SYSTEM, &Error);
  if(dbus_error_is_set(&Error)) {
    LOGFile::Error("Can not open DBus connection (%s)\n", Error.message);
    dbus_error_free(&Error);
    pDBus = NULL;
  }
  SetEnabled(false);
  string MAC = GetMac();
  MAC.erase(std::remove(MAC.begin(), MAC.end(), ':'), MAC.end());
  SetName("openZmeter_" + MAC.substr(8));
  SetEnabled(true);
  SetDiscoverable(true, 0);
  SetPairable(true, 0);
  AddBridge("br0");
  exit(1);
}
Bluetooth::~Bluetooth() {
}
bool Bluetooth::AddBridge(const char *name) {
  pBridgeSocket = socket(AF_LOCAL, SOCK_STREAM, 0);
  if(pBridgeSocket < 0) {
    return false;
  }
  char _br[IFNAMSIZ];
  unsigned long arg[3] = { BRCTL_ADD_BRIDGE, (unsigned long) _br };
  strncpy(_br, name, IFNAMSIZ);
  int Ret = ioctl(pBridgeSocket, SIOCSIFBR, arg);
  if(Ret != 0) {
    LOGFile::Error("Can not create bridge for bluetooth interface (%s)\n", strerror(errno));
  }
  return (Ret == 0);
}


void Bluetooth::SendBool(const char *property, bool value) {
  if(pDBus == NULL) return;
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
}
void Bluetooth::SendUINT32(const char *property, uint32_t value) {
  if(pDBus == NULL) return;
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
}
void Bluetooth::SendString(const char *property, const char *value) {
  if(pDBus == NULL) return;
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
}
void Bluetooth::SetDiscoverable(bool value, uint32_t timeout) {
  SendBool("Discoverable", value);
  SendUINT32("DiscoverableTimeout", timeout);
}
void Bluetooth::SetPairable(bool value, uint32_t timeout) {
  SendBool("Discoverable", value);
  SendUINT32("DiscoverableTimeout", timeout);
}
void Bluetooth::SetEnabled(bool value) {
  SendBool("Powered", value);
}
void Bluetooth::SetName(string value) {
  SendString("Alias", value.c_str());
}
string Bluetooth::GetMac() {
  string Result = "";
  if(pDBus == NULL) return Result;
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
