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
#include "UpdateManager.h"
#include "LogFile.h"
#include "ConfigFile.h"
#include "SystemManager.h"
#include "URL.h"
#include "MessageLoop.h"
//----------------------------------------------------------------------------------------------------------------------
#ifdef OPENWRT
string          UpdateManager::pUpdateURL  = "https://www.openzmeter.com/update/" __BUILD_ARCH__ "/" __BUILD_BRANCH__;
bool            UpdateManager::pAutoUpdate = true;
json            UpdateManager::pRelease    = nullptr;
pthread_mutex_t UpdateManager::pMutex      = PTHREAD_MUTEX_INITIALIZER;
#endif
// ---------------------------------------------------------------------------------------------------------------------
UpdateManager::UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("Starting UpdateManager...");
  Config(CheckConfig(ConfigFile::ReadConfig("/Update")));
  MessageLoop::RegisterListener("Updates_PeriodicTasks", PeriodicTasks, 900000);
  HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
  HTTPServer::RegisterCallback("setUpgrade",   SetUpgradeCallback);
  LogFile::Info("UpdateManager started");
#endif
}
UpdateManager::~UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("UpdateManager", "Stopping UpdateManager...");
  MessageLoop::UnregisterListener("Updates_PeriodicTasks");
  HTTPServer::UnregisterCallback("getUpdates");
  HTTPServer::UnregisterCallback("setUpgrade");
  LogFile::Info("UpdateManager", "UpdateManager stoped");
#endif
}
#ifdef OPENWRT
json UpdateManager::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["URL"] = pUpdateURL;
  ConfigOUT["Auto"] = true;
  if(config.is_object() == true) {
    if(config.contains("URL") && config["URL"].is_string()) ConfigOUT["URL"] = config["URL"];
    if(config.contains("Auto") && config["Auto"].is_boolean()) ConfigOUT["Auto"] = config["Auto"];
  }
  return ConfigOUT;
}
void UpdateManager::Config(const json &config) {
  pthread_mutex_lock(&pMutex);
  pUpdateURL  = config["URL"];
  pAutoUpdate = config["Auto"];
  pthread_mutex_unlock(&pMutex);
}
void UpdateManager::PeriodicTasks() {
  pthread_mutex_lock(&pMutex);
  if(pAutoUpdate == false) {
    pthread_mutex_unlock(&pMutex);
    return;
  }
  pthread_mutex_unlock(&pMutex);
  GetUpdates();
  pthread_mutex_lock(&pMutex);
  //Si mi version es distinta a la que obtengo, descargo, guardo y llamo a reiniciar el servicio
  pthread_mutex_unlock(&pMutex);
}
void UpdateManager::GetUpdates() {
  pthread_mutex_lock(&pMutex);
  URL Url(pUpdateURL);
  pthread_mutex_unlock(&pMutex);
  Client HTTPClient(Url.Host(), Url.Port());
  HTTPClient.set_read_timeout(20);
  HTTPClient.set_write_timeout(20);
  auto Response = HTTPClient.Get(string(Url.Path() + "/Release.json").c_str(), {{"Accept", "application/json"}, {"Accept-Encoding", "gzip, deflate"}});
  if((Response != nullptr) && (Response->status == HTTP_OK)) {
    json Content = json::parse(Response->body, NULL, false);
    if(Content.is_discarded()) {
      LogFile::Error("Malformed JSON params in UpdateManager");
    } else {
      if((Content.contains("Version") == false) || (Content["Version"].is_string() == false) || (Content.contains("ChangeLog") == false) || (Content["ChangeLog"].is_string() == false)) {
        pthread_mutex_lock(&pMutex);
        pRelease = nullptr;
        pthread_mutex_unlock(&pMutex);
      } else {
        pthread_mutex_lock(&pMutex);
        pRelease = json::object();
        pRelease["Version"] = Content["Version"];
        pRelease["ChangeLog"] = Content["ChangeLog"];
        pthread_mutex_unlock(&pMutex);
      }
    }
  }
}
void UpdateManager::GetUpdatesCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = json::object();
  GetUpdates();
  pthread_mutex_lock(&pMutex);
  Config["URL"] = pUpdateURL;
  if(pRelease != nullptr) {
    Config["Version"]   = pRelease["Version"];
    Config["ChangeLog"] = pRelease["ChangeLog"];
  }
  pthread_mutex_unlock(&pMutex);
  req.Success(Config);
}
void UpdateManager::SetUpgradeCallback(HTTPRequest &req) {
//  if(req.UserName() != "admin") return HTTPServer::FillResponse(response, "", "text/plain", HTTP_FORBIDDEN);
//  if((params.contains("Packages") == false) || (params["Packages"].is_array() == false) || (params["Packages"].size() == 0)) return HTTPServer::FillResponse(response, "'Packages' param is required", "text/plain", HTTP_BAD_REQUEST);
//  string Command = "opkg upgrade --force-depends";
//  for(auto &Package : params["Packages"]) {
//    if(Package.is_string() == false) return HTTPServer::FillResponse(response, "'Packages' param is required", "text/plain", HTTP_BAD_REQUEST);
//    Command.append(" " + Package.get<string>());
//  }
//  auto Func = [Command](uint64_t offset, DataSink &sink) {
//    char   Buffer[128];
//    string Result = "";
//    FILE  *Pipe   = popen(Command.c_str(), "r");
//    if(!Pipe) {
//      LogFile::Error("Can not exec 'opkg upgrade' (%s)", strerror(errno));
//      sink.os << "event: status\ndata: ERROR (Can not exec 'opkg upgrade')\n\n";
//      return false;
//    }
//    while((fgets(Buffer, sizeof(Buffer), Pipe) != NULL) && sink.is_writable()) sink.os << "event: text\ndata: " + string(Buffer) + "\n\n";
//    int Code = WEXITSTATUS(pclose(Pipe));
//    sink.os << ((Code == 0) ? "event: status\ndata: OK (Upgrade finished)\n\n" : "event: status\ndata: ERROR (Upgrade failed)\n\n");
//    sink.done();
//    return true;
//  };
//  response.set_chunked_content_provider("text/event-stream", Func);
}
#endif
//----------------------------------------------------------------------------------------------------------------------