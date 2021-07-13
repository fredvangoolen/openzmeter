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
string          UpdateManager::pUpdateURL  = "https://www.openzmeter.com/update";
bool            UpdateManager::pAutoUpdate = true;
pthread_mutex_t UpdateManager::pMutex      = PTHREAD_MUTEX_INITIALIZER;
#endif
// ---------------------------------------------------------------------------------------------------------------------
UpdateManager::UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("Starting UpdateManager...");
  Config(CheckConfig(ConfigFile::ReadConfig("/Update")));
  MessageLoop::RegisterListener("Updates_PeriodicTasks", PeriodicTasks, 10000);
  HTTPServer::RegisterCallback("getUpdates",   GetUpdatesCallback);
  HTTPServer::RegisterCallback("setUpdates",   SetUpdatesCallback);
  LogFile::Info("UpdateManager started");
#endif
}
UpdateManager::~UpdateManager() {
#ifdef OPENWRT
  LogFile::Info("UpdateManager", "Stopping UpdateManager...");
  MessageLoop::UnregisterListener("Updates_PeriodicTasks");
  HTTPServer::UnregisterCallback("getUpdates");
  HTTPServer::UnregisterCallback("setUpdates");
  LogFile::Info("UpdateManager", "UpdateManager stoped");
#endif
}
#ifdef OPENWRT
json UpdateManager::CheckConfig(const json &config) {
  json ConfigOUT = json::object();
  ConfigOUT["URL"] = pUpdateURL;
  ConfigOUT["Enabled"] = true;
  if(config.is_object() == true) {
    if(config.contains("URL") && config["URL"].is_string()) ConfigOUT["URL"] = config["URL"];
    if(config.contains("Enabled") && config["Enabled"].is_boolean()) ConfigOUT["Enabled"] = config["Enabled"];
  }
  return ConfigOUT;
}
void UpdateManager::Config(const json &config) {
  ConfigFile::WriteConfig("/Update", config);
  pthread_mutex_lock(&pMutex);
  pUpdateURL  = config["URL"];
  pAutoUpdate = config["Enabled"];
  pthread_mutex_unlock(&pMutex);
}
void UpdateManager::PeriodicTasks() {
  pthread_mutex_lock(&pMutex);
  if(pAutoUpdate == false) {
    pthread_mutex_unlock(&pMutex);
    return;
  }
  URL Url(pUpdateURL);
  pthread_mutex_unlock(&pMutex);
  LogFile::Debug("Checking for updates...");
  Client HTTPClient(string(Url.Protocol() + "://" + Url.Host() + ":" + to_string(Url.Port())).c_str());
  HTTPClient.set_read_timeout(20);
  HTTPClient.set_write_timeout(20);
  HTTPClient.set_compress(true);
  HTTPClient.enable_server_certificate_verification(false);
  auto Response = HTTPClient.Get(string(Url.Path() + "/" + __BUILD_ARCH__ + "/" __BUILD_BRANCH__ + "/Release.json").c_str(), {{"Accept", "application/json"}, {"Accept-Encoding", "gzip, deflate"}});
  if(Response.error() != Error::Success) {
    LogFile::Warning("Fail to check for updates (Socket error %d)", Response.error());
    return;
  }
  if(Response->status != HTTP_OK) {
    LogFile::Warning("Fail to check for updates (Error code %d)", Response->status);
    return;
  }
  json Content = json::parse(Response->body, NULL, false);
  if(Content.is_discarded()) {
    LogFile::Error("Malformed JSON params in UpdateManager");
    return;
  }
  if((Content.contains("Version") == false) || (Content["Version"].is_string() == false) || (Content.contains("Hash") == false) || (Content["Hash"].is_string() == false)) {
    LogFile::Error("Malformed JSON params in UpdateManager");
    return;
  }
  if(Content["Version"] == string(__BUILD_DATE__)) return;
  LogFile::Info("Updating from '%s' to '%s'", __BUILD_DATE__, Content["Version"].get<string>().c_str());
  string Hash = Content["Hash"];
  remove("/tmp/openZmeter");
  Response = HTTPClient.Get(string(Url.Path() + "/" + __BUILD_ARCH__ + "/" __BUILD_BRANCH__ + "/openZmeter").c_str(), {{"Accept", "application/octet-stream"}, {"Accept-Encoding", "gzip, deflate"}});
  if(Response.error() != Error::Success) {
    LogFile::Warning("Fail to check for updates (Socket error %d)", Response.error());
    return;
  }
  if(Response->status != HTTP_OK) {
    LogFile::Warning("Fail to check for updates (Error code %d)", Response->status);
    return;
  }
  uint8_t MD5Hash[MD5_DIGEST_LENGTH];
  char    MD5String[MD5_DIGEST_LENGTH * 2 + 1];
  MD5((uint8_t*)Response->body.c_str(), Response->body.size(), MD5Hash);
  for(uint8_t n = 0; n < MD5_DIGEST_LENGTH; n++) sprintf(&MD5String[n * 2], "%02x", MD5Hash[n]);
  if(Hash != string(MD5String)) {
    LogFile::Warning("Fail to check update MD5");
    return;
  }
  int File = open("/tmp/openZmeter", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
  if(File < 0) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    return;
  }
  if(write(File, Response->body.c_str(), Response->body.size()) != (ssize_t)Response->body.size()) {
    LogFile::Error("%s:%d -> %s", __FILE__, __LINE__, strerror(errno));
    close(File);
    return;
  }
  close(File);
  LogFile::Info("UpdateManager is restarting service...");
  SystemManager::Exec("chmod 755 /tmp/openZmeter && mv /tmp/openZmeter /usr/sbin/openZmeter && /etc/init.d/openZmeter restart");
  _exit(0);
}
void UpdateManager::GetUpdatesCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  json Config = ConfigFile::ReadConfig("/Update");
  Config["Build"] = __BUILD_DATE__;
  Config["Arch"] = __BUILD_ARCH__;
  Config["Branch"] = __BUILD_BRANCH__;
  req.Success(Config);
}
void UpdateManager::SetUpdatesCallback(HTTPRequest &req) {
  if(req.UserName() != "admin") return req.Error("", HTTP_FORBIDDEN);
  if(!req.CheckParamBool("Enabled") || !req.CheckParamString("URL")) return;
  Config(CheckConfig(req.Params()));
  req.Success(json::object());
}
#endif
//----------------------------------------------------------------------------------------------------------------------