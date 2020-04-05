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
//----------------------------------------------------------------------------------------------------------------------
string          Telegram::pToken          = "";
pthread_t       Telegram::pThread         = 0;
pthread_mutex_t Telegram::pMutex          = PTHREAD_MUTEX_INITIALIZER;
bool            Telegram::pTerminate      = false;
//----------------------------------------------------------------------------------------------------------------------
Telegram::Telegram() {
  HTTPServer::RegisterCallback("getTelegram", GetTelegramCallback);
  HTTPServer::RegisterCallback("setTelegram", SetTelegramCallback);
  Configure(CheckConfig(ConfigManager::ReadConfig("/Telegram")));
}
Telegram::~Telegram() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pthread_mutex_unlock(&pMutex);
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
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "Telegram");
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
  LOGFile::Debug("Telegram BOT started\n");
  nice(10);
  int32_t    LastUpdate = 0;
  Headers    Headers    = {{"Accept", "application/json"}, {"Host", "api.telegram.org"}, {"Accept-Encoding", "gzip, deflate"}, {"Connection", "keep-alive"}};
  SSLClient *Client     = new SSLClient("api.telegram.org", 443);
  Client->set_keep_alive_max_count(100);
  Client->set_read_timeout(20, 0);
  Client->set_timeout_sec(30);
  Client->set_compress(true);
  while(pTerminate == false) {
    json Commands = nullptr;
    string URL   = "/bot" + pToken + "/getUpdates";
    Params Param = { { "timeout", to_string(TELEGRAM_POLLING_SEC) }, { "offset", to_string(LastUpdate + 1) }, { "limit", to_string(TELEGRAM_UPDATE_LIMIT) } };
    auto Response = Client->Post(URL.c_str(), Headers, Param);
    if(Response == nullptr) {
      usleep(100000);
      continue;
    }
    if(Response->status != 200) continue;
    json Params = json::parse(Response->body, NULL, false);
    if(Params.is_discarded() == true) continue;
    if((Params["ok"].is_boolean() == false) || (Params["ok"] == false)) continue;
    if(Params["result"].is_array() == false) continue;
    for(auto &Item : Params["result"]) {
      if(Item["update_id"].is_number()) LastUpdate = Item["update_id"];
      printf("%s\n", Item.dump().c_str());
      json Msg = json::object();
      Msg["chat_id"] = 242623189;
      Msg["text"] = "Soy un mensaje!";
      string URL = "/bot" + pToken + "/sendMessage";
      auto Response = Client->Post(URL.c_str(), Headers, Msg.dump(), "application/json");


    }
  }
  LOGFile::Debug("Telegram BOT stopped\n");
  delete Client;
  return NULL;
}
