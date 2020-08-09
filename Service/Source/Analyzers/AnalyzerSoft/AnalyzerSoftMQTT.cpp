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
 #include "URL.h"
 #include "LogFile.h"
// ---------------------------------------------------------------------------------------------------------------------
AnalyzerSoftMQTT::AnalyzerSoftMQTT(const string &serial) {
  pMutex    = PTHREAD_MUTEX_INITIALIZER;
  pSerial   = serial;
  pThread   = 0;
  pBuffSize = 0x040000;
  pSendBuff = NULL;
  pRecvBuff = NULL;
  p3S       = false;
  p1M       = false;
  p10M      = false;
  p15M      = false;
  p1H       = false;
  pAlarms   = false;
  pConfig   = false;
}
AnalyzerSoftMQTT::~AnalyzerSoftMQTT() {
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
}
json AnalyzerSoftMQTT::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Enabled"] = false;
  ConfigOUT["Prefix"] = "";
  ConfigOUT["Server"] = "";
  ConfigOUT["User"] = "";
  ConfigOUT["Pass"] = "";
  ConfigOUT["Export3S"] = false;
  ConfigOUT["Export1M"] = false;
  ConfigOUT["Export10M"] = false;
  ConfigOUT["Export15M"] = false;
  ConfigOUT["Export1H"] = false;
  ConfigOUT["ExportAlarms"] = false;
  ConfigOUT["ExportConfig"] = false;
  if(config.is_object() == true) {
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
    if(config.contains("Prefix") && config["Prefix"].is_string()) ConfigOUT["Prefix"] = config["Prefix"];
    if(config.contains("Server") && config["Server"].is_string()) ConfigOUT["Server"] = config["Server"];
    if(config.contains("User") && config["User"].is_string()) ConfigOUT["User"] = config["User"];
    if(config.contains("Pass") && config["Pass"].is_string()) ConfigOUT["Pass"] = config["Pass"];
    if(config.contains("Export3S") && config["Export3S"].is_boolean()) ConfigOUT["Export3S"] = config["Export3S"];
    if(config.contains("Export1M") && config["Export1M"].is_boolean()) ConfigOUT["Export1M"] = config["Export1M"];
    if(config.contains("Export10M") && config["Export10M"].is_boolean()) ConfigOUT["Export10M"] = config["Export10M"];
    if(config.contains("Export15M") && config["Export15M"].is_boolean()) ConfigOUT["Export15M"] = config["Export15M"];
    if(config.contains("Export1H") && config["Export1H"].is_boolean()) ConfigOUT["Export1H"] = config["Export1H"];
    if(config.contains("ExportAlarms") && config["ExportAlarms"].is_boolean()) ConfigOUT["ExportAlarms"] = config["ExportAlarms"];
    if(config.contains("ExportConfig") && config["ExportConfig"].is_boolean()) ConfigOUT["ExportConfig"] = config["ExportConfig"];
  }
  return ConfigOUT;
}
void AnalyzerSoftMQTT::Config(const json &config) {
  pthread_mutex_lock(&pMutex);
  bool Enabled = config["Enabled"];
  if((Enabled == false) && (pThread != 0)) {
    pTerminate = false;
    pthread_join(pThread, NULL);
    free(pSendBuff);
    free(pRecvBuff);
    pThread = 0;
    p3S     = false;
    p1M     = false;
    p10M    = false;
    p15M    = false;
    p1H     = false;
    pAlarms = false;
    pConfig = false;
  } else if((Enabled == true) && (pThread == 0)) {
    pPrefix    = config["Prefix"];
    pServer    = config["Server"];
    pUser      = config["User"];
    pPass      = config["Pass"];
    p3S        = config["Export3S"];
    p1M        = config["Export1M"];
    p10M       = config["Export10M"];
    p15M       = config["Export15M"];
    p1H        = config["Export1H"];
    pAlarms    = config["ExportAlarms"];
    pConfig    = config["ExportConfig"];
    pTerminate = false;
    pSendBuff  = (uint8_t*)calloc(pBuffSize, 1);
    pRecvBuff  = (uint8_t*)calloc(pBuffSize, 1);
    mqtt_init_reconnect(&pClient, NULL, NULL, NULL);
    if(pthread_create(&pThread, NULL, Thread, this) != 0) {
      LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
      pThread = 0;
    } else {
      pthread_setname_np(pThread, "MQTT");
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftMQTT::PublishAggreg(const AnalyzerSoftAggreg *aggreg) {
/*  if(pMQTTClient.socketfd == -1) return;
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
  mqtt_publish(&pMQTTClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);*/
  LogFile::Debug("Analyzer '%s' publish message to MQTT broker", pSerial.c_str());
}
void AnalyzerSoftMQTT::PublishConfig(const json &options) {
  pthread_mutex_lock(&pMutex);
  if((pConfig == true) && (pClient.socketfd != -1)) {
    string TextMsg = options.dump();
    string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/Config" : pSerial + "/Config";
    mqtt_publish(&pClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
    LogFile::Debug("Analyzer '%s' publish message to MQTT broker", pSerial.c_str());
  }
  pthread_mutex_unlock(&pMutex);
}
void *AnalyzerSoftMQTT::Thread(void *params) {
  AnalyzerSoftMQTT *This = (AnalyzerSoftMQTT*)params;
  nice(10);
  string User, Pass, Server;
  while(This->pTerminate == false) {
    pthread_mutex_lock(&This->pMutex);
    if((This->pUser != User) || (This->pPass != Pass) || (This->pServer != Server)) {
      if(This->pClient.socketfd != -1) {
        close(This->pClient.socketfd);
        This->pClient.socketfd = -1;
      }
      User   = This->pUser;
      Pass   = This->pPass;
      Server = This->pServer;
    }
    pthread_mutex_unlock(&This->pMutex);
    mqtt_sync(&This->pClient);
    if(This->pClient.error == MQTT_ERROR_CONNECT_NOT_CALLED) {
      mqtt_connect(&This->pClient, NULL, NULL, NULL, 0, (User == "") ? NULL : User.c_str(), (Pass == "") ? NULL : Pass.c_str(), MQTT_CONNECT_CLEAN_SESSION, 400);
    } else if(This->pClient.error == MQTT_ERROR_SOCKET_ERROR) {
      if(This->pClient.socketfd != -1) {
        close(This->pClient.socketfd);
        This->pClient.socketfd = -1;
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
        This->pClient.socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(This->pClient.socketfd == -1) {
          LogFile::Error("%s:%d -> (%s)", __func__, __LINE__, strerror(errno));
          continue;
        }
        if(connect(This->pClient.socketfd, p->ai_addr, p->ai_addrlen) == -1) {
          close(This->pClient.socketfd);
          This->pClient.socketfd = -1;
          continue;
        }
        fcntl(This->pClient.socketfd, F_SETFL, fcntl(This->pClient.socketfd, F_GETFL) | O_NONBLOCK);
        break;
      }
      freeaddrinfo(ServInfo);
      if(This->pClient.socketfd == -1) {
        LogFile::Debug("Analyzer '%s' can not connect to MQTT broker", This->pSerial.c_str());
        usleep(1000000);
        continue;
      } else {
        mqtt_reinit(&This->pClient, This->pClient.socketfd, This->pSendBuff, This->pBuffSize, This->pRecvBuff, This->pBuffSize);
      }
    } else if(This->pClient.error != MQTT_OK) {
      LogFile::Error("MQTT ERROR: %s (%d)", mqtt_error_str(This->pClient.error), This->pClient.error);
    }
    usleep(100000);
  }
  if(This->pClient.socketfd != -1) {
    close(This->pClient.socketfd);
    This->pClient.socketfd = -1;
  }
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------