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
#include <sys/ioctl.h>
#include <poll.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <math.h>
#include "DeviceRemote.h"
#include "DeviceManager.h"
#include "AnalyzerManager.h"
#include "LogFile.h"
#include "ConfigFile.h"
//----------------------------------------------------------------------------------------------------------------------
bool                         DeviceRemote::pRegistered = DeviceRemote::Register();
RWLocks                      DeviceRemote::pLocks;
map<string, AnalyzerRemote*> DeviceRemote::pAnalyzers;
//----------------------------------------------------------------------------------------------------------------------
bool DeviceRemote::Register() {
  DeviceManager::AddDriver(GetInstances);
  return true;
}
void DeviceRemote::GetInstances() {
  LogFile::Info("Adding 'Remote' device driver instance with serial 'REMOTE'");
  DeviceManager::AddDevice(new DeviceRemote());
}
DeviceRemote::DeviceRemote() : Device("REMOTE", "Remote", "Allow repplication of remote openZmeter devices in local machine") {
  pThread       = 0;
  pServerSocket = -1;
  Configure(CheckConfig(ConfigFile::ReadConfig("/Devices/" + pSerial)));
}
DeviceRemote::~DeviceRemote() {
  LogFile::Info("Stopping device with serial '%s'", pSerial.c_str());
  HTTPServer::UnregisterJSONCallback("setRemote");
  HTTPServer::UnregisterJSONCallback("getRemote");
  pLocks.WriteLock();
  while(pAnalyzers.size() > 0) {
    auto Analyzer = pAnalyzers.begin();
    AnalyzerManager::DelAnalyzer(Analyzer->second->Serial());
    delete Analyzer->second;
    pAnalyzers.erase(Analyzer);
  }
  pLocks.Unlock();
  LogFile::Info("Device with serial '%s' stopped", pSerial.c_str());
}
uint32_t DeviceRemote::AnalyzersCount() {
  return pAnalyzers.size();
}
json DeviceRemote::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Port"]    = 8081;
  if(config.is_object()) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Port") && config["Port"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void DeviceRemote::Configure(const json &config) {
  ConfigFile::WriteConfig("/Devices/" + pSerial, config);
  pLocks.WriteLock();
  bool Enabled = config["Enabled"];
  if((pEnabled == false) && (Enabled == true)) {
    LogFile::Info("Starting 'Remote' driver");
    pTerminate = false;
    pServerSocket = socket(AF_INET , SOCK_STREAM , 0);
    if(pServerSocket == -1) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      pLocks.Unlock();
      return;
    }
    int Opt = 1;
    setsockopt(pServerSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&Opt, sizeof(Opt));
    ioctl(pServerSocket, FIONBIO, (char*)&Opt);
    sockaddr_in Server;
    Server.sin_family = AF_INET;
    Server.sin_addr.s_addr = INADDR_ANY;
    Server.sin_port = htons(pPort);
    if(bind(pServerSocket,(struct sockaddr *)&Server , sizeof(Server)) < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      close(pServerSocket);
      pServerSocket = -1;
      pLocks.Unlock();
      return;
    }
    if(listen(pServerSocket, 10) == -1) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      close(pServerSocket);
      pServerSocket = -1;
      pLocks.Unlock();
      return;
    }
    if(pthread_create(&pThread, NULL, [](void *param) { return ((DeviceRemote*)param)->Thread(); }, this) != 0) {
      close(pServerSocket);
      pServerSocket = -1;
      pThread = 0;
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "DeviceRemote");
      pEnabled = true;
    }
  } else if((pEnabled == true) && (Enabled == false)) {
    LogFile::Info("Stopping 'Remote' driver...");
    pTerminate = true;
    if(pServerSocket != -1) close(pServerSocket);
    pthread_join(pThread, NULL);
    while(pAnalyzers.size() > 0) {
      auto Analyzer = pAnalyzers.begin();
      AnalyzerManager::DelAnalyzer(Analyzer->second->Serial());
      delete Analyzer->second;
      pAnalyzers.erase(Analyzer);
    }
  }
  pLocks.Unlock();
}
void *DeviceRemote::Thread() {
  queue<Client_t>  Clients = queue<Client_t>(10);
  pollfd           FDS[11];
  while(pTerminate == false) {
    uint64_t Time  = Tools::GetTime64();
    FDS[0].revents = 0;
    FDS[0].events  = POLLIN;
    FDS[0].fd      = pServerSocket;
    while(Clients.size() > 0) {
      if(Clients[0].Time > Time) Clients.p
    }
    for(uint8_t n = 0; n < Clients.size(); n++) {
      FDS[n + 1].revents = 0;
      FDS[n + 1].events  = POLL_IN;
      FDS[n + 1].fd      = Clients[n].Socket;
    }
    int Result = poll(FDS, Clients.size() + 1, 100);
    if((Result <= 0) || (FDS[0].revents != POLLIN)) continue;

    Client_t *Client = new Client_t;
    socklen_t   ClientAddrLen = sizeof(Client->Address);
    Client->Socket = accept(pServerSocket, (sockaddr*)&Client->Address, &ClientAddrLen);
    if(Client->Socket < 0) {
      delete Client;
      continue;
    }
    if(pthread_create(&Client->Thread, NULL, Client_Thread, (void*)Client) < 0) {
      LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
      close(Client->Socket);
      delete Client;
    }

        //Now join the thread , so that we dont terminate before the thread
        //pthread_join( thread_id , NULL);
//        puts("Handler assigned");
//    }

  }
  return NULL;
}
void DeviceRemote::GetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Devices/" + pSerial);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void DeviceRemote::SetDevice(const json &params, Response &response) {
  string UserName = params["UserName"];
  if(UserName != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "text/plain", HTTP_OK);
}
void DeviceRemote::GetRemoteCallback(const json &params, Response &response) {
  if((params.contains("Analyzer") == false) || (params["Analyzer"].is_string() == false)) return HTTPServer::FillResponse(response, "'Analyzer' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  string Serial = params["Analyzer"];
  pLocks.ReadLock();
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->GetRemote(params, response);
    pLocks.Unlock();
    return;
  }
  pLocks.Unlock();
  AnalyzerRemote *Analyzer = new AnalyzerRemote(Serial);
  if(AnalyzerManager::AddAnalyzer(Analyzer, params["UserName"]) == false) {
    delete Analyzer;
    return HTTPServer::FillResponse(response, "'Analyzer' not allowed on remote system", "text/plain", HTTP_BAD_REQUEST);
  }
  pLocks.WriteLock();
  LogFile::Info("Added remote analyzer '%s'", Serial.c_str());
  pAnalyzers.insert( { Serial, Analyzer } );
  pLocks.Unlock();
  pLocks.ReadLock();
  Analyzer->GetRemote(params, response);
  pLocks.Unlock();
}
void DeviceRemote::SetRemoteCallback(const map<string, string> &params, const ContentReader &content_reader, Response &response) {
  string Serial = params.find("Analyzer")->second;
  pLocks.ReadLock();
  auto Tmp = pAnalyzers.find(Serial);
  if(Tmp != pAnalyzers.end()) {
    Tmp->second->SetRemote(params, content_reader, response);
    pLocks.Unlock();
    return;
  }
  pLocks.Unlock();
  HTTPServer::FillResponse(response, "Analyzer not found", "text/plain", HTTP_BAD_REQUEST);
}
//----------------------------------------------------------------------------------------------------------------------