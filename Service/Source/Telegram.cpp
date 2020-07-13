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
#include "Telegram.h"
#include "ConfigManager.h"
#include "LogFile.h"
//----------------------------------------------------------------------------------------------------------------------
json                      Telegram::pStatus      = json::object();
string                    Telegram::pToken       = "";
pthread_t                 Telegram::pThread      = 0;
pthread_mutex_t           Telegram::pMutex       = PTHREAD_MUTEX_INITIALIZER;
volatile bool             Telegram::pTerminate   = false;
uint32_t                  Telegram::pUsedWorkers = 0;
Headers                   Telegram::pHeaders     = {{"Accept", "application/json"}, {"Host", "api.telegram.org"}, {"Accept-Encoding", "gzip, deflate"}, {"Connection", "keep-alive"}};
list<Telegram::Worker_t*> Telegram::pFreeWorkers;
//----------------------------------------------------------------------------------------------------------------------
Telegram::Telegram() {
  HTTPServer::RegisterCallback("getTelegram", GetTelegramCallback);
  HTTPServer::RegisterCallback("setTelegram", SetTelegramCallback);
  Configure(CheckConfig(ConfigManager::ReadConfig("/Telegram")));
}
Telegram::~Telegram() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  pthread_mutex_unlock(&pMutex);
  if(pThread != 0) pthread_join(pThread, NULL);
  pthread_mutex_lock(&pMutex);
  while(pUsedWorkers > 0) {
    pthread_mutex_unlock(&pMutex);
    usleep(100000);
    pthread_mutex_lock(&pMutex);
  }
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.front();
    delete Tmp->Thread;
    delete Tmp->Client;
    pFreeWorkers.pop_front();
    delete Tmp;
  }

}
void Telegram::Configure(const json &config) {
  ConfigManager::WriteConfig("/Telegram", config);
  bool Enable = config["Enabled"];
  pthread_mutex_lock(&pMutex);
  pToken = config["Token"];
  if((Enable == true) && (pThread == 0)) {
    pTerminate = false;
    if(pthread_create(&pThread, NULL, Thread_Func, NULL) != 0) {
      pThread = 0;
      LogFile::Error("%s:%d -> %s", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "TelegramServer");
    }
  } else if((Enable == false) && (pThread != 0)) {
    pTerminate = true;
    pthread_join(pThread, NULL);
    pThread = 0;
  }
  pthread_mutex_unlock(&pMutex);
}
json Telegram::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["Token"] = "";
  ConfigOUT["Enabled"] = false;
  if(config.is_object() == true) {
    if(config.contains("Token") && config["Token"].is_string()) ConfigOUT["Token"] = config["Token"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void Telegram::GetTelegramCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  json Config = ConfigManager::ReadConfig("/Telegram");
  pthread_mutex_lock(&pMutex);
  Config["Status"] = pStatus;
  pthread_mutex_unlock(&pMutex);
  HTTPServer::FillResponse(response, Config.dump(), "application/json", HTTP_OK);
}
void Telegram::SetTelegramCallback(const json &params, Response &response) {
  if(params["UserName"] != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
  if((params.contains("Enabled") == false) || (params["Enabled"].is_boolean() == false)) return HTTPServer::FillResponse(response, "'Enabled' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  if((params.contains("Token") == false) || (params["Token"].is_string() == false)) return HTTPServer::FillResponse(response, "'Token' param is requiered", "text/plain", HTTP_BAD_REQUEST);
  Configure(CheckConfig(params));
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *Telegram::Thread_Func(void *params) {
  LogFile::Debug("Telegram BOT started");
  nice(10);
  bool       Connected  = false;
  int32_t    LastUpdate = 0;
  SSLClient  Client("api.telegram.org", 443);
  Client.set_keep_alive(true);
  Client.set_connection_timeout(30);
  Client.set_write_timeout(30);
  Client.set_read_timeout(30);
  Client.set_compress(true);
  while(pTerminate == false) {
    if(Connected == false) {
      string URL   = "/bot" + pToken + "/getMe";
      auto Response = Client.Get(URL.c_str(), pHeaders);
      if((Response != nullptr) && (Response->status == 200)) {
        json Params = json::parse(Response->body, NULL, false);
        if((Params.is_discarded() == false) && (Params.is_object() == true) && (Params["ok"].is_boolean() == true) && (Params["ok"] == true) && (Params["result"].is_object() == true)) {
          pthread_mutex_lock(&pMutex);
          pStatus["ID"] = Params["result"]["id"];
          pStatus["FirstName"] = Params["result"]["first_name"];
          pStatus["LastName"] = Params["result"]["last_name"];
          pStatus["UserName"] = Params["result"]["username"];
          pthread_mutex_unlock(&pMutex);
          Connected = true;
        }
      }
    }
    if(Connected == false) {
      pthread_mutex_lock(&pMutex);
      pStatus = json::object();
      pthread_mutex_unlock(&pMutex);
      usleep(100000);
      continue;
    }
    for(uint8_t Count = 0; Count < 100; Count++) {
      Database DB;
      string   URL = "/bot" + pToken + "/sendMessage";
      DB.ExecSentenceAsyncResult("SELECT * FROM telegram ORDER BY id ASC LIMIT 1");
      if(DB.FecthRow() == false) break;
      uint64_t Seq = DB.ResultUInt64("id");
      string   Msg = DB.ResultString("msg");
      auto Response = Client.Post(URL.c_str(), pHeaders, Msg, "application/json");
      if(Response != nullptr) DB.ExecSentenceNoResult("DELETE FROM telegram WHERE id = " + DB.Bind(Seq));
    }
    string URL   = "/bot" + pToken + "/getUpdates";
    Params Param = { { "timeout", to_string(TELEGRAM_POLLING_SEC) }, { "offset", to_string(LastUpdate + 1) }, { "limit", to_string(TELEGRAM_UPDATE_LIMIT) } };
    auto Response = Client.Post(URL.c_str(), pHeaders, Param);
    if((Response != nullptr) && (Response->status == 200)) {
      json Params = json::parse(Response->body, NULL, false);
      if((Params.is_discarded() == false) && (Params.is_object() == true) && (Params["ok"].is_boolean() == true) && (Params["ok"] == true) && (Params["result"].is_array() == true)) {
        for(auto &Item : Params["result"]) {
          if(Item["update_id"].is_number()) LastUpdate = Item["update_id"];
          Worker_t *Tmp = NULL;
          pthread_mutex_lock(&pMutex);
          pUsedWorkers++;
          if(pFreeWorkers.size() > 0) {
            Tmp = pFreeWorkers.front();
            pFreeWorkers.pop_front();
          }
          pthread_mutex_unlock(&pMutex);
          if(Tmp == NULL) {
            Tmp = new Worker_t;
            Tmp->Client = new SSLClient("api.telegram.org", 443);
            Tmp->Thread = new Worker("TelegramWorker", [](void *param2) {
              Worker_t *Work = (Worker_t*)param2;
              HandleUpdate(Work->Client, Work->JSON);
              Work->Token = "";
              Work->JSON = nullptr;
              pthread_mutex_lock(&pMutex);
              Work->LastTime = Tools::GetTime64();
              pFreeWorkers.push_front(Work);
              pUsedWorkers--;
              pthread_mutex_unlock(&pMutex);
            }, 15);
          }
          if(Tmp != NULL) {
            Tmp->Token = pToken;
            Tmp->JSON  = Item;
            Tmp->Thread->Run(Tmp);
          }
        }
      } else {
        Connected = false;
      }
    } else {
      Connected = false;
    }
  }
  LogFile::Debug("Telegram BOT stopped");
  pthread_mutex_lock(&pMutex);
  pStatus = json::object();
  pthread_mutex_unlock(&pMutex);
  return NULL;
}
void Telegram::PeriodicTasks() {
  pthread_mutex_lock(&pMutex);
  uint64_t Time = Tools::GetTime64();
  while(pFreeWorkers.size() > 0) {
    auto Tmp = pFreeWorkers.back();
    if((Time - Tmp->LastTime) < 60000) break;
    delete Tmp->Thread;
    delete Tmp->Client;
    pFreeWorkers.pop_back();
    delete Tmp;
  }
  LogFile::Debug("Adjusted Telegram workers pool to %d connections (plus %u in use)", pFreeWorkers.size(), pUsedWorkers);
  pthread_mutex_unlock(&pMutex);
}
void Telegram::SendAlarm(const string &analyzer, const time_zone &tz, const uint64_t timestamp, const string &name, const uint8_t priority, const bool triggered) {
  Database  DB;
//  time_zone Timezone;
  json      Tmp = json::object();
//  cctz::load_time_zone(tz, &Timezone);
  Tmp["text"] = (triggered ? "↗️ *" : "↘️ *") + name + "*\n" + (triggered ? "Triggered" : "Released") + " at " + cctz::format("%H:%M:%S - %Y/%m/%d \n", chrono::system_clock::from_time_t(timestamp / 1000), tz);
  Tmp["parse_mode"] = "Markdown";
  DB.ExecSentenceNoResult("INSERT INTO telegram(msg) SELECT jsonb_build_object('chat_id', telegram) || " + DB.Bind(Tmp.dump()) + "::jsonb FROM permissions NATURAL JOIN users WHERE serial = " + DB.Bind(analyzer) + " AND telegram IS NOT NULL");
}
//bool Telegram::SendMessage(const json &msg, Client *client) {
//  string URL = "/bot" + pToken + "/sendMessage";
//  SSLClient Client("api.telegram.org", 443);
//  auto Response = Client.Post(URL.c_str(), pHeaders, msg.dump(), "application/json");
//  return true; ///TODO: Comprobar valores devueltos
//}
//----------------------------------------------------------------------------------------------------------------------
void Telegram::HandleUpdate(SSLClient *client, const json &JSON) {
  printf("%s\n", JSON.dump().c_str());
  Database DB;
  if(JSON.contains("message") && JSON["message"].is_object() &&
     JSON["message"].contains("chat") && JSON["message"]["chat"].is_object() &&
     JSON["message"]["chat"].contains("id") && JSON["message"]["chat"]["id"].is_number() &&
     JSON["message"].contains("text") && JSON["message"]["text"].is_string()) {
    int64_t ChatID = JSON["message"]["chat"]["id"];
    if(DB.ExecSentenceAsyncResult("SELECT username FROM users WHERE telegram = " + DB.Bind(to_string(ChatID))) == false) return;
    json Msg = json::object();
    Msg["chat_id"] = ChatID;
    if(DB.FecthRow() == false) {
      Msg["text"] = "Your user ID is not found, please check it is correctly set in user profile and try again\n\n*Your user ID is '" + to_string(ChatID) + "'*";
      Msg["parse_mode"] = "Markdown";
      DB.ExecSentenceNoResult("INSERT INTO telegram(msg) VALUES(" + DB.Bind(Msg.dump()) + ")");
      return;
    }
    if(JSON["message"]["text"] == "/start") {
      Msg["text"] = "*Welcome* to *openZmeter*\n\nYou are ready to receive alarms and query your analyzers information.";
      Msg["parse_mode"] = "Markdown";
      DB.ExecSentenceNoResult("INSERT INTO telegram(msg) VALUES(" + DB.Bind(Msg.dump()) + ")");
      //Envio teclado con analyzers
      return;
    }
    Msg["text"] = "Unknown command";
    DB.ExecSentenceNoResult("INSERT INTO telegram(msg) VALUES(" + DB.Bind(Msg.dump()) + ")");
  }
}



//void Telegram::HandleUpdate(SSLClient *client, const json &message) {
//  printf("%s\n", message.dump().c_str());
//

//
//        } else {
//            String command = message.text();
//            switch (command) {
//                case CDASHBOARD:
//                {
//                   sendDashboard(message.chat().id());
//                   break;
//                }
//                case CENERGY:
//                {
//                    sendMessage(message.chat().id(), "📅", ENERGY_KB);
//                    break;
//                }
//                case CEVENT:
//                {
//                    sendMessage(message.chat().id(), "🚩", EVENTS_KB);
//                    break;
//                }
//                case CALARM:
//                {
//                    sendMessage(message.chat().id(), "Coming soon", EVENTS_KB);
//                    break;
//                }
//                case CSYSTEM:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    OZMSystem ozmSys = new OZMSystem(true);
//                    sendMessageMarkDown(message.chat().id(), ozmSys.getSystemParameters(), MAINMENU_KB);
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    ozmSys.closeSession();
//                    break;
//                }
//
//                case CREPORT:
//                {
//                    sendMessage(message.chat().id(), CREPORT, REPORT_KB);
//                    //updateIma(message.chat().id());
//                    sendMessage(36166036, "http://telegra.ph/oZm-daily-report-11-30", MAINMENU_KB);
//                    sendMessage(828068, "http://telegra.ph/oZm-daily-report-11-30", MAINMENU_KB);
//                    break;
//                }
//
//                case CBACKHOME:
//                {
//                    sendMessage(message.chat().id(), HOME, MAINMENU_KB);
//                    break;
//                }
//
//
//                case ETODAY:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyToday(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case EYESTERDAY:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyYesterday(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case EWEEK:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyThisWeek(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//
//
//                    break;
//                }
//                case ELASTWEEK:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyLastWeek(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case ETHISMONTH:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyThisMonth(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case ELASTMONTH:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyLastMonth(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case EYEAR:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyThisYear(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case ELASTYEAR:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEnergyLastYear(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//
//                case EVLASTEVENTS:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendLastEvents(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case EVTODAY:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEventToday(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//
//                }
//                case EVMONTH:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEventMonth(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//                case EVYEAR:
//                {
//                    int messageid = sendWaitGif(message.chat().id());
//                    sendEventYear(message.chat().id());
//                    sendDeleteWaitGif(message.chat().id(), messageid);
//                    break;
//                }
//
//                default: {
//                    sendMessage(message.chat().id(), "Comando Incorrecto", MAINMENU_KB);
//                    System.out.println("No action");
//                }
//
//            }
//
//        }
//
//}