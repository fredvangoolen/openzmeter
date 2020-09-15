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
 #include "Status.h"
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
  pParentConf = config;
  bool Enabled = config["MQTT"]["Enabled"];
  if(Enabled == false) {
    if(pThread != 0) {
      pTerminate = false;
      pthread_join(pThread, NULL);
      free(pSendBuff);
      free(pRecvBuff);
      pThread = 0;
    }
    p3S     = false;
    p1M     = false;
    p10M    = false;
    p15M    = false;
    p1H     = false;
    pAlarms = false;
    pConfig = false;
  } else if(Enabled == true) {
    pPrefix    = config["MQTT"]["Prefix"];
    pServer    = config["MQTT"]["Server"];
    pUser      = config["MQTT"]["User"];
    pPass      = config["MQTT"]["Pass"];
    p3S        = config["MQTT"]["Export3S"];
    p1M        = config["MQTT"]["Export1M"];
    p10M       = config["MQTT"]["Export10M"];
    p15M       = config["MQTT"]["Export15M"];
    p1H        = config["MQTT"]["Export1H"];
    pAlarms    = config["MQTT"]["ExportAlarms"];
    pConfig    = config["MQTT"]["ExportConfig"];
    if(pThread == 0) {
      pTerminate = false;
      if(pSendBuff == NULL) pSendBuff  = (uint8_t*)calloc(pBuffSize, 1);
      if(pRecvBuff == NULL) pRecvBuff  = (uint8_t*)calloc(pBuffSize, 1);
      mqtt_init_reconnect(&pClient, NULL, NULL, NULL);
      if(pthread_create(&pThread, NULL, [](void *params) { return ((AnalyzerSoftMQTT*)params)->Thread(); }, this) != 0) {
        LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
        pThread = 0;
      } else {
        pthread_setname_np(pThread, "MQTT");
      }
    }
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftMQTT::PublishAggreg(const string &aggreg, const AnalyzerSoftAggreg *params) {
  pthread_mutex_lock(&pMutex);
  if((pClient.socketfd != -1) && (((aggreg == "3S") && (p3S == true)) || ((aggreg == "1M") && (p1M == true)) || ((aggreg == "10M") && (p10M == true)) || ((aggreg == "15M") && (p15M == true)) || ((aggreg == "1H") && (p1H == true)))) {
    json Result = json::object();
    Result["Time"] = params->Timestamp();
    params->GetSeriesJSON("TIME", Result);
    params->GetSeriesJSON("FREQUENCY", Result);
    params->GetSeriesJSON("FLAG", Result);
    params->GetSeriesJSON("UNBALANCE", Result);
    params->GetSeriesJSON("VOLTAGE", Result);
    params->GetSeriesJSON("CURRENT", Result);
    params->GetSeriesJSON("ACTIVE_POWER", Result);
    params->GetSeriesJSON("REACTIVE_POWER", Result);
    params->GetSeriesJSON("ACTIVE_ENERGY", Result);
    params->GetSeriesJSON("REACTIVE_ENERGY", Result);
    params->GetSeriesJSON("APPARENT_POWER", Result);
    params->GetSeriesJSON("POWER_FACTOR", Result);
    params->GetSeriesJSON("VOLTAGE_THD", Result);
    params->GetSeriesJSON("CURRENT_THD", Result);
    params->GetSeriesJSON("PHI", Result);
    params->GetSeriesJSON("VOLTAGE_PHASE", Result);
    string TextMsg = Result.dump();
    string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/" + aggreg : pSerial + "/" + aggreg;
    mqtt_publish(&pClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1);
    LogFile::Debug("Analyzer '%s' publish aggregation to MQTT broker", pSerial.c_str());
  }
  pthread_mutex_unlock(&pMutex);
}
void AnalyzerSoftMQTT::PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered) {
  pthread_mutex_lock(&pMutex);
  if((pAlarms == true) && (pClient.socketfd == -1)) {
    json Msg = json::object();
    Msg["AlarmTime"] = time;
    Msg["Name"] = name;
    Msg["Priority"] = prio;
    Msg["Triggered"] = triggered;
    string TextMsg = Msg.dump();
    string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/Alarms" : pSerial + "/Alarms";
    mqtt_publish(&pClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1);
    LogFile::Debug("Analyzer '%s' publish alarm to MQTT broker", pSerial.c_str());
  }
  pthread_mutex_unlock(&pMutex);
}
void *AnalyzerSoftMQTT::Thread() {
  nice(10);
  string   User, Pass, Server;
  uint64_t StatusTimer = 0;
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    if((pUser != User) || (pPass != Pass) || (pServer != Server)) {
      if(pClient.socketfd != -1) {
        close(pClient.socketfd);
        pClient.socketfd = -1;
      }
      User   = pUser;
      Pass   = pPass;
      Server = pServer;
    }
    pthread_mutex_unlock(&pMutex);
    mqtt_sync(&pClient);
    if(pClient.error == MQTT_ERROR_CONNECT_NOT_CALLED) {
      string WillMsg = "{ \"Online\": false }";
      string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/Status" : pSerial + "/Status";
      StatusTimer = 0;
      mqtt_connect(&pClient, NULL, Topic.c_str(), WillMsg.c_str(), WillMsg.size(), (User == "") ? NULL : User.c_str(), (Pass == "") ? NULL : Pass.c_str(), MQTT_CONNECT_CLEAN_SESSION, 400);
    } else if(pClient.error == MQTT_ERROR_SOCKET_ERROR) {
      if(pClient.socketfd != -1) {
        close(pClient.socketfd);
        pClient.socketfd = -1;
      }
      URL  Url(Server);
      char Port[16];
      addrinfo Hints = { .ai_flags = 0, .ai_family = AF_UNSPEC, .ai_socktype = SOCK_STREAM };
      addrinfo *ServInfo;
      sprintf(Port, "%u", Url.Port());
      int Error = getaddrinfo(Url.Host().c_str(), Port, &Hints, &ServInfo);
      if(Error != 0) {
        LogFile::Error("%s:%d -> (%s)", __FILE__, __LINE__, strerror(errno));
        usleep(1000000);
        continue;
      }
      for(addrinfo *p = ServInfo; p != NULL; p = p->ai_next) {
        pClient.socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if(pClient.socketfd == -1) {
          LogFile::Error("%s:%d -> (%s)", __FILE__, __LINE__, strerror(errno));
          continue;
        }
        if(connect(pClient.socketfd, p->ai_addr, p->ai_addrlen) == -1) {
          close(pClient.socketfd);
          pClient.socketfd = -1;
          continue;
        }
        fcntl(pClient.socketfd, F_SETFL, fcntl(pClient.socketfd, F_GETFL) | O_NONBLOCK);
        break;
      }
      freeaddrinfo(ServInfo);
      if(pClient.socketfd == -1) {
        LogFile::Debug("Analyzer '%s' can not connect to MQTT broker", pSerial.c_str());
        usleep(1000000);
        continue;
      } else {
        mqtt_reinit(&pClient, pClient.socketfd, pSendBuff, pBuffSize, pRecvBuff, pBuffSize);
      }
    } else if(pClient.error == MQTT_OK) {
      pthread_mutex_lock(&pMutex);
      if((pConfig == true) && (pParentConf != nullptr) && (pClient.socketfd != -1)) {
        string TextMsg = pParentConf.dump();
        string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/Config" : pSerial + "/Config";
        mqtt_publish(&pClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1 | MQTT_PUBLISH_RETAIN);
        LogFile::Debug("Analyzer '%s' publish config to MQTT broker", pSerial.c_str());
        pParentConf = nullptr;
      }
      uint64_t Now = Tools::GetTime64();
      if((StatusTimer == 0) || (StatusTimer + 10000) < Now) {
        json   Msg = Status::GetJSON();
        Msg["Online"] = true;
        string TextMsg = Msg.dump();
        string Topic   = (pPrefix != "") ? pPrefix + "/" + pSerial + "/Status" : pSerial + "/Status";
        mqtt_publish(&pClient, Topic.c_str(), (void*)TextMsg.c_str(), TextMsg.size(), MQTT_PUBLISH_QOS_1);
        LogFile::Debug("Analyzer '%s' publish config to MQTT broker", pSerial.c_str());
        StatusTimer = Now;
      }
      pthread_mutex_unlock(&pMutex);
    } else {
      LogFile::Warning("Analyzer '%s' MQTT broker fail (%s)", pSerial.c_str(), mqtt_error_str(pClient.error));
      pClient.error = MQTT_OK;
    }
    usleep(100000);
  }
  if(pClient.socketfd != -1) {
    close(pClient.socketfd);
    pClient.socketfd = -1;
  }
  return NULL;
}
// ---------------------------------------------------------------------------------------------------------------------