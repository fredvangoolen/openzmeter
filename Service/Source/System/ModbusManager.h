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
#include <pthread.h>
#include "nlohmann/json.hpp"
#include "HTTPRequest.h"
#include "Worker.h"
// ---------------------------------------------------------------------------------------------------------------------
#define MODBUS_FUNC_NONE 0x00
//#define MB_FC_READ_COILS 1
//#define MB_FC_READ_DISCRETE_INPUT 2
#define MODBUS_FUNC_READ_REGISTERS 0x03
//#define MB_FC_READ_INPUT_REGISTERS 4        //implemented
//#define MB_FC_WRITE_COIL 5
//#define MB_FC_WRITE_REGISTER 6              //implemented
//#define MB_FC_WRITE_MULTIPLE_COILS 15
//#define MB_FC_WRITE_MULTIPLE_REGISTERS 16   //implemented
// ---------------------------------------------------------------------------------------------------------------------
using json = nlohmann::json;
// ---------------------------------------------------------------------------------------------------------------------
class ModbusManager final {
  private :
    typedef struct {
      uint64_t       LastTime;     //Last time event
      Worker        *Thread;
      sockaddr_in    Client;
      int            ClientSock;
    } Worker_t;
    typedef struct {
      uint16_t       TransactionID;
      uint16_t       ProtocolID;
      uint16_t       Length;
      uint8_t        UnitID;
      uint8_t        FunctionCode;
      uint16_t       StartAddress;
      uint16_t       QuantityRegisters;
    } PacketIN_t;
    typedef struct {
      uint16_t       TransactionID;
      uint16_t       ProtocolID;
      uint16_t       Length;
      uint8_t        UnitID;
      uint8_t        FunctionCode;
      uint8_t        ResponseLen;
      uint8_t        ResponseBuffer[256];
    } PacketOUT_t;
  private :
    static pthread_mutex_t pMutex;
    static pthread_t       pThread;
    static volatile bool   pTerminate;
    static uint16_t        pPort;
    static list<Worker_t*> pFreeWorkers;
    static uint32_t        pUsedWorkers;
    static vector<string>  pAnalyzers;
  private :
    static json  CheckConfig(const json &config);
    static void  Config(const json &config);
    static void *Thread(void *args);
    static void  Connection_Handle(const int clientSock, const sockaddr_in &client);
    static void  GetModbusTCP(HTTPRequest &req);
    static void  SetModbusTCP(HTTPRequest &req);
  public :
    ModbusManager();
    ~ModbusManager();
};
//#ifndef ModbusTCPSlave_h
//#define ModbusTCPSlave_h
//
//#define MB_PORT 1502
//
////#define MB_ETHERNET
////#define MB_CC3000
//#define MB_ESP8266
//
//#define MBDebug     //serial debug enable
//
//#include "Arduino.h"
//
//#ifdef MB_ETHERNET
//#include <Ethernet.h>
//#define LED_PIN 13
//#endif
//#ifdef MB_CC3000
//#define LED_PIN 13
//#include <Adafruit_CC3000.h>
//#include <SPI.h>
//#define ADAFRUIT_CC3000_IRQ   3
//#define ADAFRUIT_CC3000_VBAT  5
//#define ADAFRUIT_CC3000_CS    10
//#endif
//#ifdef MB_ESP8266
//#define LED_PIN 5
//#include <ESP8266WiFi.h>
//#endif
//
//#define maxInputRegister    20
//#define maxHoldingRegister    20
//
////
//// MODBUS Function Codes
////
//#define MB_FC_NONE 0
//#define MB_FC_READ_COILS 1
//#define MB_FC_READ_DISCRETE_INPUT 2
//#define MB_FC_READ_REGISTERS 3              //implemented
//#define MB_FC_READ_INPUT_REGISTERS 4        //implemented
//#define MB_FC_WRITE_COIL 5
//#define MB_FC_WRITE_REGISTER 6              //implemented
//#define MB_FC_WRITE_MULTIPLE_COILS 15
//#define MB_FC_WRITE_MULTIPLE_REGISTERS 16   //implemented
////
//// MODBUS Error Codes
////
//#define MB_EC_NONE 0
//#define MB_EC_ILLEGAL_FUNCTION 1
//#define MB_EC_ILLEGAL_DATA_ADDRESS 2
//#define MB_EC_ILLEGAL_DATA_VALUE 3
//#define MB_EC_SLAVE_DEVICE_FAILURE 4
////
//// MODBUS MBAP offsets
////
//#define MB_TCP_TID          0
//#define MB_TCP_PID          2
//#define MB_TCP_LEN          4
//#define MB_TCP_UID          6
//#define MB_TCP_FUNC         7
//#define MB_TCP_REGISTER_START         8
//#define MB_TCP_REGISTER_NUMBER         10
//


//class ModbusTCP {
//private :

//  public:
//    ModbusTCP();
//#ifdef MB_ETHERNET
//    void begin();
//    void begin(uint8_t ip[4],uint8_t gateway[4],uint8_t subnet[4]);
//#endif
//#ifdef MB_CC3000
//    void begin(const char *ssid, const char *key, uint8_t secmode);
//#endif
//#ifdef MB_ESP8266
//    void begin(const char *ssid, const char *key);
//    void begin(const char *ssid, const char *key,uint8_t ip[4],uint8_t gateway[4],uint8_t subnet[4]);
//#endif
//    void Run();
//    unsigned int  MBInputRegister[maxInputRegister];
//    unsigned int  MBHoldingRegister[maxHoldingRegister];
//private:
//    byte ByteArray[260];
//    bool ledPinStatus = LOW;
//
//
//#ifdef MB_ETHERNET
//    EthernetServer MBServer;
//#endif
//
//#ifdef MB_CC3000
//    Adafruit_CC3000 MBClient;
//    Adafruit_CC3000_Server MBServer;
//#endif
//
//#ifdef MB_ESP8266
//    WiFiServer MBServer;
//#endif
//};

//#endif
