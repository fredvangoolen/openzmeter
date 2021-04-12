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
#include "ModbusManager.h"
#include "AnalyzerManager.h"
#include "HTTPServer.h"
// ---------------------------------------------------------------------------------------------------------------------
pthread_mutex_t                ModbusManager::pMutex          = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t                ModbusManager::pMutexConfig    = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t                 ModbusManager::pCond           = PTHREAD_COND_INITIALIZER;
pthread_t                      ModbusManager::pThread         = 0;
volatile bool                  ModbusManager::pTerminate      = false;
uint16_t                       ModbusManager::pPort           = 502;
uint32_t                       ModbusManager::pRunningWorkers = 0;
vector<string>                 ModbusManager::pAnalyzers;
// ---------------------------------------------------------------------------------------------------------------------
ModbusManager::ModbusManager() {
  Config(CheckConfig(ConfigFile::ReadConfig("/ModbusTCP")));
  HTTPServer::RegisterCallback("getModbusTCP", GetModbusTCP);
  HTTPServer::RegisterCallback("setModbusTCP", SetModbusTCP);
}
ModbusManager::~ModbusManager() {
  LogFile::Info("Stopping ModbusManager...");
  HTTPServer::UnregisterCallback("getModbusTCP");
  HTTPServer::UnregisterCallback("setModbusTCP");
  if(pThread != 0) {
    pTerminate = true;
    if(pthread_join(pThread, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  }
  LogFile::Info("ModbusManager stoped");
}
json ModbusManager::CheckConfig(const json &config) {
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
void ModbusManager::Config(const json &config) {
  if(pthread_mutex_lock(&pMutexConfig) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  if(pThread != 0) {
    LogFile::Info("Stopping ModbusManager...");
    pTerminate = true;
    if(pthread_join(pThread, NULL) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    pThread = 0;
    pAnalyzers.clear();
    LogFile::Info("ModbusManager stoped");
  }
  ConfigFile::WriteConfig("/ModbusTCP", config);
  if(config["Enabled"] == true) {
    pPort      = config["Port"];
    pTerminate = false;
    for(uint16_t n = 0; n < config["Analyzers"].size(); n++) pAnalyzers.push_back(config["Analyzers"][n]);
    LogFile::Info("Starting ModbusManager");
    if(pthread_create(&pThread, NULL, Thread, NULL) != 0) {
      pThread = 0;
      LogFile::Error("ModbusTCP", "%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      if(pthread_setname_np(pThread, "ModbusTCP") != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      TaskPool::SetThreadProperties(pThread, TaskPool::NORMAL);
      LogFile::Info("ModbusManager started on port %u", pPort);
    }
  }
  if(pthread_mutex_unlock(&pMutexConfig) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
}
void ModbusManager::GetModbusTCP(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/ModbusTCP");
  req.Success(Config);
}
void ModbusManager::SetModbusTCP(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Enabled") || !req.CheckParamNumber("Port") || !req.CheckParamArrayString("Analyzers")) return;
  Config(CheckConfig(req.Params()));
  req.Success(json::object());
}
void *ModbusManager::Thread(void *) {
  sockaddr_in Server;
  pollfd      Poll;
  int         Opt  = 1;
  int         Sock = socket(AF_INET, SOCK_STREAM, 0);
	if(Sock < 0) {
		LogFile::Error("ModbusTCP", "%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return NULL;
  }
  if(setsockopt(Sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&Opt , sizeof(int)) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
	Server.sin_family = AF_INET;
	Server.sin_addr.s_addr = INADDR_ANY;
	Server.sin_port = htons(pPort);
	if(bind(Sock, (sockaddr *)&Server, sizeof(Server)) < 0) {
		LogFile::Error("ModbusTCP", "%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return NULL;
  }
  if(listen(Sock, 5) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
  Poll.events = POLLIN;
  Poll.fd = Sock;
  while(!pTerminate) {
    Poll.revents = 0;
    if(poll(&Poll, 1, 100) == 1) {
      auto Func = [&]() {
        if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        pRunningWorkers++;
        if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        Connection_Handle(Sock);
        if(pthread_mutex_lock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        pRunningWorkers--;
        if(pRunningWorkers == 0) {
          if(pthread_cond_signal(&pCond) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        }
        if(pthread_mutex_unlock(&pMutex) != 0) LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      };
      TaskPool::Run(Func, TaskPool::NORMAL);
      //Worker_t *Tmp = NULL;
//      pthread_mutex_lock(&pMutex);
//      pUsedWorkers++;
//      if(pFreeWorkers.size() > 0) {
//        Tmp = pFreeWorkers.front();
//        pFreeWorkers.pop_front();
//      }
//      pthread_mutex_unlock(&pMutex);
//      if(Tmp == NULL) {
//        Tmp         = new Worker_t;
//        Tmp->Thread = new Worker("ModbusTCPWorker", [](void *param) {
//          Worker_t *Work = (Worker_t*)param;
//          Connection_Handle(Work->ClientSock, Work->Client);
//          close(Work->ClientSock);
//          pthread_mutex_lock(&pMutex);
//          Work->LastTime = Tools::GetTime64();
//          pFreeWorkers.push_front(Work);
//          pUsedWorkers--;
//          pthread_mutex_unlock(&pMutex);
//        }, 15);
//      }
//      if(Tmp != NULL) {
//        int Size        = sizeof(sockaddr_in);
//        Tmp->ClientSock = accept(Sock, (sockaddr *)&Tmp->Client, (socklen_t*)&Size);
//        fcntl(Tmp->ClientSock, F_SETFL, O_NONBLOCK);
//        Tmp->Thread->Run(Tmp);
//      }
    }
	}
	return NULL;
}
void ModbusManager::Connection_Handle(int sock) {
  sockaddr_in Client;
  char        Buff[INET_ADDRSTRLEN];
  PacketIN_t  PacketIN;
  PacketOUT_t PacketOUT;
  int Size       = sizeof(sockaddr_in);
  int ClientSock = accept(sock, (sockaddr *)&Client, (socklen_t*)&Size);
  if(ClientSock < 0) return;
  memset(&PacketIN,  0, sizeof(PacketIN_t));
  memset(&PacketOUT, 0, sizeof(PacketOUT_t));
  inet_ntop(AF_INET, &Client.sin_addr, Buff, sizeof(Buff));
  LogFile::Debug("ModbusTCP", "New Modbus connection from '%s'", Buff);
  while(pTerminate == false) {
    ssize_t Size = recv(ClientSock, &PacketIN, sizeof(PacketIN), MSG_DONTWAIT);
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
      PacketOUT.FunctionCode     |= 0x80;
      PacketOUT.ResponseLen       = 1;
      PacketOUT.ResponseBuffer[0] = 1;
    } else if(PacketIN.UnitID >= pAnalyzers.size()) {
      PacketOUT.FunctionCode     |= 0x80;
      PacketOUT.ResponseLen       = 1;
      PacketOUT.ResponseBuffer[0] = 10;
    } else if(PacketIN.QuantityRegisters > 125) {
      PacketOUT.FunctionCode     |= 0x80;
      PacketOUT.ResponseLen       = 1;
      PacketOUT.ResponseBuffer[0] = 2;
    } else {
      PacketOUT.ResponseLen = sizeof(uint16_t) * PacketIN.QuantityRegisters;
      if(AnalyzerManager::HandleModbus(PacketOUT.ResponseBuffer, pAnalyzers[PacketOUT.UnitID], PacketIN.StartAddress, PacketIN.QuantityRegisters) == false) {
        PacketOUT.FunctionCode     |= 0x80;
        PacketOUT.ResponseLen       = 1;
        PacketOUT.ResponseBuffer[0] = 11;
      }
    }
    PacketOUT.Length = htons(PacketOUT.ResponseLen + 3);
    Size = send(ClientSock, &PacketOUT, PacketOUT.ResponseLen + 9, 0);
  }
  close(ClientSock);
  LogFile::Debug("ModbusTCP", "Modbus connection terminated");
}
//----------------------------------------------------------------------------------------------------------------------