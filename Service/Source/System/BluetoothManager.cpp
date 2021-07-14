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
#include "TaskPool.h"
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
    if(pthread_setname_np(pThread, "Bluetooth") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    TaskPool::SetThreadProperties(pThread, TaskPool::LOW);
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
  if(!req.CheckParamBool("Full")) return;
  json Config = ConfigFile::ReadConfig("/Bluetooth");
  if(req.Param("Full")) {
    if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    Config["Name"] = pServerName;
    Config["Running"] = pEnabled;
    if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    req.Success(Config);
  } else {
    req.Success({{"Enabled", Config["Enabled"]}});
  }
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



#include "improv.h"








namespace esphome {
namespace esp32_improv {

static const char *const TAG = "esp32_improv.component";

ESP32ImprovComponent::ESP32ImprovComponent() { global_improv_component = this; }

void ESP32ImprovComponent::setup() {
  this->service_ = global_ble_server->create_service(improv::SERVICE_UUID, true);
  this->setup_characteristics();
}

void ESP32ImprovComponent::setup_characteristics() {
  this->status_ = this->service_->create_characteristic(
      improv::STATUS_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor *status_descriptor = new BLE2902();
  this->status_->add_descriptor(status_descriptor);

  this->error_ = this->service_->create_characteristic(
      improv::ERROR_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor *error_descriptor = new BLE2902();
  this->error_->add_descriptor(error_descriptor);

  this->rpc_ = this->service_->create_characteristic(improv::RPC_COMMAND_UUID, BLECharacteristic::PROPERTY_WRITE);
  this->rpc_->on_write([this](const std::vector<uint8_t> &data) {
    if (data.size() > 0) {
      this->incoming_data_.insert(this->incoming_data_.end(), data.begin(), data.end());
    }
  });
  BLEDescriptor *rpc_descriptor = new BLE2902();
  this->rpc_->add_descriptor(rpc_descriptor);

  this->rpc_response_ = this->service_->create_characteristic(
      improv::RPC_RESULT_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  BLEDescriptor *rpc_response_descriptor = new BLE2902();
  this->rpc_response_->add_descriptor(rpc_response_descriptor);

  this->capabilities_ =
      this->service_->create_characteristic(improv::CAPABILITIES_UUID, BLECharacteristic::PROPERTY_READ);
  BLEDescriptor *capabilities_descriptor = new BLE2902();
  this->capabilities_->add_descriptor(capabilities_descriptor);
  uint8_t capabilities = 0x00;
  if (this->status_indicator_ != nullptr)
    capabilities |= improv::CAPABILITY_IDENTIFY;
  this->capabilities_->set_value(capabilities);
  this->setup_complete_ = true;
}

void ESP32ImprovComponent::loop() {
  if (this->incoming_data_.size() > 0)
    this->process_incoming_data_();
  uint32_t now = millis();

  switch (this->state_) {
    case improv::STATE_STOPPED:
      if (this->status_indicator_ != nullptr)
        this->status_indicator_->turn_off();

      if (this->service_->is_created() && this->should_start_ && this->setup_complete_) {
        if (this->service_->is_running()) {
          esp32_ble::global_ble->get_advertising()->start();

          this->set_state_(improv::STATE_AWAITING_AUTHORIZATION);
          this->set_error_(improv::ERROR_NONE);
          this->should_start_ = false;
          ESP_LOGD(TAG, "Service started!");
        } else {
          this->service_->start();
        }
      }
      break;
    case improv::STATE_AWAITING_AUTHORIZATION: {
      if (this->authorizer_ == nullptr || this->authorizer_->state) {
        this->set_state_(improv::STATE_AUTHORIZED);
        this->authorized_start_ = now;
      } else {
        if (this->status_indicator_ != nullptr) {
          if (!this->check_identify_())
            this->status_indicator_->turn_on();
        }
      }
      break;
    }
    case improv::STATE_AUTHORIZED: {
      if (this->authorizer_ != nullptr) {
        if (now - this->authorized_start_ > this->authorized_duration_) {
          ESP_LOGD(TAG, "Authorization timeout");
          this->set_state_(improv::STATE_AWAITING_AUTHORIZATION);
          return;
        }
      }
      if (this->status_indicator_ != nullptr) {
        if (!this->check_identify_()) {
          if ((now % 1000) < 500) {
            this->status_indicator_->turn_on();
          } else {
            this->status_indicator_->turn_off();
          }
        }
      }
      break;
    }
    case improv::STATE_PROVISIONING: {
      if (this->status_indicator_ != nullptr) {
        if ((now % 200) < 100) {
          this->status_indicator_->turn_on();
        } else {
          this->status_indicator_->turn_off();
        }
      }
      if (wifi::global_wifi_component->is_connected()) {
        wifi::global_wifi_component->save_wifi_sta(this->connecting_sta_.get_ssid(),
                                                   this->connecting_sta_.get_password());
        this->connecting_sta_ = {};
        this->cancel_timeout("wifi-connect-timeout");
        this->set_state_(improv::STATE_PROVISIONED);

        std::string url = "https://my.home-assistant.io/redirect/config_flow_start?domain=esphome";
        std::vector<uint8_t> data = improv::build_rpc_response(improv::WIFI_SETTINGS, {url});
        this->send_response(data);
        this->set_timeout("end-service", 1000, [this] {
          this->service_->stop();
          this->set_state_(improv::STATE_STOPPED);
        });
      }
      break;
    }
    case improv::STATE_PROVISIONED: {
      this->incoming_data_.clear();
      if (this->status_indicator_ != nullptr)
        this->status_indicator_->turn_off();
      break;
    }
  }
}

bool ESP32ImprovComponent::check_identify_() {
  uint32_t now = millis();

  bool identify = this->identify_start_ != 0 && now - this->identify_start_ <= this->identify_duration_;

  if (identify) {
    uint32_t time = now % 1000;
    if (time < 600 && time % 200 < 100) {
      this->status_indicator_->turn_on();
    } else {
      this->status_indicator_->turn_off();
    }
  }
  return identify;
}

void ESP32ImprovComponent::set_state_(improv::State state) {
  ESP_LOGV(TAG, "Setting state: %d", state);
  this->state_ = state;
  if (this->status_->get_value().size() == 0 || this->status_->get_value()[0] != state) {
    uint8_t data[1]{state};
    this->status_->set_value(data, 1);
    if (state != improv::STATE_STOPPED)
      this->status_->notify();
  }
}

void ESP32ImprovComponent::set_error_(improv::Error error) {
  if (error != improv::ERROR_NONE)
    ESP_LOGE(TAG, "Error: %d", error);
  if (this->error_->get_value().size() == 0 || this->error_->get_value()[0] != error) {
    uint8_t data[1]{error};
    this->error_->set_value(data, 1);
    if (this->state_ != improv::STATE_STOPPED)
      this->error_->notify();
  }
}

void ESP32ImprovComponent::send_response(std::vector<uint8_t> &response) {
  this->rpc_response_->set_value(response);
  if (this->state_ != improv::STATE_STOPPED)
    this->rpc_response_->notify();
}

void ESP32ImprovComponent::start() {
  if (this->state_ != improv::STATE_STOPPED)
    return;

  ESP_LOGD(TAG, "Setting Improv to start");
  this->should_start_ = true;
}

void ESP32ImprovComponent::stop() {
  this->set_timeout("end-service", 1000, [this] {
    this->service_->stop();
    this->set_state_(improv::STATE_STOPPED);
  });
}

float ESP32ImprovComponent::get_setup_priority() const { return setup_priority::AFTER_BLUETOOTH; }

void ESP32ImprovComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "ESP32 Improv:");
  LOG_BINARY_SENSOR("  ", "Authorizer", this->authorizer_);
  ESP_LOGCONFIG(TAG, "  Status Indicator: '%s'", YESNO(this->status_indicator_ != nullptr));
}

void ESP32ImprovComponent::process_incoming_data_() {
  uint8_t length = this->incoming_data_[1];

  ESP_LOGD(TAG, "Processing bytes - %s", hexencode(this->incoming_data_).c_str());
  if (this->incoming_data_.size() - 3 == length) {
    this->set_error_(improv::ERROR_NONE);
    improv::ImprovCommand command = improv::parse_improv_data(this->incoming_data_);
    switch (command.command) {
      case improv::BAD_CHECKSUM:
        ESP_LOGW(TAG, "Error decoding Improv payload");
        this->set_error_(improv::ERROR_INVALID_RPC);
        this->incoming_data_.clear();
        break;
      case improv::WIFI_SETTINGS: {
        if (this->state_ != improv::STATE_AUTHORIZED) {
          ESP_LOGW(TAG, "Settings received, but not authorized");
          this->set_error_(improv::ERROR_NOT_AUTHORIZED);
          this->incoming_data_.clear();
          return;
        }
        wifi::WiFiAP sta{};
        sta.set_ssid(command.ssid);
        sta.set_password(command.password);
        this->connecting_sta_ = sta;

        wifi::global_wifi_component->set_sta(sta);
        wifi::global_wifi_component->start_scanning();
        this->set_state_(improv::STATE_PROVISIONING);
        ESP_LOGD(TAG, "Received Improv wifi settings ssid=%s, password=" LOG_SECRET("%s"), command.ssid.c_str(),
                 command.password.c_str());

        auto f = std::bind(&ESP32ImprovComponent::on_wifi_connect_timeout_, this);
        this->set_timeout("wifi-connect-timeout", 30000, f);
        this->incoming_data_.clear();
        break;
      }
      case improv::IDENTIFY:
        this->incoming_data_.clear();
        this->identify_start_ = millis();
        break;
      default:
        ESP_LOGW(TAG, "Unknown Improv payload");
        this->set_error_(improv::ERROR_UNKNOWN_RPC);
        this->incoming_data_.clear();
    }
  } else if (this->incoming_data_.size() - 2 > length) {
    ESP_LOGV(TAG, "Too much data came in, or malformed resetting buffer...");
    this->incoming_data_.clear();
  } else {
    ESP_LOGV(TAG, "Waiting for split data packets...");
  }
}

void ESP32ImprovComponent::on_wifi_connect_timeout_() {
  this->set_error_(improv::ERROR_UNABLE_TO_CONNECT);
  this->set_state_(improv::STATE_AUTHORIZED);
  if (this->authorizer_ != nullptr)
    this->authorized_start_ = millis();
  ESP_LOGW(TAG, "Timed out trying to connect to given WiFi network");
  wifi::global_wifi_component->clear_sta();
}

void ESP32ImprovComponent::on_client_disconnect() { this->set_error_(improv::ERROR_NONE); };

ESP32ImprovComponent *global_improv_component = nullptr;

}  // namespace esp32_improv
}  // namespace esphome










namespace improv {

ImprovCommand parse_improv_data(const std::vector<uint8_t> &data) {
  return parse_improv_data(data.data(), data.size());
}

ImprovCommand parse_improv_data(const uint8_t *data, size_t length) {
  Command command = (Command) data[0];
  uint8_t data_length = data[1];

  if (data_length != length - 3) {
    return {.command = UNKNOWN};
  }

  uint8_t checksum = data[length - 1];

  uint32_t calculated_checksum = 0;
  for (uint8_t i = 0; i < length - 1; i++) {
    calculated_checksum += data[i];
  }

  if ((uint8_t) calculated_checksum != checksum) {
    return {.command = BAD_CHECKSUM};
  }

  if (command == WIFI_SETTINGS) {
    uint8_t ssid_length = data[2];
    uint8_t ssid_start = 3;
    size_t ssid_end = ssid_start + ssid_length;

    uint8_t pass_length = data[ssid_end];
    size_t pass_start = ssid_end + 1;
    size_t pass_end = pass_start + pass_length;

    std::string ssid(data + ssid_start, data + ssid_end);
    std::string password(data + pass_start, data + pass_end);
    return {.command = command, .ssid = ssid, .password = password};
  }

  return {
      .command = command,
  };
}

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (auto str : datum) {
    uint8_t len = str.length();
    length += len;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  uint32_t calculated_checksum = 0;

  for (uint8_t byte : out) {
    calculated_checksum += byte;
  }
  out.push_back(calculated_checksum);
  return out;
}

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum) {
  std::vector<uint8_t> out;
  uint32_t length = 0;
  out.push_back(command);
  for (auto str : datum) {
    uint8_t len = str.length();
    length += len;
    out.push_back(len);
    out.insert(out.end(), str.begin(), str.end());
  }
  out.insert(out.begin() + 1, length);

  uint32_t calculated_checksum = 0;

  for (uint8_t byte : out) {
    calculated_checksum += byte;
  }
  out.push_back(calculated_checksum);
  return out;
}

}  // namespace improv

//----------------------------------------------------------------------------------------------------------------------