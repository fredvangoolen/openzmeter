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
#include <poll.h>
#include "MODBus.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t            ModbusTCP::pMutex       = PTHREAD_MUTEX_INITIALIZER;
pthread_t                  ModbusTCP::pThread      = 0;
volatile bool              ModbusTCP::pTerminate   = false;
uint16_t                   ModbusTCP::pPort        = 502;
uint32_t                   ModbusTCP::pUsedWorkers = 0;
list<ModbusTCP::Worker_t*> ModbusTCP::pFreeWorkers;
// ---------------------------------------------------------------------------------------------------------------------
ModbusTCP::ModbusTCP() {
  Configure(CheckConfig(ConfigManager::ReadConfig("/ModbusTCP")));
  HTTPServer::RegisterCallback("getModbusTCP", GetModbusTCP);
  HTTPServer::RegisterCallback("setModbusTCP", SetModbusTCP);
}
ModbusTCP::~ModbusTCP() {
  LOGFile::Info("Stopping ModbusTCP server...\n");
  HTTPServer::UnregisterCallback("getModbusTCP");
  HTTPServer::UnregisterCallback("setModbusTCP");
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
  }
  LOGFile::Info("ModbusTCP server stoped\n");
}
json ModbusTCP::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Port"]    = 502;
  ConfigOUT["Enabled"] = false;
  if(config.is_object()) {
    if(config["Port"].is_number_unsigned()) ConfigOUT["Port"] = config["Port"];
    if(config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void ModbusTCP::Configure(const json &config) {
  pthread_mutex_lock(&pMutex);
  ConfigManager::WriteConfig("/ModbusTCP", config);
  bool     Enabled = config["Enabled"];
  uint16_t Port    = config["Port"];
  if(((Enabled == false) && (pThread != 0)) || (pPort != Port)) {
    LOGFile::Info("Stopping ModbusTCP server...\n");
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    LOGFile::Info("ModbusTCP server stoped\n");
  }
  if((Enabled == true) && (pThread == 0)) {
    pPort      = Port;
    pTerminate = false;
    LOGFile::Info("Starting ModbusTCP server on port %u...\n", pPort);
    if(pthread_create(&pThread, NULL, Thread, NULL) != 0) {
      pThread = 0;
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "ModbusTCP");
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void ModbusTCP::GetModbusTCP(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/ModbusTCP");
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void ModbusTCP::SetModbusTCP(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Port") == false) || (params["Token"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'Port' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *ModbusTCP::Thread(void *) {
  sockaddr_in Server;
  pollfd      Poll;
  int         Opt = 1;
  int         Sock   = socket(AF_INET, SOCK_STREAM, 0);
	if(Sock < 0) {
		LOGFile::Error("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&Opt , sizeof(int));
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = INADDR_ANY;
	Server.sin_port = htons(pPort);
	if(bind(Sock, (sockaddr *)&Server, sizeof(Server)) < 0) {
		LOGFile::Error("ERROR", "%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  listen(Sock, 5);
  Poll.events = POLLIN;
  Poll.fd = Sock;
  while(!pTerminate) {
    Poll.revents = 0;
    if(poll(&Poll, 1, 100) == 1) {
      Worker_t *Tmp = NULL;
      pthread_mutex_lock(&pMutex);
      pUsedWorkers++;
      if(pFreeWorkers.size() > 0) {
        Tmp = pFreeWorkers.front();
        pFreeWorkers.pop_front();
      }
      pthread_mutex_unlock(&pMutex);
      if(Tmp == NULL) {
        Tmp         = new Worker_t;
        Tmp->Thread = new Worker("ModbusTCPWorker", [](void *param) {
          Worker_t *Work = (Worker_t*)param;
          Connection_Handle(Work->ClientSock, Work->Client);
          close(Work->ClientSock);
          pthread_mutex_lock(&pMutex);
          Work->LastTime = Tools::GetTime64();
          pFreeWorkers.push_front(Work);
          pUsedWorkers--;
          pthread_mutex_unlock(&pMutex);
        }, 15);
      }
      if(Tmp != NULL) {
        int Size        = sizeof(sockaddr_in);
        Tmp->ClientSock = accept(Sock, (sockaddr *)&Tmp->Client, (socklen_t*)&Size);
        fcntl(Tmp->ClientSock, F_SETFL, O_NONBLOCK);
        Tmp->Thread->Run(Tmp);
      }
    }
	}
	return NULL;
}
void ModbusTCP::Connection_Handle(const int clientSock, const sockaddr_in &client) {
  char        Buff[INET_ADDRSTRLEN];
  PacketIN_t  PacketIN;
  PacketOUT_t PacketOUT;
  memset(&PacketIN,  0, sizeof(PacketIN_t));
  memset(&PacketOUT, 0, sizeof(PacketOUT_t));
  inet_ntop(AF_INET, &client.sin_addr, Buff, sizeof(Buff));
  LOGFile::Debug("New Modbus connection from '%s'\n", Buff);
  while(pTerminate == false) {
    ssize_t Size = recv(clientSock, &PacketIN, sizeof(PacketIN), MSG_DONTWAIT);
    if((Size == -1) && (errno == EAGAIN)) {
      usleep(100000);
      continue;
    } else if(Size <= 0) {
      break;
    }
    printf("MODBUS(%d) <- ", Size); for(uint8_t n = 0; n < Size; n++) printf(" %02X", ((uint8_t*)&PacketIN)[n]); printf("\n");
    PacketIN.Length            = ntohs(PacketIN.Length);
    PacketIN.StartAddress      = ntohs(PacketIN.StartAddress);
    PacketIN.QuantityRegisters = ntohs(PacketIN.QuantityRegisters);
    printf("Size: %u, Length: %u, Proto: %u, Registers: %u\n", Size, PacketIN.Length, PacketIN.ProtocolID, PacketIN.QuantityRegisters);
    if((Size != 12) || (PacketIN.Length != 6) || (PacketIN.ProtocolID != 0x0000)) continue;
    PacketOUT.TransactionID = PacketIN.TransactionID;
    PacketOUT.UnitID        = PacketIN.UnitID;
    PacketOUT.FunctionCode  = PacketIN.FunctionCode;
    if(PacketIN.FunctionCode == MODBUS_FUNC_READ_REGISTERS) {
      PacketOUT.ResponseLen = sizeof(uint16_t) * PacketIN.QuantityRegisters;
      PacketOUT.Length      = htons(PacketOUT.ResponseLen + 3);
      uint16_t *Ptr = (uint16_t*)&PacketOUT.ResponseBuffer;
      for(uint8_t n = 0; n < 120; n++) *Ptr++ = n;
    }
    Size = send(clientSock, &PacketOUT, PacketOUT.ResponseLen + 9, 0);
    printf("MODBUS(%d) -> ", Size);
    for(uint8_t n = 0; n < Size; n++) printf(" %02X", ((uint8_t*)&PacketOUT)[n]);
    printf("\n");

//    boolean flagClientConnected = 0;
//    byte byteFN = MB_FC_NONE;
//    int Start;
//    int WordDataLength;
//    int ByteDataLength;
//    int MessageLength;
//
//
//    uint8_t byteFN = MB_FC_NONE;
//    if(client.available())
//    {
//        flagClientConnected = 1;
//        int i = 0;
//        while(client.available())
//        {
//            ByteArray[i] = client.read();
//            i++;
//        }
//        #ifdef MB_ESP8266
//            client.flush();
//        #endif
//        byteFN = ByteArray[MB_TCP_FUNC];
//        Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
//        WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
//        #ifdef MBDebug
//        Serial.println();
//        Serial.print("RX: ");
//        for (byte thisByte = 0; thisByte < 20; thisByte++) {
//            Serial.print(ByteArray[thisByte], DEC);
//            Serial.print("-");
//        }
//        Serial.println();
//        Serial.print("Ricevuta Funzione: ");
//        Serial.println(byteFN);
//        #endif
//    }
//
//    // Handle request
//
//    switch(byteFN) {
//      case MB_FC_NONE :
//        break;
//      case MB_FC_READ_REGISTERS :  // 03 Read Holding Registers
//            ByteDataLength = WordDataLength * 2;
//            ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
//            ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
//            for(int i = 0; i < WordDataLength; i++)
//            {
//                ByteArray[ 9 + i * 2] = highByte(MBHoldingRegister[Start + i]);
//                ByteArray[10 + i * 2] =  lowByte(MBHoldingRegister[Start + i]);
//            }
//            MessageLength = ByteDataLength + 9;
//            client.write((const uint8_t *)ByteArray,MessageLength);
//            byteFN = MB_FC_NONE;
//            break;
//
//        case MB_FC_READ_INPUT_REGISTERS:  // 04 Read Input Registers
//            Start = word(ByteArray[MB_TCP_REGISTER_START],ByteArray[MB_TCP_REGISTER_START+1]);
//            WordDataLength = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
//            ByteDataLength = WordDataLength * 2;
//            ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
//            ByteArray[8] = ByteDataLength;     //Number of bytes after this one (or number of bytes of data).
//            for(int i = 0; i < WordDataLength; i++)
//            {
//                ByteArray[ 9 + i * 2] = highByte(MBInputRegister[Start + i]);
//                ByteArray[10 + i * 2] =  lowByte(MBInputRegister[Start + i]);
//            }
//            MessageLength = ByteDataLength + 9;
//            client.write((const uint8_t *)ByteArray,MessageLength);
//            byteFN = MB_FC_NONE;
//            break;
//
//        case MB_FC_WRITE_REGISTER:  // 06 Write Holding Register
//            MBHoldingRegister[Start] = word(ByteArray[MB_TCP_REGISTER_NUMBER],ByteArray[MB_TCP_REGISTER_NUMBER+1]);
//            ByteArray[5] = 6; //Number of bytes after this one.
//            MessageLength = 12;
//            client.write((const uint8_t *)ByteArray,MessageLength);
//            byteFN = MB_FC_NONE;
//            break;
//
//        case MB_FC_WRITE_MULTIPLE_REGISTERS:    //16 Write Holding Registers
//            ByteDataLength = WordDataLength * 2;
//            ByteArray[5] = ByteDataLength + 3; //Number of bytes after this one.
//            for(int i = 0; i < WordDataLength; i++)
//            {
//                MBHoldingRegister[Start + i] =  word(ByteArray[ 13 + i * 2],ByteArray[14 + i * 2]);
//            }
//            MessageLength = 12;
//            client.write((const uint8_t *)ByteArray,MessageLength);
//            #ifdef MBDebug
//                Serial.print("TX: ");
//                for (byte thisByte = 0; thisByte <= MessageLength; thisByte++) {
//                    Serial.print(ByteArray[thisByte], DEC);
//                    Serial.print("-");
//                }
//                Serial.println();
//                Serial.print("Write Holding Registers from: ");
//                Serial.print(Start);
//                Serial.print("=");
//                Serial.println(WordDataLength);
//            #endif
//            byteFN = MB_FC_NONE;
//            break;
//    }
  }
  LOGFile::Debug("Modbus connection terminated\n");
}