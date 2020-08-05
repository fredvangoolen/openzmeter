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
#include "LogFile.h"
#include "ConfigFile.h"
#include "MODBus.h"
#include "DeviceManager.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t            ModbusTCP::pMutex       = PTHREAD_MUTEX_INITIALIZER;
pthread_t                  ModbusTCP::pThread      = 0;
volatile bool              ModbusTCP::pTerminate   = false;
uint16_t                   ModbusTCP::pPort        = 502;
uint32_t                   ModbusTCP::pUsedWorkers = 0;
list<ModbusTCP::Worker_t*> ModbusTCP::pFreeWorkers;
vector<string>             ModbusTCP::pAnalyzers;
// ---------------------------------------------------------------------------------------------------------------------
ModbusTCP::ModbusTCP() {
  Configure(CheckConfig(ConfigFile::ReadConfig("/ModbusTCP")));
  HTTPServer::RegisterCallback("getModbusTCP", GetModbusTCP);
  HTTPServer::RegisterCallback("setModbusTCP", SetModbusTCP);
}
ModbusTCP::~ModbusTCP() {
  LogFile::Info("Stopping ModbusTCP server...");
  HTTPServer::UnregisterCallback("getModbusTCP");
  HTTPServer::UnregisterCallback("setModbusTCP");
  if(pThread != 0) {
    pTerminate = true;
    pthread_join(pThread, NULL);
  }
  LogFile::Info("ModbusTCP server stoped");
}
json ModbusTCP::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Port"]    = 502;
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Analyzers"] = json::array();
  if(config.is_object()) {
    if(config.contains("Port") && config["Port"].is_number_unsigned()) ConfigOUT["Port"] = config["Port"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Analyzers") && config["Analyzers"].is_array()) {
      for(uint16_t n = 0; n < config["Analyzers"].size(); n++) {
        if(config["Analyzers"][n].is_string()) ConfigOUT["Analyzers"].push_back(config["Analyzers"][n]);
      }
    }
  }
  return ConfigOUT;
}
void ModbusTCP::Configure(const json &config) {
  pthread_mutex_lock(&pMutex);
  if(pThread != 0) {
    LogFile::Info("Stopping ModbusTCP server...");
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
    pAnalyzers.clear();
    LogFile::Info("ModbusTCP server stoped");
  }
  ConfigFile::WriteConfig("/ModbusTCP", config);
  bool     Enabled = config["Enabled"];
  if(Enabled == true) {
    pPort      = config["Port"];
    pTerminate = false;
    for(uint16_t n = 0; n < config["Analyzers"].size(); n++) pAnalyzers.push_back(config["Analyzers"][n]);
    LogFile::Info("Starting ModbusTCP server on port %u...", pPort);
    if(pthread_create(&pThread, NULL, Thread, NULL) != 0) {
      pThread = 0;
      LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "ModbusTCP");
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void ModbusTCP::GetModbusTCP(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/ModbusTCP");
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void ModbusTCP::SetModbusTCP(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Port") == false) || (params["Port"].is_number_unsigned() == false)) return HTTPServer::FillResponse(response, "'Port' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Analyzers") == false) || (params["Analyzers"].is_array() == false)) return HTTPServer::FillResponse(response, "'Analyzers' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *ModbusTCP::Thread(void *) {
  sockaddr_in Server;
  pollfd      Poll;
  int         Opt = 1;
  int         Sock   = socket(AF_INET, SOCK_STREAM, 0);
	if(Sock < 0) {
		LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    return NULL;
  }
  setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&Opt , sizeof(int));
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = INADDR_ANY;
	Server.sin_port = htons(pPort);
	if(bind(Sock, (sockaddr *)&Server, sizeof(Server)) < 0) {
		LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
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
  LogFile::Debug("New Modbus connection from '%s'", Buff);
  while(pTerminate == false) {
    ssize_t Size = recv(clientSock, &PacketIN, sizeof(PacketIN), MSG_DONTWAIT);
    if((Size == -1) && (errno == EAGAIN)) {
      usleep(100000);
      continue;
    } else if(Size <= 0) {
      break;
    }
    PacketIN.Length            = ntohs(PacketIN.Length);
    PacketIN.StartAddress      = ntohs(PacketIN.StartAddress);
    PacketIN.QuantityRegisters = ntohs(PacketIN.QuantityRegisters);
    if((Size != 12) || (PacketIN.Length != 6) || (PacketIN.ProtocolID != 0x0000)) continue;
    PacketOUT.TransactionID = PacketIN.TransactionID;
    PacketOUT.UnitID        = PacketIN.UnitID;
    PacketOUT.FunctionCode  = PacketIN.FunctionCode;
    if(PacketOUT.FunctionCode != MODBUS_FUNC_READ_REGISTERS) {
      PacketOUT.FunctionCode |= 0x80;
      PacketOUT.ResponseLen = 1;
      PacketOUT.ResponseBuffer[0] = 1;
    } else if(PacketIN.UnitID >= pAnalyzers.size()) {
      PacketOUT.FunctionCode |= 0x80;
      PacketOUT.ResponseLen = 1;
      PacketOUT.ResponseBuffer[0] = 10;
    } else if(PacketIN.QuantityRegisters > 125) {
      PacketOUT.FunctionCode |= 0x80;
      PacketOUT.ResponseLen = 1;
      PacketOUT.ResponseBuffer[0] = 2;
    } else {
      PacketOUT.ResponseLen = sizeof(uint16_t) * PacketIN.QuantityRegisters;
//      if(DeviceManager::HandleModbus(PacketOUT.ResponseBuffer, pAnalyzers[PacketOUT.UnitID], PacketIN.StartAddress, PacketIN.QuantityRegisters) == false) {
//        PacketOUT.FunctionCode |= 0x80;
//        PacketOUT.ResponseLen = 1;
//        PacketOUT.ResponseBuffer[0] = 11;
//      }
    }
    PacketOUT.Length = htons(PacketOUT.ResponseLen + 3);
    Size = send(clientSock, &PacketOUT, PacketOUT.ResponseLen + 9, 0);
  }
  LogFile::Debug("Modbus connection terminated");
}