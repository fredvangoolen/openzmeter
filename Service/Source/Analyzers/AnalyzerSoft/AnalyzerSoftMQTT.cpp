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
 #include "AnalyzerSoftMQTT.h"
// ---------------------------------------------------------------------------------------------------------------------
/*AnalyzerSoftMQTT::AnalyzerSoftMQTT() {
  pPrevSamples       = NULL;
  pPrevCycles        = NULL;

  pMQTTBuffSize      = 0x00020000;
}

void AnalyzerSoft::PublishAggreg(const Params_t &params, const string &topic) {
  if(pMQTTClient.socketfd == -1) return;
  json Msg = json::object();
  Msg["Sampletime"] = params.Timestamp;
  Msg["Frequency"] = params.Frequency;
  Msg["Flag"] = params.Flag;
  Msg["Unbalance"] = params.Unbalance;
  Msg["Voltage"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage"].push_back(params.Voltage[n]);
  Msg["Current"] = json::array();
  for(uint8_t n = 0; n < pCurrentChannels; n++) Msg["Current"].push_back(params.Current[n]);
  Msg["Active_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Active_Power"].push_back(params.Active_Power[n]);
  Msg["Reactive_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Reactive_Power"].push_back(params.Reactive_Power[n]);
  Msg["Active_Energy"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Active_Energy"].push_back(params.Active_Energy[n]);
  Msg["Reactive_Energy"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Reactive_Energy"].push_back(params.Reactive_Energy[n]);
  Msg["Apparent_Power"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Apparent_Power"].push_back(params.Apparent_Power[n]);
  Msg["Power_Factor"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Power_Factor"].push_back(params.Power_Factor[n]);
  Msg["Voltage_THD"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage_THD"].push_back(params.Voltage_THD[n]);
  Msg["Current_THD"] = json::array();
  for(uint8_t n = 0; n < pCurrentChannels; n++) Msg["Current_THD"].push_back(params.Current_THD[n]);
  Msg["Phi"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Phi"].push_back(params.Phi[n]);
  Msg["Voltage_Phase"] = json::array();
  for(uint8_t n = 0; n < pVoltageChannels; n++) Msg["Voltage_Phase"].push_back(params.Voltage_Phase[n]);
  string TextMsg = Msg.dump();
  string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + pSerial + "/" + topic : pSerial + "/" + topic;
  mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
  LogFile::Debug("Analyzer '%s' publish message to MQTT broker", pSerial.c_str());
}
void AnalyzerSoft::PublishConfig(const json &options) {
  if(pMQTTClient.socketfd != -1) {
    string TextMsg = options.dump();
    string Topic   = (pMQTTPrefix != "") ? pMQTTPrefix + "/" + pSerial + "/Config" : pSerial + "/Config";
    mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
    LogFile::Debug("Analyzer '%s' publish message to MQTT broker", pSerial.c_str());
  }
}
void *AnalyzerSoft::MQTT_Thread(void *params) {
  AnalyzerSoft *This = (AnalyzerSoft*)params;
  nice(10);
  string User, Pass, Server;
  while(This->pMQTTTerminate == false) {
    This->pAnalyzerLock.ReadLock();
    if((This->pMQTTUser != User) || (This->pMQTTPass != Pass) || (This->pMQTTServer != Server)) {
      if(This->pMQTTClient.socketfd != -1) {
        close(This->pMQTTClient.socketfd);
        This->pMQTTClient.socketfd = -1;
      }
      User   = This->pMQTTUser;
      Pass   = This->pMQTTPass;
      Server = This->pMQTTServer;
    }
    This->pAnalyzerLock.Unlock();
    mqtt_sync(&This->pMQTTClient);
    if(This->pMQTTClient.error == MQTT_ERROR_CONNECT_NOT_CALLED) {
      mqtt_connect(&This->pMQTTClient, NULL, NULL, NULL, 0, (User == "") ? NULL : User.c_str(), (Pass == "") ? NULL : Pass.c_str(), MQTT_CONNECT_CLEAN_SESSION, 400);
      if(This->pMQTTConfig == true) This->PublishConfig(CheckConfig(ConfigManager::ReadConfig(This->pConfigPath), This->pMode));
    } else if(This->pMQTTClient.error == MQTT_ERROR_SOCKET_ERROR) {
      if(This->pMQTTClient.socketfd != -1) {
        close(This->pMQTTClient.socketfd);
        This->pMQTTClient.socketfd = -1;
      }
      URL  Url(Server);
      char Port[16];
      addrinfo Hints = { .ai_flags = 0, .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
      addrinfo *ServInfo;
      sprintf(Port, "%u", Url.Port());
      int Error = getaddrinfo(Url.Host().c_str(), Port, &Hints, &ServInfo);
      if(Error != 0) {
        LogFile::Error("%s:%d -> %d", __func__, __LINE__, Error);
        usleep(1000000);
        continue;
      }
      for(addrinfo *p = ServInfo; p != NULL; p = p->ai_next) {
        This->pMQTTClient.socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(This->pMQTTClient.socketfd == -1) {
          LogFile::Error("%s:%d -> (%s)", __func__, __LINE__, strerror(errno));
          continue;
        }
        if(connect(This->pMQTTClient.socketfd, p->ai_addr, p->ai_addrlen) == -1) {
          close(This->pMQTTClient.socketfd);
          This->pMQTTClient.socketfd = -1;
          continue;
        }
        fcntl(This->pMQTTClient.socketfd, F_SETFL, fcntl(This->pMQTTClient.socketfd, F_GETFL) | O_NONBLOCK);
        break;
      }
      freeaddrinfo(ServInfo);
      if(This->pMQTTClient.socketfd == -1) {
        LogFile::Debug("Analyzer '%s' can not connect to MQTT broker", This->pSerial.c_str());
        usleep(1000000);
        continue;
      } else {
        mqtt_reinit(&This->pMQTTClient, This->pMQTTClient.socketfd, This->pMQTTSendBuff, This->pMQTTBuffSize, This->pMQTTRecvBuff, This->pMQTTBuffSize);
      }
    } else if(This->pMQTTClient.error != MQTT_OK) {
      LogFile::Error("MQTT ERROR: %s (%d)", mqtt_error_str(This->pMQTTClient.error), This->pMQTTClient.error);
    }
    usleep(100000);
  }
  if(This->pMQTTClient.socketfd != -1) {
    close(This->pMQTTClient.socketfd);
    This->pMQTTClient.socketfd = -1;
  }
  return NULL;
}*/
// ---------------------------------------------------------------------------------------------------------------------