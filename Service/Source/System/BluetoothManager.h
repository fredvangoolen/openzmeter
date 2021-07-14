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
#pragma once
#include "HTTPRequest.h"
#ifdef OPENWRT
  #include <dbus/dbus.h>
#endif
// ---------------------------------------------------------------------------------------------------------------------
class BluetoothManager final {
#ifdef OPENWRT
  private :
    static volatile bool    pTerminate;    //Terminate flag
    static pthread_mutex_t  pMutex;        //Shared variables lock
    static pthread_t        pThread;       //bluetooth manager thread
    static bool             pEnable;       //Enable or disable
    static bool             pEnabled;      //Current status
    static string           pServerName;   //bluetooth network name
    static string           pInterface;    //bluetooth bridge name
  private :
    static void     ToggleEnable();
    static string   GetMacBT(DBusConnection *bus);
    static bool     SendBoolBT(DBusConnection *bus, const char *property, bool value);
    static bool     SendUINT32BT(DBusConnection *bus, const char *property, uint32_t value);
    static bool     SendStringBT(DBusConnection *bus, const char *property, const char *value);
    static void    *Thread(void *params);
#endif
  private :
    static void     GetBluetoothCallback(HTTPRequest &req);
    static void     SetBluetoothCallback(HTTPRequest &req);
  public :
    static bool Enabled();
  public :
    BluetoothManager();
    ~BluetoothManager();
};



#pragma once

#include "esphome/components/binary_sensor/binary_sensor.h"
#include "esphome/components/esp32_ble_server/ble_server.h"
#include "esphome/components/esp32_ble_server/ble_characteristic.h"
#include "esphome/components/improv/improv.h"
#include "esphome/components/output/binary_output.h"
#include "esphome/components/wifi/wifi_component.h"
#include "esphome/core/component.h"
#include "esphome/core/helpers.h"
#include "esphome/core/preferences.h"

#ifdef ARDUINO_ARCH_ESP32

namespace esphome {
namespace esp32_improv {

using namespace esp32_ble_server;

class ESP32ImprovComponent : public Component, public BLEServiceComponent {
 public:
  ESP32ImprovComponent();
  void dump_config() override;
  void loop() override;
  void setup() override;
  void setup_characteristics();
  void on_client_disconnect() override;

  float get_setup_priority() const override;
  void start() override;
  void stop() override;
  bool is_active() const { return this->state_ != improv::STATE_STOPPED; }

  void set_authorizer(binary_sensor::BinarySensor *authorizer) { this->authorizer_ = authorizer; }
  void set_status_indicator(output::BinaryOutput *status_indicator) { this->status_indicator_ = status_indicator; }
  void set_identify_duration(uint32_t identify_duration) { this->identify_duration_ = identify_duration; }
  void set_authorized_duration(uint32_t authorized_duration) { this->authorized_duration_ = authorized_duration; }

 protected:
  bool should_start_{false};
  bool setup_complete_{false};

  uint32_t identify_start_{0};
  uint32_t identify_duration_;
  uint32_t authorized_start_{0};
  uint32_t authorized_duration_;

  std::vector<uint8_t> incoming_data_;
  wifi::WiFiAP connecting_sta_;

  BLEService *service_;
  BLECharacteristic *status_;
  BLECharacteristic *error_;
  BLECharacteristic *rpc_;
  BLECharacteristic *rpc_response_;
  BLECharacteristic *capabilities_;

  binary_sensor::BinarySensor *authorizer_{nullptr};
  output::BinaryOutput *status_indicator_{nullptr};

  improv::State state_{improv::STATE_STOPPED};
  improv::Error error_state_{improv::ERROR_NONE};

  void set_state_(improv::State state);
  void set_error_(improv::Error error);
  void send_response(std::vector<uint8_t> &response);
  void process_incoming_data_();
  void on_wifi_connect_timeout_();
  bool check_identify_();
};

extern ESP32ImprovComponent *global_improv_component;

}  // namespace esp32_improv
}  // namespace esphome

#endif



static const char *const SERVICE_UUID = "00467768-6228-2272-4663-277478268000";
static const char *const STATUS_UUID = "00467768-6228-2272-4663-277478268001";
static const char *const ERROR_UUID = "00467768-6228-2272-4663-277478268002";
static const char *const RPC_COMMAND_UUID = "00467768-6228-2272-4663-277478268003";
static const char *const RPC_RESULT_UUID = "00467768-6228-2272-4663-277478268004";
static const char *const CAPABILITIES_UUID = "00467768-6228-2272-4663-277478268005";

enum Error : uint8_t {
  ERROR_NONE = 0x00,
  ERROR_INVALID_RPC = 0x01,
  ERROR_UNKNOWN_RPC = 0x02,
  ERROR_UNABLE_TO_CONNECT = 0x03,
  ERROR_NOT_AUTHORIZED = 0x04,
  ERROR_UNKNOWN = 0xFF,
};

enum State : uint8_t {
  STATE_STOPPED = 0x00,
  STATE_AWAITING_AUTHORIZATION = 0x01,
  STATE_AUTHORIZED = 0x02,
  STATE_PROVISIONING = 0x03,
  STATE_PROVISIONED = 0x04,
};

enum Command : uint8_t {
  UNKNOWN = 0x00,
  WIFI_SETTINGS = 0x01,
  IDENTIFY = 0x02,
  BAD_CHECKSUM = 0xFF,
};

static const uint8_t CAPABILITY_IDENTIFY = 0x01;

struct ImprovCommand {
  Command command;
  std::string ssid;
  std::string password;
};

ImprovCommand parse_improv_data(const std::vector<uint8_t> &data);
ImprovCommand parse_improv_data(const uint8_t *data, size_t length);

std::vector<uint8_t> build_rpc_response(Command command, const std::vector<std::string> &datum);
std::vector<uint8_t> build_rpc_response(Command command, const std::vector<String> &datum);

//----------------------------------------------------------------------------------------------------------------------