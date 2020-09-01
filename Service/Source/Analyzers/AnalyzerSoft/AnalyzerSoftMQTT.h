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
#include "mqtt-c/mqtt.h"
#include "mqtt-c/mqtt_pal.h"
#include <string>
#include <pthread.h>
#include "AnalyzerSoftAggreg.h"
// ---------------------------------------------------------------------------------------------------------------------
using namespace std;
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftMQTT final {
  private :
    pthread_mutex_t   pMutex;       //Locks access to shared variables
    string            pSerial;      //Analyzer serial
    pthread_t         pThread;      //MQTT thread
    mqtt_client       pClient;      //MQTT client state
    uint32_t          pBuffSize;    //MQTT buffers size
    uint8_t          *pSendBuff;    //MQTT send buffer
    uint8_t          *pRecvBuff;    //MQTT recv buffer
    bool              pTerminate;   //MQTT terminate signal
    string            pServer;      //MQTT server address
    string            pPrefix;      //MQTT prefix
    string            pUser;        //MQTT server user
    string            pPass;        //MQTT server password
    bool              p3S;          //Enable 3 seconds publish
    bool              p1M;          //Enable 1 minute publish
    bool              p10M;         //Enable 10 minutes publish
    bool              p15M;         //Enable 15 minutes publish
    bool              p1H;          //Enable 1 hour publish
    bool              pAlarms;      //Enable alarms publish
    bool              pConfig;      //Enable config publish
  private :
    void *Thread();
  public :
    static json CheckConfig(const json &config);
  public :
    AnalyzerSoftMQTT(const string &serial);
    ~AnalyzerSoftMQTT();
    void Config(const json &config);
    void PublishAggreg(const string &aggreg, const AnalyzerSoftAggreg *params);
    void PublishConfig(const json &options);
    void PublishAlarm(const uint64_t time, const string &name, const uint8_t prio, const bool triggered);
};
// ---------------------------------------------------------------------------------------------------------------------