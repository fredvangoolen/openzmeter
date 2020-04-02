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
string          Telegram::pToken     = "";
pthread_t       Telegram::pThread    = 0;
pthread_mutex_t Telegram::pMutex     = PTHREAD_MUTEX_INITIALIZER;
bool            Telegram::pTerminate = false;
//----------------------------------------------------------------------------------------------------------------------
Telegram::Telegram() {
  bool Enable = false;
  json Config = ConfigManager::ReadConfig("/Telegram");
  if(!Config.is_object()) Config = json::object();
  if(Config["Token"].is_string()) pToken = Config["Token"];
  if(Config["Enabled"].is_boolean()) Enable = Config["Enabled"];
  Config = json::object();
  Config["Token"] = pToken;
  Config["Enabled"] = Enable;
  ConfigManager::WriteConfig("/Telegram", Config);
  if(Enable == true) {
    if(pthread_create(&pThread, NULL, Thread_Func, (void*)this) != 0) {
      pThread = 0;
      LOGFile::Error("%s:%d -> %s\n", __func__, __LINE__, strerror(errno));
    } else {
      pthread_setname_np(pThread, "Telegram");
    }
  }
  HTTPServer::RegisterCallback("getTelegram", GetTelegramCallback);
  HTTPServer::RegisterCallback("setTelegram", SetTelegramCallback);
}
Telegram::~Telegram() {
  pthread_mutex_lock(&pMutex);
  pTerminate = true;
  if(pThread != 0) pthread_join(pThread, NULL);
  pthread_mutex_unlock(&pMutex);
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
  json Config = json::object();
  bool Enable = params["Enabled"];
  Config["Enabled"] = params["Enabled"];
  Config["Token"]   = params["Token"];
  ConfigManager::WriteConfig("/Telegram", Config);
  pthread_mutex_lock(&pMutex);
  pToken = Config["Token"];
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
  HTTPServer::FillResponse(response, "", "application/json", HTTP_OK);
}
void *Telegram::Thread_Func(void *params) {
  nice(10);
  LOGFile::Debug("Telegram BOT started\n");
  string Token = "";
  while(pTerminate == false) {
    pthread_mutex_lock(&pMutex);
    Token = pThread;
    pthread_mutex_unlock(&pMutex);



    usleep(100000);
  }
  LOGFile::Debug("Telegram BOT stopped\n");
  return NULL;
}
