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
extern "C" {
  #include "mqtt-c/mqtt.h"
  #include "mqtt-c/mqtt_pal.h"
}
// ---------------------------------------------------------------------------------------------------------------------
class AnalyzerSoftMQTT final {
     pthread_t                  pMQTTThread;              //MQTT thread
    mqtt_client                pMQTTClient;              //MQTT client state
    uint32_t                   pMQTTBuffSize;            //MQTT buffers size
    uint8_t                   *pMQTTSendBuff;            //MQTT send buffer
    uint8_t                   *pMQTTRecvBuff;            //MQTT recv buffer
    bool                       pMQTTTerminate;           //MQTT terminate signal
    string                     pMQTTServer;              //MQTT server address
    string                     pMQTTPrefix;              //MQTT prefix
    string                     pMQTTUser;                //MQTT server user
    string                     pMQTTPass;                //MQTT server password
    bool                       pMQTT3S;                  //Enable 3 seconds publish
    bool                       pMQTT1M;                  //Enable 1 minute publish
    bool                       pMQTT10M;                 //Enable 10 minutes publish
    bool                       pMQTT15M;                 //Enable 15 minutes publish
    bool                       pMQTT1H;                  //Enable 1 hour publish
    bool                       pMQTTAlarms;              //Enable alarms publish
    bool                       pMQTTConfig;              //Enable config publish
        static void *MQTT_Thread(void *params);
    void PublishAggreg(const Params_t &params, const string &topic);
    void PublishConfig(const json &options);

};
// ---------------------------------------------------------------------------------------------------------------------